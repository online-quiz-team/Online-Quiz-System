#include "quiz_scoring.h"

#include <cctype>

int calculateScore(
    const std::vector<std::string>& submittedAnswers,
    const std::vector<QuizQuestion>& questions
) {
    // A mismatch means the submission is incomplete or invalid.
    if (submittedAnswers.size() != questions.size()) {
        return -1;
    }

    int score = 0;

    for (std::size_t i = 0; i < questions.size(); ++i) {
        if (submittedAnswers[i].size() != 1 ||
            questions[i].correctAnswer.size() != 1) {
            continue;
        }

        char submitted = static_cast<char>(
            std::toupper(
                static_cast<unsigned char>(submittedAnswers[i][0])
            )
        );

        char correct = static_cast<char>(
            std::toupper(
                static_cast<unsigned char>(questions[i].correctAnswer[0])
            )
        );

        if (submitted == correct) {
            ++score;
        }
    }

    return score;
}