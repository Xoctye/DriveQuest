#include <ntifs.h>
#include <windef.h>

#define My_Code CTL_CODE(FILE_DEVICE_UNKNOWN,0x800,METHOD_BUFFERED,FILE_ANY_ACCESS)

typedef struct Hread
{
	ULONG Flage;
	ULONG Addr;
	ULONG WriteBufferAddr;
	ULONG Size;
	ULONG Pid;
}_Hread, * PtrHread;

typedef struct _DEVICE_EXTENSION
{
	UNICODE_STRING SymLinkName;
} DEVICE_EXTENSION, * PDEVICE_EXTENSION;

VOID DriverUnload(PDRIVER_OBJECT pDriverObject)
{
	PDEVICE_OBJECT pDevObj;
	pDevObj = pDriverObject->DeviceObject;
	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	UNICODE_STRING pLinkName = pDevExt->SymLinkName;

	IoDeleteSymbolicLink(&pLinkName);
	IoDeleteDevice(pDevObj);
}

NTSTATUS DefDispatchRoutine(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS status = STATUS_SUCCESS;
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS IoctlDispatchRoutine(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	ULONG_PTR Informaiton = 0;
	PVOID InputData = NULL;
	ULONG InputDataLength = 0;
	PVOID OutputData = NULL;
	ULONG OutputDataLength = 0;
	PIO_STACK_LOCATION  IoStackLocation = IoGetCurrentIrpStackLocation(pIrp);          
	InputData = pIrp->AssociatedIrp.SystemBuffer;                                         
	OutputData = pIrp->AssociatedIrp.SystemBuffer;                                        
	InputDataLength = IoStackLocation->Parameters.DeviceIoControl.InputBufferLength;     
	OutputDataLength = IoStackLocation->Parameters.DeviceIoControl.OutputBufferLength;   
	ULONG Code = IoStackLocation->Parameters.DeviceIoControl.IoControlCode;            
	switch (Code)
	{
	case My_Code:
	{
		PtrHread PtrBuff = (PtrHread)InputData;
		ULONG RetFlage = PtrBuff->Flage;
		ULONG RetAddr = PtrBuff->Addr;
		ULONG RetBufferAddr = PtrBuff->WriteBufferAddr;
		ULONG Size = PtrBuff->Size;
		ULONG Pid = PtrBuff->Pid;

		DbgPrint("读取文件标志：%d", RetFlage);
		DbgPrint("读取写入地址：%x", RetAddr);
		DbgPrint("读取缓冲区大小：%d", RetBufferAddr);
		DbgPrint("读取当前大小：%d", Size);
		DbgPrint("要操作进程PID: %d", Pid);
		
		char* retBuffer = "test";
		memcpy(OutputData, retBuffer, strlen(retBuffer));
		Informaiton = strlen(retBuffer) + 1;
		Status = STATUS_SUCCESS;
		break;
	}
	}

	pIrp->IoStatus.Status = Status;                 
	pIrp->IoStatus.Information = Informaiton;       
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);     
	return Status;
}

// 驱动入口
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
	pDriverObject->DriverUnload = DriverUnload;                        
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = DefDispatchRoutine;  
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = DefDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = DefDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_READ] = DefDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoctlDispatchRoutine;

	NTSTATUS status;
	PDEVICE_OBJECT pDevObj;
	PDEVICE_EXTENSION pDevExt;

	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, L"\\Device\\MyDevice");

	status = IoCreateDevice(pDriverObject, sizeof(DEVICE_EXTENSION), &devName, FILE_DEVICE_UNKNOWN, 0, TRUE, &pDevObj);
	pDevObj->Flags |= DO_BUFFERED_IO;                     
	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension; 

	UNICODE_STRING symLinkName;
	RtlInitUnicodeString(&symLinkName, L"\\??\\MyDevice");
	pDevExt->SymLinkName = symLinkName;
	status = IoCreateSymbolicLink(&symLinkName, &devName);
	return STATUS_SUCCESS;
}
