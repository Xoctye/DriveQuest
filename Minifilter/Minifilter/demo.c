//#include<fltKernel.h>
//#include<dontuse.h>
//
//PFLT_FILTER gFilterHandle;
//
//FLT_PREOP_CALLBACK_STATUS
//PreCreateOperation(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletetionContext);
//
//FLT_POSTOP_CALLBACK_STATUS
//PostCreateOperation(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID CompletionContext, FLT_POST_OPERATION_FLAGS Flags);
//
//CONST FLT_OPERATION_REGISTRATION Callbacks[] = {
//	{IRP_MJ_CREATE,0,PreCreateOperation,PostCreateOperation},{IRP_MJ_OPERATION_END}
//};
//
//CONST FLT_REGISTRATION FilterRegistration = {
//    sizeof(FLT_REGISTRATION),         // 结构大小
//    FLT_REGISTRATION_VERSION,         // 版本
//    0,                                // 标志
//    NULL,                             // 上下文
//    Callbacks,                        // 操作回调
//    Unload,                           // 卸载例程
//    NULL,                             // 实例Setup
//    NULL,                             // 实例QueryTeardown
//    NULL,                             // 实例TeardownStart
//    NULL,                             // 实例TeardownComplete
//    NULL,                             // GenerateFileName
//    NULL,                             // GenerateDestinationFileName
//    NULL                              // NormalizeNameComponent
//};
//
//NTSTATUS DriverEntry(
//    _In_ PDRIVER_DRIVER Driver,
//    _In_ PUNICODE_STRING RegistryPath
//) {
//    NTSTATUS status = FltRegisterFilter(Driver, &FilterRegistration, &gFilterHandle);
//    if (NT_SUCCESS(status))
//    {
//        status = FltStartFiltering(gFilterHandle);
//        if (!NT_SUCCESS(status))
//        {
//            FltUnregisterFilter(gFiterHandle);
//        }
//    }
//    return status;
//}
//

//#include <fltKernel.h>
//#include<ntifs.h>
//PFLT_FILTER g_Filter = NULL;
//DECLARE_CONST_UNICODE_STRING(TXT, L"txt");
//// 预操作回调函数
//FLT_PREOP_CALLBACK_STATUS PreOperationCallback(_Inout_ PFLT_CALLBACK_DATA Data,
//    _In_ PCFLT_RELATED_OBJECTS FltObjects,
//    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext)
//{
//    if (FLT_IS_IRP_OPERATION(Data)) {
//        //判断是否是删除操作
//        if (Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess & DELETE) {
//            //若是，解析Data数据，获取文件信息，判断是否是我们要保护的txt类型文件
//            PFLT_FILE_NAME_INFORMATION name_info;
//            if (NT_SUCCESS(FltGetFileNameInformation(
//                Data,
//                FLT_FILE_NAME_NORMALIZED |
//                FLT_FILE_NAME_QUERY_ALWAYS_ALLOW_CACHE_LOOKUP,
//                name_info))) {
//                //Parse file name information
//                if (NT_SUCCESS(FltParseFileNameInformation(name_info))) {
//                    if (RtlEqualUnicodeString(&name_info->Extension, &TXT, TRUE)) {
//                        FltReleaseFileNameInformation(name_info);
//                        Data->IoStatus.Status = STATUS_ACCESS_DENIED;
//                        //已处理完毕，不再往下层发送
//                        return FLT_PREOP_COMPLETE;
//                    }
//                }
//            }
//        }
//    }
//        return FLT_PREOP_SUCCESS_NO_CALLBACK;
//}
//
//// 后操作回调函数
//FLT_POSTOP_CALLBACK_STATUS PostOperationCallback(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID CompletionContext, FLT_POST_OPERATION_FLAGS Flags)
//{
//    UNREFERENCED_PARAMETER(Data);
//    UNREFERENCED_PARAMETER(FltObjects);
//    UNREFERENCED_PARAMETER(CompletionContext);
//    UNREFERENCED_PARAMETER(Flags);
//
//    return FLT_POSTOP_FINISHED_PROCESSING;
//}
//
//// 操作注册数组
//CONST FLT_OPERATION_REGISTRATION g_Operations[] = {
//    { IRP_MJ_CREATE, 0, PreOperationCallback, PostOperationCallback },
//    { IRP_MJ_OPERATION_END }
//};
//
//// 回调注册结构体
//CONST FLT_REGISTRATION g_Registration = {
//    sizeof(FLT_REGISTRATION),
//    FLT_REGISTRATION_VERSION,
//    0,
//    NULL,
//    g_Operations,
//    NULL,
//    NULL,
//    NULL,
//    NULL,
//    NULL,
//    NULL,
//    NULL,
//    NULL
//};
//
//// 驱动入口函数
//NTSTATUS DriverEntry(PDRIVER_OBJECT Driver, PUNICODE_STRING RegistryPath)
//{
//    NTSTATUS status = STATUS_SUCCESS;
//
//    // 注册过滤驱动
//    status = FltRegisterFilter(Driver, &g_Registration, &g_Filter);
//    if (!NT_SUCCESS(status)) {
//        return status;
//    }
//
//    // 启动过滤驱动
//    status = FltStartFiltering(g_Filter);
//    if (!NT_SUCCESS(status)) {
//        FltUnregisterFilter(g_Filter);
//        return status;
//    }
//
//    return status;
//}
//
//// 驱动卸载函数
//VOID DriverUnload(PDRIVER_OBJECT DriverObject)
//{
//    UNREFERENCED_PARAMETER(DriverObject);
//
//    if (g_Filter != NULL) {
//        FltUnregisterFilter(g_Filter);
//    }
//}

#include<fltKernel.h>
#include<dontuse.h>
#include<suppress.h>

NTSTATUS DriverDefaultHandle(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS status = STATUS_SUCCESS;	
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp,IO_NO_INCREMENT);
	return status;
}

VOID DriverUnload(PDRIVER_OBJECT pDriverObject)
{
	
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING pRegPath)
{
	NTSTATUS status = STATUS_SUCCESS;
	driver->DriverUnload = DriverUnload;
	for (ULONG i = 0;i < IRP_MJ_MAXIMUM_FUNCTION;i++)
	{
		driver->MajorFunction[i] = DriverDefaultHandle;
	}
	ULONG ulFilterListSize = 0;
	PFLT_FILTER* ppFilterList = NULL;
	ULONG i = 0;
	LONG lOperationOffset = 0;
	PFLT_OPERATION_REGISTRATION pFltOperationRegistration = NULL;
	FltEnumerateFilters(NULL, 0, &ulFilterListSize);
	ppFilterList = (PFLT_FILTER*)ExAllocatePool(NonPagedPool, ulFilterListSize * sizeof(PFLT_FILTER));
	__try
	{
		for (i = 0;i < ulFilterListSize;i++)
		{
			pFltOperationRegistration = (PFLT_OPERATION_REGISTRATION)ExAllocatePool(NonPagedPool, sizeof(FLT_OPERATION_REGISTRATION));
			while(IRP_MJ_OPERATION_END!=pFltOperationRegistration->MajorFunction)

		}
	}


}






