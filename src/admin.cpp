#include "admin.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdio>

namespace {

std::vector<std::string> split(const std::string& line, char delimiter) {
    std::vector<std::string> fields;
    std::stringstream stream(line);
    std::string field;

    while (std::getline(stream, field, delimiter)) {
        fields.push_back(field);
    }

    return fields;
}

bool containsInvalidDelimiter(const std::string& value) {
    return value.empty() ||
           value.find('|') != std::string::npos ||
           value.find('\n') != std::string::npos ||
           value.find('\r') != std::string::npos;
}

bool parseQuestion(const std::string& line, QuizQuestion& question) {
    std::vector<std::string> fields = split(line, '|');

    if (fields.size() != 8) {
        return false;
    }

    try {
        std::size_t parsedLength = 0;
        question.questionNumber = std::stoi(fields[1], &parsedLength);

        if (parsedLength != fields[1].size()) {
            return false;
        }

        question.quizId = fields[0];
        question.questionText = fields[2];
        question.options = {
            fields[3], fields[4], fields[5], fields[6]
        };
        question.correctAnswer = fields[7];
    }
    catch (...) {
        return false;
    }

    return true;
}

std::string serializeQuestion(const QuizQuestion& question) {
    std::ostringstream output;

    output << question.quizId << '|'
           << question.questionNumber << '|'
           << question.questionText << '|'
           << question.options[0] << '|'
           << question.options[1] << '|'
           << question.options[2] << '|'
           << question.options[3] << '|'
           << question.correctAnswer;

    return output.str();
}

bool fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

bool writeTemporaryFile(
    const std::string& filename,
    const std::string& content
) {
    std::ofstream output(filename, std::ios::trunc);

    if (!output.is_open()) {
        return false;
    }

    output << content;
    output.close();

    return output.good();
}

void restoreFile(
    const std::string& filename,
    const std::string& backupFile,
    bool hadOriginal,
    bool backupCreated
) {
    if (backupCreated) {
        std::remove(filename.c_str());
        std::rename(backupFile.c_str(), filename.c_str());
    }
    else if (!hadOriginal) {
        std::remove(filename.c_str());
    }
}

// Replaces two files as one logical operation.
// If either replacement fails, it attempts to restore both originals.
bool replaceTwoFiles(
    const std::string& firstFile,
    const std::string& firstContent,
    const std::string& secondFile,
    const std::string& secondContent
) {
    const std::string firstTemp = firstFile + ".tmp";
    const std::string secondTemp = secondFile + ".tmp";
    const std::string firstBackup = firstFile + ".bak";
    const std::string secondBackup = secondFile + ".bak";

    if (firstFile == secondFile) {
        return false;
    }

    std::remove(firstTemp.c_str());
    std::remove(secondTemp.c_str());

    if (!writeTemporaryFile(firstTemp, firstContent) ||
        !writeTemporaryFile(secondTemp, secondContent)) {
        std::remove(firstTemp.c_str());
        std::remove(secondTemp.c_str());
        return false;
    }

    const bool firstHadOriginal = fileExists(firstFile);
    const bool secondHadOriginal = fileExists(secondFile);

    std::remove(firstBackup.c_str());
    std::remove(secondBackup.c_str());

    bool firstBackupCreated = false;
    bool secondBackupCreated = false;

    if (firstHadOriginal) {
        if (std::rename(firstFile.c_str(), firstBackup.c_str()) != 0) {
            std::remove(firstTemp.c_str());
            std::remove(secondTemp.c_str());
            return false;
        }

        firstBackupCreated = true;
    }

    if (secondHadOriginal) {
        if (std::rename(secondFile.c_str(), secondBackup.c_str()) != 0) {
            restoreFile(
                firstFile, firstBackup,
                firstHadOriginal, firstBackupCreated
            );

            std::remove(firstTemp.c_str());
            std::remove(secondTemp.c_str());
            return false;
        }

        secondBackupCreated = true;
    }

    if (std::rename(firstTemp.c_str(), firstFile.c_str()) != 0) {
        restoreFile(
            firstFile, firstBackup,
            firstHadOriginal, firstBackupCreated
        );
        restoreFile(
            secondFile, secondBackup,
            secondHadOriginal, secondBackupCreated
        );

        std::remove(firstTemp.c_str());
        std::remove(secondTemp.c_str());
        return false;
    }

    if (std::rename(secondTemp.c_str(), secondFile.c_str()) != 0) {
        restoreFile(
            firstFile, firstBackup,
            firstHadOriginal, firstBackupCreated
        );
        restoreFile(
            secondFile, secondBackup,
            secondHadOriginal, secondBackupCreated
        );

        std::remove(firstTemp.c_str());
        std::remove(secondTemp.c_str());
        return false;
    }

    std::remove(firstBackup.c_str());
    std::remove(secondBackup.c_str());

    return true;
}

// Builds the updated quiz catalog without modifying the original file.
bool buildUpdatedQuizContent(
    const std::string& filename,
    const std::string& quizId,
    int count,
    std::string& updatedContent
) {
    std::ifstream input(filename);

    if (!input.is_open() || count < 1) {
        return false;
    }

    std::ostringstream output;
    std::string line;
    int matches = 0;

    while (std::getline(input, line)) {
        std::vector<std::string> fields = split(line, '|');

        if (fields.size() != 4) {
            return false;
        }

        if (fields[0] == quizId) {
            fields[3] = std::to_string(count);
            ++matches;
        }

        output << fields[0] << '|'
               << fields[1] << '|'
               << fields[2] << '|'
               << fields[3] << '\n';
    }

    if (matches != 1) {
        return false;
    }

    updatedContent = output.str();
    return true;
}

} // namespace

