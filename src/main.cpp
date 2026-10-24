
#include "registration.h"
#include "authentication.h"
#include "quiz.h"
#include "quiz_submission.h"
#include "quiz_scoring.h"

#include <iostream>
#include <string>
#include <limits>
#include <cctype>
#include <vector>
#include <iomanip>
#include <termios.h>
#include <unistd.h>

// Read a password without displaying its characters.
std::string readPassword()
{
    std::string password;
    char ch;

    termios oldSettings{};
    termios newSettings{};

    if (tcgetattr(STDIN_FILENO, &oldSettings) != 0) {
        std::getline(std::cin, password);
        return password;
    }

    newSettings = oldSettings;
    newSettings.c_lflag &=
        static_cast<tcflag_t>(~(ECHO | ICANON));

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &newSettings) != 0) {
        std::getline(std::cin, password);
        return password;
    }

    while (true) {
        ssize_t bytesRead = read(STDIN_FILENO, &ch, 1);

        if (bytesRead <= 0 || ch == '\n' || ch == '\r') {
            break;
        }

        if (ch == 127 || ch == '\b') {
            if (!password.empty()) {
                password.pop_back();
                std::cout << "\b \b" << std::flush;
            }
        } else {
            password += ch;
            std::cout << '*' << std::flush;
        }
    }

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &oldSettings);
    std::cout << '\n';

    return password;
}

void displayQuizzes(const QuizCatalog& quizCatalog)
{
    const auto quizzes = quizCatalog.getAvailableQuizzes();

    if (quizzes.empty()) {
        std::cout << "No quizzes are currently available.\n";
        return;
    }

    std::cout << "\n--- Available Quizzes ---\n";

    for (const auto& quiz : quizzes) {
        std::cout << "\nQuiz ID: " << quiz.id
                  << "\nTitle: " << quiz.title
                  << "\nDescription: " << quiz.description
                  << "\nQuestions: " << quiz.questionCount
                  << "\n";
    }
}

