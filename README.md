# UFCD0810-4-Projeto-Final-C-
projeto final de c++ para a UFCD0810;

## GUI Menu (Raylib)

This project now includes a simple graphical menu implemented with Raylib in `src/main.cpp`.

Features:
- Main Menu with Start / Options / Exit
- Keyboard navigation (Up/Down + Enter) and mouse clicking
- Simple placeholder Play screen and Options toggle (fullscreen)

## Building on Windows

You must install Raylib and a compatible toolchain. Two common approaches:

1) MSYS2 / MinGW-w64

- Install MSYS2, then in an MSYS2 MinGW shell:

```powershell
pacman -Syu; pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-raylib
```

- Compile:

```powershell
g++ src/main.cpp -o bin\menu.exe -lraylib -lopengl32 -lgdi32 -lwinmm
```

2) Visual Studio (MSVC)

- Build raylib as a MSVC library (see raylib docs) and link the produced lib and include path in your project settings.

Notes:
- If your compiler cannot find `raylib.h`, ensure the include path points to the raylib `include` folder and the linker has the raylib library.
- The project currently doesn't include a build system (CMake/Makefile). You can add a small CMake file if desired.

Run:

```powershell
bin\menu.exe
```

If you want, I can add a CMakeLists.txt or a simple build script next.
