# Pre-Commit

Unit tests and lint checks before commit

## Executable Scripts
- Linux/macOS: `scripts/pre-commit.sh`
- Windows: `scripts/pre-commit.bat`

## Usage
```bash
# Linux/macOS
./scripts/pre-commit.sh

# Windows
scripts\pre-commit.bat
```

## What it does
- Runs unit tests
- Runs basic lint checks (if clang-tidy is available)

## Agents (for AI workflow)
- code-auditor
- test-runner
