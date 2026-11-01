
#include "registration.h"
#include "authentication.h"
#include "quiz.h"
#include "quiz_submission.h"
#include "quiz_scoring.h"
#include "security_log.h"
#include "admin.h"

#include <iostream>
#include <string>
#include <cctype>
#include <vector>
#include <iomanip>
#include <fstream>
#include <sstream>

// Cross-platform password input
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

std::string readPassword() {
    std::string password;
    char ch;

#ifdef _WIN32
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
#endif

    return password;
}

bool readInteger(const std::string& prompt, int& value) {
    std::string input;

    std::cout << prompt;

    if (!std::getline(std::cin, input)) {
        return false;
    }

    try {
        std::size_t parsed = 0;
        value = std::stoi(input, &parsed);
        return parsed == input.size();
    } catch (...) {
        return false;
    }
}

bool hasAdministrator() {
    std::ifstream file("admins.txt");
    std::string line;

    while (std::getline(file, line)) {
        if (!line.empty()) {
            return true;
        }
    }

    return false;
}

void displayQuizzes(const QuizCatalog& quizCatalog) {
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
                  << "\nQuestions: " << quiz.questionCount << '\n';
    }
}

// Supports both old and new result formats.
// Old: username,quizId,score
// New: username,quizId,score,total
bool parseResult(
    const std::string& line,
    std::string& username,
    std::string& quizId,
    int& score,
    int& total,
    const QuizCatalog& catalog
) {
    std::istringstream stream(line);
    std::string scoreText;
    std::string totalText;
    std::string extra;

    if (!std::getline(stream, username, ',') ||
        !std::getline(stream, quizId, ',') ||
        !std::getline(stream, scoreText, ',')) {
        return false;
    }

    if (std::getline(stream, totalText, ',')) {
        if (std::getline(stream, extra, ',')) {
            return false;
        }
    }

    try {
        std::size_t parsed = 0;
        score = std::stoi(scoreText, &parsed);

        if (parsed != scoreText.size() || score < 0) {
            return false;
        }

        if (totalText.empty()) {
            QuizInfo quiz;

            if (!catalog.findQuizById(quizId, quiz)) {
                return false;
            }

            total = quiz.questionCount;
        } else {
            parsed = 0;
            total = std::stoi(totalText, &parsed);

            if (parsed != totalText.size() || total <= 0) {
                return false;
            }
        }

        return total > 0 && score <= total;
    } catch (...) {
        return false;
    }
}

void printResult(
    const std::string& username,
    const std::string& quizId,
    int score,
    int total
) {
    const double percentage =
        (static_cast<double>(score) / total) * 100.0;

    std::cout << std::left
              << std::setw(20) << username
              << std::setw(15) << quizId
              << std::setw(12) << (std::to_string(score) + "/" +
                                    std::to_string(total))
              << std::fixed << std::setprecision(2)
              << percentage << "%\n";
}

// OQS-F-013: View student's previous results.
void viewPreviousResults(
    const std::string& username,
    const QuizCatalog& catalog
) {
    std::ifstream file("submissions.txt");

    if (!file) {
        std::cout << "No previous results found.\n";
        return;
    }

    std::string line;
    bool found = false;

    std::cout << "\n--- Your Previous Results ---\n";
    std::cout << std::left
              << std::setw(20) << "Username"
              << std::setw(15) << "Quiz ID"
              << std::setw(12) << "Score"
              << "Percentage\n";

    while (std::getline(file, line)) {
        std::string resultUser;
        std::string quizId;
        int score = 0;
        int total = 0;

        if (!parseResult(line, resultUser, quizId, score, total, catalog)) {
            continue;
        }

        if (resultUser == username) {
            printResult(resultUser, quizId, score, total);
            found = true;
        }
    }

    if (!found) {
        std::cout << "No results found for you yet.\n";
    }
}

