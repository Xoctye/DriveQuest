/*
InitializeListHead：初始化链表头
KeInitializeSpinLock：初始化自旋锁
KeAcquireSpinLock：获取自旋锁
KeReleaseSpinLock：释放自旋锁
ExAllocatePool：分配内存
ExInterlockedInsertHeadList：插入链表头
ExInterlockedInsertTailList：插入链表尾
ExInterlockedRemoveHeadList：移除链表头
CONTAINING_RECORD：根据结构体成员地址获取结构体首地址
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

	// 输出链表数据
	while (remove_entry != &my_list_header)
	{
		// 计算出成员距离结构体顶部内存距离
		PMyStruct ptr = CONTAINING_RECORD(remove_entry, MyStruct, lpListEntry);
		DbgPrint("节点元素X = %d 节点元素Y = %d \n", ptr->X, ptr->y);

		// 得到下一个元素地址
		remove_entry = remove_entry->Flink;
	}

	Driver->DriverUnload = UnDriver;
	return STATUS_SUCCESS;
}