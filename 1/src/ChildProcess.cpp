#include <windows.h>

int main() {
    HANDLE readHandle = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE writeHandle = GetStdHandle(STD_OUTPUT_HANDLE);
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
