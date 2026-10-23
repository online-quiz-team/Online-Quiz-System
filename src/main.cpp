
#include "registration.h"
#include "authentication.h"
#include "quiz.h"

#include <iostream>
#include <string>
#include <limits>
#include <cctype>
#include <vector>
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
    newSettings.c_lflag &= static_cast<tcflag_t>(~(ECHO | ICANON));

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

    int choice = 0;

    do {
        std::cout << "\n====================================\n";
        std::cout << "       ONLINE QUIZ SYSTEM\n";
        std::cout << "====================================\n";
        std::cout << "1. Register Student\n";
        std::cout << "2. Student Login\n";
        std::cout << "3. Display Available Quizzes\n";
        std::cout << "4. Select a Quiz\n";
        std::cout << "5. Retrieve Quiz Questions and Options\n";
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
                std::cout << "Login successful.\n";
            } else {
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
                quizCatalog.getQuestionsForQuiz(quizId, "questions.txt");

            if (questions.empty()) {
                std::cout << "No questions found for this quiz.\n";
                break;
            }

            // OQS-F-008: Store accepted answers in question order.
            std::vector<char> answers(questions.size(), '\0');

            for (std::size_t i = 0; i < questions.size(); ++i) {
                const auto& question = questions[i];

                // OQS-F-006: Display one MCQ at a time.
                std::cout << "\033[2J\033[H";
                std::cout << "====================================\n";
                std::cout << "Quiz: " << selectedQuiz.title << '\n';
                std::cout << "Question " << i + 1
                          << " of " << questions.size() << '\n';
                std::cout << "====================================\n\n";
                std::cout << question.questionText << "\n\n";

                for (std::size_t j = 0;
                     j < question.options.size(); ++j) {
                    std::cout << static_cast<char>('A' + j)
                              << ". " << question.options[j] << '\n';
                }

                // OQS-F-007: Validate the student's answer.
                std::string answer;
                char selectedOption = '\0';

                while (true) {
                    std::cout << "\nEnter your answer (A-"
                              << static_cast<char>(
                                     'A' + question.options.size() - 1
                                 )
                              << "): " << std::flush;

                    if (!std::getline(std::cin, answer)) {
                        std::cout << "\nInput ended. Returning to menu.\n";
                        return 0;
                    }

                    if (answer.size() == 1) {
                        const char option = static_cast<char>(
                            std::toupper(
                                static_cast<unsigned char>(answer[0])
                            )
                        );

                        if (option >= 'A' &&
                            option < 'A' +
                                static_cast<int>(question.options.size())) {
                            selectedOption = option;
                            break;
                        }
                    }

                    std::cout
                        << "Invalid option. Please enter a valid option.\n";
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
                        std::cout << "\nInput ended. Returning to menu.\n";
                        return 0;
                    }
                } else {
                    std::cout << "\nYou have reached the last question.\n";
                    std::cout << "All questions have been answered.\n";
                }
            }

            // Answers are held in memory; submission and scoring
            // belong to their respective requirements.
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
