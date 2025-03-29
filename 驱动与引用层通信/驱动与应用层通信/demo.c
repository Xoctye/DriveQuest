/*

*/
#include<ntddk.h>

NTSTATUS CreateDriverObject(PDRIVER_OBJECT pDriver)
{
	NTSTATUS Status;
	PDRIVER_OBJECT pDevObj;
	UNICODE_STRING DriverName;
	UNICODE_STRING SymLinkName;

	RtlInitUnicodeString(&DriverName, L"\\Device\\My_Device");
	Status = IoCreateDevice(pDriver, 0, &DriverName, FILE_DEVICE_UNKNOWN, 0, TRUE, &pDevObj);
	pDevObj->Flags |= DO_BUFFERED_IO;
	RtlInitUnicodeString(&SymLinkName, L"\\??\\My_Device");
	Status = IoCreateSymbolicLink(&SymLinkName, &DriverName);
	return STATUS_SUCCESS;
}


NTSTATUS DispatchCreate(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS DispatchRead(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PIO_STACK_LOCATION Stack = IoGetCurrentIrpStackLocation(pIrp);
	ULONG ulReadLength = Stack->Parameters.Read.Length;
	pIrp->IoStatus.Status = Status;
	pIrp->IoStatus.Information = ulReadLength;
	memset(pIrp->AssociatedIrp.SystemBuffer, 0x68, ulReadLength);
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return Status;
}



NTSTATUS DispatchClose(PDEVICE_OBJECT pDevObj,PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

VOID UnDriver(PDRIVER_OBJECT pDriver)
{
	PDEVICE_OBJECT pDev;
	UNICODE_STRING SymLinkName;
	pDev = pDriver->DeviceObject;
	IoDeleteDevice(pDev);
	RtlInitUnicodeString(&SymLinkName, L"\\??\\My_Driver");
	IoDeleteSymbolicLink(&SymLinkName);
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING RegistryPath)
{
	CreateDriverObject(pDriver);
	pDriver->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
	pDriver->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;
	pDriver->DriverUnload = UnDriver;
	return STATUS_SUCCESS;
}













