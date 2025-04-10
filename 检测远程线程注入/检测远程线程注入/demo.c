#include <ntddk.h>

// 定义线程数据结构体
// 用于存储线程的相关信息
typedef struct {
    // 线程的 ID
    ULONG ThreadId;
    // 创建该线程的进程 ID
    ULONG Create_ProcessId;
    // 该线程所属的进程 ID
    ULONG Belong_ProcessId;
} ThreadData;

// 定义链表项结构体
// 包含一个链表项和线程数据
typedef struct {
    // 链表项，用于将该结构体插入到双向链表中
    LIST_ENTRY Entry;
    // 存储线程相关信息的结构体
    ThreadData Data;
} Item;

// 定义全局数据结构体
// 包含一个快速互斥锁、项计数和链表头
typedef struct {
    // 快速互斥锁，用于保护对链表的并发访问
    FAST_MUTEX Mutex;
    // 链表中当前的项数
    ULONG ItemCount;
    // 双向链表的头节点
    LIST_ENTRY Header;
} Global;

// 全局变量，用于存储线程信息的链表
Global global;

// 驱动卸载函数声明
// 当驱动程序被卸载时调用此函数
void DriverUnload(PDRIVER_OBJECT pDriverObject);
// 线程通知回调函数声明
// 当有线程创建或销毁时会调用此函数
void ThreadNotify(HANDLE ProcessId, HANDLE ThreadId, BOOLEAN Create);
// 处理读请求的分发函数声明
// 用于处理用户态对设备对象的读请求
NTSTATUS DispatchFuncRead(PDEVICE_OBJECT DeviceObject, PIRP Irp);
// 默认分发函数声明
// 用于处理创建和关闭设备对象的请求
NTSTATUS DispatchFuncDefault(PDEVICE_OBJECT DeviceObject, PIRP Irp);
// 将链表项插入链表的函数声明
// 用于将新的线程信息插入到全局链表中
void PushItem(LIST_ENTRY* entry);

// 驱动入口函数，相当于 main 函数
// 当驱动程序加载时首先调用此函数
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegPath)
{
    // 避免编译器警告，因为 pRegPath 未被使用
    UNREFERENCED_PARAMETER(pRegPath);
    // 定义设备名称的 Unicode 字符串
    UNICODE_STRING DevName = RTL_CONSTANT_STRING(L"\\Device\\RemoteThreadDev");
    // 设备对象指针
    PDEVICE_OBJECT DeviceObject;
    // 创建设备对象
    NTSTATUS status = IoCreateDevice(pDriverObject, 0, &DevName, FILE_DEVICE_UNKNOWN, 0, 0, &DeviceObject);
    // 检查设备对象是否创建成功
    if (!NT_SUCCESS(status)) {
        // 若失败，打印错误信息
        KdPrint(("设备对象创建失败 (0x%08X)\n", status));
        return status;
    }
    // 设置设备对象的标志，使用直接 I/O 模式
    DeviceObject->Flags |= DO_DIRECT_IO;
    // 定义符号链接名称的 Unicode 字符串
    UNICODE_STRING SymbolicLink = RTL_CONSTANT_STRING(L"\\??\\RemoteThreadCheck");
    // 创建符号链接，使用户态程序能够访问该设备
    status = IoCreateSymbolicLink(&SymbolicLink, &DevName);
    // 检查符号链接是否创建成功
    if (!NT_SUCCESS(status)) {
        // 若失败，打印错误信息并删除设备对象
        KdPrint(("符号链接创建失败 (0x%08X)\n", status));
        IoDeleteDevice(DeviceObject);
        return status;
    }
    // 注册线程创建和销毁的回调通知
    status = PsSetCreateThreadNotifyRoutine(ThreadNotify);
    // 检查回调注册是否成功
    if (!NT_SUCCESS(status)) {
        // 若失败，打印错误信息，删除设备对象和符号链接
        KdPrint(("注册线程回调失败 (0x%08X)\n", status));
        IoDeleteDevice(DeviceObject);
        IoDeleteSymbolicLink(&SymbolicLink);
        return status;
    }
    // 若注册成功，打印成功信息
    KdPrint(("线程回调注册成功\n"));
    // 设置驱动卸载函数
    pDriverObject->DriverUnload = DriverUnload;
    // 初始化全局链表的头节点
    InitializeListHead(&global.Header);
    // 初始化快速互斥锁
    ExInitializeFastMutex(&global.Mutex);
    // 指定创建和关闭设备对象的分发例程
    pDriverObject->MajorFunction[IRP_MJ_CLOSE] = pDriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchFuncDefault;
    // 指定读设备对象的分发例程
    pDriverObject->MajorFunction[IRP_MJ_READ] = DispatchFuncRead;
    // 设置返回状态为成功
    status = STATUS_SUCCESS;
    return status;
}

