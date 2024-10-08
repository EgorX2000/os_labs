#include <windows.h>

#include <iostream>

HANDLE hFile = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

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