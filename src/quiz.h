#ifndef QUIZ_H
#define QUIZ_H

#include <string>
#include <vector>

struct QuizInfo {
    std::string id;
    std::string title;
    std::string description;
    int questionCount;
};

struct QuizQuestion {
    std::string quizId;
    int questionNumber;
    std::string questionText;
    std::vector<std::string> options;
    std::string correctAnswer;
};

class QuizCatalog {
public:
    explicit QuizCatalog(const std::string& filename);
    std::vector<QuizInfo> getAvailableQuizzes() const;
    bool findQuizById(const std::string& id, QuizInfo& quiz) const;
    std::vector<QuizQuestion> getQuestionsForQuiz(
        const std::string& quizId,
        const std::string& questionFile
    ) const;

private:
    std::string storageFile;
};

#endif
