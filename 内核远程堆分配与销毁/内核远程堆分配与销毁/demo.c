#include<ntifs.h>
#include<windef.h>

NTSTATUS ZwAllocateVirtualMemory(HANDLE ProcessHandle, PVOID* BaseAddress, ULONG_PTR ZeroBits, PSIZE_T RegionSize, ULONG AllocationType, ULONG Protect);
NTSTATUS AllocMemory(ULONG ProcessPid, SIZE_T Length, PVOID Buffer)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PEPROCESS pEProcess = NULL;
	KAPC_STATE ApcState = { 0 };
	PVOID BaseAddress = NULL;
	Status = PsLookupProcessByProcessId((HANDLE)ProcessPid, &pEProcess);
	if (NT_SUCCESS(Status) && !MmIsAddressValid(pEProcess))
	{
		return STATUS_UNSUCCESSFUL;
	}
	if (!MmIsAddressValid(pEProcess))
	{
		return STATUS_UNSUCCESSFUL;
	}
	__try
	{
		KeStackAttachProcess(pEProcess, &ApcState);
		Status = ZwAllocateVirtualMemory(NtCurrentProcess(), &BaseAddress, 0, &Length, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		RtlZeroMemory(BaseAddress, Length);
		*(PVOID*)Buffer = BaseAddress;
		KeUnstackDetachProcess(&ApcState);
		Status = STATUS_UNSUCCESSFUL;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		KeUnstackDetachProcess(&ApcState);
		Status = STATUS_UNSUCCESSFUL;
	}
	ObDereferenceObject(pEProcess);
	return Status;
}

VOID UnDriver(PDRIVER_OBJECT driver)
{
	DbgPrint("Unloading Driver\n");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING RegisterPath)
{
	DWORD process_id = 4160;
	DWORD create_size = 1024;
	DWORD ref_address = 0;

	NTSTATUS Status = AllocMemory(process_id, create_size, &ref_address);
	driver->DriverUnload = UnDriver;
	return STATUS_SUCCESS;
}

