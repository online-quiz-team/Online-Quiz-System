
#include "quiz_submission.h"

#include <fstream>
#include <string>
#include <vector>

bool QuizSubmission::submitQuiz(
    const std::string& studentId,
    const std::string& quizId,
    const std::vector<std::string>& answers
) {
    if (studentId.empty() || quizId.empty() || answers.empty()) {
        return false;
    }

    // Save submissions in the project root.
    std::ofstream file("submissions.txt", std::ios::app);

    if (!file.is_open()) {
        return false;
    }

    file << "Student ID: " << studentId << '\n';
    file << "Quiz ID: " << quizId << '\n';
    file << "Total Answers: " << answers.size() << '\n';

    for (std::size_t i = 0; i < answers.size(); ++i) {
        file << "Question " << (i + 1)
             << ": " << answers[i] << '\n';
    }

    file << "---\n";
    file.close();

    return !file.fail();
}