// 处理读请求的分发函数
NTSTATUS DispatchFuncRead(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    // 避免编译器警告，因为 DeviceObject 未被使用
    UNREFERENCED_PARAMETER(DeviceObject);
    // 获取当前 IRP 栈位置
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    // 获取用户请求的缓冲区长度
    ULONG len = stack->Parameters.Read.Length;
    // 已复制到缓冲区的字节数
    ULONG count = 0;
    // 断言 MdlAddress 不为空
    NT_ASSERT(Irp->MdlAddress);
    // 获取缓冲区的非分页系统虚拟机地址
    UCHAR* buffer = (UCHAR*)MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
    // 检查是否成功获取缓冲区地址
    if (!buffer) {
        // 若失败，打印错误信息
        KdPrint(("获取缓冲区非分页虚拟地址失败"));
        return STATUS_UNSUCCESSFUL;
    }
    // 获取快速互斥锁，保护对链表的访问
    ExAcquireFastMutex(&global.Mutex);
    // 循环读取链表中的数据
    while (1) {
        // 检查链表是否为空
        if (IsListEmpty(&global.Header)) break;
        // 从链表头部移除一个项
        PLIST_ENTRY entry = RemoveHeadList(&global.Header);
        // 根据链表项的地址计算出 Item 结构体的地址
        Item* info = CONTAINING_RECORD(entry, Item, Entry);
        // 线程数据结构体的大小
        ULONG size = sizeof(ThreadData);
        // 检查缓冲区是否足够容纳线程数据
        if (len < size) {
            // 若不够，将项重新插入链表尾部并退出循环
            InsertTailList(&global.Header, entry);
            break;
        }
        // 链表中的项数减 1
        global.ItemCount--;
        // 将线程数据复制到缓冲区
        memcpy(buffer, &info->Data, size);
        // 移动缓冲区指针
        buffer += size;
        // 剩余缓冲区长度减少
        len -= size;
        // 已复制的字节数增加
        count += size;
        // 释放 Item 结构体占用的内存
        ExFreePool(info);
    }
    // 释放快速互斥锁
    ExReleaseFastMutex(&global.Mutex);
    // 设置 IRP 的信息字段为已复制的字节数
    Irp->IoStatus.Information = count;
    // 设置 IRP 的状态为成功
    Irp->IoStatus.Status = STATUS_SUCCESS;
    // 完成 IRP 请求
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

// 默认分发函数，处理创建和关闭设备对象的请求
NTSTATUS DispatchFuncDefault(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    // 避免编译器警告，因为 DeviceObject 未被使用
    UNREFERENCED_PARAMETER(DeviceObject);
    // 设置 IRP 的信息字段为 0
    Irp->IoStatus.Information = 0;
    // 设置 IRP 的状态为成功
    Irp->IoStatus.Status = STATUS_SUCCESS;
    // 完成 IRP 请求
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

// 驱动卸载函数，在驱动程序卸载时调用
void DriverUnload(PDRIVER_OBJECT pDriverObject)
{
    // 定义符号链接名称的 Unicode 字符串
    UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\RemoteThreadCheck");
    // 删除符号链接
    IoDeleteSymbolicLink(&symLink);
    // 删除设备对象
    IoDeleteDevice(pDriverObject->DeviceObject);
    // 移除线程创建和销毁的回调通知
    PsRemoveCreateThreadNotifyRoutine(ThreadNotify);
    // 清空链表
    while (!IsListEmpty(&global.Header)) {
        // 从链表头部移除一个项
        PLIST_ENTRY item = RemoveHeadList(&global.Header);
        // 根据链表项的地址计算出 Item 结构体的地址
        Item* fullitem = CONTAINING_RECORD(item, Item, Entry);
        // 释放 Item 结构体占用的内存
        ExFreePool(fullitem);
    }
    // 打印驱动卸载成功信息
    KdPrint(("DriverUnload ok\n"));
    return;
}

// 线程通知回调函数，当有线程创建或销毁时调用
void ThreadNotify(HANDLE ProcessId, HANDLE ThreadId, BOOLEAN Create) {
    // 检查是否是线程创建事件
    if (Create) {
        // 分配内存用于存储新的线程信息
        Item* item = (Item*)ExAllocatePool2(POOL_FLAG_PAGED, sizeof(Item), 'abcd');
        // 检查内存分配是否成功
        if (item) {
            // 设置线程 ID
            item->Data.ThreadId = HandleToUlong(ThreadId);
            // 设置创建线程的进程 ID
            item->Data.Create_ProcessId = HandleToUlong(PsGetCurrentProcessId());
            // 设置线程所属的进程 ID
            item->Data.Belong_ProcessId = HandleToUlong(ProcessId);
            // 检查是否为远程线程注入（排除 System 进程创建的第一个线程）
            if (item->Data.Create_ProcessId != item->Data.Belong_ProcessId && item->Data.Create_ProcessId != 4) {
                // 若检测到远程线程注入，打印相关信息
                KdPrint(("检测到远程线程注入,tid %d ,cpid %d , bpid %d\n", item->Data.ThreadId, item->Data.Create_ProcessId, item->Data.Belong_ProcessId));
                // 将线程信息插入到全局链表中
                PushItem(&item->Entry);
            }
            // 注释掉的代码，不释放内存，因为数据需要存储在链表中
            // ExFreePool(item);
        }
    }
}

// 将链表项插入链表的函数
void PushItem(LIST_ENTRY* entry) {
    // 获取快速互斥锁，保护对链表的访问
    ExAcquireFastMutex(&global.Mutex);
    // 检查链表中的项数是否超过 1024
    if (global.ItemCount > 1024) {
        // 若超过，移除链表头部的项
        PLIST_ENTRY pitem = RemoveHeadList(&global.Header);
        // 根据链表项的地址计算出 Item 结构体的地址
        ExFreePool(CONTAINING_RECORD(pitem, Item, Entry));
        // 链表中的项数减 1
        global.ItemCount--;
    }
    // 将新的链表项插入到链表尾部
    InsertTailList(&global.Header, entry);
    // 链表中的项数加 1
    global.ItemCount++;
    // 释放快速互斥锁
    ExReleaseFastMutex(&global.Mutex);
}