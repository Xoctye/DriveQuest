/*
当我们需要保存一个结构体数组时，需要使用LIST_ENTRY；

PsGetProcessPeb：用于获取进程的进程环境快指针；
PsLookupProcessByProcessId：依据进程ID查找进程的EPROCESS结构体；
PsGetProcessWow64Process：获取进程的Wow64进程信息；
PsGetProcessImageFileName：获取进程的映像文件名
PsGetProcessInheritedFromUniqueProcessId：获取进程的父进程ID；
ExAllocatePool：用于分配内存；
ExFreePool：用于释放内存；
ObDereferenceObject：减少对进程对象的引用计数；
RtlInitString：初始化支付穿结构体；
RtlCopyMemory：复制内存块；
RtlZeroMemory：将指定内存块清零；
InitializeListHead：初始化链表头部；
InsertTailList：把节点插入链表尾部；
RemoveHeadList：从链表头部移除一个节点；
*/
//枚举用户进程，将枚举到的进程存储到链表结构体内
#include<ntifs.h>
#include<windef.h>
extern PVOID PsGetProcessPeb(PEPROCESS Process);
NTKERNELAPI NTSTATUS PsLooukupProcessByProcessId(HANDLE ProcessId, PEPROCESS* Process);
extern NTKERNELAPI PVOID PsGetProcessWow64Process(PEPROCESS Process);
extern NTKERNELAPI UCHAR* PsGetProcessImageFileName(PEPROCESS Process);
extern NTKERNELAPI HANDLE PsGetProcessInheritedFromUniqueProcessId(PEPROCESS Process);//获取进程的父进程ID

//定义进程列表结构体
typedef struct
{
	DWORD Pid;
	UCHAR ProcessName[2048];
	DWORD Handle; //父进程ID
	LIST_ENTRY ListEntry;//用于将该结构体插入到链表中
}ProcessList;

//根据进程ID返回进程EPROCESS结构体
PEPROCESS LookupProcess(HANDLE Pid)
{
	PEPROCESS eprocess = NULL;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	Status = PsLookupProcessByProcessId(Pid, &eprocess);
	if (NT_SUCCESS(Status))
	{
		return eprocess;
	}
    return NULL;
}

//内核链表操作
BOOLEAN GetAllProcess()
{
	PEPROCESS eproc = NULL;//用于存储当前处理的进程的EPROCESS结构体指针
	LIST_ENTRY linkListHead;//链表头，用于管理进程列表
	InitializeListHead(&linkListHead);//初始化链表头部
	ProcessList *pData= NULL;//用于存储进程信息的结构体指针
	//遍历可能的进程ID
	for(int temp=0;temp<100000;temp++)
	{
		eproc=LookupProcess((HANDLE)temp);//根据当前进程ID查找进程的EPROCESS结构体
		if (eproc != NULL)
		{
			STRING nowProcessnameString = { 0 };//用于存储当前进程名称的字符串结构体
			RtlInitString(&nowProcessnameString, PsGetProcessImageFileName(eproc));
			pData = (ProcessList*)ExAllocatePool(PagedPool, sizeof(ProcessList));//申请内存,用于存储进程信息
			RtlZeroMemory(pData, sizeof(ProcessList));
			pData->Pid=(DWORD)PsGetProcessId(eproc);//设置进程信息结构体中的进程ID
			//复制进程名到进程信息结构体中
			RtlCopyMemory(pData->ProcessName, PsGetProcessImageFileName(eproc), strlen(PsGetProcessImageFileName(eproc)) * 2);
			pData->Handle=(DWORD)PsGetProcessInheritedFromUniqueProcessId(eproc);//设置进程信息结构体中的父进程ID
			InsertTailList(&linkListHead, &pData->ListEntry);
			ObDereferenceObject(eproc);
		}
	}
	while (!IsListEmpty(&linkListHead))
	{
		LIST_ENTRY* pEntry = RemoveHeadList(&linkListHead);
        pData = CONTAINING_RECORD(pEntry, ProcessList, ListEntry);
		// 打印进程ID
		DbgPrint("%d \n", pData->Pid);
		// 打印进程名
		DbgPrint("%s \n", pData->ProcessName);
		// 打印父进程ID
		DbgPrint("%d \n", pData->Handle);
		// 释放分配的内存
		ExFreePool(pData);
	}
	return TRUE;
}

VOID UnDriver(PDRIVER_OBJECT driver)
{
	DbgPrint("UnDriver\n");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING registryPath)
{
	DbgPrint("DriverEntry\n");
	GetAllProcess();
	driver->DriverUnload = UnDriver;
    return STATUS_SUCCESS;
}