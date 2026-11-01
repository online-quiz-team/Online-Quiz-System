
#include "admin.h"
#include "quiz.h"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

int main() {
    const std::string quizFile = "test_admin_quizzes.txt";
    const std::string questionFile = "test_admin_questions.txt";

    // Prepare independent test data.
    {
        std::ofstream file(quizFile);
        assert(file.is_open());

        file << "Q001|C++ Fundamentals|Basic syntax|2\n";
        file << "Q002|Databases|SQL basics|1\n";
    }

    {
        std::ofstream file(questionFile);
        assert(file.is_open());

        file << "Q001|1|What is C++?|Language|Database|Protocol|OS|A\n";
        file << "Q001|2|What is a class?|Object|Blueprint|Function|Variable|B\n";
        file << "Q002|1|What is SQL?|Language|Query language|Protocol|OS|B\n";
    }

    Admin admin(quizFile, questionFile);
    QuizCatalog catalog(quizFile);

    // 1. Test adding a question.
    QuizQuestion newQuestion = {
        "Q001",
        3,
        "Which header provides cout?",
        {"iostream", "string", "vector", "fstream"},
        "A"
    };

    assert(admin.addQuestion(newQuestion));

    auto questions = catalog.getQuestionsForQuiz("Q001", questionFile);

    assert(questions.size() == 3);
    assert(questions[2].questionNumber == 3);
    assert(questions[2].questionText == "Which header provides cout?");

    QuizInfo quiz;
    assert(catalog.findQuizById("Q001", quiz));
    assert(quiz.questionCount == 3);

    // 2. Duplicate question number must be rejected.
    assert(!admin.addQuestion(newQuestion));

    // 3. Invalid question must be rejected.
    QuizQuestion invalidQuestion = {
        "Q001",
        4,
        "Invalid question",
        {"Only one option"},
        "A"
    };

    assert(!admin.addQuestion(invalidQuestion));

    // 4. Unknown quiz must be rejected.
    QuizQuestion unknownQuizQuestion = {
        "Q999",
        1,
        "Unknown quiz question?",
        {"A", "B", "C", "D"},
        "A"
    };

    assert(!admin.addQuestion(unknownQuizQuestion));

    // 5. Test editing a question.
    QuizQuestion editedQuestion = {
        "Q001",
        2,
        "What is an object?",
        {"Blueprint", "Instance of a class", "Function", "Variable"},
        "B"
    };

    assert(admin.editQuestion("Q001", 2, editedQuestion));

    questions = catalog.getQuestionsForQuiz("Q001", questionFile);

    assert(questions.size() == 3);
    assert(questions[1].questionText == "What is an object?");
    assert(questions[1].options[1] == "Instance of a class");

    // Editing a nonexistent question must fail.
    assert(!admin.editQuestion("Q001", 99, editedQuestion));

    // Editing with a mismatched quiz ID must fail.
    QuizQuestion mismatchedQuestion = editedQuestion;
    mismatchedQuestion.quizId = "Q002";

    assert(!admin.editQuestion("Q001", 2, mismatchedQuestion));

    // 6. Test deleting a question.
    assert(admin.deleteQuestion("Q001", 1));

    questions = catalog.getQuestionsForQuiz("Q001", questionFile);

    assert(questions.size() == 2);
    assert(questions[0].questionNumber == 2);
    assert(questions[1].questionNumber == 3);

    assert(catalog.findQuizById("Q001", quiz));
    assert(quiz.questionCount == 2);

    // Deleting a nonexistent question must fail.
    assert(!admin.deleteQuestion("Q001", 99));

    // 7. Verify another quiz remains unchanged.
    auto otherQuestions =
        catalog.getQuestionsForQuiz("Q002", questionFile);

    assert(otherQuestions.size() == 1);
    assert(otherQuestions[0].questionText == "What is SQL?");

    assert(catalog.findQuizById("Q002", quiz));
    assert(quiz.questionCount == 1);

    // 8. The final question in a quiz cannot be deleted.
    assert(!admin.deleteQuestion("Q002", 1));

    // Verify that the question still exists.
    otherQuestions =
        catalog.getQuestionsForQuiz("Q002", questionFile);

    assert(otherQuestions.size() == 1);
    assert(otherQuestions[0].questionText == "What is SQL?");

    // Verify that the question count remains unchanged.
    assert(catalog.findQuizById("Q002", quiz));
    assert(quiz.questionCount == 1);

    // Clean up temporary files.
    std::remove(quizFile.c_str());
    std::remove(questionFile.c_str());

    std::remove((quizFile + ".tmp").c_str());
    std::remove((quizFile + ".bak").c_str());
    std::remove((questionFile + ".tmp").c_str());
    std::remove((questionFile + ".bak").c_str());

    std::cout << "All admin tests passed.\n";

    return 0;
}
