#include <windows.h>

#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

struct NodeInfo {
    HANDLE processHandle;
    HANDLE pipeHandle;
    bool available;
};

class NodeManager {
   public:
    void createNode(int id, int parentId = -1) {
        if (nodes.find(id) != nodes.end()) {
            std::cout << "Error: Already exists\n";
            return;
        }

        if (parentId != -1 && nodes.find(parentId) == nodes.end()) {
            std::cout << "Error: Parent not found\n";
            return;
        }

        if (parentId != -1 && !isNodeAvailable(parentId)) {
            std::cout << "Error: Parent is unavailable\n";
            return;
        }

        // Создание именованного канала для узла
        std::string pipeName = "\\\\.\\pipe\\node_" + std::to_string(id);
        HANDLE pipeHandle = CreateNamedPipe(
            pipeName.c_str(), PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES, 1024, 1024, 0, nullptr);

        if (pipeHandle == INVALID_HANDLE_VALUE) {
            std::cout << "Error: Failed to create named pipe\n";
            return;
        }

        // Создание нового процесса
        STARTUPINFO si = {sizeof(STARTUPINFO)};
        PROCESS_INFORMATION pi;

        std::string command = "worker.exe " + std::to_string(id);
        if (CreateProcess(nullptr, const_cast<char *>(command.c_str()), nullptr,
                          nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
            CloseHandle(pi.hThread);
            nodes[id] = {pi.hProcess, pipeHandle, true};
            tree[parentId].insert(id);
            std::cout << "Ok: " << pi.dwProcessId << "\n";
        } else {
            CloseHandle(pipeHandle);
            std::cout << "Error: Failed to create process\n";
        }
    }

    void execCommand(int id, const std::string &text,
                     const std::string &pattern) {
        if (nodes.find(id) == nodes.end()) {
            std::cout << "Error: Not found\n";
            return;
        }

        if (!isNodeAvailable(id)) {
            std::cout << "Error: Node is unavailable\n";
            return;
        }

        HANDLE pipe = nodes[id].pipeHandle;
        std::string command = text + "|" + pattern;

        DWORD bytesWritten;
        if (!WriteFile(pipe, command.c_str(), command.size(), &bytesWritten,
                       nullptr)) {
            std::cout << "Error: Failed to send command to node\n";
            return;
        }

        char buffer[1024];
        DWORD bytesRead;
        if (ReadFile(pipe, buffer, sizeof(buffer), &bytesRead, nullptr)) {
            buffer[bytesRead] = '\0';
            std::cout << "Ok:" << id << ": " << buffer << "\n";
        } else {
            std::cout << "Error: Failed to read response from node\n";
        }
    }

    void pingAll() {
        std::vector<int> unavailable;
        for (auto &[id, node] : nodes) {
            if (!isNodeAvailable(id)) {
                unavailable.push_back(id);
            }
        }

        if (unavailable.empty()) {
            std::cout << "Ok: -1\n";
        } else {
            for (size_t i = 0; i < unavailable.size(); ++i) {
                std::cout << unavailable[i];
                if (i < unavailable.size() - 1) std::cout << ";";
            }
            std::cout << "\n";
        }
    }

   private:
    std::unordered_map<int, NodeInfo> nodes;
    std::unordered_map<int, std::set<int>> tree;

    bool isNodeAvailable(int id) {
        if (nodes.find(id) == nodes.end()) return false;
        HANDLE pipe = nodes[id].pipeHandle;

        char pingMessage[] = "ping";
        DWORD bytesWritten;
        if (!WriteFile(pipe, pingMessage, sizeof(pingMessage), &bytesWritten,
                       nullptr)) {
            return false;
        }

        char buffer[1024];
        DWORD bytesRead;
        if (ReadFile(pipe, buffer, sizeof(buffer), &bytesRead, nullptr)) {
            buffer[bytesRead] = '\0';
            return std::string(buffer) == "pong";
        }

        return false;
    }
};

int main() {
    NodeManager manager;

    std::string command;
    while (std::getline(std::cin, command)) {
        if (command.substr(0, 6) == "create") {
            int id, parentId = -1;
            std::stringstream ss(command);
            std::string cmd;
            ss >> cmd >> id >> parentId;
            manager.createNode(id, parentId);
        } else if (command.substr(0, 4) == "exec") {
            int id;
            std::string text, pattern;
            std::stringstream ss(command);
            std::string cmd;
            ss >> cmd >> id;
            std::getline(std::cin, text);
            std::getline(std::cin, pattern);
            manager.execCommand(id, text, pattern);
        } else if (command == "pingall") {
            manager.pingAll();
        } else {
            std::cout << "Unknown command\n";
        }
    }

    return 0;
}