Admin::Admin(
    const std::string& quizFile,
    const std::string& questionFile
)
    : quizFile(quizFile),
      questionFile(questionFile) {}

bool Admin::isValidQuestion(const QuizQuestion& question) const {
    if (question.quizId.empty() ||
        question.questionNumber <= 0 ||
        containsInvalidDelimiter(question.quizId) ||
        containsInvalidDelimiter(question.questionText) ||
        question.options.size() != 4) {
        return false;
    }

    for (const std::string& option : question.options) {
        if (containsInvalidDelimiter(option)) {
            return false;
        }
    }

    return question.correctAnswer == "A" ||
           question.correctAnswer == "B" ||
           question.correctAnswer == "C" ||
           question.correctAnswer == "D";
}

bool Admin::replaceFile(
    const std::string& filename,
    const std::string& content
) {
    const std::string temporaryFile = filename + ".tmp";
    const std::string backupFile = filename + ".bak";

    std::remove(temporaryFile.c_str());

    if (!writeTemporaryFile(temporaryFile, content)) {
        std::remove(temporaryFile.c_str());
        return false;
    }

    const bool originalExists = fileExists(filename);

    std::remove(backupFile.c_str());

    if (originalExists &&
        std::rename(filename.c_str(), backupFile.c_str()) != 0) {
        std::remove(temporaryFile.c_str());
        return false;
    }

    if (std::rename(temporaryFile.c_str(), filename.c_str()) != 0) {
        if (originalExists) {
            std::rename(backupFile.c_str(), filename.c_str());
        }

        std::remove(temporaryFile.c_str());
        return false;
    }

    if (originalExists) {
        std::remove(backupFile.c_str());
    }

    return true;
}

bool Admin::updateQuestionCount(
    const std::string& quizId,
    int count
) {
    std::string updatedContent;

    if (!buildUpdatedQuizContent(
            quizFile, quizId, count, updatedContent)) {
        return false;
    }

    return replaceFile(quizFile, updatedContent);
}

bool Admin::addQuestion(const QuizQuestion& question) {
    if (!isValidQuestion(question)) {
        return false;
    }

    QuizCatalog catalog(quizFile);
    QuizInfo quiz;

    if (!catalog.findQuizById(question.quizId, quiz)) {
        return false;
    }

    std::ifstream input(questionFile);

    if (!input.is_open()) {
        return false;
    }

    std::ostringstream updatedQuestions;
    std::string line;
    int count = 0;

    while (std::getline(input, line)) {
        QuizQuestion existing;

        if (!parseQuestion(line, existing)) {
            return false;
        }

        if (existing.quizId == question.quizId) {
            if (existing.questionNumber == question.questionNumber) {
                return false;
            }

            ++count;
        }

        updatedQuestions << line << '\n';
    }

    input.close();

    updatedQuestions << serializeQuestion(question) << '\n';

    std::string updatedQuizCatalog;

    if (!buildUpdatedQuizContent(
            quizFile, question.quizId, count + 1, updatedQuizCatalog)) {
        return false;
    }

    return replaceTwoFiles(
        questionFile,
        updatedQuestions.str(),
        quizFile,
        updatedQuizCatalog
    );
}

bool Admin::editQuestion(
    const std::string& quizId,
    int questionNumber,
    const QuizQuestion& updatedQuestion
) {
    if (updatedQuestion.quizId != quizId ||
        updatedQuestion.questionNumber != questionNumber ||
        !isValidQuestion(updatedQuestion)) {
        return false;
    }

    QuizCatalog catalog(quizFile);
    QuizInfo quiz;

    if (!catalog.findQuizById(quizId, quiz)) {
        return false;
    }

    std::ifstream input(questionFile);

    if (!input.is_open()) {
        return false;
    }

    std::ostringstream updatedContent;
    std::string line;
    bool found = false;

    while (std::getline(input, line)) {
        QuizQuestion existing;

        if (!parseQuestion(line, existing)) {
            return false;
        }

        if (existing.quizId == quizId &&
            existing.questionNumber == questionNumber) {
            updatedContent << serializeQuestion(updatedQuestion) << '\n';
            found = true;
        }
        else {
            updatedContent << line << '\n';
        }
    }

    input.close();

    if (!found) {
        return false;
    }

    return replaceFile(questionFile, updatedContent.str());
}

bool Admin::deleteQuestion(
    const std::string& quizId,
    int questionNumber
) {
    QuizCatalog catalog(quizFile);
    QuizInfo quiz;

    if (!catalog.findQuizById(quizId, quiz)) {
        return false;
    }

    std::ifstream input(questionFile);

    if (!input.is_open()) {
        return false;
    }

    std::ostringstream updatedQuestions;
    std::string line;
    bool found = false;
    int remainingCount = 0;

    while (std::getline(input, line)) {
        QuizQuestion existing;

        if (!parseQuestion(line, existing)) {
            return false;
        }

        if (existing.quizId == quizId &&
            existing.questionNumber == questionNumber) {
            found = true;
            continue;
        }

        if (existing.quizId == quizId) {
            ++remainingCount;
        }

        updatedQuestions << line << '\n';
    }

    input.close();

    if (!found || remainingCount == 0) {
        return false;
    }

    std::string updatedQuizCatalog;

    if (!buildUpdatedQuizContent(
            quizFile, quizId, remainingCount, updatedQuizCatalog)) {
        return false;
    }

    return replaceTwoFiles(
        questionFile,
        updatedQuestions.str(),
        quizFile,
        updatedQuizCatalog
    );
}