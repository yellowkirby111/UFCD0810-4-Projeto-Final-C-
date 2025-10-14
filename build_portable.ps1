Write-Host "Building Raylib project (portable)..." -ForegroundColor Green

# Check if raylib files exist
if (-not (Test-Path "lib\libraylib.a")) {
    Write-Host "Error: Raylib library not found in lib\libraylib.a" -ForegroundColor Red
    Write-Host "Please ensure raylib is properly installed in the project." -ForegroundColor Yellow
    Read-Host "Press Enter to continue"
    exit 1
}

if (-not (Test-Path "include\raylib\raylib.h")) {
    Write-Host "Error: Raylib headers not found in include\raylib\" -ForegroundColor Red
    Write-Host "Please ensure raylib headers are properly installed in the project." -ForegroundColor Yellow
    Read-Host "Press Enter to continue"
    exit 1
}

# Build configuration
$source = "src\main.cpp"
$output = "src\output\main.exe" 
$includes = "-Iinclude\raylib"
$libs = "-Llib"
$linkerFlags = "-lraylib", "-lopengl32", "-lgdi32", "-lwinmm"
$compilerFlags = "-Wall", "-Wextra", "-std=c++17"

Write-Host "Compiling with g++..." -ForegroundColor Cyan

& g++ $source -o $output $includes $libs $linkerFlags $compilerFlags

if ($LASTEXITCODE -eq 0) {
    Write-Host "Build successful!" -ForegroundColor Green
    Write-Host "Run with: src\output\main.exe" -ForegroundColor Cyan
} else {
    Write-Host "Build failed!" -ForegroundColor Red
    Write-Host "Please check your compiler installation and error messages above." -ForegroundColor Yellow
}