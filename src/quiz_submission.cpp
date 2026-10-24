#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>

#include "quiz_submission.h"

#include <string>

bool QuizSubmission::submitQuiz(
    const std::string& studentId,
    const std::string& quizId,
    const std::vector<std::string>& answers
) {
    if (studentId.empty() || quizId.empty() || answers.empty()) {
        return false;
    }

    WSADATA wsaData{};
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return false;
    }

    SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == INVALID_SOCKET) {
        WSACleanup();
        return false;
    }

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(5050);
    server.sin_addr.s_addr = inet_addr("10.187.150.25");

    if (connect(client, reinterpret_cast<sockaddr*>(&server),
                sizeof(server)) == SOCKET_ERROR) {
        closesocket(client);
        WSACleanup();
        return false;
    }

    std::string data = "Student ID: " + studentId +
                       "\nQuiz ID: " + quizId +
                       "\nTotal Answers: " + std::to_string(answers.size()) +
                       "\n";

    for (std::size_t i = 0; i < answers.size(); ++i) {
        data += "Question " + std::to_string(i + 1) +
                ": " + answers[i] + "\n";
    }

    std::size_t sent = 0;
    while (sent < data.size()) {
        int n = send(client, data.data() + sent,
                     static_cast<int>(data.size() - sent), 0);
        if (n == SOCKET_ERROR || n == 0) {
            closesocket(client);
            WSACleanup();
            return false;
        }
        sent += static_cast<std::size_t>(n);
    }

    shutdown(client, SD_SEND);

    char response[16]{};
    int received = recv(client, response, sizeof(response) - 1, 0);

    closesocket(client);
    WSACleanup();

    return received > 0 && std::string(response).find("OK") == 0;
}