int main()
{
    Registration registration("students.txt");
    Authentication authentication("students.txt");
    QuizCatalog quizCatalog("quizzes.txt");
    QuizSubmission quizSubmission;

    // Track the currently authenticated student.
    bool isLoggedIn = false;
    std::string loggedInUsername;

    int choice = 0;

    do {
        std::cout << "\n====================================\n";
        std::cout << "       ONLINE QUIZ SYSTEM\n";
        std::cout << "====================================\n";
        std::cout << "1. Register Student\n";
        std::cout << "2. Student Login\n";
        std::cout << "3. Display Available Quizzes\n";
        std::cout << "4. Select a Quiz\n";
        std::cout << "5. Take Quiz and Submit\n";
        std::cout << "0. Exit\n";
        std::cout << "Enter your choice: ";

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(), '\n'
            );
            std::cout << "Invalid input. Please enter a number.\n";
            continue;
        }

        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(), '\n'
        );

        switch (choice) {
        case 1: {
            std::string username;
            std::string password;

            std::cout << "\n--- Student Registration ---\n";
            std::cout << "Enter username: ";
            std::getline(std::cin, username);

            std::cout << "Enter password: ";
            password = readPassword();

            if (registration.registerStudent(username, password)) {
                std::cout << "Registration successful.\n";
            } else {
                std::cout
                    << "Registration failed. Check your details "
                    << "or try a different username.\n";
            }
            break;
        }

        case 2: {
            std::string username;
            std::string password;

            std::cout << "\n--- Student Login ---\n";
            std::cout << "Enter username: ";
            std::getline(std::cin, username);

            std::cout << "Enter password: ";
            password = readPassword();

            if (authentication.login(username, password)) {
                loggedInUsername = username;
                isLoggedIn = true;
                std::cout << "Login successful. Welcome, "
                          << loggedInUsername << "!\n";
            } else {
                loggedInUsername.clear();
                isLoggedIn = false;
                std::cout << "Invalid username or password.\n";
            }
            break;
        }

        case 3:
            displayQuizzes(quizCatalog);
            break;

        case 4: {
            displayQuizzes(quizCatalog);

            std::string quizId;
            QuizInfo selectedQuiz;

            std::cout << "\nEnter Quiz ID: ";
            std::getline(std::cin, quizId);

            if (quizCatalog.findQuizById(quizId, selectedQuiz)) {
                std::cout << "Quiz selected successfully: "
                          << selectedQuiz.title << '\n';
            } else {
                std::cout << "Invalid Quiz ID. Please try again.\n";
            }
            break;
        }

        case 5: {
            // A student must log in before taking and submitting a quiz.
            if (!isLoggedIn) {
                std::cout
                    << "Please log in before taking a quiz.\n";
                break;
            }

            displayQuizzes(quizCatalog);

            std::string quizId;
            QuizInfo selectedQuiz;

            std::cout << "\nEnter Quiz ID: ";
            std::getline(std::cin, quizId);

            if (!quizCatalog.findQuizById(quizId, selectedQuiz)) {
                std::cout << "Invalid Quiz ID. Please try again.\n";
                break;
            }

            const auto questions =
                quizCatalog.getQuestionsForQuiz(
                    quizId, "questions.txt"
                );

            if (questions.empty()) {
                std::cout << "No questions found for this quiz.\n";
                break;
            }

            // OQS-F-008: Store answers in question order.
            std::vector<char> answers(questions.size(), '\0');

            bool inputEnded = false;

            for (std::size_t i = 0; i < questions.size(); ++i) {
                const auto& question = questions[i];

                // OQS-F-006: Display one question at a time.
                std::cout << "\033[2J\033[H";
                std::cout << "====================================\n";
                std::cout << "Quiz: " << selectedQuiz.title << '\n';
                std::cout << "Student: " << loggedInUsername << '\n';
                std::cout << "Question " << i + 1
                          << " of " << questions.size() << '\n';
                std::cout << "====================================\n\n";
                std::cout << question.questionText << "\n\n";

                for (std::size_t j = 0;
                     j < question.options.size(); ++j) {
                    std::cout << static_cast<char>('A' + j)
                              << ". " << question.options[j] << '\n';
                }

                // OQS-F-007: Validate the selected option.
                std::string answer;
                char selectedOption = '\0';

                while (true) {
                    if (question.options.empty()) {
                        std::cout
                            << "This question has no answer options.\n";
                        inputEnded = true;
                        break;
                    }

                    std::cout << "\nEnter your answer (A-"
                              << static_cast<char>(
                                     'A' + question.options.size() - 1
                                 )
                              << "): " << std::flush;

                    if (!std::getline(std::cin, answer)) {
                        inputEnded = true;
                        break;
                    }

                    if (answer.size() == 1) {
                        const char option = static_cast<char>(
                            std::toupper(
                                static_cast<unsigned char>(answer[0])
                            )
                        );

                        if (option >= 'A' &&
                            option < 'A' +
                                static_cast<int>(
                                    question.options.size()
                                )) {
                            selectedOption = option;
                            break;
                        }
                    }

                    std::cout
                        << "Invalid option. Please enter a valid option.\n";
                }

                if (inputEnded) {
                    break;
                }

                answers[i] = selectedOption;

                std::cout << "Answer accepted: "
                          << answers[i] << '\n';

                if (i + 1 < questions.size()) {
                    std::cout
                        << "\nPress Enter for the next question..."
                        << std::flush;

                    std::string input;
                    if (!std::getline(std::cin, input)) {
                        inputEnded = true;
                        break;
                    }
                } else {
                    std::cout << "\nYou have reached the last question.\n";
                    std::cout << "All questions have been answered.\n";
                }
            }

            if (inputEnded) {
                std::cout
                    << "\nQuiz was not submitted because the input "
                    << "ended or a question could not be answered.\n";
                break;
            }

            // OQS-F-009: Confirm submission after all answers are collected.
            std::string confirmation;

            while (true) {
                std::cout
                    << "\nDo you want to submit your completed quiz? (Y/N): ";

                if (!std::getline(std::cin, confirmation)) {
                    std::cout << "\nSubmission cancelled.\n";
                    return 0;
                }

                if (confirmation.size() == 1) {
                    const char response = static_cast<char>(
                        std::toupper(
                            static_cast<unsigned char>(confirmation[0])
                        )
                    );

                    if (response == 'Y' || response == 'N') {
                        confirmation = response;
                        break;
                    }
                }

                std::cout << "Please enter Y or N.\n";
            }

            if (confirmation == "N") {
                std::cout
                    << "Submission cancelled. Your answers were not saved.\n";
                break;
            }

            // Convert answers to the format expected by QuizSubmission.
            std::vector<std::string> submissionAnswers;
            submissionAnswers.reserve(answers.size());

            for (char answer : answers) {
                submissionAnswers.emplace_back(1, answer);
            }

            // OQS-F-009: Save the submission.
            if (quizSubmission.submitQuiz(
                    loggedInUsername,
                    selectedQuiz.id,
                    submissionAnswers)) {

                std::cout << "\nQuiz submitted successfully!\n";

                // OQS-F-010: Calculate score after successful submission.
                const int score =
                    calculateScore(submissionAnswers, questions);

                if (score >= 0) {
                    const double percentage =
                        (static_cast<double>(score) /
                         static_cast<double>(questions.size())) * 100.0;

                    std::cout << "\n========== QUIZ RESULT ==========\n";
                    std::cout << "Quiz: " << selectedQuiz.title << '\n';
                    std::cout << "Correct answers: " << score << '\n';
                    std::cout << "Incorrect answers: "
                              << questions.size() -
                                     static_cast<std::size_t>(score)
                              << '\n';
                    std::cout << "Total questions: "
                              << questions.size() << '\n';
                    std::cout << "Score: " << score
                              << "/" << questions.size() << '\n';
                    std::cout << std::fixed << std::setprecision(2);
                    std::cout << "Percentage: " << percentage << "%\n";
                    std::cout << "=================================\n";
                } else {
                    std::cout
                        << "Unable to calculate the score because "
                        << "the answer count does not match the "
                        << "question count.\n";
                }
            } else {
                std::cout
                    << "\nQuiz submission failed. "
                    << "Your submission could not be saved.\n";
            }

            break;
        }

        case 0:
            std::cout << "Exiting Online Quiz System. Goodbye!\n";
            break;

        default:
            std::cout << "Invalid choice. Please select a valid option.\n";
            break;
        }

    } while (choice != 0);

    return 0;
}
