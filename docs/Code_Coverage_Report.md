# Code Coverage Report

**Project:** Online Quiz System
**Environment:** MSYS2 UCRT64, GCC 16.2.0
**Test result:** 9/9 test programs passed

## Measured Scope

Coverage was measured across all production C++ files under `src/` after a clean rebuild and complete test-suite run.

```bash
gcovr --root . --filter "src/.*\.cpp$" --print-summary
```

## Results

| Metric    | Covered | Total | Percentage |
|-----------|--------:|------:|-----------:|
| Lines     | 1,151   | 1,318 | 87.3%      |
| Functions | 58      | 61    | 95.1%      |

## Coverage Status

The verified full-project result is **87.3% line coverage** and **95.1% function coverage**. Branch coverage is reported separately in the Branch Coverage Report.

Remaining gaps are mainly server startup/concurrency failures, platform-specific paths, and forced filesystem failures.
