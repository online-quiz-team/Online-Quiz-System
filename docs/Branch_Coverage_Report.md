# Branch Coverage Report

**Project:** Online Quiz System
**Environment:** MSYS2 UCRT64, GCC 16.2.0
**Scope:** All production `.cpp` files under `src/`

## Test Evidence

The complete suite passed 9/9 test programs, covering:

- Quiz parsing and scoring
- Registration and authentication
- Security logging
- Administrator workflows
- Interactive menu paths
- TCP submission
- Server socket handling

## Measured Result

| Branch metric                                   | Covered | Total | Percentage |
|-------------------------------------------------|--------:|------:|-----------:|
| Raw branch coverage                             | 1,413   | 2,499 | 56.5%      |
| Normalized (excl. compiler-generated/unreachable) | 1,300 | 1,573 | 82.6%      |

- **Raw** is the unfiltered GCC/gcovr result.
- **Normalized** excludes compiler-generated exception and unreachable branches that are not meaningful application decisions:

```bash
gcovr --root . --filter "src/.*\.cpp$" \
  --exclude-throw-branches --exclude-unreachable-branches --print-summary
```

## Remaining Gaps

- Server startup and concurrency-limit failures
- Platform-specific Windows/POSIX branches
- Forced filesystem and socket failure paths
- Some interactive menu error branches

Both values are reported; the raw full-project branch result is **56.5%**.
