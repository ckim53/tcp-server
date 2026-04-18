#pragma once

#include <mutex>
#include <unordered_map>
#include <string>
#include <iostream>
#include "protocol.hpp"

// Tracks all connected clients and provides thread-safe operations on them.
//
// Multiple worker threads read/write this state concurrently, so every
// operation locks the mutex before touching the map. Without this, two threads
// could modify the map simultaneously causing a race condition

struct ServerState {
    std::mutex                           mtx;
    std::unordered_map<int, std::string> clients; // fd -> client identifier

    void add(int fd) {
        std::lock_guard<std::mutex> lock(mtx);
        clients[fd] = "client_" + std::to_string(fd);
        std::cout << "[+] Client " << fd << " connected. Total: "
                  << clients.size() << "\n";
    }

    void remove(int fd) {
        std::lock_guard<std::mutex> lock(mtx);
        clients.erase(fd);
        std::cout << "[-] Client " << fd << " disconnected. Total: "
                  << clients.size() << "\n";
    }

    // Forward a message to every connected client except the sender.
    // Note: we hold the lock during send(). Acceptable here since sends are
    // fast, but a more advanced design would copy the fd list then send outside
    // the lock to reduce contention.
    void broadcast(int sender_fd, const std::string& msg) {
        std::lock_guard<std::mutex> lock(mtx);
        for (auto& [fd, id] : clients) {
            if (fd != sender_fd)
                send_message(fd, msg);
        }
    }
};