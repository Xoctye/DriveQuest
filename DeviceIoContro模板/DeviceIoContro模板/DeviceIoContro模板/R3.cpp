#include<iostream>
#include<windows.h>
#include<winioctl.h>
#define IOCTL_IO_TEST CTL_CODE(FILE_DEVICE_UNKNOWN,0X800,METHOD_BUFFERED,FILE_ANY_ACCESS)

int main()
{
	HANDLE hDevice = CreateFileA("\\\\.\\MyDriver", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hDevice);
		return 0;
	}

	DWORD input = 1,output = 2,refLen = 0;
	DeviceIoControl(hDevice, IOCTL_IO_TEST, &input, sizeof(input), &output, sizeof(output), &refLen, 0);
	CloseHandle(hDevice);
	return 0;
}