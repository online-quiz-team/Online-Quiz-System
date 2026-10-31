#include "net_compat.h"

#include "quiz_submission.h"

#include <cstdlib>
#include <string>

namespace {
// Server address can be overridden with the QUIZ_SERVER_IP environment variable.
const char* serverIp() {
    const char* env = std::getenv("QUIZ_SERVER_IP");
    return (env && *env) ? env : "127.0.0.1";
}
}  // namespace

bool QuizSubmission::submitQuiz(
    const std::string& studentId,
    const std::string& quizId,
    const std::vector<std::string>& answers
) {
    if (studentId.empty() || quizId.empty() || answers.empty()) {
        return false;
    }

    if (!netInit()) {
        return false;
    }

    sock_t client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == SOCK_INVALID) {
        netCleanup();
        return false;
    }

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(5050);
    if (inet_pton(AF_INET, serverIp(), &server.sin_addr) != 1) {
        sockClose(client);
        netCleanup();
        return false;
    }

    if (connect(client, reinterpret_cast<sockaddr*>(&server),
                sizeof(server)) == SOCK_ERROR) {
        sockClose(client);
        netCleanup();
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
        long n = static_cast<long>(
            send(client, data.data() + sent,
                 static_cast<sock_len_t>(data.size() - sent), 0));
        if (n == SOCK_ERROR || n == 0) {
            sockClose(client);
            netCleanup();
            return false;
        }
        sent += static_cast<std::size_t>(n);
    }

    shutdown(client, SOCK_SHUT_WR);

    char response[16]{};
    long received = static_cast<long>(
        recv(client, response, static_cast<sock_len_t>(sizeof(response) - 1), 0));

    sockClose(client);
    netCleanup();

    return received > 0 && std::string(response).find("OK") == 0;
}
