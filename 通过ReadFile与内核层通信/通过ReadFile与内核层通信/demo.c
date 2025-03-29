#include<ntifs.h>
NTSTATUS DispatchRead(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION Stack = IoGetCurrentIrpStackLocation(pIrp);
	ULONG ulReadLength = Stack->Parameters.Read.Length;

	char szBuf[128] = "test";
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = ulReadLength;
	memcpy(pIrp->AssociatedIrp.SystemBuffer, szBuf, ulReadLength);
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS DispatchWrite(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION Stack = IoGetCurrentIrpStackLocation(pIrp);
	ULONG ulWriteLength = Stack->Parameters.Write.Length;
	char szBuf[128] = { 0 };
	memcpy(szBuf, pIrp->AssociatedIrp.SystemBuffer, ulWriteLength);
	DbgPrint("Write Data: %s\n", szBuf);
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = ulWriteLength;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}