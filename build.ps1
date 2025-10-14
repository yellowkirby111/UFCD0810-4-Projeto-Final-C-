# Portable PowerShell script to compile Raylib project
Write-Host "Building Portable Raylib Project..." -ForegroundColor Green

# Check if output directory exists
if (-not (Test-Path "src\output")) {
    New-Item -ItemType Directory -Path "src\output" | Out-Null
}

# Try to find g++ compiler
$gccPath = $null

# Check if g++ is in PATH
try {
    $gccPath = (Get-Command "g++" -ErrorAction Stop).Source
    Write-Host "Found g++ in PATH: $gccPath" -ForegroundColor Yellow
} catch {
    # Check common MinGW locations
    $possiblePaths = @(
        "C:\Users\$env:USERNAME\AppData\Local\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin\g++.exe",
        "C:\MinGW\bin\g++.exe",
        "C:\mingw64\bin\g++.exe",
        "C:\TDM-GCC-64\bin\g++.exe"
    )
    
    foreach ($path in $possiblePaths) {
        if (Test-Path $path) {
            $gccPath = $path
            Write-Host "Found g++ at: $path" -ForegroundColor Yellow
            break
        }
    }
}

if (-not $gccPath) {
    Write-Host "ERROR: Could not find g++ compiler!" -ForegroundColor Red
    Write-Host "Please install MinGW-w64, TDM-GCC, or add g++ to your PATH" -ForegroundColor Red
    Write-Host "You can download MinGW-w64 from: https://www.mingw-w64.org/downloads/" -ForegroundColor Cyan
    Read-Host "Press Enter to exit"
    exit 1
}

Write-Host "Using compiler: $gccPath" -ForegroundColor Cyan
Write-Host "Compiling..." -ForegroundColor Yellow

# Set up build parameters using portable paths
$source = "src\main.cpp"
$output = "src\output\main.exe"
$includes = "-Iinclude\raylib"
$libs = "-Llib"
$linkerFlags = "-lraylib", "-lopengl32", "-lgdi32", "-lwinmm"
$compilerFlags = "-Wall", "-Wextra", "-std=c++17"

# Compile using local/portable paths
& $gccPath $source -o $output $includes $libs $linkerFlags $compilerFlags

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "✅ Build successful!" -ForegroundColor Green
    Write-Host "Executable created at: src\output\main.exe" -ForegroundColor Cyan
    Write-Host "Run with: .\run.bat or src\output\main.exe" -ForegroundColor Cyan
} else {
    Write-Host ""
    Write-Host "❌ Build failed!" -ForegroundColor Red
    Write-Host "Check the error messages above for details." -ForegroundColor Red
}

Read-Host "Press Enter to continue"
