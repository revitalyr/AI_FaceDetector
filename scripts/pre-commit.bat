@echo off
REM Pre-commit hook script for Windows
REM Runs quick code + test checks before commit

echo Running pre-commit checks...

REM Run tests
echo Running tests...
cd build
if exist "GlanceRawViewer_Tests.exe" (
    GlanceRawViewer_Tests.exe
) else (
    echo Warning: Test executable not found. Skipping tests.
)
cd ..

echo Pre-commit checks completed.
