
#ifndef QUIZ_SUBMISSION_H
#define QUIZ_SUBMISSION_H

#include <string>
#include <vector>

class QuizSubmission {
public:
    bool submitQuiz(
        const std::string& studentId,
        const std::string& quizId,
        const std::vector<std::string>& answers
    );

};

#endif
