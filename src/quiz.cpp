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

std::vector<QuizQuestion> QuizCatalog::getQuestionsForQuiz(
    const std::string& quizId,
    const std::string& questionFile
) const {
    std::vector<QuizQuestion> questions;

    QuizInfo quiz;
    if (!findQuizById(quizId, quiz)) {
        return questions;
    }

    std::ifstream file(questionFile);
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream stream(line);
        std::string rowQuizId;
        std::string numberText;
        std::string questionText;
        std::string optionA;
        std::string optionB;
        std::string optionC;
        std::string optionD;
        std::string correctAnswer;
        std::string extra;

        if (!std::getline(stream, rowQuizId, '|') ||
            !std::getline(stream, numberText, '|') ||
            !std::getline(stream, questionText, '|') ||
            !std::getline(stream, optionA, '|') ||
            !std::getline(stream, optionB, '|') ||
            !std::getline(stream, optionC, '|') ||
            !std::getline(stream, optionD, '|') ||
            !std::getline(stream, correctAnswer, '|')) {
            continue;
        }

        if (rowQuizId != quizId || questionText.empty() ||
            optionA.empty() || optionB.empty() ||
            optionC.empty() || optionD.empty() ||
            (correctAnswer != "A" && correctAnswer != "B" &&
             correctAnswer != "C" && correctAnswer != "D") ||
            (stream >> extra)) {
            continue;
        }

        try {
            std::size_t parsed = 0;
            int number = std::stoi(numberText, &parsed);

            if (parsed != numberText.size() || number <= 0) {
                continue;
            }

            questions.push_back({
                rowQuizId,
                number,
                questionText,
                {optionA, optionB, optionC, optionD},
                correctAnswer
            });
        } catch (...) {
            continue;
        }
    }

    return questions;
}
