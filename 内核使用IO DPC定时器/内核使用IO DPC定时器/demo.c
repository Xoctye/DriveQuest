#include<ntifs.h>
#include<wdm.h>
#include<ntstrsafe.h>
LONG count = 0;
VOID MyTimerProcess(struct _DRIVER_OBJECT* DeviceObject, PVOID Context)
{
	InterlockedIncrement(&count);
}
VOID UnDriver(PDRIVER_OBJECT driver)
{
	IoStopTimer(driver->DeviceObject);
	IoDeleteDevice(driver->DeviceObject);
}
NTSTATUS DriverEntry(PDRIVER_OBJECT Driver, PUNICODE_STRING RegistryPath)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	UNICODE_STRING dev_name = RTL_CONSTANT_STRING(L"");
	PDRIVER_OBJECT dev;
	status = IoCreateDevice(Driver, 0, &dev_name, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &dev);
	if (!NT_SUCCESS(status))
	{
		return STATUS_UNSUCCESSFUL;
	}
	else
	{
		IoInitializeTimer(dev, MyTimerProcess, NULL);
		IoStartTimer(dev);
	}
	Driver->DriverUnload = UnDriver;
	return STATUS_SUCCESS;
}










