#include <windows.h>

#include <iostream>
#include <string>
#include <vector>

std::vector<size_t> searchPattern(const std::string &text,
                                  const std::string &pattern) {
    std::vector<size_t> positions;
    size_t pos = text.find(pattern);
    while (pos != std::string::npos) {
        positions.push_back(pos);
        pos = text.find(pattern, pos + 1);
    }
    return positions;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Error: Node ID not provided\n";
        return 1;
    }

    int id = std::stoi(argv[1]);
    std::string pipeName = "\\\\.\\pipe\\node_" + std::to_string(id);

    HANDLE pipe = CreateFile(pipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0,
                             nullptr, OPEN_EXISTING, 0, nullptr);

    if (pipe == INVALID_HANDLE_VALUE) {
        std::cerr << "Error: Failed to connect to pipe\n";
        return 1;
    }

    char buffer[1024];
    while (true) {
        DWORD bytesRead;
        if (!ReadFile(pipe, buffer, sizeof(buffer), &bytesRead, nullptr)) {
            continue;
        }
        buffer[bytesRead] = '\0';

        std::string command(buffer);
        if (command == "ping") {
            const char *response = "pong";
            DWORD bytesWritten;
            WriteFile(pipe, response, strlen(response), &bytesWritten, nullptr);
        } else {
            auto delimiter = command.find('|');
            std::string text = command.substr(0, delimiter);
            std::string pattern = command.substr(delimiter + 1);

            auto positions = searchPattern(text, pattern);
            std::string result = positions.empty() ? "-1" : "";
            for (size_t pos : positions) {
                result += std::to_string(pos) + ";";
            }
            if (!result.empty() && result.back() == ';') {
                result.pop_back();
            }

            DWORD bytesWritten;
            WriteFile(pipe, result.c_str(), result.size(), &bytesWritten,
                      nullptr);
        }
    }

    CloseHandle(pipe);
    return 0;
}
