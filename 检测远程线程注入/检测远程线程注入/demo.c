#include <ntddk.h>

// �����߳����ݽṹ��
// ���ڴ洢�̵߳������Ϣ
typedef struct {
    // �̵߳� ID
    ULONG ThreadId;
    // �������̵߳Ľ��� ID
    ULONG Create_ProcessId;
    // ���߳������Ľ��� ID
    ULONG Belong_ProcessId;
} ThreadData;

// ����������ṹ��
// ����һ����������߳�����
typedef struct {
    // ��������ڽ��ýṹ����뵽˫��������
    LIST_ENTRY Entry;
    // �洢�߳������Ϣ�Ľṹ��
    ThreadData Data;
} Item;

// ����ȫ�����ݽṹ��
// ����һ�����ٻ������������������ͷ
typedef struct {
    // ���ٻ����������ڱ���������Ĳ�������
    FAST_MUTEX Mutex;
    // �����е�ǰ������
    ULONG ItemCount;
    // ˫�������ͷ�ڵ�
    LIST_ENTRY Header;
} Global;

// ȫ�ֱ��������ڴ洢�߳���Ϣ������
Global global;

// ����ж�غ�������
// ����������ж��ʱ���ô˺���
void DriverUnload(PDRIVER_OBJECT pDriverObject);
// �߳�֪ͨ�ص���������
// �����̴߳���������ʱ����ô˺���
void ThreadNotify(HANDLE ProcessId, HANDLE ThreadId, BOOLEAN Create);
// ���������ķַ���������
// ���ڴ����û�̬���豸����Ķ�����
NTSTATUS DispatchFuncRead(PDEVICE_OBJECT DeviceObject, PIRP Irp);
// Ĭ�Ϸַ���������
// ���ڴ������͹ر��豸���������
NTSTATUS DispatchFuncDefault(PDEVICE_OBJECT DeviceObject, PIRP Irp);
// ���������������ĺ�������
// ���ڽ��µ��߳���Ϣ���뵽ȫ��������
void PushItem(LIST_ENTRY* entry);

// ������ں������൱�� main ����
// �������������ʱ���ȵ��ô˺���
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegPath)
{
    // ������������棬��Ϊ pRegPath δ��ʹ��
    UNREFERENCED_PARAMETER(pRegPath);
    // �����豸���Ƶ� Unicode �ַ���
    UNICODE_STRING DevName = RTL_CONSTANT_STRING(L"\\Device\\RemoteThreadDev");
    // �豸����ָ��
    PDEVICE_OBJECT DeviceObject;
    // �����豸����
    NTSTATUS status = IoCreateDevice(pDriverObject, 0, &DevName, FILE_DEVICE_UNKNOWN, 0, 0, &DeviceObject);
    // ����豸�����Ƿ񴴽��ɹ�
    if (!NT_SUCCESS(status)) {
        // ��ʧ�ܣ���ӡ������Ϣ
        KdPrint(("�豸���󴴽�ʧ�� (0x%08X)\n", status));
        return status;
    }
    // �����豸����ı�־��ʹ��ֱ�� I/O ģʽ
    DeviceObject->Flags |= DO_DIRECT_IO;
    // ��������������Ƶ� Unicode �ַ���
    UNICODE_STRING SymbolicLink = RTL_CONSTANT_STRING(L"\\??\\RemoteThreadCheck");
    // �����������ӣ�ʹ�û�̬�����ܹ����ʸ��豸
    status = IoCreateSymbolicLink(&SymbolicLink, &DevName);
    // �����������Ƿ񴴽��ɹ�
    if (!NT_SUCCESS(status)) {
        // ��ʧ�ܣ���ӡ������Ϣ��ɾ���豸����
        KdPrint(("�������Ӵ���ʧ�� (0x%08X)\n", status));
        IoDeleteDevice(DeviceObject);
        return status;
    }
    // ע���̴߳��������ٵĻص�֪ͨ
    status = PsSetCreateThreadNotifyRoutine(ThreadNotify);
    // ���ص�ע���Ƿ�ɹ�
    if (!NT_SUCCESS(status)) {
        // ��ʧ�ܣ���ӡ������Ϣ��ɾ���豸����ͷ�������
        KdPrint(("ע���̻߳ص�ʧ�� (0x%08X)\n", status));
        IoDeleteDevice(DeviceObject);
        IoDeleteSymbolicLink(&SymbolicLink);
        return status;
    }
    // ��ע��ɹ�����ӡ�ɹ���Ϣ
    KdPrint(("�̻߳ص�ע��ɹ�\n"));
    // ��������ж�غ���
    pDriverObject->DriverUnload = DriverUnload;
    // ��ʼ��ȫ�������ͷ�ڵ�
    InitializeListHead(&global.Header);
    // ��ʼ�����ٻ�����
    ExInitializeFastMutex(&global.Mutex);
    // ָ�������͹ر��豸����ķַ�����
    pDriverObject->MajorFunction[IRP_MJ_CLOSE] = pDriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchFuncDefault;
    // ָ�����豸����ķַ�����
    pDriverObject->MajorFunction[IRP_MJ_READ] = DispatchFuncRead;
    // ���÷���״̬Ϊ�ɹ�
    status = STATUS_SUCCESS;
    return status;
}

