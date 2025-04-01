#include<ntifs.h>
#include<windef.h>
//使用CTL_CODE宏生成一个设备控制码，FILE_DEVICE_UNKNOWN表示未知设备类型，METHOD_BUFFERED表示使用缓冲区I/O方式
#define READ_PROCESS_CODE CTL_CODE(FILE_DEVICE_UNKNOWN,0X800,METHOD_BUFFERED,FILE_ALL_ACCESS)
#define WRITE_PROCESS_CODE CTL_CODE(FILE_DEVICE_UNKNOWN,0X801,METHOD_BUFFERED,FILE_ALL_ACCESS)
#define DEVICENAME L"\\DEVICE\\READWRITEDEVICE"
#define SYMBOLNAME L"\\??\\READWRITESYMBOLNAME"

typedef struct
{
	DWORD pid;
	UINT64 address;
	DWORD size;
	BYTE* data;
}ProcessData;

BOOLEAN ReadProcessMemory(ProcessData* ProcessData)
{
	BOOLEAN bRet = TRUE;
	PEPROCESS process = NULL;
	PsLookupProcessByProcessId(ProcessData->pid, &process);
	if (process == NULL)
	{
		return FALSE;
	}
	BYTE* GetProcessData = NULL;
	__try
	{
		GetProcessData = ExAllocatePool(NonPagedPool, ProcessData->size);
	}
	__except (1)
	{
		return FALSE;
	}
	KAPC_STATE stack = { 0 };
	//附加到目标进程的地址空间
	KeStackAttachProcess(process, &stack);

	__try
	{
		//检查目标地址是否可读
		ProbeForRead(ProcessData->address, ProcessData->size, 1);
		RtlCopyMemory(GetProcessData, ProcessData->address, ProcessData->size);
	}
	__except (1)
	{
		bRet = FALSE;
	}
	//减少进程对象的引用计数
	ObDereferenceObject(process);
	KeUnstackDetachProcess(&stack);
	RtlCopyMemory(ProcessData->data, GetProcessData, ProcessData->size);
	ExFreePool(GetProcessData);
	return bRet;
}

BOOLEAN WriteProcessMemory(ProcessData* ProcessData)
{
	BOOLEAN bRet = TRUE;
	PEPROCESS process = NULL;
	PsLookupProcessByProcessId(ProcessData->pid, &process);
	if (process == NULL)
	{
		return FALSE;
	}
	BYTE* GetProcessData = NULL;
	__try
	{
		GetProcessData = ExAllocatePool(NonPagedPool, ProcessData->size);
	}
	__except (1)
	{
		return FALSE;
	}
	for (int i = 0;i < ProcessData->size;i++)
	{
		GetProcessData[i] = ProcessData->data[i];
	}
	KAPC_STATE stack = { 0 };
	KeStackAttachProcess(process, &stack);
	//分配一个MDL（内存描述符表）来描述要写入的内存区域；
	PMDL mdl = IoAllocateMdl(ProcessData->address, ProcessData->size, 0, 0, NULL);
	if (mdl == NULL)
	{
		return FALSE;
	}
	//为非分页内存池构建MDL
	MmBuildMdlForNonPagedPool(mdl);
	BYTE* ChangeProcessData = NULL;
	__try
	{
		ChangeProcessData = MmMapLockedPages(mdl, KernelMode);
		//将本地缓冲区的数据复制到目标进程的内存中
		RtlCopyMemory(ChangeProcessData, GetProcessData, ProcessData->size);
	}
	__except (1)
	{
		bRet = FALSE;
		goto END;
	}
END:
	IoFreeMdl(mdl);
	ExFreePool(GetProcessData);
	KeUnstackDetachProcess(&stack);
	ObDereferenceObject(process);
	return bRet;
}

NTSTATUS DriverIrpCtl(PDEVICE_OBJECT device, PIRP pirp)
{
	PIO_STACK_LOCATION stack;
	stack = IoGetCurrentIrpStackLocation(pirp);
	ProcessData* ProcessData;
	switch (stack->MajorFunction)
	{
	case IRP_MJ_CREATE:
	{
		break;
	}
	case IRP_MJ_CLOSE:
	{
		break;
	}

	case IRP_MJ_DEVICE_CONTROL:
	{
		ProcessData = pirp->AssociatedIrp.SystemBuffer;
		DbgPrint("进程ID: %d | 读写地址: %p | 读写长度: %d \n", ProcessData->pid, ProcessData->address, ProcessData->size);
		switch (stack->Parameters.DeviceIoControl.IoControlCode)
		{
		case READ_PROCESS_CODE:
		{
			ReadProcessMemory(ProcessData);
			break;
		}
		// 写入函数
		case WRITE_PROCESS_CODE:
		{
			WriteProcessMemory(ProcessData);
			break;
		}
		}
		pirp->IoStatus.Information = sizeof(ProcessData);
		break;
	}
	}
	pirp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pirp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}
VOID UnDriver(PDRIVER_OBJECT driver)
{
	if (driver->DeviceObject)
	{
		UNICODE_STRING SymbolName;
		RtlInitUnicodeString(&SymbolName, SYMBOLNAME);
		IoDeleteSymbolicLink(&SymbolName);
		IoDeleteDevice(driver->DeviceObject);
	}
}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT Driver, PUNICODE_STRING RegistryPath)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_OBJECT device = NULL;
	UNICODE_STRING DeviceName;
	RtlInitUnicodeString(&DeviceName, DEVICENAME);
	status = IoCreateDevice(Driver, sizeof(Driver->DriverExtension), &DeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &device);
	if (status == STATUS_SUCCESS)
	{
		UNICODE_STRING SymbolName;
		RtlInitUnicodeString(&SymbolName, SYMBOLNAME);
		status=IoCreateSymbolicLink(&SymbolName, &DeviceName);
		if (status != STATUS_SUCCESS)
		{
			IoDeleteDevice(device);
		}
	}
	Driver->MajorFunction[IRP_MJ_CREATE] = DriverIrpCtl;
	Driver->MajorFunction[IRP_MJ_CLOSE] = DriverIrpCtl;
	Driver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverIrpCtl;
	Driver->DriverUnload = UnDriver;
	return STATUS_SUCCESS;
}