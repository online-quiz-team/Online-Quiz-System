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

The whole project, including the optional TCP client and server, builds from the same source on Windows, Linux, and macOS. A C++17 compiler and OpenSSL are required.

Platform differences in networking (Winsock on Windows, BSD sockets on Linux and macOS) are isolated in `src/net_compat.h`. The client and server code is identical on every OS.

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
- Failed logins are written to `security.log` (UTC timestamp, event, username). Passwords are never logged: the logging API has no password parameter. Newlines and control characters in logged fields are replaced so user input cannot forge log entries, and a failed log write never crashes the app. `logAdminAction()` is provided for the administrator functions (OQS-F-015). On Linux and macOS the log file is created with owner-only permissions; on Windows the folder's default permissions apply.

### Optional TCP Client-Server Component

The optional network component allows quiz results to be submitted through TCP communication. The client and server use TCP port `5050` through the cross-platform layer in `src/net_compat.h`.

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
│   ├── security_log.cpp/.h       # Timestamped security event log
│   ├── net_compat.h              # Winsock / BSD socket compatibility layer
│   ├── quiz_submission.cpp/.h    # TCP submission client
│   └── quiz_server.cpp           # TCP result server
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

| OS | Install |
|---|---|
| Linux (Debian/Ubuntu) | `sudo apt update && sudo apt install build-essential libssl-dev` |
| macOS | `xcode-select --install` and `brew install openssl` |
| Windows (MSYS2 UCRT64) | In an MSYS2 UCRT64 shell: `pacman -S --needed make mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-openssl` |
| Windows (MSVC) | Visual Studio Build Tools with **Desktop development with C++** and a 64-bit OpenSSL install (see the MSVC section below) |

## Quick Start (Linux, macOS, Windows with MSYS2)

Run all commands from the project directory, because the application reads `students.txt`, `quizzes.txt`, `questions.txt`, and `submissions.txt` by relative path. On Windows, use the **MSYS2 UCRT64** shell.

### 1. Build

```bash
make
```

This produces two programs: `registration_app` (the quiz client; `registration_app.exe` on Windows) and `quiz_server` (the optional results server; `quiz_server.exe` on Windows).

### 2. Run the quiz application

```bash
./registration_app
```

If no server is reachable, quiz results are saved locally to `submissions.txt`.

### 3. (Optional) Run with the results server

Terminal 1, start the server:

```bash
./quiz_server
```

Terminal 2, start the client. On the same machine no configuration is needed:

```bash
./registration_app
```

To submit to a server on another machine, set `QUIZ_SERVER_IP` to the server's IP address before starting the client:

| Shell | Command |
|---|---|
| Linux / macOS / MSYS2 | `QUIZ_SERVER_IP=192.168.1.20 ./registration_app` |
| Windows cmd | `set QUIZ_SERVER_IP=192.168.1.20` then `registration_app.exe` |
| Windows PowerShell | `$env:QUIZ_SERVER_IP="192.168.1.20"` then `.\registration_app.exe` |

The server listens on TCP port `5050` on all interfaces. Allow inbound TCP `5050` through the server machine's firewall. Without `QUIZ_SERVER_IP`, the client connects to `127.0.0.1`.

### 4. Run the tests

```bash
make test
```

## Build and Run on Windows with MSVC

Open an **x64 Native Tools Command Prompt for Visual Studio**. Adjust the OpenSSL paths if your installation is elsewhere.

### Build the client

```bat
cl /EHsc /std:c++17 /I "C:\OpenSSL-Win64\include" ^
 src\main.cpp src\registration.cpp src\authentication.cpp ^
 src\quiz.cpp src\quiz_submission.cpp src\quiz_scoring.cpp src\security_log.cpp ^
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

The server listens on TCP port `5050`. Set `QUIZ_SERVER_IP` (see Quick Start) and allow TCP port `5050` through the firewall before using network submission.

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
- Security logging: failed-login and admin-action entries, timestamp format, log-injection protection, truncation, and failure handling.

Remove generated test binaries with:

```bash
make clean
```

**Platform note:** `make` builds on Linux, macOS, and Windows (MSYS2). The Makefile links `ws2_32` automatically on Windows and finds Homebrew OpenSSL on macOS. The unit tests do not require the network server.

## Test Report and Continuous Integration

The Sprint 1 test report is maintained in [`tests/Sprint1_Test_Report.md`](tests/Sprint1_Test_Report.md).

GitHub Actions runs the unit tests and a full `make` build on pushes and pull requests. The workflow is defined in [`.github/workflows/ci.yml`](.github/workflows/ci.yml).

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