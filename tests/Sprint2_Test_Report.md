# Sprint 2 Test Report
**Project:** Online Quiz System
**Tester:** Vageesh (Test Engineer)
**Date:** November 2026

## 1. Executive Summary
During Sprint 2, the primary focus was on testing the newly integrated Administrator functions, the Student History/Leaderboard features (OQS-F-013 & OQS-F-014), and the Non-Functional Requirements (NFRs). Additionally, a native C++ automated testing suite using Google Test (gtest) was implemented to verify security constraints. All critical test cases passed.

## 2. Features Tested (Functional Requirements)
- [x] **OQS-F-015 (Admin Management):** Verified Administrator Login. Successfully tested adding a new question to the database, editing, and deleting.
- [x] **OQS-F-013 (Previous Results):** Logged in as a student, completed a quiz, and verified that the result was accurately displayed in the "View Previous Results" menu.
- [x] **OQS-F-014 (Leaderboard):** Verified that the global leaderboard accurately aggregates and displays scores from all test submissions.
- [x] **Option 4/5 Validation:** Verified boundary condition where selecting Quiz A in Option 4 and entering Quiz B in Option 5 correctly triggers an error message, preventing mismatched quiz attempts.

## 3. Automated Security Testing (Google Test / gtest)
Implemented a comprehensive C++ automated test suite (`tests/gtest_security_test.cpp`) utilizing `vcpkg` and `gtest`. 
- [x] **OQS-SR-001 (Client-Server):** Tested TCP socket communication. Server successfully received payload. Tested network fallback when server is offline (no crashes).
- [x] **OQS-SR-002 (Password Hashing):** Automated test registers a user, reads `students.txt`, and asserts that the plaintext password is not present, verifying PBKDF2 hashing.
- [x] **Security Logging:** Automated test verifies that failed logins and admin actions are logged. Also verified **Log Injection** prevention (newline attacks are sanitized).

## 4. Non-Functional Requirements (NFRs)
- [x] **OQS-NF-001 (Performance):** Server response time was measured at **4 ms** during test submissions, vastly exceeding the requirement of ≤ 3 seconds.
- [x] **OQS-NF-002 (Usability):** Manual audit of the console UI. Menus are clear, numbered, and error messages guide the user correctly without crashing the app.
- [x] **OQS-NF-004 (Concurrent Sessions):** Ran 4 simultaneous client instances connecting to the local server. No data corruption or socket binding errors occurred.

## 5. CI/CD Pipeline
- [x] Automated GitHub Actions pipeline is fully operational.
- [x] Successfully compiles and passes tests on **both Ubuntu (Linux) and Windows** runners.
- [x] 4/4 CI checks passed on the final PR merge.

## 6. Defects Found & Status
- **BUG-001 (Session Management):** While logged in as a student, the "Administrator Login" option remains visible. *Status: Logged in GitHub Issue #23 as a minor bug. Recommendation: Hide admin option if `isLoggedIn == true` in next release.*
- **BUG-002 (Input Buffer):** Quiz submission prompt occasionally skips if the Enter key is held down. *Status: Mitigated by adding input validation loops.*

## 7. Exit Criteria & Conclusion
All high-priority functional and security requirements have been implemented and verified. The codebase is OS-independent (Windows/Linux), fully automated via CI/CD, and all unit tests pass successfully. The system is ready for the final evaluation.