#include <windows.h>

#include <iostream>

#define MAPPED_FILE_NAME "SharedMemory"
#define BUFFER_SIZE 1024
#define EXIT_CHILD            \
    pBuffer->exit = true;     \
    UnmapViewOfFile(pBuffer); \
    CloseHandle(hMapFile);    \
    exit(1);

struct SharedMemory {
    char data[BUFFER_SIZE];
    bool flag;
    bool exit;
};

int main(int argc, char* argv[]) {
    HANDLE hMapFile =
        OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, MAPPED_FILE_NAME);

    if (!hMapFile) {
        std::cerr << "fail c1" << std::endl;
        return -1;
    }

    SharedMemory* pBuffer = (SharedMemory*)MapViewOfFile(
        hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);

    if (!pBuffer) {
        std::cerr << "fail c2" << std::endl;
        CloseHandle(hMapFile);
        return -1;
    }

    HANDLE readHandle = GetStdHandle(STD_INPUT_HANDLE);
    DWORD readBytes;

    char buffer[1];
    std::string written;

    while (1) {
        int res = 0;
        while (1) {
            if (ReadFile(readHandle, buffer, 1, &readBytes, NULL)) {
                if (readBytes == 0) {
                    EXIT_CHILD;
                }

                buffer[readBytes] = 0;

                if ('0' <= buffer[0] && buffer[0] <= '9') {
                    res = res * 10 + (buffer[0] - '0');
                } else if (buffer[0] == ' ') {
                    break;
                } else if (buffer[0] == '\r') {
                    if (ReadFile(readHandle, buffer, 1, &readBytes, NULL)) {
                        if (readBytes == 0) {
                            EXIT_CHILD;
                        }

                        if (buffer[0] == '\n') {
                            while (pBuffer->flag == true) {
                                continue;
                            }
                            written = std::to_string(res);
                            memcpy(pBuffer->data, written.c_str(),
                                   (written.size() + 1) * sizeof(char));
                            pBuffer->flag = true;

                            break;
                        } else {
                            EXIT_CHILD;
                        }
                    } else {
                        EXIT_CHILD;
                    }
                } else {
                    EXIT_CHILD;
                }
            } else {
                EXIT_CHILD;
            }
        }

        int div = 0;
        if (buffer[0] == ' ') {
            while (1) {
                if (ReadFile(readHandle, buffer, 1, &readBytes, NULL)) {
                    if (readBytes == 0) {
                        EXIT_CHILD;
                    }

                    buffer[readBytes] = 0;

                    if ('0' <= buffer[0] && buffer[0] <= '9') {
                        div = div * 10 + (buffer[0] - '0');
                    } else if (buffer[0] == ' ') {
                        if (div == 0) {
                            EXIT_CHILD;
                        }
                        res /= div;
                        div = 0;
                    } else if (buffer[0] == '\r') {
                        if (ReadFile(readHandle, buffer, 1, &readBytes, NULL)) {
                            if (readBytes == 0) {
                                EXIT_CHILD;
                            }

                            if (buffer[0] == '\n') {
                                if (div == 0) {
                                    EXIT_CHILD;
                                }
                                res /= div;
                                div = 0;

                                while (pBuffer->flag == true) {
                                    continue;
                                }
                                written = std::to_string(res);
                                memcpy(pBuffer->data, written.c_str(),
                                       (written.size() + 1) * sizeof(char));
                                pBuffer->flag = true;

                                break;
                            } else {
                                EXIT_CHILD;
                            }
                        } else {
                            EXIT_CHILD;
                        }
                    } else {
                        EXIT_CHILD;
                    }
                } else {
                    EXIT_CHILD;
                }
            }
        }
    }

    pBuffer->exit = true;
    UnmapViewOfFile(pBuffer);
    CloseHandle(hMapFile);

    return 0;
}
