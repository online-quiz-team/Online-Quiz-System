#include "registration.h"

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

    int choice;
    std::string username;

    std::cout << "1. Register Student\n";
    std::cout << "Enter choice: ";

    if (!(std::cin >> choice) || choice != 1) {
        std::cout << "Invalid choice.\n";
        return 1;
    }

    std::cout << "Enter username: ";
    std::cin >> username;

    std::cout << "Enter password: " << std::flush;
    std::string password = readPassword();

    if (password.empty()) {
        std::cout << "Password cannot be empty or read.\n";
        return 1;
    }

    if (registration.registerStudent(username, password)) {
        std::cout << "Registration successful.\n";
    } else {
        std::cout << "Registration failed. Username may already exist "
                     "or the input is invalid.\n";
    }

    return 0;
}
