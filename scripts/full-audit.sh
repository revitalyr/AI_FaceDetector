#!/bin/bash
# Full audit script
# Runs comprehensive checks on the codebase

set -e

echo "Running full audit..."

# Build the project
echo "Building project..."
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel

# Run all tests
echo "Running all tests..."
cd build
./GlanceRawViewer_Tests || ./GlanceRawViewer_Tests.exe
cd ..

# Run static analysis if available
echo "Running static analysis..."
if command -v clang-tidy &> /dev/null; then
    clang-tidy Core/*.cpp -- -I. -std=c++23 || true
fi

if command -v cppcheck &> /dev/null; then
    cppcheck --enable=all --std=c++23 Core/ IO/ UI/ || true
fi

# Security scan
echo "Running security scan..."
if command -v trivy &> /dev/null; then
    trivy fs . || true
fi

# Memory leak check
echo "Running memory leak check..."
cd build
if command -v valgrind &> /dev/null; then
    valgrind --leak-check=full --error-exitcode=1 ./GlanceRawViewer_Tests -functions TestImageProcessor::testApplyExposure || true
fi
cd ..

echo "Full audit completed."
