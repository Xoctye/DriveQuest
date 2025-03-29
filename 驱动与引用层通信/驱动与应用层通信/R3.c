#include<windows.h>
#include<stdio.h>
#include<winioctl.h>

int main()
{
	HANDLE hDevice = CreateFile(L"\\\\.\\My_Device", GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		printf("%d\n",GetLastError());
	}
	UCHAR buffer[10];
	ULONG ulRead;
	ReadFile(hDevice, buffer, 10, &ulRead, 0);
	for (int i = 0;i < (int)ulRead;i++)
	{
		printf("%02X", buffer[i]);
	}
	CloseHandle(hDevice);
	return 0;
}
