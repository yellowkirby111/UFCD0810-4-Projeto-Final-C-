@echo off
echo Running Raylib program...
if exist "src\output\main.exe" (
    "src\output\main.exe"
) else (
    echo Error: main.exe not found. Please build the project first.
    pause
)