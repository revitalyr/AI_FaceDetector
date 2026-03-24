# Full Audit

All 11 auditors in parallel → fix-planner

## Executable Scripts
- Linux/macOS: `scripts/full-audit.sh`
- Windows: `scripts/full-audit.bat`

## Usage
```bash
# Linux/macOS
./scripts/full-audit.sh

# Windows
scripts\full-audit.bat
```

## What it does
- Builds the project
- Runs all tests
- Runs static analysis (clang-tidy, cppcheck if available)
- Runs security scan (trivy if available)
- Runs memory leak check (valgrind if available)

## Agents (for AI workflow)
- code-auditor
- bug-auditor
- security-auditor
- doc-auditor
- infra-auditor
- ui-auditor
- db-auditor
- perf-auditor
- dep-auditor
- seo-auditor
- api-tester
- fix-planner
