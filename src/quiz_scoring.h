#ifndef QUIZ_SCORING_H
#define QUIZ_SCORING_H

#include "quiz.h"
#include <string>
#include <vector>

int calculateScore(
    const std::vector<std::string>& submittedAnswers,
    const std::vector<QuizQuestion>& questions
);

#endif