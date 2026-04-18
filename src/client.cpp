#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <thread>
#include "protocol.hpp"

void listen_for_messages(int fd) {
    while (true) {
        std::string msg = recv_message(fd);
        if (msg.empty()) {
            std::cout << "Disconnected from server\n";
            break;
        }
        std::cout << "\n" << msg << "\n> ";
    }
}

int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect"); return 1;
    }
    std::cout << "Connected to server\n";

    std::thread listener(listen_for_messages, fd);
    listener.detach();

    std::string input;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, input)) break;
        if (input.empty()) continue;
        send_message(fd, input);
    }

    close(fd);
}