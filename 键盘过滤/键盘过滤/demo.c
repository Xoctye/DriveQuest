/*
内核层分为：执行体层、微内核层、HAL层
EPROCESS属于执行体层，KPROCESS属于微内核层

*/


#include <wdm.h>
#include <ntddkbd.h>

NTKERNELAPI
NTSTATUS
ObReferenceObjectByName(
    IN PUNICODE_STRING ObjectName,
    IN ULONG Attributes,
    IN PACCESS_STATE PassedAccessState OPTIONAL,
    IN ACCESS_MASK DesiredAccess OPTIONAL,
    IN POBJECT_TYPE ObjectType,
    IN KPROCESSOR_MODE AccessMode,
    IN OUT PVOID ParseContext OPTIONAL,
    OUT PVOID* Object
);
extern POBJECT_TYPE IoDeviceObjectType;

#define KS_KDBCLASS_NAME L"\\Driver\\Kbdclass"

PDRIVER_OBJECT KS_gDriver;
ULONG KS_KeyCount;

typedef struct KS_DEV_EXT
{
    ULONG Size;
    PDEVICE_OBJECT KS_Ndevice;
    KSPIN_LOCK KS_Lock;
    KEVENT KS_Event;
    PDEVICE_OBJECT KS_Ldevice;
    PDEVICE_OBJECT KS_Rdevice;
}KS_C2P_DEV_EXT, * KS_PC2P_DEV_EXT;

