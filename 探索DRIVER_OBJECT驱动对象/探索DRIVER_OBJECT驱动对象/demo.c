#include <ntdef.h>
#include <wdm.h>

typedef struct _DRIVER_OBJECT {
    CSHORT Type;                                // ��������
    CSHORT Size;                                // ������С
    PDEVICE_OBJECT DeviceObject;                // ��������
    ULONG Flags;                                // �����ı�־
    PVOID DriverStart;                          // ��������ʼλ��
    ULONG DriverSize;                           // �����Ĵ�С
    PVOID DriverSection;                        // ָ����������ӳ����ڴ�������
    PDRIVER_EXTENSION DriverExtension;          // ��������չ�ռ�
    UNICODE_STRING DriverName;                  // ��������
    PUNICODE_STRING HardwareDatabase;
    PFAST_IO_DISPATCH FastIoDispatch;
    PDRIVER_INITIALIZE DriverInit;
    PDRIVER_STARTIO DriverStartIo;
    PDRIVER_UNLOAD DriverUnload;                 // ���������ж�ص�ַ
    PDRIVER_DISPATCH MajorFunction[IRP_MJ_MAXIMUM_FUNCTION + 1];
} DRIVER_OBJECT;

typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG Flags;
    USHORT LoadCount;
    USHORT TlsIndex;
    union {
        LIST_ENTRY HashLinks;
        struct {
            PVOID SectionPointer;
            ULONG CheckSum;
        };
    };
    union {
        struct {
            ULONG TimeDateStamp;
        };
        struct {
            PVOID LoadedImports;
        };
    };
}LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

VOID DriverUnLoad(PDRIVER_OBJECT pDriverObj)
{
	DbgPrint("Driver UnLoad \r\n");
}
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObj, PUNICODE_STRING pRegPath)
{
    ULONG iCount = 0;
    NTSTATUS ntStatus;
    pDriverObj->DriverUnload = DriverUnLoad;
    KdBreakPoint();
    PLDR_DATA_TABLE_ENTRY pLdr = NULL;
    PLIST_ENTRY pListEntry = NULL;
    PLIST_ENTRY pCurrentListEntry = NULL;

    PLDR_DATA_TABLE_ENTRY pCurrentModule = NULL;
    pLdr = (PLDR_DATA_TABLE_ENTRY)pDriverObj->DriverSection;
    pListEntry = pLdr->InLoadOrderLinks.Flink;
    pCurrentListEntry = pListEntry->Flink;

    while (pCurrentListEntry != pListEntry) //ǰ�����
    {
        //��ȡLDR_DATA_TABLE_ENTRY�ṹ
        pCurrentModule = CONTAINING_RECORD(pCurrentListEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

        if (pCurrentModule->BaseDllName.Buffer != 0)
        {
            DbgPrint("ModuleName = %wZ ModuleBase = %p \r\n",
                pCurrentModule->BaseDllName,
                pCurrentModule->DllBase);
        }
        pCurrentListEntry = pCurrentListEntry->Flink;
    }
    return STATUS_SUCCESS;
}


