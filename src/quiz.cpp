#include "quiz.h"

#include <fstream>
#include <sstream>

QuizCatalog::QuizCatalog(const std::string& filename)
    : storageFile(filename) {}

std::vector<QuizInfo> QuizCatalog::getAvailableQuizzes() const {
    std::vector<QuizInfo> quizzes;
    std::ifstream file(storageFile);
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream stream(line);
        std::string id;
        std::string title;
        std::string description;
        std::string questionCountText;
        std::string extra;

        if (!std::getline(stream, id, '|') ||
            !std::getline(stream, title, '|') ||
            !std::getline(stream, description, '|') ||
            !std::getline(stream, questionCountText, '|')) {
            continue;
        }

        if (id.empty() || title.empty() || description.empty() ||
            questionCountText.empty() || (stream >> extra)) {
            continue;
        }

        try {
            std::size_t parsed = 0;
            int questionCount = std::stoi(questionCountText, &parsed);

            if (parsed != questionCountText.size() || questionCount <= 0) {
                continue;
            }

            quizzes.push_back({id, title, description, questionCount});
        } catch (...) {
            continue;
        }
    }

    return quizzes;
}

bool QuizCatalog::findQuizById(const std::string& id, QuizInfo& quiz) const {
    const auto quizzes = getAvailableQuizzes();

    for (const auto& availableQuiz : quizzes) {
        if (availableQuiz.id == id) {
            quiz = availableQuiz;
            return true;
        }
    }

    return false;
}
