#include <windows.h>

#include <algorithm>
#include <iostream>
#include <mutex>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

struct Game {
    std::string name;
    int playerCount;
    std::vector<std::string> players;
    std::vector<std::string> guesses;
    std::string secretNumber;
    bool isFull;
    bool isStarted;
};

class GameServer {
   public:
    GameServer() : pipeName("\\\\.\\pipe\\bulls_and_cows_pipe") {}

    void start() {
        std::cout << "Starting server...\n";

        while (true) {
            HANDLE pipe = CreateNamedPipe(
                pipeName.c_str(), PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES, 512, 512, 0, NULL);

            if (pipe == INVALID_HANDLE_VALUE) {
                std::cerr << "Failed to create named pipe. Exiting...\n";
                return;
            }

            std::cout << "Waiting for client connection...\n";
            if (ConnectNamedPipe(pipe, NULL) != FALSE) {
                std::cout << "Client connected.\n";

                std::thread clientThread(&GameServer::handleClient, this, pipe);
                clientThread.detach();

            } else {
                std::cerr << "Failed to connect to client.\n";
            }
        }
    }

   private:
    void handleClient(HANDLE pipe) {
        char buffer[512];
        DWORD bytesRead;

        while (true) {
            if (!ReadFile(pipe, buffer, sizeof(buffer), &bytesRead, NULL) ||
                bytesRead == 0) {
                std::cerr << "Failed to read from client. Exiting...\n";
                break;
            }

            buffer[bytesRead] = '\0';
            std::string request(buffer);

            std::stringstream ss(request);
            std::string command;

            ss >> command;

            if (command == "create_game") {
                std::string gameName, playerCountString;
                ss >> gameName >> playerCountString;
                int playerCount = std::stoi(playerCountString);
                createGame(pipe, gameName, playerCount);
            } else if (command == "join_game") {
                std::string gameName;
                ss >> gameName;
                joinGame(pipe, gameName);
            } else if (command == "search_game") {
                searchGame(pipe);
            } else if (command == "make_guess") {
                std::string gameName, guess;
                ss >> gameName >> guess;
                makeGuess(pipe, gameName, guess);
            } else {
                std::cerr << "Unknown request.\n";
            }
        }
        CloseHandle(pipe);
    }

    void createGame(HANDLE pipe, const std::string& gameName, int playerCount) {
        std::cout << "Creating a new game...\n";

        Game newGame;
        newGame.name = gameName;
        newGame.playerCount = playerCount;
        newGame.isFull = false;
        newGame.isStarted = false;
        newGame.secretNumber = generateSecretNumber();
        games.push_back(newGame);

        WriteFile(pipe, "Game created successfully.\n", 24, NULL, NULL);
    }

    void joinGame(HANDLE pipe, const std::string& gameName) {
        std::cout << "Joining a game...\n";

        Game* game = findGameByName(gameName);
        if (!game) {
            WriteFile(pipe, "Game not found.\n", 15, NULL, NULL);
            return;
        }

        if (game->isFull) {
            WriteFile(pipe, "Game is full.\n", 13, NULL, NULL);
            return;
        }

        game->players.push_back("Player" +
                                std::to_string(game->players.size() + 1));
        if (game->players.size() == game->playerCount) {
            game->isFull = true;
            game->isStarted = true;
            WriteFile(pipe, "Game is full. Game starting...\n", 27, NULL, NULL);
        } else {
            WriteFile(pipe, "Joined game successfully.\n", 24, NULL, NULL);
        }
    }

    void searchGame(HANDLE pipe) {
        std::cout << "Searching for a game...\n";

        std::string response = "Games available:\n";
        for (const auto& game : games) {
            response += game.name +
                        " - Players: " + std::to_string(game.players.size()) +
                        "/" + std::to_string(game.playerCount) + "\n";
        }
        WriteFile(pipe, response.c_str(), response.length(), NULL, NULL);
    }

    void makeGuess(HANDLE pipe, const std::string& gameName,
                   const std::string& guess) {
        std::cout << "Making a guess...\n";

        Game* game = findGameByName(gameName);
        if (!game || !game->isStarted) {
            WriteFile(pipe, "Game not found or not started.\n", 30, NULL, NULL);
            return;
        }

        game->guesses.push_back(guess);

        std::string result = checkGuess(game->secretNumber, guess);

        if (guess == game->secretNumber) {
            result += "\nCongratulations! You've won the game.\n";
            WriteFile(pipe, result.c_str(), result.length(), NULL, NULL);
            auto it = std::remove_if(games.begin(), games.end(), [&](auto& g) {
                return g.name == game->name;
            });
            games.erase(it, games.end());
        } else {
            WriteFile(pipe, result.c_str(), result.length(), NULL, NULL);
        }
    }

    Game* findGameByName(const std::string& name) {
        for (auto& game : games) {
            if (game.name == name) {
                return &game;
            }
        }
        return nullptr;
    }

    std::string generateSecretNumber() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 9);

        std::string number;
        for (int i = 0; i < 4; ++i) {
            number += std::to_string(dis(gen));
        }

        std::cout << number << std::endl;

        return number;
    }

    std::string checkGuess(const std::string& secret,
                           const std::string& guess) {
        int bulls = 0, cows = 0;
        std::vector<bool> secret_used(4, false), guess_used(4, false);

        for (int i = 0; i < 4; ++i) {
            if (guess[i] == secret[i]) {
                bulls++;
                secret_used[i] = true;
                guess_used[i] = true;
            }
        }

        for (int i = 0; i < 4; ++i) {
            if (!guess_used[i]) {
                for (int j = 0; j < 4; ++j) {
                    if (!secret_used[j] && guess[i] == secret[j]) {
                        cows++;
                        secret_used[j] = true;
                        break;
                    }
                }
            }
        }

        return std::to_string(bulls) + " Bulls, " + std::to_string(cows) +
               " Cows";
    }

    std::vector<Game> games;
    std::string pipeName;
};

int main() {
    GameServer server;
    server.start();
    return 0;
}
