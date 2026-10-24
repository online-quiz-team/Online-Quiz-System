#include "../src/quiz_scoring.h"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

int main() {
    std::vector<QuizQuestion> questions(3);

    questions[0].correctAnswer = "A";
    questions[1].correctAnswer = "C";
    questions[2].correctAnswer = "B";

    // TC-F-10: All answers correct.
    assert(calculateScore({"A", "C", "B"}, questions) == 3);

    // TC-F-10: Two correct answers out of three.
    assert(calculateScore({"A", "D", "B"}, questions) == 2);

    // TC-F-10: No answers correct.
    assert(calculateScore({"D", "A", "C"}, questions) == 0);

    // Lowercase answers should also be accepted.
    assert(calculateScore({"a", "c", "b"}, questions) == 3);

    // Incomplete submission must not produce a score.
    assert(calculateScore({"A", "C"}, questions) == -1);

    std::cout << "All quiz scoring tests passed.\n";

    return 0;
}