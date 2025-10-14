@echo off
echo Building Portable Raylib Project...

REM Check if output directory exists
if not exist "src\output\" mkdir "src\output\"

REM Try to find g++ compiler in common locations
set GCC_PATH=
where g++ >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    set GCC_PATH=g++
    echo Found g++ in PATH
    goto :compile
)

REM Check WinLibs location (common WinGet installation)
set WINLIBS_PATH="C:\Users\%USERNAME%\AppData\Local\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin\g++.exe"
if exist %WINLIBS_PATH% (
    set GCC_PATH=%WINLIBS_PATH%
    echo Found g++ in WinLibs location
    goto :compile
)

REM Check common MinGW locations
if exist "C:\MinGW\bin\g++.exe" (
    set GCC_PATH="C:\MinGW\bin\g++.exe"
    echo Found g++ in MinGW
    goto :compile
)

if exist "C:\mingw64\bin\g++.exe" (
    set GCC_PATH="C:\mingw64\bin\g++.exe"
    echo Found g++ in mingw64
    goto :compile
)

REM Check TDM-GCC
if exist "C:\TDM-GCC-64\bin\g++.exe" (
    set GCC_PATH="C:\TDM-GCC-64\bin\g++.exe"
    echo Found g++ in TDM-GCC
    goto :compile
)

echo ERROR: Could not find g++ compiler!
echo Please install MinGW-w64, TDM-GCC, or add g++ to your PATH
echo You can download MinGW-w64 from: https://www.mingw-w64.org/downloads/
exit /b 1

:compile
echo Using compiler: %GCC_PATH%
echo Compiling...

REM Compile using local/portable paths
%GCC_PATH% "src\main.cpp" -o "src\output\main.exe" -I"include\raylib" -L"lib" -lraylib -lopengl32 -lgdi32 -lwinmm -Wall -Wextra -std=c++17

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ✅ Build successful!
    echo Executable created at: src\output\main.exe
) else (
    echo.
    echo ❌ Build failed!
    echo Check the error messages above for details.
    exit /b 1
)

REM No pause for VS Code - exit cleanly
exit /b 0