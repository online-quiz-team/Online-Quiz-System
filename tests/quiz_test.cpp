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

    std::remove(filename);

    std::cout << "All quiz tests passed.\n";
    return 0;
}
