
#include "net_compat.h"

#include "quiz.h"
#include "quiz_scoring.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr int SERVER_PORT = 5050;
constexpr std::size_t MAX_REQUEST_SIZE = 65536;

bool sendResponse(sock_t client, const std::string& response) {
    std::size_t sent = 0;

    while (sent < response.size()) {
        long bytes = static_cast<long>(
            send(client, response.data() + sent,
                 static_cast<sock_len_t>(response.size() - sent), 0));

        if (bytes == SOCK_ERROR || bytes == 0) {
            return false;
        }

        sent += static_cast<std::size_t>(bytes);
    }

    return true;
}

bool isSafeField(const std::string& value) {
    if (value.empty()) {
        return false;
    }

    return std::none_of(value.begin(), value.end(),
        [](unsigned char ch) {
            return ch == ',' || ch == '\n' || ch == '\r' ||
                   ch == '|' || ch < 32;
        });
}

bool parsePositiveInt(const std::string& text, int& value) {
    try {
        std::size_t parsed = 0;
        value = std::stoi(text, &parsed);

        return parsed == text.size() && value > 0;
    } catch (...) {
        return false;
    }
}

bool parseSubmission(
    const std::string& data,
    std::string& studentId,
    std::string& quizId,
    std::vector<std::string>& answers
) {
    std::istringstream input(data);
    std::string line;
    std::string totalText;
    std::map<int, std::string> answerMap;

    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line.compare(0, 12, "Student ID: ") == 0) {
            if (!studentId.empty()) return false;
            studentId = line.substr(12);
        }
        else if (line.compare(0, 9, "Quiz ID: ") == 0) {
            if (!quizId.empty()) return false;
            quizId = line.substr(9);
        }
        else if (line.compare(0, 15, "Total Answers: ") == 0) {
            if (!totalText.empty()) return false;
            totalText = line.substr(15);
        }
        else if (line.compare(0, 10, "Question ") == 0) {
            const std::size_t colon = line.find(':');

            if (colon == std::string::npos) return false;

            const std::string numberText =
                line.substr(9, colon - 9);

            int number = 0;
            if (!parsePositiveInt(numberText, number)) {
                return false;
            }

            std::string answer = line.substr(colon + 1);

            const std::size_t first = answer.find_first_not_of(" \t");
            if (first == std::string::npos) return false;

            answer.erase(0, first);

            const std::size_t last = answer.find_last_not_of(" \t");
            answer.erase(last + 1);

            if (answer.size() != 1) return false;

            const char option = static_cast<char>(
                std::toupper(static_cast<unsigned char>(answer[0])));

            if (option < 'A' || option > 'D') return false;

            if (!answerMap.emplace(number, std::string(1, option)).second) {
                return false;
            }
        }
        else if (!line.empty()) {
            return false;
        }
    }

    int totalAnswers = 0;

    if (!parsePositiveInt(totalText, totalAnswers) ||
        !isSafeField(studentId) ||
        !isSafeField(quizId) ||
        static_cast<int>(answerMap.size()) != totalAnswers) {
        return false;
    }

    answers.clear();

    for (int i = 1; i <= totalAnswers; ++i) {
        const auto it = answerMap.find(i);

        if (it == answerMap.end()) {
            return false;
        }

        answers.push_back(it->second);
    }

    return true;
}

bool processSubmission(const std::string& data) {
    std::string studentId;
    std::string quizId;
    std::vector<std::string> answers;

    if (!parseSubmission(data, studentId, quizId, answers)) {
        return false;
    }

    QuizCatalog catalog("quizzes.txt");

    QuizInfo quiz;
    if (!catalog.findQuizById(quizId, quiz)) {
        return false;
    }

    const auto questions =
        catalog.getQuestionsForQuiz(quizId, "questions.txt");

    // Reject incomplete or inconsistent quiz data.
    if (questions.empty() ||
        questions.size() != static_cast<std::size_t>(quiz.questionCount) ||
        answers.size() != questions.size()) {
        return false;
    }

    const int score = calculateScore(answers, questions);

    if (score < 0) {
        return false;
    }

    // Save only the server-calculated result.
    std::ofstream file("submissions.txt", std::ios::app);

    if (!file.is_open()) {
        return false;
    }

    file << studentId << ','
         << quizId << ','
         << score << ','
         << questions.size() << '\n';

    return file.good();
}

}  // namespace

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

#ifndef _WIN32
    int reuse = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&reuse), sizeof(reuse));
#endif

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(SERVER_PORT);

    if (bind(server, reinterpret_cast<sockaddr*>(&address),
             sizeof(address)) == SOCK_ERROR ||
        listen(server, 5) == SOCK_ERROR) {
        std::cerr << "Failed to start server on port "
                  << SERVER_PORT << '\n';
        sockClose(server);
        netCleanup();
        return 1;
    }

    std::cout << "Quiz results server listening on port "
              << SERVER_PORT << std::endl;

    while (true) {
        sock_t client = accept(server, nullptr, nullptr);

        if (client == SOCK_INVALID) {
            continue;
        }

        std::string data;
        char buffer[1024];
        long bytes = 0;
        bool validRequestSize = true;

        while ((bytes = static_cast<long>(
                    recv(client, buffer,
                         static_cast<sock_len_t>(sizeof(buffer)), 0))) > 0) {

            if (data.size() + static_cast<std::size_t>(bytes) >
                MAX_REQUEST_SIZE) {
                validRequestSize = false;
                break;
            }

            data.append(buffer, static_cast<std::size_t>(bytes));
        }

        if (validRequestSize && !data.empty() &&
            processSubmission(data)) {
            sendResponse(client, "OK\n");
            std::cout << "Validated quiz result saved\n";
        } else {
            sendResponse(client, "ERROR\n");
            std::cout << "Invalid quiz submission rejected\n";
        }

        sockClose(client);
    }

    sockClose(server);
    netCleanup();
    return 0;
}