// ���������ķַ�����
NTSTATUS DispatchFuncRead(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    // ������������棬��Ϊ DeviceObject δ��ʹ��
    UNREFERENCED_PARAMETER(DeviceObject);
    // ��ȡ��ǰ IRP ջλ��
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    // ��ȡ�û�����Ļ���������
    ULONG len = stack->Parameters.Read.Length;
    // �Ѹ��Ƶ����������ֽ���
    ULONG count = 0;
    // ���� MdlAddress ��Ϊ��
    NT_ASSERT(Irp->MdlAddress);
    // ��ȡ�������ķǷ�ҳϵͳ�������ַ
    UCHAR* buffer = (UCHAR*)MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
    // ����Ƿ�ɹ���ȡ��������ַ
    if (!buffer) {
        // ��ʧ�ܣ���ӡ������Ϣ
        KdPrint(("��ȡ�������Ƿ�ҳ�����ַʧ��"));
        return STATUS_UNSUCCESSFUL;
    }
    // ��ȡ���ٻ�����������������ķ���
    ExAcquireFastMutex(&global.Mutex);
    // ѭ����ȡ�����е�����
    while (1) {
        // ��������Ƿ�Ϊ��
        if (IsListEmpty(&global.Header)) break;
        // ������ͷ���Ƴ�һ����
        PLIST_ENTRY entry = RemoveHeadList(&global.Header);
        // ����������ĵ�ַ����� Item �ṹ��ĵ�ַ
        Item* info = CONTAINING_RECORD(entry, Item, Entry);
        // �߳����ݽṹ��Ĵ�С
        ULONG size = sizeof(ThreadData);
        // ��黺�����Ƿ��㹻�����߳�����
        if (len < size) {
            // ���������������²�������β�����˳�ѭ��
            InsertTailList(&global.Header, entry);
            break;
        }
        // �����е������� 1
        global.ItemCount--;
        // ���߳����ݸ��Ƶ�������
        memcpy(buffer, &info->Data, size);
        // �ƶ�������ָ��
        buffer += size;
        // ʣ�໺�������ȼ���
        len -= size;
        // �Ѹ��Ƶ��ֽ�������
        count += size;
        // �ͷ� Item �ṹ��ռ�õ��ڴ�
        ExFreePool(info);
    }
    // �ͷſ��ٻ�����
    ExReleaseFastMutex(&global.Mutex);
    // ���� IRP ����Ϣ�ֶ�Ϊ�Ѹ��Ƶ��ֽ���
    Irp->IoStatus.Information = count;
    // ���� IRP ��״̬Ϊ�ɹ�
    Irp->IoStatus.Status = STATUS_SUCCESS;
    // ��� IRP ����
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

// Ĭ�Ϸַ��������������͹ر��豸���������
NTSTATUS DispatchFuncDefault(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    // ������������棬��Ϊ DeviceObject δ��ʹ��
    UNREFERENCED_PARAMETER(DeviceObject);
    // ���� IRP ����Ϣ�ֶ�Ϊ 0
    Irp->IoStatus.Information = 0;
    // ���� IRP ��״̬Ϊ�ɹ�
    Irp->IoStatus.Status = STATUS_SUCCESS;
    // ��� IRP ����
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

// ����ж�غ���������������ж��ʱ����
void DriverUnload(PDRIVER_OBJECT pDriverObject)
{
    // ��������������Ƶ� Unicode �ַ���
    UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\RemoteThreadCheck");
    // ɾ����������
    IoDeleteSymbolicLink(&symLink);
    // ɾ���豸����
    IoDeleteDevice(pDriverObject->DeviceObject);
    // �Ƴ��̴߳��������ٵĻص�֪ͨ
    PsRemoveCreateThreadNotifyRoutine(ThreadNotify);
    // �������
    while (!IsListEmpty(&global.Header)) {
        // ������ͷ���Ƴ�һ����
        PLIST_ENTRY item = RemoveHeadList(&global.Header);
        // ����������ĵ�ַ����� Item �ṹ��ĵ�ַ
        Item* fullitem = CONTAINING_RECORD(item, Item, Entry);
        // �ͷ� Item �ṹ��ռ�õ��ڴ�
        ExFreePool(fullitem);
    }
    // ��ӡ����ж�سɹ���Ϣ
    KdPrint(("DriverUnload ok\n"));
    return;
}

// �߳�֪ͨ�ص������������̴߳���������ʱ����
void ThreadNotify(HANDLE ProcessId, HANDLE ThreadId, BOOLEAN Create) {
    // ����Ƿ����̴߳����¼�
    if (Create) {
        // �����ڴ����ڴ洢�µ��߳���Ϣ
        Item* item = (Item*)ExAllocatePool2(POOL_FLAG_PAGED, sizeof(Item), 'abcd');
        // ����ڴ�����Ƿ�ɹ�
        if (item) {
            // �����߳� ID
            item->Data.ThreadId = HandleToUlong(ThreadId);
            // ���ô����̵߳Ľ��� ID
            item->Data.Create_ProcessId = HandleToUlong(PsGetCurrentProcessId());
            // �����߳������Ľ��� ID
            item->Data.Belong_ProcessId = HandleToUlong(ProcessId);
            // ����Ƿ�ΪԶ���߳�ע�루�ų� System ���̴����ĵ�һ���̣߳�
            if (item->Data.Create_ProcessId != item->Data.Belong_ProcessId && item->Data.Create_ProcessId != 4) {
                // ����⵽Զ���߳�ע�룬��ӡ�����Ϣ
                KdPrint(("��⵽Զ���߳�ע��,tid %d ,cpid %d , bpid %d\n", item->Data.ThreadId, item->Data.Create_ProcessId, item->Data.Belong_ProcessId));
                // ���߳���Ϣ���뵽ȫ��������
                PushItem(&item->Entry);
            }
            // ע�͵��Ĵ��룬���ͷ��ڴ棬��Ϊ������Ҫ�洢��������
            // ExFreePool(item);
        }
    }
}

// ���������������ĺ���
void PushItem(LIST_ENTRY* entry) {
    // ��ȡ���ٻ�����������������ķ���
    ExAcquireFastMutex(&global.Mutex);
    // ��������е������Ƿ񳬹� 1024
    if (global.ItemCount > 1024) {
        // ���������Ƴ�����ͷ������
        PLIST_ENTRY pitem = RemoveHeadList(&global.Header);
        // ����������ĵ�ַ����� Item �ṹ��ĵ�ַ
        ExFreePool(CONTAINING_RECORD(pitem, Item, Entry));
        // �����е������� 1
        global.ItemCount--;
    }
    // ���µ���������뵽����β��
    InsertTailList(&global.Header, entry);
    // �����е������� 1
    global.ItemCount++;
    // �ͷſ��ٻ�����
    ExReleaseFastMutex(&global.Mutex);
}