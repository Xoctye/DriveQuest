#define _CRT_SECURE_NO_WARNINGS
#include<windows.h>
#include<iostream>
#include<inttypes.h>
#include<capstone/capstone.h>

#pragma comment(lib,"capstone.lib")

#define READ_PROCESS_CODE CTL_CODE(FILE_DEVICE_UNKNOWN,0x800,METHOD_BUFFERED,FILE_ALL_ACCESS)
#define WRITE_PROCESS_CODE CTL_CODE(FILE_DEVICE_UNKNOWN,0x801,METHOD_BUFFERED,FILE_ALL_ACCESS)

typedef struct
{
	DWORD pid;
	UINT64 address;
	DWORD size;
	BYTE* data;
}ProcessData;

int main()
{
	HANDLE handle = CreateFileA("\\??\\READWRITESYMBOLNAME", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	ProcessData data;
	DWORD dwSize = 0;

	// 指定需要读写的进程
	data.pid = 1234;
	data.address = 0x401000;
	data.size = 64;

	data.data = new BYTE[data.size];
	DeviceIoControl(handle, READ_PROCESS_CODE, &data, sizeof(data), &data, sizeof(data), &dwSize, NULL);
	for (int i = 0; i < data.size; i++)
	{
		printf("0x%02X ", data.data[i]);
	}

	printf("\n");

	csh dasm_handle;
	cs_insn* insn;
	size_t count;

	if (cs_open(CS_ARCH_X86, CS_MODE_32, &dasm_hanle) != CS_ERR_OK)
	{
		return 0;
	}
	count = cs_disasm(dasm_handle, (UCHAR*)data.data, data.size, data.address, 0, &insn);
	if (count > 0)
	{
		size_t index;
		for (index = 0; index < count; index++)
		{
			/*
			for (int x = 0; x < insn[index].size; x++)
			{
				printf("机器码: %d -> %02X \n", x, insn[index].bytes[x]);
			}
			*/

			printf("地址: 0x%"PRIx64" | 长度: %d 反汇编: %s %s \n", insn[index].address, insn[index].size, insn[index].mnemonic, insn[index].op_str);
		}
		cs_free(insn, count);
	}
	cs_close(&dasm_handle);

	getchar();
	CloseHandle(handle);
	return 0;
}












