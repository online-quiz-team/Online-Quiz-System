# QA Coverage Audit Report

**Project:** Online Quiz System  
**Environment:** MSYS2 UCRT64, GCC 16.2.0  
**Scope:** All production `.cpp` files under `src/`

## Executive summary

The complete clean test-suite run passed **9/9 test programs**. Coverage was
measured with GCC/gcovr after executing every test target.

| Metric | Covered | Total | Coverage |
|---|---:|---:|---:|
| Lines | 1,151 | 1,318 | **87.3%** |
| Functions | 58 | 61 | **95.1%** |
| Raw branches | 1,413 | 2,499 | **56.5%** |
| Normalized branches | 1,300 | 1,573 | **82.6%** |

Normalized branches exclude compiler-generated exception and unreachable
branches. The raw full-project branch value remains **56.5%**.

## Test traceability

| Component | Test evidence | Result |
|---|---|---|
| Quiz parsing and scoring | `quiz_test.cpp`, `quiz_scoring_test.cpp` | Pass |
| Registration and authentication | `registration_test.cpp`, `authentication_test.cpp` | Pass |
| Administrator operations | `admin_test.cpp`, feature workflow tests | Pass |
| Interactive application | `feature_13_14_test.cpp` | Pass |
| TCP submission | `quiz_submission_test.cpp` | Pass |
| Results server | `quiz_server_test.cpp` | Pass |
| Security logging | `security_log_test.cpp` | Pass |

## Remaining risk areas

- Server startup and concurrency-limit failure branches
- Platform-specific Windows/POSIX branches
- Forced filesystem and socket failures
- Some interactive error paths

## Conclusion

The tested implementation is functional and the measured line coverage is
87.3%. Function coverage is 95.1%. Branch coverage must be reported as raw
56.5% and normalized 82.6%; these values must not be presented as the same
metric.
