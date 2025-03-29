#include<ntifs.h>
#include<ndis.h>
#include<stdio.h>

HANDLE g_hClient;
IO_STATUS_BLOCK g_ioStatusBlock;
KEVENT g_event;

typedef struct
{
	INT type;
	ULONG  address;
	ULONG buffer_data_len;
	CHAR buffer_data[0];
}Networkreport;

VOID NdisMSleep(ULONG MicrosecondsToSleep);

void init()
{
	UNICODE_STRING uniName;
	OBJECT_ATTRIBUTES objAttr;
	RtlInitUnicodeString(&uniName, L"\\DosDevice\\Pipe\\TEST");
	InitializeObjectAttributes(&objAttr, &uniName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	ZwCreateFile(&g_hClient, GENERIC_READ | GENERIC_WRITE, &objAttr, &g_ioStatusBlock, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	KeInitializeEvent(&g_event, SynchronizationEvent, TRUE);
}

VOID ReportToR3(Networkreport* m_parameter, int lent)
{
	if (!NT_SUCCESS(ZwWriteFile(g_hClient, NULL, NULL, NULL, &g_ioStatusBlock, (void*)m_parameter, lent, NULL, NULL)))
	{
		DbgPrint("写出错误");
	}
}
VOID UnDriver(PDRIVER_OBJECT driver)
{
	DbgPrint("驱动卸载成功 \n");
}
NTSTATUS DriverEntry(IN PDRIVER_OBJECT Driver, PUNICODE_STRING RegistryPath)
{
	init();
	for (int x = 0;x < 10;x++)
	{
		Networkreport* report = (Networkreport*)ExAllocatePoolWithTag(NonPagedPool, 4096,'lysh');
		if (report)
		{
			RtlZeroMemory(report, 4096);
			report->type = x;
			report->address = 0x12345678;
			report->buffer_data_len = 0x100;
			unsigned char* tmp = (unsigned char*)report + sizeof(Networkreport);
			memcpy(tmp, "test", 4);
			ReportToR3(report, 4096);
			ExFreePool(report);
		}
	}
	Driver->DriverUnload = UnDriver;
	return STATUS_SUCCESS;
}



















