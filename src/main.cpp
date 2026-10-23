
#include "registration.h"
#include "authentication.h"
#include "quiz.h"

#include <iostream>
#include <string>
#include <limits>
#include <cctype>
#include <termios.h>
#include <unistd.h>

std::string readPassword() {
    termios oldSettings{};

    if (tcgetattr(STDIN_FILENO, &oldSettings) != 0) {
        return "";
    }

    termios newSettings = oldSettings;
    newSettings.c_lflag &= ~(ECHO | ICANON);

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &newSettings) != 0) {
        return "";
    }

    std::string password;
    char ch;

    while (read(STDIN_FILENO, &ch, 1) == 1) {
        if (ch == '\n' || ch == '\r') {
            break;
        }

        if (ch == 127 || ch == '\b') {
            if (!password.empty()) {
                password.pop_back();
                std::cout << "\b \b" << std::flush;
            }
        } else if (ch >= 32 && ch <= 126) {
            password += ch;
            std::cout << '*' << std::flush;
        }
    }

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &oldSettings);
    std::cout << '\n';

    return password;
}

int main() {
    Registration registration("students.txt");
    Authentication authentication("students.txt");
    QuizCatalog quizCatalog("quizzes.txt");

    int choice;
    std::string username;

    std::cout << "1. Register Student\n";
    std::cout << "2. Student Login\n";
    std::cout << "3. Display Available Quizzes\n";
    std::cout << "4. Select a Quiz\n";
    std::cout << "5. Retrieve Quiz Questions and Options\n";
    std::cout << "Enter choice: ";

    if (!(std::cin >> choice) || choice < 1 || choice > 5) {
        std::cout << "Invalid choice.\n";
        return 1;
    }

    if (choice == 3) {
        const auto quizzes = quizCatalog.getAvailableQuizzes();

        if (quizzes.empty()) {
            std::cout << "No available quizzes.\n";
        } else {
            std::cout << "\nAvailable Quizzes:\n";

            for (const auto& quiz : quizzes) {
                std::cout << quiz.id << " - " << quiz.title << "\n";
                std::cout << "Description: " << quiz.description << "\n";
                std::cout << "Questions: " << quiz.questionCount << "\n\n";
            }
        }

        return 0;
    }

    if (choice == 4) {
        const auto quizzes = quizCatalog.getAvailableQuizzes();

        if (quizzes.empty()) {
            std::cout << "No available quizzes to select.\n";
            return 0;
        }

        std::cout << "Enter quiz ID: ";

        std::string quizId;
        std::cin >> quizId;

        QuizInfo selectedQuiz;

        if (quizCatalog.findQuizById(quizId, selectedQuiz)) {
            std::cout << "Quiz selected successfully.\n";
            std::cout << "ID: " << selectedQuiz.id << "\n";
            std::cout << "Title: " << selectedQuiz.title << "\n";
            std::cout << "Description: " << selectedQuiz.description << "\n";
            std::cout << "Questions: " << selectedQuiz.questionCount << "\n";
        } else {
            std::cout << "Invalid quiz ID. Select an available quiz.\n";
        }

        return 0;
    }

    if (choice == 5) {
        const auto quizzes = quizCatalog.getAvailableQuizzes();

        if (quizzes.empty()) {
            std::cout << "No available quizzes.\n";
            return 0;
        }

        std::cout << "Enter quiz ID: ";

        std::string quizId;
        std::cin >> quizId;

        QuizInfo selectedQuiz;

        if (!quizCatalog.findQuizById(quizId, selectedQuiz)) {
            std::cout << "Invalid quiz ID.\n";
            return 0;
        }

        const auto questions =
            quizCatalog.getQuestionsForQuiz(quizId, "questions.txt");

        if (questions.empty()) {
            std::cout << "No questions available for this quiz.\n";
            return 0;
        }

        // Consume the leftover newline after reading the quiz ID.
        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(), '\n');

        for (std::size_t i = 0; i < questions.size(); ++i) {
            // Clear the console before displaying each question.
            std::cout << "\033[2J\033[H";

            const auto& question = questions[i];

            std::cout << "========================================\n";
            std::cout << "          ONLINE QUIZ SYSTEM\n";
            std::cout << "========================================\n\n";

            std::cout << "Quiz: " << selectedQuiz.title << "\n";
            std::cout << "Question " << i + 1
                      << " of " << questions.size() << "\n\n";

            std::cout << question.questionText << "\n\n";

            for (std::size_t j = 0; j < question.options.size(); ++j) {
                std::cout << static_cast<char>('A' + j) << ". "
                          << question.options[j] << "\n";
            }

            // OQS-F-007: Validate the student's answer.
            std::string answer;
            char selectedOption = '\0';

            while (true) {
                std::cout << "\nEnter your answer (A-"
                          << static_cast<char>(
                                 'A' + question.options.size() - 1)
                          << "): " << std::flush;

                std::getline(std::cin, answer);

                if (answer.size() == 1) {
                    char option = static_cast<char>(
                        std::toupper(
                            static_cast<unsigned char>(answer[0])));

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

            std::cout << "Answer accepted: "
                      << selectedOption << "\n";

            if (i + 1 < questions.size()) {
                std::cout << "\nPress Enter for the next question..."
                          << std::flush;

                std::string input;
                std::getline(std::cin, input);
            } else {
                std::cout << "\nYou have reached the last question.\n";
            }
        }

        return 0;
    }

    std::cout << "Enter username: ";
    std::cin >> username;

    std::cout << "Enter password: " << std::flush;
    std::string password = readPassword();

    if (password.empty()) {
        std::cout << "Password cannot be empty or read.\n";
        return 1;
    }

    if (choice == 1) {
        if (registration.registerStudent(username, password)) {
            std::cout << "Registration successful.\n";
        } else {
            std::cout << "Registration failed. Username may already exist "
                         "or the input is invalid.\n";
        }
    } else {
        if (authentication.login(username, password)) {
            std::cout << "Login successful.\n";
        } else {
            std::cout << "Login failed. Check your username and password.\n";
        }
    }

    return 0;
}
