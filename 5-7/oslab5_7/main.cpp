#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>
#include <sstream>
#include <windows.h>
#include <zmq.hpp>

struct NodeInfo {
    zmq::socket_t socket;
    std::string address;
    HANDLE processHandle;
    bool available;
};

class NodeManager {
public:
    NodeManager() : context(1) {}

    void createNode(int id, int parentId = -1) {
        if (nodes.find(id) != nodes.end()) {
            std::cout << "Error: Already exists\n";
            return;
        }

        if (parentId != -1 && nodes.find(parentId) == nodes.end()) {
            std::cout << "Error: Parent not found\n";
            return;
        }

        // Проверка доступности родительского узла
        if (parentId != -1 && !isNodeAvailable(parentId)) {
            std::cout << "Error: Parent is unavailable\n";
            return;
        }

        // Выбор минимального поддерева для добавления нового узла
        int selectedParent = (parentId != -1) ? parentId : findMinimalSubtree();

        std::string address = "tcp://127.0.0.1:*";
        zmq::socket_t nodeSocket(context, zmq::socket_type::req);
        nodeSocket.bind(address);
        address = nodeSocket.get(zmq::sockopt::last_endpoint);

        // Создаем новый процесс
        STARTUPINFO si = { sizeof(STARTUPINFO) };
        PROCESS_INFORMATION pi;

        std::string command = "node.exe " + address;
        if (CreateProcess(nullptr, const_cast<char*>(command.c_str()), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
            CloseHandle(pi.hThread);
            nodes[id] = { std::move(nodeSocket), address, pi.hProcess, true };
            tree[selectedParent].insert(id);
            std::cout << "Ok: " << pi.dwProcessId << "\n";
        }
        else {
            std::cout << "Error: Failed to create process\n";
        }
    }

    void execCommand(int id, const std::string& text, const std::string& pattern) {
        if (nodes.find(id) == nodes.end()) {
            std::cout << "Error: Not found\n";
            return;
        }

        if (!isNodeAvailable(id)) {
            std::cout << "Error: Node is unavailable\n";
            return;
        }

        auto& [socket, address, processHandle, available] = nodes[id];
        zmq::message_t request(text.size() + pattern.size() + 2);
        snprintf(request.data<char>(), request.size(), "%s|%s", text.c_str(), pattern.c_str());
        try {
            socket.send(request, zmq::send_flags::none);
            zmq::message_t reply;
            if (socket.recv(reply, zmq::recv_flags::none)) {
                std::cout << "Ok:" << id << ": " << reply.to_string() << "\n";
            }
            else {
                std::cout << "Error: Node is unavailable\n";
            }
        }
        catch (const zmq::error_t& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
    }

    void pingAll() {
        std::vector<int> unavailable;
        for (const auto& [id, node] : nodes) {
            if (!isNodeAvailable(id)) {
                unavailable.push_back(id);
            }
        }

        if (unavailable.empty()) {
            std::cout << "Ok: -1\n";
        }
        else {
            for (size_t i = 0; i < unavailable.size(); ++i) {
                std::cout << unavailable[i];
                if (i < unavailable.size() - 1) std::cout << ";";
            }
            std::cout << "\n";
        }
    }

private:
    zmq::context_t context;
    std::unordered_map<int, NodeInfo> nodes;
    std::unordered_map<int, std::set<int>> tree;

    bool isNodeAvailable(int id) {
        if (nodes.find(id) == nodes.end()) return false;
        auto& [socket, address, processHandle, available] = nodes[id];
        if (!available) return false;

        try {
            zmq::message_t ping(4);
            memcpy(ping.data(), "ping", 4);
            socket.send(ping, zmq::send_flags::none);
            zmq::message_t reply;
            if (socket.recv(reply, zmq::recv_flags::none)) {
                return true;
            }
            else {
                return false;
            }
        }
        catch (...) {
            return false;
        }
    }

    int findMinimalSubtree() {
        // Простейший алгоритм для нахождения минимального поддерева
        int minId = -1;
        size_t minSize = SIZE_MAX;
        for (const auto& [parentId, children] : tree) {
            if (children.size() < minSize) {
                minSize = children.size();
                minId = parentId;
            }
        }
        return minId;
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
        }
        else if (command.substr(0, 4) == "exec") {
            int id;
            std::string text, pattern;
            std::stringstream ss(command);
            std::string cmd;
            ss >> cmd >> id;
            std::getline(std::cin, text);
            std::getline(std::cin, pattern);
            manager.execCommand(id, text, pattern);
        }
        else if (command == "pingall") {
            manager.pingAll();
        }
        else {
            std::cout << "Unknown command\n";
        }
    }


    return 0;
}
