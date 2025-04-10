#include <windows.h>
#include <iostream>
#include <Psapi.h>
#include <memory>
#include <string>

#pragma comment(lib,"psapi.lib")

// 定义一个结构体 ThreadData，用于存储线程的相关信息
struct ThreadData {
    ULONG ThreadId;
    ULONG Create_ProcessId;
    ULONG Belong_ProcessId;
};

// 根据进程 ID 获取进程的可执行文件路径
std::string GetPathByProcessId(DWORD dwPid) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
    if (hProcess == NULL) {
        return "";
    }
    std::unique_ptr<char[]> path = std::make_unique<char[]>(MAX_PATH);
    if (GetModuleFileNameExA(hProcess, NULL, path.get(), MAX_PATH) == 0) {
        CloseHandle(hProcess);
        return "";
    }
    CloseHandle(hProcess);
    return path.get();
}

int main() {
    // 打开一个名为 "\\\\.\\RemoteThreadCheck" 的设备对象，以只读模式打开
    HANDLE hFile = CreateFile(L"\\\\.\\RemoteThreadCheck", GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    // 如果打开设备对象失败，输出错误信息并返回 0
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cout << "设备对象打开失败" << std::endl;
        return 0;
    }
    // 进入一个无限循环，持续读取设备对象中的数据
    while (true) {
        std::unique_ptr<BYTE[]> buffer = std::make_unique<BYTE[]>(1024 * 200);
        DWORD bytes = 0;
        // 从设备对象中读取数据
        if (!ReadFile(hFile, buffer.get(), 1024, &bytes, nullptr)) {
            std::cout << "读取文件失败" << std::endl;
            break;
        }
        // 当读取到的数据字节数大于 0 时，处理这些数据
        for (size_t i = 0; i < bytes; i += sizeof(ThreadData)) {
            ThreadData* item = reinterpret_cast<ThreadData*>(buffer.get() + i);
            // 输出线程的相关信息
            std::cout << "ThreadId:" << item->ThreadId << ", BelongPid:" << item->Belong_ProcessId << ", CreatePid:" << item->Create_ProcessId << std::endl;
            std::string process1 = GetPathByProcessId(item->Create_ProcessId);
            std::string process2 = GetPathByProcessId(item->Belong_ProcessId);
            // 如果两个进程的路径都获取成功且不为空
            if (!process1.empty() && !process2.empty()) {
                std::string warningText = "进程:\n " + process1 + " \n在向进程:\n " + process2 + " \n进行远程线程注入, 注入的线程ID为: " + std::to_string(item->ThreadId);
                // 弹出一个消息框显示警告信息
                MessageBoxA(NULL, warningText.c_str(), "警告", MB_OK);
            }
        }
        // 线程休眠 1 秒
        Sleep(1000);
    }
    // 关闭设备对象句柄
    CloseHandle(hFile);
    return 0;
}