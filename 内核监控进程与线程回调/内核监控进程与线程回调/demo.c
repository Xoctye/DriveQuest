#include<ntifs.h>
NTKERNELAPI PCHAR PsGetProcessImageFileName(PEPROCESS Process);
NTKERNELAPI NTSTATUS PsLookupProcessByProcessId(HANDLE ProcessId, PEPROCESS* Process);
NTKERNELAPI NTSTATUS PsLookupThreadByThreadId(HANDLE ThreadId, PETHREAD* Thread);

BOOLEAN BypassCheckSign(PDRIVER_OBJECT pDriverObject)
{
#ifdef _WIN64
	typedef struct _KLDR_DATA_TABLE_ENTRY
	{
		LIST_ENTRY listEntry;
		ULONG64 __Undefined1;
		ULONG64 __Undefined2;
		ULONG64 __Undefined3;
		ULONG64 NonPagedDebugInfo;
		ULONG64 DllBase;
		ULONG64 EntryPoint;
		ULONG SizeOfImage;
		UNICODE_STRING path;
		UNICODE_STRING name;
		ULONG   Flags;
		USHORT  LoadCount;
		USHORT  __Undefined5;
		ULONG64 __Undefined6;
		ULONG   CheckSum;
		ULONG   __padding1;
		ULONG   TimeDateStamp;
		ULONG   __padding2;
	} KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;
#else
	typedef struct _KLDR_DATA_TABLE_ENTRY
	{
		LIST_ENTRY listEntry;
		ULONG unknown1;
		ULONG unknown2;
		ULONG unknown3;
		ULONG unknown4;
		ULONG unknown5;
		ULONG unknown6;
		ULONG unknown7;
		UNICODE_STRING path;
		UNICODE_STRING name;
		ULONG   Flags;
	} KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;
#endif

	PKLDR_DATA_TABLE_ENTRY pLdrData = (PKLDR_DATA_TABLE_ENTRY)pDriverObject->DriverSection;
	pLdrData->Flags = pLdrData->Flags | 0x20;

	return TRUE;
}

VOID fnCreateThreadNotify(HANDLE ProcessId, HANDLE ThreadId, BOOLEAN CreateInfo)
{
	PEPROCESS eprocess = NULL;
	PETHREAD ethread = NULL;
	UCHAR* pWin32Address = NULL;

	PsLookupProcessByProcessId(ProcessId, &eprocess);
	PsLookupThreadByThreadId(ThreadId, &ethread);

	if (CreateInfo)
	{
		DbgPrint("[TEST] �߳�TID: %1d | ����������: %s | ����PID: %1d \n", ThreadId, PsGetProcessImageFileName(eprocess), PsGetProcessId(eprocess));
	}
	if (eprocess)
		ObDereferenceObject(eprocess);
	if (ethread)
		ObDereferenceObject(ethread);
}
VOID UnDriver(PDRIVER_OBJECT driver)
{
	NTSTATUS status;

	// ע�����̻ص�
	status = PsRemoveCreateThreadNotifyRoutine(fnCreateThreadNotify);
}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT Driver, PUNICODE_STRING RegistryPath)
{
	NTSTATUS status;

	DbgPrint("hello TEST \n");

	// �ƹ�ǩ�����
	// LINKER_FLAGS=/INTEGRITYCHECK
	BypassCheckSign(Driver);

	// �����̻߳ص�
	// ����1: ���߳�ProcessID
	// ����2: ���߳�ThreadID
	// ����3: �̴߳���/�˳���־
	status = PsSetCreateThreadNotifyRoutine(fnCreateThreadNotify);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("�����̻߳ص�����");
	}

	Driver->DriverUnload = UnDriver;
	return STATUS_SUCCESS;
}

