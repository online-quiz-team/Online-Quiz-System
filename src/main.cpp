#include "registration.h"
#include "authentication.h"
#include "quiz.h"
#include "quiz_submission.h"
#include "quiz_scoring.h"
#include "security_log.h"

#include <iostream>
#include <string>
#include <limits>
#include <cctype>
#include <vector>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>

// --- WINDOWS COMPATIBILITY FIX ---
#ifdef _WIN32
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif

// Read a password without displaying its characters.
std::string readPassword()
{
    std::string password;
    char ch;

#ifdef _WIN32
    // WINDOWS VERSION
    while ((ch = _getch()) != '\r') {
        if (ch == '\b') {
            if (!password.empty()) {
                password.pop_back();
                std::cout << "\b \b";
            }
        } else {
            password += ch;
            std::cout << '*';
        }
    }
    std::cout << '\n';
#else
    // LINUX VERSION
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
        if (bytesRead <= 0 || ch == '\n' || ch == '\r') break;

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
#endif
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
                  << "\nQuestions: " << quiz.questionCount << "\n";
    }
}

// OQS-F-013: View Previous Results
void viewPreviousResults(const std::string& username) {
    std::ifstream file("submissions.txt");
    if (!file) {
        std::cout << "No previous results found.\n";
        return;
    }
    
    std::string line;
    bool found = false;
    std::cout << "\n--- Your Previous Results ---\n";
    while (std::getline(file, line)) {
        // Assuming submissions.txt format is: username,quizId,score
        if (line.find(username + ",") == 0) {
            std::cout << line << "\n";
            found = true;
        }
    }
    if (!found) std::cout << "No results found for you yet.\n";
}

// OQS-F-014: Leaderboard
void viewLeaderboard() {
    std::ifstream file("submissions.txt");
    if (!file) {
        std::cout << "Leaderboard is empty.\n";
        return;
    }
    
    std::string line;
    std::cout << "\n--- Global Leaderboard ---\n";
    while (std::getline(file, line)) {
        std::cout << line << "\n";
    }
}

