@echo off
echo Building Raylib project (portable)...

REM Check if raylib files exist
if not exist "lib\libraylib.a" (
    echo Error: Raylib library not found in lib\libraylib.a
    echo Please ensure raylib is properly installed in the project.
    pause
    exit /b 1
)

if not exist "include\raylib\raylib.h" (
    echo Error: Raylib headers not found in include\raylib\
    echo Please ensure raylib headers are properly installed in the project.
    pause
    exit /b 1
)

REM Build the project using relative paths
g++ "src\main.cpp" -o "src\output\main.exe" -I"include\raylib" -L"lib" -lraylib -lopengl32 -lgdi32 -lwinmm -Wall -Wextra -std=c++17

if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    echo Run with: src\output\main.exe
) else (
    echo Build failed!
    echo Please check your compiler installation and error messages above.
    pause
)