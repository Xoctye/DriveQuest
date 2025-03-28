#include <ntdef.h>
#include <wdm.h>

typedef struct _DRIVER_OBJECT {
    CSHORT Type;                                // 驱动类型
    CSHORT Size;                                // 驱动大小
    PDEVICE_OBJECT DeviceObject;                // 驱动对象
    ULONG Flags;                                // 驱动的标志
    PVOID DriverStart;                          // 驱动的起始位置
    ULONG DriverSize;                           // 驱动的大小
    PVOID DriverSection;                        // 指向驱动程序映像的内存区对象
    PDRIVER_EXTENSION DriverExtension;          // 驱动的扩展空间
    UNICODE_STRING DriverName;                  // 驱动名字
    PUNICODE_STRING HardwareDatabase;
    PFAST_IO_DISPATCH FastIoDispatch;
    PDRIVER_INITIALIZE DriverInit;
    PDRIVER_STARTIO DriverStartIo;
    PDRIVER_UNLOAD DriverUnload;                 // 驱动对象的卸载地址
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

    while (pCurrentListEntry != pListEntry) //前后不相等
    {
        //获取LDR_DATA_TABLE_ENTRY结构
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


