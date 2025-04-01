/*
串口设备在windows中以设备栈的形式存在，过滤驱动通过创建过滤设备对象并附加到目标设备栈顶部，截获所有流经的I/O请求包（IRP）；
过滤驱动可以处理特定IRP，修改数据或参数后，再将IRP传递给下层真实设备；

对于过滤驱动，通常不直接创建设备，而是在检测到目标设备时动态附加过滤设备（IoAttachDeviceToDeviceStackSafe）；

IoAttachDeviceToDeviceStackSafe：将过滤设备对象安全附加到目标设备栈顶部，返回原栈顶设备对象；

IoSkipCurrentIrpStackLocation：跳过当前IRP栈位置，将IRP传递给下层设备；

IoCallDriver：将处理后的IRP传递给下层设备；

IoCreateDevice：创建过滤设备对象

MmGetSystemAddressForMdlSafe：获取MDL（内存描述符列表）对应的系统空间地址，用于访问缓冲区。

IoDetachDevice：分离过滤设备对象

IoDeleteDevice：删除过滤设备对象

*/

#include<ntddk.h>
#include<ntstrsafe.h>

PDEVICE_OBJECT KS_MyPorts[32];
PDEVICE_OBJECT KS_OtherPorts[32];

PDEVICE_OBJECT KS_OpenPort(ULONG64 id, NTSTATUS* status)
{
    WCHAR PortName[32] = { 0 };
    UNICODE_STRING KS_Name;
    PFILE_OBJECT KS_FileObj = NULL;
    PDEVICE_OBJECT KS_Dev = NULL;
    RtlStringCchPrintfW(PortName, 32, L"\\Device\\Serial%d", id);
    RtlInitUnicodeString(&KS_Name, PortName);
    *status = IoGetDeviceObjectPointer(&KS_Name, FILE_ALL_ACCESS, &KS_FileObj, &KS_Dev);
    if (*status == STATUS_SUCCESS)
    {
        ObDereferenceObject(KS_FileObj);
    }
    return KS_Dev;
}

NTSTATUS KS_DeviceAttach(
    PDRIVER_OBJECT driver,
    PDEVICE_OBJECT KS_Lobj,
    PDEVICE_OBJECT* KS_Nobj,
    PDEVICE_OBJECT* next)
{
    NTSTATUS status;
    PDEVICE_OBJECT KS_newDev;
    status = IoCreateDevice(driver, 0, NULL, KS_Lobj->DeviceType, 0, FALSE, KS_Nobj);
    if (!NT_SUCCESS(status))
    {
        DbgPrint("Create device failed!\r\n");
        status = STATUS_UNSUCCESSFUL;
        return status;
    }
    if (KS_Lobj->Flags & DO_BUFFERED_IO)
        (*KS_Nobj)->Flags |= DO_BUFFERED_IO;
    if (KS_Lobj->Flags & DO_DIRECT_IO)
        (*KS_Nobj)->Flags |= DO_DIRECT_IO;
    if (KS_Lobj->Flags & FILE_DEVICE_SECURE_OPEN)
        (*KS_Nobj)->Flags |= FILE_DEVICE_SECURE_OPEN;
    (*KS_Nobj)->Flags |= DO_POWER_PAGABLE;

    // 绑定设备
    KS_newDev = IoAttachDeviceToDeviceStack(*KS_Nobj, KS_Lobj);
    if (KS_newDev == NULL)
    {
        IoDeleteDevice(*KS_Nobj);
        *KS_Nobj = NULL;
        status = STATUS_UNSUCCESSFUL;
        return status;
    }
    *next = KS_newDev;
    (*KS_Nobj)->Flags = (*KS_Nobj)->Flags & ~DO_DEVICE_INITIALIZING;
    return STATUS_SUCCESS;
}

NTSTATUS KS_Dispatch(PDEVICE_OBJECT dev, PIRP irp)
{
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(irp);
    NTSTATUS status;
    ULONG i, j;
    for (i = 0; i < 32; i++)
    {
        if (KS_MyPorts[i] == dev)
        {
            if (IrpStack->MajorFunction == IRP_MJ_POWER)
            {
                PoStartNextPowerIrp(irp);
                IoSkipCurrentIrpStackLocation(irp);
                return PoCallDriver(KS_OtherPorts[i], irp);
            }
            if (IrpStack->MajorFunction == IRP_MJ_WRITE)
            {
                ULONG len = IrpStack->Parameters.Write.Length;
                PUCHAR buffer = NULL;
                if (irp->MdlAddress != NULL)
                {
                    buffer = (PUCHAR)MmGetSystemAddressForMdlSafe(irp->MdlAddress, NormalPagePriority);
                }
                else if (irp->UserBuffer != NULL)
                {
                    buffer = (PUCHAR)irp->UserBuffer;
                }
                else if (irp->AssociatedIrp.SystemBuffer != NULL)
                {
                    buffer = (PUCHAR)irp->AssociatedIrp.SystemBuffer;
                }
                if (buffer != NULL)
                {
                    for (j = 0; j < len; j++)
                    {
                        DbgPrint("The data:%2x\r\n", buffer[j]);
                    }
                }
                IoSkipCurrentIrpStackLocation(irp);
                return IoCallDriver(KS_OtherPorts[i], irp);
            }
        }
    }
    irp->IoStatus.Information = 0;
    irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
    IoCompleteRequest(irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

VOID KS_driverUnload(PDRIVER_OBJECT KS_driver)
{
    ULONG i;
    for (i = 0; i < 32; i++)
    {
        if (KS_MyPorts[i] != NULL)
        {
            IoDetachDevice(KS_MyPorts[i]);
            IoDeleteDevice(KS_MyPorts[i]);
            KS_MyPorts[i] = NULL;
            KS_OtherPorts[i] = NULL;
        }
    }
    DbgPrint("My driver is unloading...");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT KS_driver, PUNICODE_STRING KS_UNS)
{
    ULONG i;
    NTSTATUS status;
    PDEVICE_OBJECT KS_Ports;
    for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
    {
        KS_driver->MajorFunction[i] = KS_Dispatch;
    }
    KS_driver->DriverUnload = KS_driverUnload;
    for (i = 9; i < 32; i++)
    {
        KS_Ports = KS_OpenPort(i, &status);
        if (KS_Ports == NULL)
        {
            continue;
        }
        KS_DeviceAttach(KS_driver, KS_Ports, &KS_MyPorts[i], &KS_OtherPorts[i]);
    }
    return STATUS_SUCCESS;
}