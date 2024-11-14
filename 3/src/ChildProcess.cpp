#include <windows.h>

#include <fstream>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Не указано имя отображаемой памяти." << std::endl;
        return 1;
    }

    const char* mappingName = argv[1];

    HANDLE hMapFile = OpenFileMappingA(FILE_MAP_READ, FALSE, mappingName);

    if (hMapFile == NULL) {
        std::cerr << "Не удалось открыть отображение файла." << std::endl;
        return 1;
    }

    LPVOID pBuffer = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);

    if (pBuffer == NULL) {
        std::cerr << "Не удалось отобразить файл в память." << std::endl;
        CloseHandle(hMapFile);
        return 1;
    }

    const char* filename = static_cast<const char*>(pBuffer);

    HANDLE hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Не удалось открыть файл: " << filename << std::endl;
        UnmapViewOfFile(pBuffer);
        CloseHandle(hMapFile);
        return 1;
    }

    DWORD readBytes, writtenBytes;
    char buffer[1];
    while (1) {
        int res = 0;
        while (1) {
            if (ReadFile(readHandle, buffer, 1, &readBytes, NULL)) {
                if (readBytes == 0) {
                    exit(1);
                }

                buffer[readBytes] = 0;

                if ('0' <= buffer[0] && buffer[0] <= '9') {
                    res = res * 10 + (buffer[0] - '0');
                } else if (buffer[0] == ' ') {
                    break;
                } else if (buffer[0] == '\r') {
                    if (ReadFile(readHandle, buffer, 1, &readBytes, NULL)) {
                        if (readBytes == 0) {
                            exit(1);
                        }

                        if (buffer[0] == '\n') {
                            WriteFile(writeHandle, &res, sizeof(int),
                                      &writtenBytes, NULL);

                            break;
                        } else {
                            exit(1);
                        }
                    } else {
                        exit(1);
                    }
                } else {
                    exit(1);
                }
            } else {
                exit(1);
            }
        }

        int div = 0;
        if (buffer[0] == ' ') {
            while (1) {
                if (ReadFile(readHandle, buffer, 1, &readBytes, NULL)) {
                    if (readBytes == 0) {
                        exit(1);
                    }

                    buffer[readBytes] = 0;

                    if ('0' <= buffer[0] && buffer[0] <= '9') {
                        div = div * 10 + (buffer[0] - '0');
                    } else if (buffer[0] == ' ') {
                        if (div == 0) {
                            exit(1);
                        }
                        res /= div;
                        div = 0;
                    } else if (buffer[0] == '\r') {
                        if (ReadFile(readHandle, buffer, 1, &readBytes, NULL)) {
                            if (readBytes == 0) {
                                exit(1);
                            }

                            if (buffer[0] == '\n') {
                                if (div == 0) {
                                    exit(1);
                                }
                                res /= div;
                                div = 0;

                                WriteFile(writeHandle, &res, sizeof(int),
                                          &writtenBytes, NULL);

                                break;
                            } else {
                                exit(1);
                            }
                        } else {
                            exit(1);
                        }
                    } else {
                        exit(1);
                    }
                } else {
                    exit(1);
                }
            }
        }
    }

    return 0;
}