VOID KS_driverUnload(PDRIVER_OBJECT KS_driver)
{
    PRKTHREAD KS_pThread;
    LARGE_INTEGER KS_Delay;
    PDEVICE_OBJECT KS_device;
    KS_PC2P_DEV_EXT KS_dev;
    KS_Delay = RtlConvertLongToLargeInteger(-10 * 3000000);
    KS_pThread = KeGetCurrentThread();
    KeSetPriorityThread(KS_pThread, LOW_REALTIME_PRIORITY);

    KS_device = KS_driver->DeviceObject;
    while (KS_device)
    {
        __try
        {
            KS_dev = (KS_PC2P_DEV_EXT)KS_device->DeviceExtension;
            IoDetachDevice(KS_dev->KS_Ldevice);
            KS_dev->KS_Ldevice = NULL;
            IoDeleteDevice(KS_device);
            KS_dev->KS_Ndevice = NULL;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
        KS_device = KS_device->NextDevice;
    }
    while (KS_KeyCount)
    {
        KeDelayExecutionThread(KernelMode, FALSE, &KS_Delay);
    }
    return;
}

VOID KS_pDEV_Init(KS_PC2P_DEV_EXT pDEV, PDEVICE_OBJECT KS_Ndevice,
    PDEVICE_OBJECT KS_Ldevice, PDEVICE_OBJECT KS_Rdevice)
{
    pDEV->KS_Ndevice = KS_Ndevice;
    pDEV->KS_Rdevice = KS_Rdevice;
    pDEV->KS_Ldevice = KS_Ldevice;
    pDEV->Size = sizeof(KS_C2P_DEV_EXT);
    KeInitializeSpinLock(&(pDEV->KS_Lock));
    KeInitializeEvent(&(pDEV->KS_Event), NotificationEvent, FALSE);
}

NTSTATUS KS_deviceAttach(PDRIVER_OBJECT KS_driver)
{
    UNICODE_STRING KS_kbdName;
    PDRIVER_OBJECT KS_KbdDriver = NULL;
    NTSTATUS status;
    PDEVICE_OBJECT KS_Ldevice = NULL;
    PDEVICE_OBJECT KS_Ndevice = NULL;
    PDEVICE_OBJECT KS_Rdevice = NULL;
    KS_PC2P_DEV_EXT pExtension = NULL;

    RtlInitUnicodeString(&KS_kbdName, KS_KDBCLASS_NAME);
    status = ObReferenceObjectByName(&KS_kbdName, OBJ_CASE_INSENSITIVE, NULL,
        FILE_ALL_ACCESS, IoDeviceObjectType, KernelMode, NULL, &KS_KbdDriver);
    if (!NT_SUCCESS(status))
    {
        return status;
    }
    else
    {
        ObDereferenceObject(KS_driver);
    }

    KS_Ldevice = KS_KbdDriver->DeviceObject;
    while (KS_Ldevice)
    {
        status = IoCreateDevice(KS_driver, sizeof(KS_C2P_DEV_EXT), NULL, KS_Ldevice->DeviceType,
            KS_Ldevice->Characteristics, FALSE, &KS_Ndevice);
        if (!NT_SUCCESS(status))
        {
            return status;
        }
        KS_Rdevice = IoAttachDeviceToDeviceStack(KS_Ndevice, KS_Ldevice);
        if (!KS_Rdevice)
        {
            IoDeleteDevice(KS_Ndevice);
            KS_Ndevice = NULL;
            return status;
        }
        pExtension = (KS_PC2P_DEV_EXT)KS_Ndevice->DeviceExtension;
        KS_pDEV_Init(pExtension, KS_Ndevice, KS_Ldevice, KS_Rdevice);
        KS_Ndevice->Characteristics = KS_Rdevice->Characteristics;
        KS_Ndevice->DeviceType = KS_Rdevice->DeviceType;
        KS_Ndevice->StackSize = KS_Rdevice->StackSize + 1;
        KS_Ndevice->Flags |= KS_Rdevice->Flags & (DO_BUFFERED_IO | DO_DIRECT_IO | DO_POWER_PAGABLE);

        KS_Ldevice = KS_Ldevice->NextDevice;
    }
    return status;
}

NTSTATUS KS_Completiom(PDEVICE_OBJECT KS_device, PIRP Irp, PVOID KS_text)
{
    PIO_STACK_LOCATION KS_sIrp = IoGetCurrentIrpStackLocation(Irp);
    PUCHAR KS_buf = NULL;
    ULONG len, i;
    if (NT_SUCCESS(Irp->IoStatus.Status))
    {
        KS_buf = Irp->AssociatedIrp.SystemBuffer;
        len = Irp->IoStatus.Information;
        for (i = 0; i < len; i++)
        {
        }
    }
    KS_KeyCount--;
    if (Irp->PendingReturned)
    {
        IoMarkIrpPending(Irp);
    }
    return Irp->IoStatus.Status;
}

NTSTATUS KS_Dispatch(PDEVICE_OBJECT KS_deviceObj, PIRP Irp)
{
    KS_PC2P_DEV_EXT KS_pDEV;
    PIO_STACK_LOCATION KS_sIrp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status;
    KS_pDEV = (KS_PC2P_DEV_EXT)KS_deviceObj->DeviceExtension;
    if (KS_sIrp->MajorFunction == IRP_MJ_POWER)
    {
        PoStartNextPowerIrp(Irp);
        IoSkipCurrentIrpStackLocation(Irp);
        return PoCallDriver(KS_pDEV->KS_Rdevice, Irp);
    }

    if (KS_sIrp->MajorFunction == IRP_MJ_PNP)
    {
        if (KS_sIrp->MinorFunction == IRP_MN_REMOVE_DEVICE)
        {
            IoSkipCurrentIrpStackLocation(Irp);
            IoCallDriver(KS_pDEV->KS_Rdevice, Irp);
            IoDetachDevice(KS_pDEV->KS_Rdevice);
            IoDeleteDevice(KS_deviceObj);
            return STATUS_SUCCESS;
        }
        else
        {
            IoSkipCurrentIrpStackLocation(Irp);
            return IoCallDriver(KS_pDEV->KS_Rdevice, Irp);
        }
    }
    if (KS_sIrp->MajorFunction == IRP_MJ_READ)
    {
        KEVENT KS_waitEvent;
        KeInitializeEvent(&KS_waitEvent, NotificationEvent, FALSE);
        if (Irp->CurrentLocation == 1)
        {
            status = STATUS_INVALID_DEVICE_REQUEST;
            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return status;
        }
        KS_KeyCount++;
        KS_pDEV = (KS_PC2P_DEV_EXT)KS_deviceObj->DeviceExtension;
        IoCopyCurrentIrpStackLocationToNext(Irp);
        IoSetCompletionRoutine(Irp, KS_Completiom, KS_deviceObj, TRUE, TRUE, TRUE);
        return IoCallDriver(KS_pDEV->KS_Rdevice, Irp);
    }
}

NTSTATUS DriverEntry(PDRIVER_OBJECT KS_driver, PUNICODE_STRING KS_ustr)
{
    ULONG i;
#if DBG
    __asm int 3
#endif
    for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
    {
        KS_driver->MajorFunction[i] = KS_Dispatch;
    }
    KS_driver->DriverUnload = KS_driverUnload;
    KS_gDriver = KS_driver;
    return KS_deviceAttach(KS_driver);
}
