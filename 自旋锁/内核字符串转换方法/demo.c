/*
内核中字符串有两种格式：ANSI_STRING、UNICODE_STRING

RtlInitAnsiString: 初始化ANSI_STRING
RtlInitUnicodeString: 初始化UNICODE_STRING
RtlUnicodeStringInit: 初始化UNICODE_STRING
RtlInitString: 初始化STRING
RtlUnicodeStringToInteger: UNICODE_STRING转INT
RtlIntegerToUnicodeString: INT转UNICODE_STRING
RtlUnicodeStringToAnsiString: UNICODE_STRING转ANSI_STRING
RtlAnsiStringToUnicodeString: ANSI_STRING转UNICODE_STRING
RtlFreeAnsiString: 释放ANSI_STRING
RtlFreeUnicodeString: 释放UNICODE_STRING
*/
#include<ntstrsafe.h>
#include<ntifs.h>

VOID UnDriver(PDRIVER_OBJECT driver)
{
	DbgPrint("驱动卸载成功 \n");
}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT Driver, PUNICODE_STRING RegistryPath)
{
	// 定义内核字符串
	ANSI_STRING ansi;
	UNICODE_STRING unicode;
	UNICODE_STRING str;

	// 定义普通字符串
	char* char_string = "hello lyshark";
	wchar_t* wchar_string = (WCHAR*)"hello lyshark";

	// 初始化字符串的多种方式
	RtlInitAnsiString(&ansi, char_string);
	RtlInitUnicodeString(&unicode, wchar_string);
	RtlUnicodeStringInit(&str, L"hello lyshark");

	// 改变原始字符串（乱码位置，此处仅用于演示赋值方式）
	char_string[0] = (CHAR)"A";         // char类型每个占用1字节
	char_string[1] = (CHAR)"B";

	wchar_string[0] = (WCHAR)"A";        // wchar类型每个占用2字节
	wchar_string[2] = (WCHAR)"B";

	// 输出字符串 %Z
	DbgPrint("输出ANSI: %Z \n", &ansi);
	DbgPrint("输出WCHAR: %Z \n", &unicode);
	DbgPrint("输出字符串: %wZ \n", &str);

	DbgPrint("驱动加载成功 \n");
	Driver->DriverUnload = UnDriver;
	return STATUS_SUCCESS;
}




