int main()
{
    Registration registration("students.txt");
    Authentication authentication("students.txt");
    QuizCatalog quizCatalog("quizzes.txt");
    QuizSubmission quizSubmission;

    bool isLoggedIn = false;
    std::string loggedInUsername;
    int choice = 0;
    std::string currentQuizId = "";
    QuizInfo currentQuiz;

    do {
        std::cout << "\n====================================\n";
        std::cout << "       ONLINE QUIZ SYSTEM\n";
        std::cout << "====================================\n";
        std::cout << "1. Register Student\n";
        std::cout << "2. Student Login\n";
        std::cout << "3. Display Available Quizzes\n";
        std::cout << "4. Select a Quiz\n";
        std::cout << "5. Take Quiz and Submit\n";
        std::cout << "6. View Previous Results\n";
        std::cout << "7. View Leaderboard\n";
        std::cout << "0. Exit\n";
        std::cout << "Enter your choice: ";

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a number.\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice) {
        case 1: {
            std::string username, password;
            std::cout << "\n--- Student Registration ---\n";
            std::cout << "Enter username: ";
            std::getline(std::cin, username);
            std::cout << "Enter password: ";
            password = readPassword();
            if (registration.registerStudent(username, password)) {
                std::cout << "Registration successful.\n";
            } else {
                std::cout << "Registration failed.\n";
            }
            break;
        }
        case 2: {
            std::string username, password;
            std::cout << "\n--- Student Login ---\n";
            std::cout << "Enter username: ";
            std::getline(std::cin, username);
            std::cout << "Enter password: ";
            password = readPassword();
            if (authentication.login(username, password)) {
                loggedInUsername = username;
                isLoggedIn = true;
                std::cout << "Login successful. Welcome, " << loggedInUsername << "!\n";
            } else {
                loggedInUsername.clear();
                isLoggedIn = false;
                securitylog::logFailedLogin(username);
                std::cout << "Invalid username or password.\n";
            }
            break;
        }
        case 3:
            displayQuizzes(quizCatalog);
            break;
        case 4: {
            displayQuizzes(quizCatalog);
            std::cout << "\nEnter Quiz ID: ";
            std::getline(std::cin, currentQuizId);
            if (quizCatalog.findQuizById(currentQuizId, currentQuiz)) {
                std::cout << "Quiz selected successfully: " << currentQuiz.title << '\n';
            } else {
                std::cout << "Invalid Quiz ID.\n";
                currentQuizId = "";
            }
            break;
        }
        case 5: {
            if (!isLoggedIn) {
                std::cout << "Please log in before taking a quiz.\n";
                break;
            }

            if (currentQuizId.empty()) {
                std::cout << "Please select a quiz first (Option 4).\n";
                break;
            }

            displayQuizzes(quizCatalog);
            std::string quizId;
            QuizInfo selectedQuiz;
            std::cout << "\nEnter Quiz ID: ";
            std::getline(std::cin, quizId);
            if (!quizCatalog.findQuizById(quizId, selectedQuiz)) {
                std::cout << "Invalid Quiz ID.\n";
                break;
            }

            if (quizId != currentQuizId) {
                std::cout << "Error: You entered " << quizId << " but you selected " << currentQuizId << " in Option 4. Please enter the same Quiz ID.\n";
                break;
            }

            const auto questions = quizCatalog.getQuestionsForQuiz(quizId, "questions.txt");
            if (questions.empty()) {
                std::cout << "No questions found for this quiz.\n";
                break;
            }
            std::vector<char> answers(questions.size(), '\0');
            bool inputEnded = false;
            for (std::size_t i = 0; i < questions.size(); ++i) {
                const auto& question = questions[i];
                std::cout << "\033[2J\033[H";
                std::cout << "====================================\n";
                std::cout << "Quiz: " << selectedQuiz.title << '\n';
                std::cout << "Student: " << loggedInUsername << '\n';
                std::cout << "Question " << i + 1 << " of " << questions.size() << '\n';
                std::cout << "====================================\n\n";
                std::cout << question.questionText << "\n\n";
                for (std::size_t j = 0; j < question.options.size(); ++j) {
                    std::cout << static_cast<char>('A' + j) << ". " << question.options[j] << '\n';
                }
                std::string answer;
                char selectedOption = '\0';
                while (true) {
                    std::cout << "\nEnter your answer: " << std::flush;
                    if (!std::getline(std::cin, answer)) {
                        inputEnded = true;
                        break;
                    }
                    if (answer.size() == 1) {
                        const char option = static_cast<char>(std::toupper(static_cast<unsigned char>(answer[0])));
                        if (option >= 'A' && option < 'A' + static_cast<int>(question.options.size())) {
                            selectedOption = option;
                            break;
                        }
                    }
                    std::cout << "Invalid option.\n";
                }
                if (inputEnded) break;
                answers[i] = selectedOption;
                if (i + 1 < questions.size()) {
                    std::cout << "\nPress Enter for the next question..." << std::flush;
                    std::string input;
                    if (!std::getline(std::cin, input)) { inputEnded = true; break; }
                }
            }
            if (inputEnded) break;
            std::string confirmation;
            while (true) {
                std::cout << "\nSubmit your completed quiz? (Y/N): " << std::flush;
                if (!std::getline(std::cin, confirmation)) break;
                
                if (confirmation == "Y" || confirmation == "y" || confirmation == "N" || confirmation == "n") {
                    break; // Valid input, exit loop
                }
                std::cout << "Invalid input. Please enter Y or N.\n";
            }
            
            if (confirmation == "N" || confirmation == "n") {
                std::cout << "Quiz submission cancelled.\n";
                break;
            }
            std::vector<std::string> submissionAnswers;
            for (char answer : answers) submissionAnswers.emplace_back(1, answer);
            
            // 1. Try to submit to the server first (Required by SRS)
            bool submittedToServer = quizSubmission.submitQuiz(loggedInUsername, selectedQuiz.id, submissionAnswers);
            
            // 2. If the server is offline, fallback to local save so the app doesn't crash
            if (!submittedToServer) {
                std::cout << "\n[Warning] Server offline. Saving result locally...\n";
                std::ofstream outFile("submissions.txt", std::ios::app);
                if (outFile) {
                    int tempScore = calculateScore(submissionAnswers, questions);
                    outFile << loggedInUsername << "," << selectedQuiz.id << "," << tempScore << "\n";
                    outFile.close();
                }
            } else {
                std::cout << "\n[Success] Result saved to online server.\n";
            }

            // 3. Always display the result
            std::cout << "\nQuiz submitted successfully!\n";
            const int score = calculateScore(submissionAnswers, questions);
            if (score >= 0) {
                const double percentage = (static_cast<double>(score) / static_cast<double>(questions.size())) * 100.0;
                std::cout << "\n========== QUIZ RESULT ==========\n";
                std::cout << "Score: " << score << "/" << questions.size() << '\n';
                std::cout << std::fixed << std::setprecision(2);
                std::cout << "Percentage: " << percentage << "%\n";
                std::cout << "=================================\n";
            }
            break;
        }
        case 6: {
            // OQS-F-013
            if (!isLoggedIn) {
                std::cout << "Please log in to view your results.\n";
            } else {
                viewPreviousResults(loggedInUsername);
            }
            break;
        }
        case 7: {
            // OQS-F-014
            viewLeaderboard();
            break;
        }
        case 0:
            std::cout << "Exiting Online Quiz System. Goodbye!\n";
            break;
        default:
            std::cout << "Invalid choice.\n";
            break;
        }
    } while (choice != 0);

    return 0;
}