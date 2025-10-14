@echo off
echo Building Raylib project...
"C:\Users\a25633\AppData\Local\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin\g++.exe" "src\main.cpp" -o "src\output\main.exe" -I"C:\raylib\include" -L"C:\raylib\lib" -lraylib -lopengl32 -lgdi32 -lwinmm -Wall -Wextra -std=c++17
if %ERRORLEVEL% EQU 0 (
    echo Build successful!
) else (
    echo Build failed!
)