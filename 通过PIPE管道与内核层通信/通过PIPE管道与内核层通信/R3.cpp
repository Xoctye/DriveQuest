#include <iostream>
#include <windows.h>

typedef struct
{
	int type;
	unsigned long address;
	unsigned long buffer_data_len;
	char* buffer_data;
}Networkreport;

int main(int argc, char* argv[])
{
	HANDLE hPipe = CreateNamedPipe(TEXT("\\\\.\\pipe\\TEST"), PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, 0, 0, NMPWAIT_WAIT_FOREVER, NULL);
	if (INVALID_HANDLE_VALUE == hPipe)
	{
		return false;
	}

	const int size = 1024 * 10;
	char buf[size];
	DWORD rlen = 0;
	while (true)
	{
		if (1 == 1)
		{
			if (ReadFile(hPipe, buf, size, &rlen, NULL) == FALSE)
			{
				continue;
			}
			else
			{
				Networkreport* buffer_tmp = (Networkreport*)&buf;
				SIZE_T buffer_len = sizeof(Networkreport) + buffer_tmp->buffer_data_len;
				Networkreport* buffer = (Networkreport*)malloc(buffer_len);
				memcpy(buffer, buffer_tmp, buffer_len);
				char* data = (char*)malloc(buffer->buffer_data_len);
				unsigned char* tmp = (unsigned char*)buffer + sizeof(Networkreport) - 4;
				memcpy(data, tmp, buffer->buffer_data_len);
				printf("输出数据: %s \n", data);
				printf("地址: %d \n", buffer_tmp->address);
				printf("长度: %d \n", buffer_tmp->type);
				printf("输出长度: %d \n", buffer_tmp->buffer_data_len);
				free(data);
				free(buffer);
			}
		}
	}
	system("pause");
	return 0;
}
