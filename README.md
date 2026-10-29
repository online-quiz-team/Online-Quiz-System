# Online Quiz System

A Software Engineering mini-project: a console-based C++ application for taking Multiple Choice Questions (MCQ) quizzes. The system supports student registration, authentication, quiz selection, automated scoring, result persistence, previous results, and a leaderboard. An optional Windows TCP client-server component supports network-based result submission.

## Sprint 1 Scope

Sprint 1 covers the current implemented feature set, including student authentication, quiz-taking functionality, scoring, result storage, previous results, and the leaderboard. Testing and implementation status are documented in the Sprint 1 test report.

## Highlights

- **Portable core:** Registration, authentication, quiz parsing, scoring, and unit tests use C++ and OpenSSL APIs.
- **Cross-platform password input:** Uses `conio.h` on Windows and `termios` on POSIX systems through `_WIN32` conditional compilation.
- **Quiz management:** Supports quiz selection, question display, answer validation, scoring, and result retrieval.
- **Secure password storage:** Uses PBKDF2-HMAC-SHA256 password hashing with random salts through OpenSSL.
- **Automated testing:** GitHub Actions runs the unit tests on pushes and pull requests.

## Operating-System Support

The core quiz application logic and unit tests are designed to compile on Windows and Linux, provided a compatible C++17 compiler and OpenSSL are installed.

The optional network components, `src/quiz_submission.cpp` and `src/quiz_server.cpp`, currently use the Windows Winsock API, including `winsock2.h`, `WSAStartup`, and `closesocket`. These components are intended for Windows and are not currently portable to POSIX systems.

## Implemented Features

| Requirement | Feature | Description |
|---|---|---|
| OQS-F-001 | Student registration | Creates student accounts and rejects empty, duplicate, or malformed input. |
| OQS-F-002 | Student authentication | Verifies credentials against registered student data. |
| OQS-F-003 | Quiz catalogue | Reads available quizzes from `quizzes.txt` and displays quiz IDs, titles, descriptions, and question counts. |
| OQS-F-004 | Quiz selection | Validates the selected quiz ID. |
| OQS-F-005 | Retrieve questions | Loads questions and options from `questions.txt`. |
| OQS-F-006 | Display questions | Presents one MCQ at a time. |
| OQS-F-007 | Validate answers | Validates answer options. |
| OQS-F-008 | Navigate questions | Ensures the quiz selected in Option 4 matches the quiz entered in Option 5. |
| OQS-F-009 | Submit quiz | Processes the completed quiz submission. |
| OQS-F-010 | Calculate score | Calculates the student's quiz score. |
| OQS-F-011 | Display results | Displays the score and percentage. |
| OQS-F-012 | Store results | Records results locally in `submissions.txt`. |
| OQS-F-013 | Previous results | Displays results for the logged-in student. |
| OQS-F-014 | Leaderboard | Displays the contents of the shared results file. |

### Security

- Passwords are stored as PBKDF2-HMAC-SHA256 hashes with random salts using OpenSSL.
- Plaintext passwords are not stored in the student records file.

### Optional TCP Client-Server Component

The optional Windows network component allows quiz results to be submitted through TCP communication. The client and server use Winsock and TCP port `5050`.

The local submission workflow stores results in `submissions.txt`. Network submission is a separate optional component and depends on the client/server configuration and environment.

## Project Structure

```text
Online-Quiz-System/
├── src/
│   ├── main.cpp                  # Console menu and quiz workflow
│   ├── registration.cpp/.h       # Registration and password hashing
│   ├── authentication.cpp/.h     # Login and password verification
│   ├── quiz.cpp/.h               # Quiz and question file parsing
│   ├── quiz_scoring.cpp/.h       # Score calculation
│   ├── quiz_submission.cpp/.h    # Windows TCP submission client
│   └── quiz_server.cpp           # Windows TCP result server
├── tests/                        # Unit tests and test report
├── quizzes.txt                   # Quiz catalogue
├── questions.txt                 # Questions and answer options
├── students.txt                  # Registered student records
├── submissions.txt               # Saved quiz results
├── Makefile                      # Build and test targets
└── .github/
    └── workflows/
        └── ci.yml                # GitHub Actions workflow
```

## Prerequisites

### Windows

Use either:

- Visual Studio Build Tools with the **Desktop development with C++** workload and a compatible 64-bit OpenSSL installation; or
- MSYS2 UCRT64 with `g++`, `make`, and OpenSSL.

### Linux or POSIX

Install the portable build and testing dependencies:

```bash
sudo apt update
sudo apt install build-essential libssl-dev
```

The complete TCP client-server component currently targets Windows because it uses Winsock.

## Build and Run on Windows with MSVC

Open an **x64 Native Tools Command Prompt for Visual Studio**. Adjust the OpenSSL paths if your installation is elsewhere.

### Build the client

```bat
cl /EHsc /std:c++17 /I "C:\OpenSSL-Win64\include" ^
 src\main.cpp src\registration.cpp src\authentication.cpp ^
 src\quiz.cpp src\quiz_submission.cpp src\quiz_scoring.cpp ^
 /link /LIBPATH:"C:\OpenSSL-Win64\lib\VC\x64\MD" ^
 libssl.lib libcrypto.lib ws2_32.lib /OUT:quiz_game.exe
```

Run the client from the project directory so the application can locate its data files:

```bat
quiz_game.exe
```

### Build and run the optional server

In a second command prompt, from the project directory:

```bat
cl /EHsc src\quiz_server.cpp ws2_32.lib /Fe:server.exe
server.exe
```

The server listens on TCP port `5050`. Configure the client IP address, server IP address, and firewall rules appropriately before using network submission.

## Build and Test with GNU Make

From the project directory, run the portable test target:

```bash
make test
```

The unit tests cover:

- Student registration and duplicate/input validation.
- Authentication with correct, incorrect, unknown, and empty credentials.
- Quiz catalogue and question-file parsing.
- Score calculation, including lowercase answers and incomplete submissions.

Remove generated test binaries with:

```bash
make clean
```

**Platform note:** The default `make` target includes a Windows Winsock source file. On Linux or POSIX systems, use `make test` for the portable unit-test targets. The unit tests do not require the network server.

## Test Report and Continuous Integration

The Sprint 1 test report is maintained in [`tests/Sprint1_Test_Report.md`](tests/Sprint1_Test_Report.md).

GitHub Actions runs the unit-test command on pushes and pull requests. The workflow is defined in [`.github/workflows/ci.yml`](.github/workflows/ci.yml) and installs OpenSSL before running `make test`.

## Data File Formats

| File | Format |
|---|---|
| `students.txt` | `username:salt:password_hash` |
| `quizzes.txt` | `quiz_id|title|description|question_count` |
| `questions.txt` | `quiz_id|question_number|question|option_a|option_b|option_c|option_d|correct_option` |
| `submissions.txt` | `username,quiz_id,score` |

Keep the executable in the project directory, or configure equivalent data-file paths, because the application uses relative paths to access these files.

## Team

- **Pavan Kumar K M** — QA Lead
- **Rosalin Verma** — Developer
- **S Shreyas Kumar** — Product Owner
- **Vageesh** — Test Engineer