Write-Host "Building Raylib project..." -ForegroundColor Green

$compiler = "C:\Users\a25633\AppData\Local\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin\g++.exe"
$source = "src\main.cpp"
$output = "src\output\main.exe"
$includes = "-IC:\raylib\include"
$libs = "-LC:\raylib\lib"
$linkerFlags = "-lraylib", "-lopengl32", "-lgdi32", "-lwinmm"
$compilerFlags = "-Wall", "-Wextra", "-std=c++17"

& $compiler $source -o $output $includes $libs $linkerFlags $compilerFlags

if ($LASTEXITCODE -eq 0) {
    Write-Host "Build successful!" -ForegroundColor Green
} else {
    Write-Host "Build failed!" -ForegroundColor Red
}