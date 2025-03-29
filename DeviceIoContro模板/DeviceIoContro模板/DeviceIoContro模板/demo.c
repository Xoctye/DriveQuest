#include<ntifs.h>
#include<windef.h>
#define IOCTL_IO_TEST CTL_CODE(FILE_DEVICE_UNKNOWN,0X800,METHOD_BUFFERED,FILE_ANY_ACCESS)

NTSTATUS CreateDriverObject(PDRIVER_OBJECT pDriver)
{
	NTSTATUS Status;
	PDEVICE_OBJECT pDevObj;
	UNICODE_STRING DriverName;
	UNICODE_STRING SymLinkName;
	RtlInitUnicodeString(&DriverName, L"\\Device\\MyDriver");
	Status = IoCreateDevice(pDriver, 0, &DriverName, FILE_DEVICE_UNKNOWN, 0, FALSE, &pDevObj);
	pDriver->Flags |= DO_BUFFERED_IO;

	RtlInitUnicodeString(&SymLinkName, L"\\??\\MyDriver");
	Status = IoCreateSymbolicLink(&SymLinkName, &DriverName);
	return STATUS_SUCCESS;
}

NTSTATUS DispatchCreate(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS DispatchClose(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

VOID UnDriver(PDRIVER_OBJECT pDriver)
{
	PDEVICE_OBJECT pDevObj;
	UNICODE_STRING SymLinkName;
	pDevObj = pDriver->DeviceObject;
	IoDeleteDevice(pDevObj);
	RtlInitUnicodeString(&SymLinkName, L"\\??\\MyDriver");
	IoDeleteSymbolicLink(&SymLinkName);
	pDevObj = pDriver->DeviceObject;
}

NTSTATUS DispatchIoctl(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
	PIO_STACK_LOCATION pIrpStack;
	ULONG uIoControlCode;
	PVOID pIoBuffer;
	ULONG uInSize;
	ULONG uOutSize;

	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);//获取IRP里面的关键数据
	uIoControlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;//获取控制码
	pIoBuffer = pIrp->AssociatedIrp.SystemBuffer;//获取输入输出缓冲区
	uInSize = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
	uOutSize = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	switch (uIoControlCode)
	{
	case IOCTL_IO_TEST:
	{
		DbgPrint("IOCTL_IO_TEST\n");
		status = STATUS_SUCCESS;
		break;
	}
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = uOutSize;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
	}
	if (status == STATUS_SUCCESS)
	{
		pIrp->IoStatus.Information = uOutSize;
	}
	else
	{
		pIrp->IoStatus.Information = 0;
	}
	pIrp->IoStatus.Status = status;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegPath)
{
	CreateDriverObject(pDriver);
	pDriver->DriverUnload = UnDriver;
	pDriver->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
	pDriver->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;
	pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchIoctl;
	return STATUS_SUCCESS;
}

