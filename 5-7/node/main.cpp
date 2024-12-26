#include <iostream>
#include <string>
#include <vector>
#include <zmq.hpp>


std::vector<size_t> searchPattern(const std::string& text, const std::string& pattern) {
    std::vector<size_t> positions;
    size_t pos = text.find(pattern);
    while (pos != std::string::npos) {
        positions.push_back(pos);
        pos = text.find(pattern, pos + 1);
    }
    
    return positions;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: Address not provided\n";
        return 1;
    }

    zmq::context_t context{ 1 };
    zmq::socket_t socket(context, zmq::socket_type::rep);
    socket.connect(argv[1]);

    while (true) {
        zmq::message_t request;
        if (!socket.recv(request, zmq::recv_flags::none)) {
            continue;
        }

        std::string data = request.to_string();


        if (data == "ping") {
            socket.send(zmq::buffer("pong"), zmq::send_flags::none);
            continue;
        }


        auto delimiter = data.find('|');
        if (delimiter == std::string::npos) {
            socket.send(zmq::buffer("-1"), zmq::send_flags::none);
            continue;
        }

        std::string text = data.substr(0, delimiter);
        std::string pattern = data.substr(delimiter + 1, data.size() - delimiter - 2);


        auto positions = searchPattern(text, pattern);


        if (positions.empty()) {
            socket.send(zmq::buffer("-1"), zmq::send_flags::none);
        }
        else {
            std::string result;
            for (size_t pos : positions) {
                result += std::to_string(pos) + ";";
            }
            result.pop_back();
            socket.send(zmq::buffer(result), zmq::send_flags::none);
        }
    }

    return 0;
}
