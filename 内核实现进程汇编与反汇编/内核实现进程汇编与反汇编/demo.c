#include<ntifs.h>
#include<windef.h>
//ʹ��CTL_CODE������һ���豸�����룬FILE_DEVICE_UNKNOWN��ʾδ֪�豸���ͣ�METHOD_BUFFERED��ʾʹ�û�����I/O��ʽ
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
	//���ӵ�Ŀ����̵ĵ�ַ�ռ�
	KeStackAttachProcess(process, &stack);

	__try
	{
		//���Ŀ���ַ�Ƿ�ɶ�
		ProbeForRead(ProcessData->address, ProcessData->size, 1);
		RtlCopyMemory(GetProcessData, ProcessData->address, ProcessData->size);
	}
	__except (1)
	{
		bRet = FALSE;
	}
	//���ٽ��̶�������ü���
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
	//����һ��MDL���ڴ���������������Ҫд����ڴ�����
	PMDL mdl = IoAllocateMdl(ProcessData->address, ProcessData->size, 0, 0, NULL);
	if (mdl == NULL)
	{
		return FALSE;
	}
	//Ϊ�Ƿ�ҳ�ڴ�ع���MDL
	MmBuildMdlForNonPagedPool(mdl);
	BYTE* ChangeProcessData = NULL;
	__try
	{
		ChangeProcessData = MmMapLockedPages(mdl, KernelMode);
		//�����ػ����������ݸ��Ƶ�Ŀ����̵��ڴ���
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
		DbgPrint("����ID: %d | ��д��ַ: %p | ��д����: %d \n", ProcessData->pid, ProcessData->address, ProcessData->size);
		switch (stack->Parameters.DeviceIoControl.IoControlCode)
		{
		case READ_PROCESS_CODE:
		{
			ReadProcessMemory(ProcessData);
			break;
		}
		// д�뺯��
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