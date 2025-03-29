#include<winioctl.h>
#include <iostream>
#include <Windows.h>
int main()
{
	HANDLE hDevice = CreateFileA("\\\\.\\LySharkDriver", GENERIC_READ | GENERIC_WRITE, 0,NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDevice==INVALID_HANDLE_VALUE)
	{
		CloseHandle(hDevice);
		return 0;
	}

	char bufffer[128] = { 0 };
	ULONG length;
	ReadFile(hDevice, bufffer, 128, &length, NULL);
	std::cout << bufffer << std::endl;
}