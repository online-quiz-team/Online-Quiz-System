#include "registration.h"
#include "authentication.h"
#include "quiz.h"

#include <iostream>
#include <string>
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
    std::cout << "Enter choice: ";

    if (!(std::cin >> choice) || choice < 1 || choice > 4) {
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