// OQS-F-014: View global leaderboard.
void viewLeaderboard(const QuizCatalog& catalog) {
    std::ifstream file("submissions.txt");

    if (!file) {
        std::cout << "Leaderboard is empty.\n";
        return;
    }

    std::string line;
    bool found = false;

    std::cout << "\n--- Global Leaderboard ---\n";
    std::cout << std::left
              << std::setw(20) << "Username"
              << std::setw(15) << "Quiz ID"
              << std::setw(12) << "Score"
              << "Percentage\n";

    while (std::getline(file, line)) {
        std::string username;
        std::string quizId;
        int score = 0;
        int total = 0;

        if (!parseResult(line, username, quizId, score, total, catalog)) {
            continue;
        }

        printResult(username, quizId, score, total);
        found = true;
    }

    if (!found) {
        std::cout << "No valid results available.\n";
    }
}

// Backward-compatible wrapper for existing OQS-F-013 tests.
void viewPreviousResults(const std::string& username) {
    QuizCatalog catalog("quizzes.txt");
    viewPreviousResults(username, catalog);
}

// Backward-compatible wrapper for existing OQS-F-014 tests.
void viewLeaderboard() {
    QuizCatalog catalog("quizzes.txt");
    viewLeaderboard(catalog);
}

// OQS-F-015: Administrator views all student results.
void viewAllStudentResults(const QuizCatalog& catalog) {
    std::ifstream file("submissions.txt");

    if (!file) {
        std::cout << "No student results available.\n";
        return;
    }

    std::string line;
    bool found = false;

    std::cout << "\n--- All Student Results ---\n";
    std::cout << std::left
              << std::setw(20) << "Student"
              << std::setw(15) << "Quiz ID"
              << std::setw(12) << "Score"
              << "Percentage\n";

    while (std::getline(file, line)) {
        std::string username;
        std::string quizId;
        int score = 0;
        int total = 0;

        if (!parseResult(line, username, quizId, score, total, catalog)) {
            continue;
        }

        printResult(username, quizId, score, total);
        found = true;
    }

    if (!found) {
        std::cout << "No valid student results available.\n";
    }
}

bool readQuestionDetails(
    const std::string& quizId,
    int questionNumber,
    QuizQuestion& question
) {
    question.quizId = quizId;
    question.questionNumber = questionNumber;

    std::cout << "Enter question text: ";

    if (!std::getline(std::cin, question.questionText)) {
        return false;
    }

    for (int i = 0; i < 4; ++i) {
        std::cout << "Enter option "
                  << static_cast<char>('A' + i) << ": ";

        std::string option;

        if (!std::getline(std::cin, option)) {
            return false;
        }

        question.options.push_back(option);
    }

    std::cout << "Enter correct option (A/B/C/D): ";

    std::string correct;

    if (!std::getline(std::cin, correct) || correct.size() != 1) {
        return false;
    }

    question.correctAnswer = correct;
    return true;
}

