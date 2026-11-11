# System Validation Report

**Project:** Online Quiz System
**Test period:** 2 November 2026 to 6 November 2026
**Validation result:** Accepted for the implemented scope

## 1. Validation Objective

System validation confirms that the implemented system satisfies the user-visible functional, security, and resilience requirements described in the SRS and test plan.

## 2. Requirements Validation

| Requirement / Feature | Validation Method | Result |
|---|---|---|
| OQS-F-001 Registration | Valid account, duplicate account, and empty-input tests | Pass |
| OQS-F-002 Authentication | Valid credentials, invalid credentials, unknown user, and empty-input tests | Pass |
| OQS-F-003 to OQS-F-005 Quiz catalogue and selection | Valid files, malformed rows, unknown quiz, and empty ID tests | Pass |
| OQS-F-006 to OQS-F-010 Quiz workflow and scoring | Question parsing, answer validation, complete/incomplete scoring tests | Pass |
| OQS-F-013 Previous results | Empty submission file and result display test | Pass |
| OQS-F-014 Leaderboard | Empty submission file and leaderboard display test | Pass |
| OQS-F-015 Administrator management | Add, edit, delete, duplicate, invalid, and unknown-quiz tests | Pass |
| OQS-SR-001 Network resilience | Offline server submission test | Pass; returns false without crashing |
| OQS-SR-002 Password protection | Registration record inspected for absence of plaintext password | Pass |
| Security logging | Failed login, administrator action, injection, and unwritable-file tests | Pass |
| OQS-NF-001 Performance | Sprint 2 manual measurement recorded 4 ms response time | Pass against the 3-second target |
| OQS-NF-002 Usability | Manual console menu and error message review | Pass |

## 3. Acceptance Criteria

- Core user flows execute without crashes.
- Invalid and boundary input is rejected.
- Passwords are not stored in plaintext.
- Security log fields cannot create forged log records.
- Results and leaderboard handling tolerate an empty submissions file.
- Administrator operations preserve quiz-file consistency.

All criteria were satisfied by the available automated and manual evidence.

## 4. Known Limitations

The clean rebuild of all OpenSSL-dependent tests requires a MinGW-compatible OpenSSL installation. Existing compiled security tests passed, but the release build should be repeated in the intended MSYS2 UCRT64 environment.

## 5. Final Decision

**System validation: PASS** for the tested implementation and documented scope.
