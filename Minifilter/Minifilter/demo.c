#include <fltKernel.h>
#include<ntifs.h>
PFLT_FILTER g_Filter = NULL;
DECLARE_CONST_UNICODE_STRING(TXT, L"txt");
// Ԥ�����ص�����
FLT_PREOP_CALLBACK_STATUS PreOperationCallback(_Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext)
{
    if (FLT_IS_IRP_OPERATION(Data)) {
        //�ж��Ƿ���ɾ������
        if (Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess & DELETE) {
            //���ǣ�����Data���ݣ���ȡ�ļ���Ϣ���ж��Ƿ�������Ҫ������txt�����ļ�
            PFLT_FILE_NAME_INFORMATION name_info;
            if (NT_SUCCESS(FltGetFileNameInformation(
                Data,
                FLT_FILE_NAME_NORMALIZED |
                FLT_FILE_NAME_QUERY_ALWAYS_ALLOW_CACHE_LOOKUP,
                name_info))) {
                //Parse file name information
                if (NT_SUCCESS(FltParseFileNameInformation(name_info))) {
                    if (RtlEqualUnicodeString(&name_info->Extension, &TXT, TRUE)) {
                        FltReleaseFileNameInformation(name_info);
                        Data->IoStatus.Status = STATUS_ACCESS_DENIED;
                        //�Ѵ�����ϣ��������²㷢��
                        return FLT_PREOP_COMPLETE;
                    }
                }
            }
        }
    }
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

// ������ص�����
FLT_POSTOP_CALLBACK_STATUS PostOperationCallback(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID CompletionContext, FLT_POST_OPERATION_FLAGS Flags)
{
    UNREFERENCED_PARAMETER(Data);
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);
    UNREFERENCED_PARAMETER(Flags);

    return FLT_POSTOP_FINISHED_PROCESSING;
}

// ����ע������
CONST FLT_OPERATION_REGISTRATION g_Operations[] = {
    { IRP_MJ_CREATE, 0, PreOperationCallback, PostOperationCallback },
    { IRP_MJ_OPERATION_END }
};

// �ص�ע��ṹ��
CONST FLT_REGISTRATION g_Registration = {
    sizeof(FLT_REGISTRATION),
    FLT_REGISTRATION_VERSION,
    0,
    NULL,
    g_Operations,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

// ������ں���
NTSTATUS DriverEntry(PDRIVER_OBJECT Driver, PUNICODE_STRING RegistryPath)
{
    NTSTATUS status = STATUS_SUCCESS;

    // ע���������
    status = FltRegisterFilter(Driver, &g_Registration, &g_Filter);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    // ������������
    status = FltStartFiltering(g_Filter);
    if (!NT_SUCCESS(status)) {
        FltUnregisterFilter(g_Filter);
        return status;
    }

    return status;
}

// ����ж�غ���
VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);

    if (g_Filter != NULL) {
        FltUnregisterFilter(g_Filter);
    }
}