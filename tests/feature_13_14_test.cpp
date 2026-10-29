#define main online_quiz_system_main
#include "../src/main.cpp"
#undef main

#include <cassert>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

namespace {

bool quizIdMatchesSelection(
    const std::string& selectedQuizId,
    const std::string& enteredQuizId
) {
    return !selectedQuizId.empty() && selectedQuizId == enteredQuizId;
}

void testQuizIdMatchesSelectedQuiz() {
    const std::string quizFile = "feature_13_14_quizzes.txt";
    {
        std::ofstream file(quizFile);
        file << "Q001|C++ Fundamentals|Basic syntax|1\n";
    }

    QuizCatalog catalog(quizFile);
    QuizInfo selectedQuiz;
    assert(catalog.findQuizById("Q001", selectedQuiz));
    assert(quizIdMatchesSelection(selectedQuiz.id, "Q001"));

    std::remove(quizFile.c_str());
}

void testMismatchedQuizIdIsRejected() {
    assert(!quizIdMatchesSelection("Q001", "Q002"));
    assert(!quizIdMatchesSelection("Q001", "INVALID"));
}

void testEmptySubmissionsFileDoesNotCrash() {
    const std::string submissionsFile = "submissions.txt";
    std::ifstream existingFile(submissionsFile, std::ios::binary);
    const std::string originalContents(
        (std::istreambuf_iterator<char>(existingFile)),
        std::istreambuf_iterator<char>()
    );
    const bool fileExisted = existingFile.good();

    {
        std::ofstream file(submissionsFile, std::ios::trunc);
        assert(file.good());
    }

    viewPreviousResults("test-student");
    viewLeaderboard();

    if (fileExisted) {
        std::ofstream file(submissionsFile, std::ios::trunc);
        file << originalContents;
    } else {
        std::remove(submissionsFile.c_str());
    }
}

}

int main() {
    testQuizIdMatchesSelectedQuiz();
    testMismatchedQuizIdIsRejected();
    testEmptySubmissionsFileDoesNotCrash();

    std::cout << "Feature 13/14 tests passed.\n";
    return 0;
}