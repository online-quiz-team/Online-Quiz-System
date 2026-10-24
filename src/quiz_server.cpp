#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>

#include <fstream>
#include <iostream>
#include <string>

int main() {
    WSADATA wsaData{};
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Winsock initialization failed\n";
        return 1;
    }

    SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(5050);

    if (bind(server, reinterpret_cast<sockaddr*>(&address),
             sizeof(address)) == SOCKET_ERROR ||
        listen(server, 5) == SOCKET_ERROR) {
        std::cerr << "Failed to start server on port 5050\n";
        closesocket(server);
        WSACleanup();
        return 1;
    }

    std::cout << "Quiz results server listening on port 5050" << std::endl;

    while (true) {
        SOCKET client = accept(server, nullptr, nullptr);
        if (client == INVALID_SOCKET) continue;

        std::string data;
        char buffer[1024];
        int bytes;

        while ((bytes = recv(client, buffer, sizeof(buffer), 0)) > 0) {
            data.append(buffer, static_cast<std::size_t>(bytes));
        }

        std::ofstream file("submissions.txt", std::ios::app);

        if (!data.empty() && file.is_open()) {
            file << data << "\n---\n";
            file.close();

            const char response[] = "OK\n";
            send(client, response, sizeof(response) - 1, 0);
            std::cout << "Quiz result saved\n";
        } else {
            const char response[] = "ERROR\n";
            send(client, response, sizeof(response) - 1, 0);
        }

        closesocket(client);
    }

    closesocket(server);
    WSACleanup();
    return 0;
}
