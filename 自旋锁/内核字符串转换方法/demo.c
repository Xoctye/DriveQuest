/*
�ں����ַ��������ָ�ʽ��ANSI_STRING��UNICODE_STRING

RtlInitAnsiString: ��ʼ��ANSI_STRING
RtlInitUnicodeString: ��ʼ��UNICODE_STRING
RtlUnicodeStringInit: ��ʼ��UNICODE_STRING
RtlInitString: ��ʼ��STRING
RtlUnicodeStringToInteger: UNICODE_STRINGתINT
RtlIntegerToUnicodeString: INTתUNICODE_STRING
RtlUnicodeStringToAnsiString: UNICODE_STRINGתANSI_STRING
RtlAnsiStringToUnicodeString: ANSI_STRINGתUNICODE_STRING
RtlFreeAnsiString: �ͷ�ANSI_STRING
RtlFreeUnicodeString: �ͷ�UNICODE_STRING
*/
#include<ntstrsafe.h>
#include<ntifs.h>

VOID UnDriver(PDRIVER_OBJECT driver)
{
	DbgPrint("����ж�سɹ� \n");
}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT Driver, PUNICODE_STRING RegistryPath)
{
	// �����ں��ַ���
	ANSI_STRING ansi;
	UNICODE_STRING unicode;
	UNICODE_STRING str;

	// ������ͨ�ַ���
	char* char_string = "hello lyshark";
	wchar_t* wchar_string = (WCHAR*)"hello lyshark";

	// ��ʼ���ַ����Ķ��ַ�ʽ
	RtlInitAnsiString(&ansi, char_string);
	RtlInitUnicodeString(&unicode, wchar_string);
	RtlUnicodeStringInit(&str, L"hello lyshark");

	// �ı�ԭʼ�ַ���������λ�ã��˴���������ʾ��ֵ��ʽ��
	char_string[0] = (CHAR)"A";         // char����ÿ��ռ��1�ֽ�
	char_string[1] = (CHAR)"B";

	wchar_string[0] = (WCHAR)"A";        // wchar����ÿ��ռ��2�ֽ�
	wchar_string[2] = (WCHAR)"B";

	// ����ַ��� %Z
	DbgPrint("���ANSI: %Z \n", &ansi);
	DbgPrint("���WCHAR: %Z \n", &unicode);
	DbgPrint("����ַ���: %wZ \n", &str);

	DbgPrint("�������سɹ� \n");
	Driver->DriverUnload = UnDriver;
	return STATUS_SUCCESS;
}




















