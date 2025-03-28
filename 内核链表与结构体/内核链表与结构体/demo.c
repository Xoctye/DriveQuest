/*
��������Ҫ����һ���ṹ������ʱ����Ҫʹ��LIST_ENTRY��

PsGetProcessPeb�����ڻ�ȡ���̵Ľ��̻�����ָ�룻
PsLookupProcessByProcessId�����ݽ���ID���ҽ��̵�EPROCESS�ṹ�壻
PsGetProcessWow64Process����ȡ���̵�Wow64������Ϣ��
PsGetProcessImageFileName����ȡ���̵�ӳ���ļ���
PsGetProcessInheritedFromUniqueProcessId����ȡ���̵ĸ�����ID��
ExAllocatePool�����ڷ����ڴ棻
ExFreePool�������ͷ��ڴ棻
ObDereferenceObject�����ٶԽ��̶�������ü�����
RtlInitString����ʼ��֧�����ṹ�壻
RtlCopyMemory�������ڴ�飻
RtlZeroMemory����ָ���ڴ�����㣻
InitializeListHead����ʼ������ͷ����
InsertTailList���ѽڵ��������β����
RemoveHeadList��������ͷ���Ƴ�һ���ڵ㣻
*/
//ö���û����̣���ö�ٵ��Ľ��̴洢������ṹ����
#include<ntifs.h>
#include<windef.h>
extern PVOID PsGetProcessPeb(PEPROCESS Process);
NTKERNELAPI NTSTATUS PsLooukupProcessByProcessId(HANDLE ProcessId, PEPROCESS* Process);
extern NTKERNELAPI PVOID PsGetProcessWow64Process(PEPROCESS Process);
extern NTKERNELAPI UCHAR* PsGetProcessImageFileName(PEPROCESS Process);
extern NTKERNELAPI HANDLE PsGetProcessInheritedFromUniqueProcessId(PEPROCESS Process);//��ȡ���̵ĸ�����ID

//��������б�ṹ��
typedef struct
{
	DWORD Pid;
	UCHAR ProcessName[2048];
	DWORD Handle; //������ID
	LIST_ENTRY ListEntry;//���ڽ��ýṹ����뵽������
}ProcessList;

//���ݽ���ID���ؽ���EPROCESS�ṹ��
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

//�ں��������
BOOLEAN GetAllProcess()
{
	PEPROCESS eproc = NULL;//���ڴ洢��ǰ����Ľ��̵�EPROCESS�ṹ��ָ��
	LIST_ENTRY linkListHead;//����ͷ�����ڹ�������б�
	InitializeListHead(&linkListHead);//��ʼ������ͷ��
	ProcessList *pData= NULL;//���ڴ洢������Ϣ�Ľṹ��ָ��
	//�������ܵĽ���ID
	for(int temp=0;temp<100000;temp++)
	{
		eproc=LookupProcess((HANDLE)temp);//���ݵ�ǰ����ID���ҽ��̵�EPROCESS�ṹ��
		if (eproc != NULL)
		{
			STRING nowProcessnameString = { 0 };//���ڴ洢��ǰ�������Ƶ��ַ����ṹ��
			RtlInitString(&nowProcessnameString, PsGetProcessImageFileName(eproc));
			pData = (ProcessList*)ExAllocatePool(PagedPool, sizeof(ProcessList));//�����ڴ�,���ڴ洢������Ϣ
			RtlZeroMemory(pData, sizeof(ProcessList));
			pData->Pid=(DWORD)PsGetProcessId(eproc);//���ý�����Ϣ�ṹ���еĽ���ID
			//���ƽ�������������Ϣ�ṹ����
			RtlCopyMemory(pData->ProcessName, PsGetProcessImageFileName(eproc), strlen(PsGetProcessImageFileName(eproc)) * 2);
			pData->Handle=(DWORD)PsGetProcessInheritedFromUniqueProcessId(eproc);//���ý�����Ϣ�ṹ���еĸ�����ID
			InsertTailList(&linkListHead, &pData->ListEntry);
			ObDereferenceObject(eproc);
		}
	}
	while (!IsListEmpty(&linkListHead))
	{
		LIST_ENTRY* pEntry = RemoveHeadList(&linkListHead);
        pData = CONTAINING_RECORD(pEntry, ProcessList, ListEntry);
		// ��ӡ����ID
		DbgPrint("%d \n", pData->Pid);
		// ��ӡ������
		DbgPrint("%s \n", pData->ProcessName);
		// ��ӡ������ID
		DbgPrint("%d \n", pData->Handle);
		// �ͷŷ�����ڴ�
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