#include <windows.h>

#include <iostream>

#define MAPPED_FILE_NAME "SharedMemory"
#define BUFFER_SIZE 1024

struct SharedMemory {
    char data[BUFFER_SIZE];
    short flag;
};

int main() {
    std::string filename;
    std::cout << "Enter filename: ";
    std::getline(std::cin, filename);

    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    HANDLE hFile = CreateFile(filename.c_str(), GENERIC_READ, 0, &saAttr,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "file error" << std::endl;
        return -1;
    }

    HANDLE hMapFile =
        CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
                          sizeof(SharedMemory), MAPPED_FILE_NAME);

    if (!hMapFile) {
        std::cerr << "fail p1" << std::endl;
        return -1;
    }

    SharedMemory* pBuffer = (SharedMemory*)MapViewOfFile(
        hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);

    if (!pBuffer) {
        std::cerr << "fail p2" << std::endl;
        CloseHandle(hMapFile);
        return -1;
    }

    pBuffer->flag = 0;

    std::string szCmdline = "child.exe";
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;

    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdInput = hFile;
    siStartInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    siStartInfo.dwFlags = STARTF_USESTDHANDLES;

    if (!(CreateProcess(NULL, &szCmdline[0], NULL, NULL, TRUE, 0, NULL, NULL,
                        &siStartInfo, &piProcInfo))) {
        std::cerr << "fail p3" << std::endl;
        UnmapViewOfFile(pBuffer);
        CloseHandle(hMapFile);
        return -1;
    }

    while (pBuffer->flag != -1) {
        if (pBuffer->flag == 1) {
            std::cout << static_cast<char*>(pBuffer->data) << std::endl;
            pBuffer->flag = 0;
        }
    }

    CloseHandle(hFile);

    WaitForSingleObject(piProcInfo.hProcess, INFINITE);

    UnmapViewOfFile(pBuffer);
    CloseHandle(hMapFile);
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);

    return 0;
}