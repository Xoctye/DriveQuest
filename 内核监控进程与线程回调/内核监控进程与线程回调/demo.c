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
		DbgPrint("[TEST] 线程TID: %1d | 所属进程名: %s | 进程PID: %1d \n", ThreadId, PsGetProcessImageFileName(eprocess), PsGetProcessId(eprocess));
	}
	if (eprocess)
		ObDereferenceObject(eprocess);
	if (ethread)
		ObDereferenceObject(ethread);
}
VOID UnDriver(PDRIVER_OBJECT driver)
{
	NTSTATUS status;

	// 注销进程回调
	status = PsRemoveCreateThreadNotifyRoutine(fnCreateThreadNotify);
}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT Driver, PUNICODE_STRING RegistryPath)
{
	NTSTATUS status;

	DbgPrint("hello TEST \n");

	// 绕过签名检查
	// LINKER_FLAGS=/INTEGRITYCHECK
	BypassCheckSign(Driver);

	// 创建线程回调
	// 参数1: 新线程ProcessID
	// 参数2: 新线程ThreadID
	// 参数3: 线程创建/退出标志
	status = PsSetCreateThreadNotifyRoutine(fnCreateThreadNotify);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("创建线程回调错误");
	}

	Driver->DriverUnload = UnDriver;
	return STATUS_SUCCESS;
}

/*
#include<ntifs.h>//ntifs.h包含ntddk.h，ntddk.h包含wdm.h
NTKERNELAPI PCHAR PsGetProcessImageFileName(PEPROCESS Process);//获取指定进程对象的映像文件名
NTKERNELAPI NTSTATUS PsLookupProcessByProcessId(HANDLE ProcessId, PEPROCESS* Process);//通过PID获取进程对象

//通过PID获取进程名
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
		DbgPrint("[TEST]  进程名: %s | 进程路径：%wZ | 父进程ID: %ld | 父进程名: %s \n", PsGetProcessImageFileName(Process), CreateInfo->ImageFileName ,CreateInfo->ParentProcessId, GetProcessNameByProcessId(CreateInfo->ParentProcessId));
		if (strcmp(ProcName, "notepad.exe") == 0)
		{
			CreateInfo->CreationStatus = STATUS_UNSUCCESSFUL;
		}
	}
	else
	{
		strcpy_s(ProcName, 16, PsGetProcessImageFileName(Process));
		DbgPrint("[TEST] 进程[ %s ] 退出了, 程序被关闭", ProcName);
	}
}

VOID UnDriver(PDRIVER_OBJECT DriverObject)
{
	DWORD32 ref = 0;
	//注销回调函数
	ref = PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)fnCreateProcessNotifyRoutineEx, TRUE);
	DbgPrint("[TEST] 注销进程回调: %d \n", ref);
}

//绕过签名检查
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
	// 调用 BypassCheckSign 函数，绕过签名检查
	BypassCheckSign(pDriverObject);
	NTSTATUS status;
	DbgPrint("hello TEST \n");
	// 调用 PsSetCreateProcessNotifyRoutineEx 函数，注册进程创建通知回调函数
	status = PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)fnCreateProcessNotifyRoutineEx, FALSE);
	// 检查注册操作是否成功
	if (!NT_SUCCESS(status))
	{
		DbgPrint("[TEST] 创建进程回调错误");
	}
	// 设置驱动卸载函数
	pDriverObject->DriverUnload = UnDriver;
	// 返回成功状态
	return STATUS_SUCCESS;
}
*/

/*
杀毒软件常用的回调

一、进程、线程创建/终止回调
PsSetCreateProcessNotifyRoutineEx
PsSetCreateThreadNotifyRoutine

二、文件系统操作回调
FsRtlRegisterFileSystemFilterCallbacks
MiniFilter

三、注册表修改回调
CmReigsterCallback
CmRegisterCallbackEx

四、模块加载回调
PsSetLoadImageNotifyRoutine
PsSetLoadImageNotifyRoutineEx

五、对象操作回调
ObRegisterCallbacks

六、网络流量
NDIS
WFP
*/