/*
#include<ntifs.h>//ntifs.h����ntddk.h��ntddk.h����wdm.h
NTKERNELAPI PCHAR PsGetProcessImageFileName(PEPROCESS Process);//��ȡָ�����̶����ӳ���ļ���
NTKERNELAPI NTSTATUS PsLookupProcessByProcessId(HANDLE ProcessId, PEPROCESS* Process);//ͨ��PID��ȡ���̶���

//ͨ��PID��ȡ������
PCHAR GetProcessNameByProcessId(HANDLE ProcessId)
{
	NTSTATUS st = STATUS_SUCCESS;
	PEPROCESS ProcessObj = NULL;
	PCHAR string = NULL;
	st = PsLookupProcessByProcessId(ProcessId, &ProcessObj);
	if (NT_SUCCESS(st))
	{
		string = PsGetProcessImageFileName(ProcessObj);
		ObfDereferenceObject(ProcessObj);
	}
	return string;
}

VOID fnCreateProcessNotifyRoutineEx(PEPROCESS Process, HANDLE ProcessId, PPS_CREATE_NOTIFY_INFO CreateInfo)
{
	char ProcName[16] = { 0 };
	if (CreateInfo != NULL)
	{
		strcpy_s(ProcName, 16, PsGetProcessImageFileName(Process));
		DbgPrint("[TEST]  ������: %s | ����·����%wZ | ������ID: %ld | ��������: %s \n", PsGetProcessImageFileName(Process), CreateInfo->ImageFileName ,CreateInfo->ParentProcessId, GetProcessNameByProcessId(CreateInfo->ParentProcessId));
		if (strcmp(ProcName, "notepad.exe") == 0)
		{
			CreateInfo->CreationStatus = STATUS_UNSUCCESSFUL;
		}
	}
	else
	{
		strcpy_s(ProcName, 16, PsGetProcessImageFileName(Process));
		DbgPrint("[TEST] ����[ %s ] �˳���, ���򱻹ر�", ProcName);
	}
}

VOID UnDriver(PDRIVER_OBJECT DriverObject)
{
	DWORD32 ref = 0;
	//ע���ص�����
	ref = PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)fnCreateProcessNotifyRoutineEx, TRUE);
	DbgPrint("[TEST] ע�����̻ص�: %d \n", ref);
}

//�ƹ�ǩ�����
BOOLEAN BypassCheckSign(PDRIVER_OBJECT pDriverObject)
{
#ifdef _WIN64
	typedef struct _KLDR_DATA_TABLE_ENTRY
	{
		LIST_ENTRY listEntry;
		ULONG64 __Undefined1;
		ULONG64 __Undefined2;
		ULONG64 __Undefined3;
		ULONG64 NonPagedDebugInfo;
		ULONG64 DllBase;
		ULONG64 EntryPoint;
		ULONG SizeOfImage;
		UNICODE_STRING path;
		UNICODE_STRING name;
		ULONG   Flags;
		USHORT  LoadCount;
		USHORT  __Undefined5;
		ULONG64 __Undefined6;
		ULONG   CheckSum;
		ULONG   __padding1;
		ULONG   TimeDateStamp;
		ULONG   __padding2;
	}KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;
#else
	typedef struct _KLDR_DATA_TABLE_ENTRY
	{
		LIST_ENTRY listEntry;
		ULONG unknown1;
		ULONG unknown2;
		ULONG unknown3;
		ULONG unknown4;
		ULONG unknown5;
		ULONG unknown6;
		ULONG unknown7;
		UNICODE_STRING path;
		UNICODE_STRING name;
		ULONG   Flags;
	} KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;
#endif
	PKLDR_DATA_TABLE_ENTRY pLdrData = (PKLDR_DATA_TABLE_ENTRY)pDriverObject->DriverSection;
	pLdrData->Flags = pLdrData->Flags | 0x20;
	return TRUE;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
	// ���� BypassCheckSign �������ƹ�ǩ�����
	BypassCheckSign(pDriverObject);
	NTSTATUS status;
	DbgPrint("hello TEST \n");
	// ���� PsSetCreateProcessNotifyRoutineEx ������ע����̴���֪ͨ�ص�����
	status = PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)fnCreateProcessNotifyRoutineEx, FALSE);
	// ���ע������Ƿ�ɹ�
	if (!NT_SUCCESS(status))
	{
		DbgPrint("[TEST] �������̻ص�����");
	}
	// ��������ж�غ���
	pDriverObject->DriverUnload = UnDriver;
	// ���سɹ�״̬
	return STATUS_SUCCESS;
}
*/

/*
ɱ��������õĻص�

һ�����̡��̴߳���/��ֹ�ص�
PsSetCreateProcessNotifyRoutineEx
PsSetCreateThreadNotifyRoutine

�����ļ�ϵͳ�����ص�
FsRtlRegisterFileSystemFilterCallbacks
MiniFilter

����ע����޸Ļص�
CmReigsterCallback
CmRegisterCallbackEx

�ġ�ģ����ػص�
PsSetLoadImageNotifyRoutine
PsSetLoadImageNotifyRoutineEx

�塢��������ص�
ObRegisterCallbacks

������������
NDIS
WFP
*/













