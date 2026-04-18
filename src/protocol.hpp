#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>
#include <cstdint>

// Custom Application-Layer Protocol 
// TCP is a stream protocol, so recv() may return partial data with no message
// boundaries. We use length-prefix framing to delimit messages:
//
//   [ 4 bytes: payload length in network byte order ][ payload ]
//
// Network byte order (big-endian) is used so that machines with different
// CPU architectures (little-endian vs big-endian) interpret integers the same.
// htonl = host-to-network-long, ntohl = network-to-host-long.

static const size_t MAX_MSG_SIZE = 1024 * 1024; // 1MB sanity cap

// Loop until exactly n bytes are read — recv() may return less than requested
static bool recv_exact(int fd, char* buf, size_t n) {
    size_t total = 0;
    while (total < n) {
        ssize_t r = recv(fd, buf + total, n - total, 0);
        if (r <= 0) return false; // 0 = disconnected, -1 = error
        total += r;
    }
    return true;
}

// Read one framed message from fd. Returns empty string on disconnect/error.
inline std::string recv_message(int fd) {
    uint32_t net_len;
    if (!recv_exact(fd, (char*)&net_len, 4)) return "";

    uint32_t len = ntohl(net_len);
    if (len == 0 || len > MAX_MSG_SIZE) return "";

    std::string payload(len, '\0');
    if (!recv_exact(fd, payload.data(), len)) return "";
    return payload;
}

// Send one framed message to fd
inline void send_message(int fd, const std::string& msg) {
    uint32_t net_len = htonl(static_cast<uint32_t>(msg.size()));
    send(fd, &net_len, 4, 0);
    send(fd, msg.c_str(), msg.size(), 0);
}