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

class QuizCatalog {
public:
    explicit QuizCatalog(const std::string& filename);
    std::vector<QuizInfo> getAvailableQuizzes() const;

private:
    std::string storageFile;
};

#endif
