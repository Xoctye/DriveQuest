#include<ntifs.h>
#include<windef.h>

NTKERNELAPI PUCHAR PsGetProcessImageFileName(PEPROCESS pEProcess);
NTKERNELAPI NTSTATUS ObQueryNameString(
    _In_ PVOID Object,
    _Out_writes_bytes_opt_(Length) POBJECT_NAME_INFORMATION ObjectNameInfo,
    _In_ ULONG Length,
    _Out_ PULONG ReturnLength
);

LARGE_INTEGER g_liRegCookie;

BOOLEAN fnGetFullPath(PUNICODE_STRING pRegistryPath, PVOID pRegistryObject)
{
    if ((MmIsAddressValid(pRegistryObject) == FALSE) || (pRegistryObject == NULL))
    {
        return FALSE;
    }
    ULONG ulSize = 512;
    PVOID lpObjectNameInfo = ExAllocatePool(NonPagedPool, ulSize);
    if (lpObjectNameInfo == NULL)
    {
        return FALSE;
    }
    ULONG ulRetLen = 0;
    NTSTATUS status = ObQueryNameString(pRegistryObject, (POBJECT_NAME_INFORMATION)lpObjectNameInfo, ulSize, &ulRetLen);
    if (!NT_SUCCESS(status))
    {
        ExFreePool(lpObjectNameInfo);
        return FALSE;
    }
    RtlCopyUnicodeString(pRegistryPath, (PUNICODE_STRING)lpObjectNameInfo);
    ExFreePool(lpObjectNameInfo);
    return TRUE;
}

NTSTATUS fnCallback(_In_ PVOID CallbackContext, _In_opt_ PVOID Argument1, _In_opt_ PVOID Argument2)
{
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING ustrRegPath;
    LONG lOperateType = (REG_NOTIFY_CLASS)Argument1;
    ustrRegPath.Length = 0;
    ustrRegPath.MaximumLength = 1024 * sizeof(WCHAR);
    ustrRegPath.Buffer = ExAllocatePool(NonPagedPool, ustrRegPath.MaximumLength);
    if (NULL == ustrRegPath.Buffer)
    {
        return status;
    }
    RtlZeroMemory(ustrRegPath.Buffer, ustrRegPath.MaximumLength);
    switch (lOperateType)
    {
    case RegNtPreCreateKey:
    {
        fnGetFullPath(&ustrRegPath, ((PREG_CREATE_KEY_INFORMATION)Argument2)->RootObject);
        DbgPrint("[����ע���][%wZ][%wZ]\n", &ustrRegPath, ((PREG_CREATE_KEY_INFORMATION)Argument2)->CompleteName);
        break;
    }
    case RegNtPreOpenKey:
    {
        fnGetFullPath(&ustrRegPath, ((PREG_CREATE_KEY_INFORMATION)Argument2)->RootObject);
        // ��ӡ��ע������Ϣ
        DbgPrint("[��ע���][%wZ][%wZ]\n", &ustrRegPath, ((PREG_CREATE_KEY_INFORMATION)Argument2)->CompleteName);
        break;
    }
    case RegNtPreDeleteValueKey:
    {
        fnGetFullPath(&ustrRegPath, ((PREG_DELETE_VALUE_KEY_INFORMATION)Argument2)->Object);
        // ��ӡɾ��ע����ֵ����Ϣ
        DbgPrint("[ɾ����ֵ][%wZ][%wZ] \n", &ustrRegPath, ((PREG_DELETE_VALUE_KEY_INFORMATION)Argument2)->ValueName);
        PEPROCESS pEProcess = PsGetCurrentProcess();
        if (pEProcess != NULL)
        {
            UCHAR* lpszProcessName=PsGetProcessImageFileName(pEProcess);
            if (lpszProcessName != NULL)
            {
                DbgPrint("���� [%s] ɾ���˼�ֵ�� \n", lpszProcessName);
            }
        }
        break;
    }
    case RegNtPreSetValueKey:
    {
        // ��ȡҪ�޸ĵ�ע����ֵ������·��
        fnGetFullPath(&ustrRegPath, ((PREG_SET_VALUE_KEY_INFORMATION)Argument2)->Object);
        // ��ӡ�޸�ע����ֵ����Ϣ
        DbgPrint("[�޸ļ�ֵ][%wZ][%wZ] \n", &ustrRegPath, ((PREG_SET_VALUE_KEY_INFORMATION)Argument2)->ValueName);
        break;
    }
    default:
        break;
    }

    // �ͷŷ�����ڴ�
    if (NULL != ustrRegPath.Buffer)
    {
        ExFreePool(ustrRegPath.Buffer);
        ustrRegPath.Buffer = NULL;
    }
    return status;
}

VOID UnDriver(PDRIVER_OBJECT driver)
{
    DbgPrint(("Uninstall Driver Is OK \n"));
    if (g_liRegCookie.QuadPart > 0)
    {
        CmUnRegisterCallback(g_liRegCookie);
    }
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING registryPath)
{
    NTSTATUS status = CmRegisterCallback(fnCallback, NULL, &g_liRegCookie);
    if (!NT_SUCCESS(status))
    {
        g_liRegCookie.QuadPart = 0;
        return status;
    }
    driver = UnDriver;
    return STATUS_SUCCESS;
}



