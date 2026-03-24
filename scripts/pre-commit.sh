#!/bin/bash
# Pre-commit hook script
# Runs quick code + test checks before commit

set -e

echo "Running pre-commit checks..."

# Run tests
echo "Running tests..."
cd build
if [ -f "./GlanceRawViewer_Tests" ] || [ -f "./GlanceRawViewer_Tests.exe" ]; then
    ./GlanceRawViewer_Tests || ./GlanceRawViewer_Tests.exe
else
    echo "Warning: Test executable not found. Skipping tests."
fi
cd ..

# Run basic lint checks if available
echo "Running lint checks..."
if command -v clang-tidy &> /dev/null; then
    clang-tidy Core/*.cpp -- -I. -std=c++23 || true
fi

echo "Pre-commit checks completed."
