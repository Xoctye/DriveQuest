/*
InitializeListHead����ʼ������ͷ
KeInitializeSpinLock����ʼ��������
KeAcquireSpinLock����ȡ������
KeReleaseSpinLock���ͷ�������
ExAllocatePool�������ڴ�
ExInterlockedInsertHeadList����������ͷ
ExInterlockedInsertTailList����������β
ExInterlockedRemoveHeadList���Ƴ�����ͷ
CONTAINING_RECORD�����ݽṹ���Ա��ַ��ȡ�ṹ���׵�ַ
*/
#include<ntifs.h>
#include<ntstrsafe.h>

typedef struct _LIST_ENTRY
{
	struct _LIST_ENTRY* Flink;
	struct _LIST_ENTRY* Blink;
}LIST_ENTRY,*PLIST_ENTRY;

typedef struct _MyStruct
{
	ULONG X;
	ULONG y;
	LIST_ENTRY lpListEntry;
}MyStruct, * PMyStruct;

LIST_ENTRY my_list_header;
LIST_ENTRY my_list_lock;

void Init()
{
	InitializeListHead(&my_list_header);
	KeInitializeSpinLock(&my_list_lock);
}

VOID function_ins()
{
	KIRQL Irql;
	KeAcquireSpinLock(&my_list_lock, &Irql);
	KeReleaseSpinLock(&my_list_lock, Irql);
}

VOID UnDriver(PDRIVER_OBJECT driver)
{
	DbgPrint("UnDriver");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT Driver, PUNICODE_STRING RegistryPath)
{
	Init();
	PMyStruct testA = (PMyStruct)ExAllocatePool(NonPagedPoolExecute, sizeof(PMyStruct));
	PMyStruct testB = (PMyStruct)ExAllocatePool(NonPagedPoolExecute, sizeof(PMyStruct));
	testA->X = 100;
	testA->y = 200;
	testB->X = 300;
	testB->y = 400;
	if (NULL != testA && NULL != testB)
	{
		ExInterlockedInsertHeadList(&my_list_header, (PLIST_ENTRY)&testA->lpListEntry, &my_list_lock);
		ExInterlockedInsertTailList(&my_list_header, (PLIST_ENTRY)&testB->lpListEntry, &my_list_lock);
	}
	function_ins();
	PLIST_ENTRY remove_entry = ExInterlockedRemoveHeadList(&testA->lpListEntry, &my_list_lock);

	// �����������
	while (remove_entry != &my_list_header)
	{
		// �������Ա����ṹ�嶥���ڴ����
		PMyStruct ptr = CONTAINING_RECORD(remove_entry, MyStruct, lpListEntry);
		DbgPrint("�ڵ�Ԫ��X = %d �ڵ�Ԫ��Y = %d \n", ptr->X, ptr->y);

		// �õ���һ��Ԫ�ص�ַ
		remove_entry = remove_entry->Flink;
	}

	Driver->DriverUnload = UnDriver;
	return STATUS_SUCCESS;
}