#include "quiz.h"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>

int main() {
    const char* filename = "test_quizzes.txt";

    {
        std::ofstream file(filename);
        file << "Q001|C++ Fundamentals|Basic syntax|10\n";
        file << "Q002|Databases|SQL basics|15\n";
        file << "INVALID|Missing fields\n";
        file << "Q003|Bad count|Invalid question count|abc\n";
        file << "Q004|Zero questions|Invalid count|0\n";
    }

    QuizCatalog catalog(filename);
    const auto quizzes = catalog.getAvailableQuizzes();

    assert(quizzes.size() == 2);
    assert(quizzes[0].id == "Q001");
    assert(quizzes[0].title == "C++ Fundamentals");
    assert(quizzes[0].questionCount == 10);
    assert(quizzes[1].id == "Q002");
    assert(quizzes[1].questionCount == 15);

    QuizInfo selectedQuiz;
    assert(catalog.findQuizById("Q001", selectedQuiz));
    assert(selectedQuiz.title == "C++ Fundamentals");
    assert(selectedQuiz.questionCount == 10);

    assert(!catalog.findQuizById("UNKNOWN", selectedQuiz));
    assert(!catalog.findQuizById("", selectedQuiz));

    const char* questionFile = "test_questions.txt";
    {
        std::ofstream file(questionFile);
        file << "Q001|1|What is C++?|Language|Database|Protocol|OS|A\n";
        file << "Q002|1|What is SQL?|Language|Query language|Protocol|OS|B\n";
        file << "Q001|2|Which header provides cout?|iostream|string|vector|fstream|A\n";
        file << "Q001|3|Malformed row\n";
    }

    const auto questions = catalog.getQuestionsForQuiz("Q001", questionFile);
    assert(questions.size() == 2);
    assert(questions[0].questionNumber == 1);
    assert(questions[0].questionText == "What is C++?");
    assert(questions[0].options.size() == 4);
    assert(questions[0].options[0] == "Language");
    assert(questions[0].correctAnswer == "A");
    assert(questions[1].questionNumber == 2);

    assert(catalog.getQuestionsForQuiz("UNKNOWN", questionFile).empty());
    std::remove(questionFile);

    std::remove(filename);

    std::cout << "All quiz tests passed.\n";
    return 0;
}
