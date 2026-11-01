#ifndef ADMIN_H
#define ADMIN_H

#include "quiz.h"
#include <string>

class Admin {
public:
    Admin(
        const std::string& quizFile = "quizzes.txt",
        const std::string& questionFile = "questions.txt"
    );

    bool addQuestion(const QuizQuestion& question);

    bool editQuestion(
        const std::string& quizId,
        int questionNumber,
        const QuizQuestion& updatedQuestion
    );

    bool deleteQuestion(
        const std::string& quizId,
        int questionNumber
    );

private:
    std::string quizFile;
    std::string questionFile;

    bool updateQuestionCount(const std::string& quizId, int count);

    bool replaceFile(
        const std::string& filename,
        const std::string& content
    );

    bool isValidQuestion(const QuizQuestion& question) const;
};

#endif