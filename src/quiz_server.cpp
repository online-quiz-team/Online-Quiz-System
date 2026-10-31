#include "net_compat.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

int main() {
    if (!netInit()) {
        std::cerr << "Network initialization failed\n";
        return 1;
    }

    sock_t server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == SOCK_INVALID) {
        std::cerr << "Socket creation failed\n";
        netCleanup();
        return 1;
    }

    // Allow quick restarts on Linux/macOS (avoids "address already in use").
#ifndef _WIN32
    int reuse = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&reuse), sizeof(reuse));
#endif

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(5050);

    if (bind(server, reinterpret_cast<sockaddr*>(&address),
             sizeof(address)) == SOCK_ERROR ||
        listen(server, 5) == SOCK_ERROR) {
        std::cerr << "Failed to start server on port 5050\n";
        sockClose(server);
        netCleanup();
        return 1;
    }

    std::cout << "Quiz results server listening on port 5050" << std::endl;

    while (true) {
        sock_t client = accept(server, nullptr, nullptr);
        if (client == SOCK_INVALID) continue;

        std::string data;
        char buffer[1024];
        long bytes;

        while ((bytes = static_cast<long>(
                    recv(client, buffer, static_cast<sock_len_t>(sizeof(buffer)), 0))) > 0) {
            data.append(buffer, static_cast<std::size_t>(bytes));
        }

        std::ofstream file("submissions.txt", std::ios::app);

        if (!data.empty() && file.is_open()) {
            file << data << "\n---\n";
            file.close();

            const char response[] = "OK\n";
            send(client, response, static_cast<sock_len_t>(sizeof(response) - 1), 0);
            std::cout << "Quiz result saved\n";
        } else {
            const char response[] = "ERROR\n";
            send(client, response, static_cast<sock_len_t>(sizeof(response) - 1), 0);
        }

        sockClose(client);
    }

    sockClose(server);
    netCleanup();
    return 0;
}
