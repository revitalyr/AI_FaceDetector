@echo off
REM Full audit script for Windows
REM Runs comprehensive checks on the codebase

echo Running full audit...

REM Build the project
echo Building project...
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug --parallel

REM Run all tests
echo Running all tests...
cd build
if exist "GlanceRawViewer_Tests.exe" (
    GlanceRawViewer_Tests.exe
) else (
    echo Error: Test executable not found.
    exit /b 1
)
cd ..

echo Full audit completed.