// OQS-F-015: Administrator menu.
void administratorMenu(
    const std::string& adminUsername,
    Admin& adminManager,
    const QuizCatalog& catalog
) {
    int choice = -1;

    do {
        std::cout << "\n====================================\n";
        std::cout << "       ADMINISTRATOR MENU\n";
        std::cout << "====================================\n";
        std::cout << "Logged in as: " << adminUsername << '\n';
        std::cout << "1. Add Question\n";
        std::cout << "2. Edit Question\n";
        std::cout << "3. Delete Question\n";
        std::cout << "4. View Student Results\n";
        std::cout << "0. Logout\n";

        if (!readInteger("Enter your choice: ", choice)) {
            std::cout << "Invalid input.\n";
            choice = -1;
            continue;
        }

        switch (choice) {
        case 1: {
            displayQuizzes(catalog);

            std::string quizId;
            int questionNumber = 0;

            std::cout << "\nEnter Quiz ID: ";
            std::getline(std::cin, quizId);

            QuizInfo quiz;

            if (!catalog.findQuizById(quizId, quiz)) {
                std::cout << "Invalid Quiz ID.\n";
                break;
            }

            if (!readInteger("Enter new question number: ",
                             questionNumber) ||
                questionNumber <= 0) {
                std::cout << "Invalid question number.\n";
                break;
            }

            QuizQuestion question;

            if (!readQuestionDetails(quizId, questionNumber, question)) {
                std::cout << "Invalid question input.\n";
                break;
            }

            if (adminManager.addQuestion(question)) {
                securitylog::logAdminAction(
                    adminUsername, "ADD_QUESTION",
                    "Quiz " + quizId + ", question " +
                    std::to_string(questionNumber));

                std::cout << "Question added successfully.\n";
            } else {
                std::cout << "Failed to add question. Check the question "
                             "details and question number.\n";
            }

            break;
        }

        case 2: {
            std::string quizId;
            int questionNumber = 0;

            std::cout << "\nEnter Quiz ID: ";
            std::getline(std::cin, quizId);

            QuizInfo quiz;

            if (!catalog.findQuizById(quizId, quiz)) {
                std::cout << "Invalid Quiz ID.\n";
                break;
            }

            if (!readInteger("Enter question number to edit: ",
                             questionNumber) ||
                questionNumber <= 0) {
                std::cout << "Invalid question number.\n";
                break;
            }

            QuizQuestion updatedQuestion;

            if (!readQuestionDetails(
                    quizId, questionNumber, updatedQuestion)) {
                std::cout << "Invalid question input.\n";
                break;
            }

            if (adminManager.editQuestion(
                    quizId, questionNumber, updatedQuestion)) {
                securitylog::logAdminAction(
                    adminUsername, "EDIT_QUESTION",
                    "Quiz " + quizId + ", question " +
                    std::to_string(questionNumber));

                std::cout << "Question updated successfully.\n";
            } else {
                std::cout << "Failed to update question.\n";
            }

            break;
        }

        case 3: {
            std::string quizId;
            int questionNumber = 0;

            std::cout << "\nEnter Quiz ID: ";
            std::getline(std::cin, quizId);

            int unused = 0;
            (void)unused;

            if (!readInteger("Enter question number to delete: ",
                             questionNumber) ||
                questionNumber <= 0) {
                std::cout << "Invalid question number.\n";
                break;
            }

            std::string confirmation;

            std::cout << "Are you sure you want to delete this question? (Y/N): ";
            std::getline(std::cin, confirmation);

            if (confirmation != "Y" && confirmation != "y") {
                std::cout << "Deletion cancelled.\n";
                break;
            }

            if (adminManager.deleteQuestion(quizId, questionNumber)) {
                securitylog::logAdminAction(
                    adminUsername, "DELETE_QUESTION",
                    "Quiz " + quizId + ", question " +
                    std::to_string(questionNumber));

                std::cout << "Question deleted successfully.\n";
            } else {
                std::cout << "Failed to delete question.\n";
            }

            break;
        }

        case 4:
            viewAllStudentResults(catalog);
            securitylog::logAdminAction(
                adminUsername, "VIEW_STUDENT_RESULTS");
            break;

        case 0:
            std::cout << "Administrator logged out.\n";
            break;

        default:
            std::cout << "Invalid choice.\n";
            break;
        }

    } while (choice != 0);
}

