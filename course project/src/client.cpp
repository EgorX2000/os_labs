#include <windows.h>

#include <iostream>
#include <numeric>
#include <string>

class GameClient {
   public:
    GameClient() : pipeName("\\\\.\\pipe\\bulls_and_cows_pipe") {}

    void connectToServer() {
        HANDLE pipe = CreateFile(pipeName.c_str(), GENERIC_READ | GENERIC_WRITE,
                                 0, NULL, OPEN_EXISTING, 0, NULL);

        if (pipe == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed to connect to server.\n";
            return;
        }

        std::cout << "Connected to server.\n";
        interactWithServer(pipe);
        CloseHandle(pipe);
    }

   private:
    void interactWithServer(HANDLE pipe) {
        std::string choice;
        while (true) {
            std::cout << "Choose an option:\n1. Create game\n2. Join game\n3. "
                         "Search game\n4. Make guess\n5. Exit\n";
            std::getline(std::cin, choice);

            if (choice == "1") {
                std::string gameName;
                int playerCount;
                std::cin >> gameName >> playerCount;

                std::string request = "create_game " + gameName + " " +
                                      std::to_string(playerCount);

                sendRequest(pipe, request);
            } else if (choice == "2") {
                std::string gameName;
                std::cin >> gameName;

                std::string request = "join_game " + gameName;

                sendRequest(pipe, request);
            } else if (choice == "3") {
                sendRequest(pipe, "search_game");
            } else if (choice == "4") {
                std::string gameName, guess;
                std::cin >> gameName >> guess;

                std::string request = "make_guess " + gameName + " " + guess;

                sendRequest(pipe, request);
            } else if (choice == "5") {
                break;
            } else {
                std::cerr << "Invalid choice.\n";
            }

            std::cin.ignore(std::numeric_limits<int>::max(), '\n');
        }
    }

    void sendRequest(HANDLE pipe, const std::string& request) {
        DWORD bytesWritten;

        WriteFile(pipe, request.c_str(), request.length(), &bytesWritten, NULL);

        char buffer[512];
        DWORD bytesRead;
        if (ReadFile(pipe, buffer, sizeof(buffer), &bytesRead, NULL)) {
            buffer[bytesRead] = '\0';
            std::cout << "Server response: " << buffer << std::endl;
        }
    }

    std::string pipeName;
};

int main() {
    GameClient client;
    client.connectToServer();
    return 0;
}
