#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include "thread_pool.hpp"
#include "shared_state.hpp"
#include "protocol.hpp"

static const int PORT      = 8080;
static const int BACKLOG   = 10;  // max pending connections in OS queue
static const int POOL_SIZE = 4;   // number of worker threads


// Runs in a worker thread — handles one client for its entire lifetime
void handle_client(int client_fd, ServerState& state) {
    state.add(client_fd);

    while (true) {
        std::string msg = recv_message(client_fd);
        if (msg.empty()) break; // client disconnected or protocol error

        std::cout << "[fd=" << client_fd << "] " << msg << "\n";
        send_message(client_fd, "echo: " + msg);
        state.broadcast(client_fd, "broadcast: " + msg);
    }

    state.remove(client_fd);
    close(client_fd); // release file descriptor back to OS
}

int main() {
    // AF_INET = IPv4, SOCK_STREAM = TCP (vs SOCK_DGRAM for UDP)
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    // Without this, restarting the server immediately after a crash would fail
    // with "Address already in use" due to the OS TIME_WAIT state on the port
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY; // listen on all network interfaces
    addr.sin_port        = htons(PORT);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind");   return 1; }
    if (listen(server_fd, BACKLOG) < 0)                      { perror("listen"); return 1; }

    std::cout << "Server listening on port " << PORT << "\n";

    ServerState state;
    ThreadPool  pool(POOL_SIZE);

    // Main thread does nothing but accept connections and hand them off.
    // Keeping this loop tight ensures new clients are never kept waiting.
    while (true) {
        sockaddr_in client_addr{};
        socklen_t   len = sizeof(client_addr);
        int client_fd = accept(server_fd, (sockaddr*)&client_addr, &len);
        if (client_fd < 0) { perror("accept"); continue; }

        pool.enqueue([client_fd, &state] {
            handle_client(client_fd, state);
        });
    }

    close(server_fd);
    return 0;
}