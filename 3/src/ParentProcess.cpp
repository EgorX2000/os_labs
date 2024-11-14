#include <windows.h>

#include <iostream>

HANDLE hFile = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

int main() {
    std::string filename;
    std::cout << "Enter filename: ";
    std::getline(std::cin, filename);

    const char* mappingName = "SharedMemoryMapping";
    const size_t bufferSize = 1024;

    HANDLE hMapFile = CreateFileMappingA(
        INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, bufferSize, mappingName);

    if (hMapFile == NULL) {
        std::cerr << "Не удалось создать отображение файла." << std::endl;
        return 1;
    }

    LPVOID pBuffer = MapViewOfFile(hMapFile, FILE_MAP_WRITE, 0, 0, bufferSize);

    if (pBuffer == NULL) {
        std::cerr << "Не удалось отобразить файл в память." << std::endl;
        CloseHandle(hMapFile);
        return 1;
    }

    memcpy(pBuffer, filename.c_str(), filename.size() + 1);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    std::string cmd = "child.exe " + std::string(mappingName);

    if (!CreateProcessA(NULL, const_cast<char*>(cmd.c_str()), NULL, NULL, FALSE,
                        0, NULL, NULL, &si, &pi)) {
        std::cerr << "Ошибка при создании дочернего процесса." << std::endl;
        UnmapViewOfFile(pBuffer);
        CloseHandle(hMapFile);
        return 1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    UnmapViewOfFile(pBuffer);
    CloseHandle(hMapFile);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}
/*
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    HANDLE hFile = CreateFile(filename.c_str(), GENERIC_READ, 0, &saAttr,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        return -1;
    }

    if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0)) {
        return -1;
    }

    if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) {
        return -1;
    }

    TCHAR szCmdline[] = TEXT("child.exe");
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;

    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdInput = hFile;
    siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
    siStartInfo.hStdError = g_hChildStd_OUT_Wr;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    if (!(CreateProcess(NULL, szCmdline, NULL, NULL, TRUE, 0, NULL, NULL,
                        &siStartInfo, &piProcInfo))) {
        return -1;
    }

    CloseHandle(g_hChildStd_OUT_Wr);
    CloseHandle(hFile);

    DWORD dwWritten, dwRead;
    int result;
    while (ReadFile(g_hChildStd_OUT_Rd, &result, sizeof(int), &dwRead, NULL)) {
        std::cout << result << std::endl;
    }

    WaitForSingleObject(piProcInfo.hProcess, INFINITE);

    CloseHandle(g_hChildStd_OUT_Rd);
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);

    return 0;
}
*/