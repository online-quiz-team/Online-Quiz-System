# Unit Test Report

**Project:** Online Quiz System
**Environment:** MSYS2 UCRT64, GCC 16.2.0
**Test result:** 9/9 test programs passed

## Executed Test Areas

| Area | Coverage |
|---|---|
| Quiz catalogue and question parsing | Valid, malformed, missing, and invalid records |
| Score calculation | Correct, partial, zero, lowercase, empty, and mismatched answers |
| Administrator management | Add, edit, delete, duplicate, invalid, and rollback paths |
| Registration and authentication | Valid, duplicate, invalid, malformed, and missing-file paths |
| Security logging | Normal events, sanitization, truncation, and write failures |
| Interactive application | Registration, login, quiz, logout, cancellation, and menu errors |
| TCP submission client | Invalid input, connection errors, and OK/ERROR responses |
| Quiz results server | Parsing, persistence, loopback handling, and oversized requests |

## Conclusion

All nine test programs completed successfully in the MSYS2 UCRT64 environment. No test assertion failures were reported.