int main() {
    Registration registration("students.txt");
    Authentication authentication("students.txt");

    Registration adminRegistration("admins.txt");
    Authentication adminAuthentication("admins.txt");

    QuizCatalog quizCatalog("quizzes.txt");
    QuizSubmission quizSubmission;
    Admin adminManager("quizzes.txt", "questions.txt");

    bool isLoggedIn = false;
    std::string loggedInUsername;

    std::string currentQuizId;
    QuizInfo currentQuiz;

    int choice = -1;

    do {
        std::cout << "\n====================================\n";
        std::cout << "       ONLINE QUIZ SYSTEM\n";
        std::cout << "====================================\n";
        std::cout << "1. Register Student\n";
        std::cout << "2. Student Login\n";
        std::cout << "3. Administrator Login\n";

        if (!hasAdministrator()) {
            std::cout << "4. First Administrator Setup\n";
        }

        std::cout << "5. Display Available Quizzes\n";
        std::cout << "6. Select a Quiz\n";
        std::cout << "7. Take Quiz and Submit\n";
        std::cout << "8. View Previous Results\n";
        std::cout << "9. View Leaderboard\n";
        std::cout << "0. Exit\n";

        if (!readInteger("Enter your choice: ", choice)) {
            std::cout << "Invalid input. Please enter a number.\n";
            choice = -1;
            continue;
        }

        switch (choice) {
        case 1: {
            std::string username;

            std::cout << "\n--- Student Registration ---\n";
            std::cout << "Enter username: ";
            std::getline(std::cin, username);

            std::cout << "Enter password: ";
            const std::string password = readPassword();

            if (registration.registerStudent(username, password)) {
                std::cout << "Registration successful.\n";
            } else {
                std::cout << "Registration failed.\n";
            }

            break;
        }

        case 2: {
            std::string username;

            std::cout << "\n--- Student Login ---\n";
            std::cout << "Enter username: ";
            std::getline(std::cin, username);

            std::cout << "Enter password: ";
            const std::string password = readPassword();

            if (authentication.login(username, password)) {
                loggedInUsername = username;
                isLoggedIn = true;

                std::cout << "Login successful. Welcome, "
                          << loggedInUsername << "!\n";
            } else {
                loggedInUsername.clear();
                isLoggedIn = false;

                securitylog::logFailedLogin(username);
                std::cout << "Invalid username or password.\n";
            }

            break;
        }

        case 3: {
            std::string username;

            std::cout << "\n--- Administrator Login ---\n";
            std::cout << "Enter administrator username: ";
            std::getline(std::cin, username);

            std::cout << "Enter password: ";
            const std::string password = readPassword();

            if (adminAuthentication.login(username, password)) {
                securitylog::logAdminAction(
                    username, "ADMIN_LOGIN");

                std::cout << "Administrator login successful.\n";

                administratorMenu(username, adminManager, quizCatalog);
            } else {
                securitylog::logFailedLogin(username);
                std::cout << "Invalid administrator username or password.\n";
            }

            break;
        }

        case 4: {
            if (hasAdministrator()) {
                std::cout << "An administrator account already exists. "
                             "Use Administrator Login.\n";
                break;
            }

            std::string username;

            std::cout << "\n--- First Administrator Setup ---\n";
            std::cout << "Create administrator username: ";
            std::getline(std::cin, username);

            std::cout << "Create administrator password: ";
            const std::string password = readPassword();

            if (adminRegistration.registerAccount(username, password)) {
                securitylog::logAdminAction(
                    username, "FIRST_ADMIN_SETUP");

                std::cout << "First administrator created successfully.\n";
            } else {
                std::cout << "Administrator setup failed.\n";
            }

            break;
        }

        case 5:
            displayQuizzes(quizCatalog);
            break;

        case 6: {
            displayQuizzes(quizCatalog);

            std::cout << "\nEnter Quiz ID: ";
            std::getline(std::cin, currentQuizId);

            if (quizCatalog.findQuizById(currentQuizId, currentQuiz)) {
                std::cout << "Quiz selected successfully: "
                          << currentQuiz.title << '\n';
            } else {
                std::cout << "Invalid Quiz ID.\n";
                currentQuizId.clear();
            }

            break;
        }

        case 7: {
            if (!isLoggedIn) {
                std::cout << "Please log in before taking a quiz.\n";
                break;
            }

            if (currentQuizId.empty()) {
                std::cout << "Please select a quiz first (Option 6).\n";
                break;
            }

            QuizInfo selectedQuiz;

            if (!quizCatalog.findQuizById(currentQuizId, selectedQuiz)) {
                std::cout << "Selected quiz is no longer available.\n";
                currentQuizId.clear();
                break;
            }

            const auto questions =
                quizCatalog.getQuestionsForQuiz(
                    currentQuizId, "questions.txt");

            if (questions.empty() ||
                questions.size() !=
                    static_cast<std::size_t>(selectedQuiz.questionCount)) {
                std::cout << "Quiz questions are unavailable or incomplete.\n";
                break;
            }

            std::vector<std::string> answers;
            bool inputEnded = false;

            for (std::size_t i = 0; i < questions.size(); ++i) {
                const auto& question = questions[i];

                std::cout << "\033[2J\033[H";
                std::cout << "====================================\n";
                std::cout << "Quiz: " << selectedQuiz.title << '\n';
                std::cout << "Student: " << loggedInUsername << '\n';
                std::cout << "Question " << i + 1
                          << " of " << questions.size() << '\n';
                std::cout << "====================================\n\n";

                std::cout << question.questionText << "\n\n";

                for (std::size_t j = 0; j < question.options.size(); ++j) {
                    std::cout << static_cast<char>('A' + j)
                              << ". " << question.options[j] << '\n';
                }

                std::string answer;
                char selectedOption = '\0';

                while (true) {
                    std::cout << "\nEnter your answer: ";

                    if (!std::getline(std::cin, answer)) {
                        inputEnded = true;
                        break;
                    }

                    if (answer.size() == 1) {
                        const char option = static_cast<char>(
                            std::toupper(
                                static_cast<unsigned char>(answer[0])));

                        if (option >= 'A' &&
                            option < 'A' +
                                static_cast<int>(question.options.size())) {
                            selectedOption = option;
                            break;
                        }
                    }

                    std::cout << "Invalid option.\n";
                }

                if (inputEnded) {
                    break;
                }

                answers.emplace_back(1, selectedOption);

                if (i + 1 < questions.size()) {
                    std::cout << "\nPress Enter for the next question...";
                    std::string input;

                    if (!std::getline(std::cin, input)) {
                        inputEnded = true;
                        break;
                    }
                }
            }

            if (inputEnded) {
                break;
            }

            std::string confirmation;

            while (true) {
                std::cout << "\nSubmit your completed quiz? (Y/N): ";

                if (!std::getline(std::cin, confirmation)) {
                    inputEnded = true;
                    break;
                }

                if (confirmation == "Y" || confirmation == "y" ||
                    confirmation == "N" || confirmation == "n") {
                    break;
                }

                std::cout << "Invalid input. Please enter Y or N.\n";
            }

            if (inputEnded ||
                confirmation == "N" || confirmation == "n") {
                std::cout << "Quiz submission cancelled.\n";
                break;
            }

            const int score = calculateScore(answers, questions);

            if (score < 0) {
                std::cout << "Unable to calculate quiz score.\n";
                break;
            }

            const bool submittedToServer =
                quizSubmission.submitQuiz(
                    loggedInUsername, selectedQuiz.id, answers);

            if (!submittedToServer) {
                std::cout << "\n[Warning] Server offline. "
                             "Saving result locally...\n";

                std::ofstream outFile("submissions.txt", std::ios::app);

                if (!outFile) {
                    std::cout << "Could not save the result locally.\n";
                    break;
                }

                outFile << loggedInUsername << ','
                        << selectedQuiz.id << ','
                        << score << ','
                        << questions.size() << '\n';

                if (!outFile.good()) {
                    std::cout << "Failed to save the local result.\n";
                    break;
                }
            } else {
                std::cout << "\n[Success] Result saved to online server.\n";
            }

            const double percentage =
                (static_cast<double>(score) / questions.size()) * 100.0;

            std::cout << "\n========== QUIZ RESULT ==========\n";
            std::cout << "Score: " << score << '/'
                      << questions.size() << '\n';
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "Percentage: " << percentage << "%\n";
            std::cout << "=================================\n";

            break;
        }

        case 8:
            if (!isLoggedIn) {
                std::cout << "Please log in to view your results.\n";
            } else {
                viewPreviousResults(loggedInUsername, quizCatalog);
            }
            break;

        case 9:
            viewLeaderboard(quizCatalog);
            break;

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
