# UFCD0810-4-Projeto-Final-C-

Projeto final de C++ para a UFCD0810 usando Raylib.

## GUI Menu (Raylib)

Este projeto implementa um menu gráfico simples usando Raylib em `src/main.cpp`.

### Funcionalidades:
- Menu principal com Start / Options / Exit
- Navegação por teclado (Up/Down + Enter) e mouse
- Tela de jogo com animação simples
- Menu de opções com toggle fullscreen
- Sistema completo de estados

## Pré-requisitos

- ✅ Raylib instalado em `C:\raylib`
- ✅ MinGW/GCC compilador configurado
- ✅ Visual Studio Code (recomendado)

## Como usar no VS Code

### Configuração automática já incluída:
- **IntelliSense** configurado para Raylib
- **Build tasks** para compilação
- **Debug/Run** configurado
- **Problem matcher** para errors

### Comandos VS Code:
1. **Compilar:** `Ctrl+Shift+B` 
2. **Executar/Debug:** `F5`
3. **Escolher task:** `Ctrl+Shift+P` → "Tasks: Run Task"

## Build Scripts

### Opção 1: Batch Script (Windows)
```batch
build.bat
```

### Opção 2: PowerShell Script
```powershell
PowerShell -ExecutionPolicy Bypass -File build.ps1
```

### Opção 3: Compilação Manual
```bash
g++ src/main.cpp -o src/output/main.exe -IC:/raylib/include -LC:/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm -Wall -Wextra -std=c++17
```

## Executar o Programa

```batch
run.bat
```
ou
```batch
src\output\main.exe
```

## Estrutura do Projeto

```
UFCD0810-4-Projeto-Final-C-/
├── .vscode/                 # Configurações VS Code
│   ├── c_cpp_properties.json
│   ├── tasks.json
│   ├── launch.json
│   └── settings.json
├── src/
│   ├── main.cpp            # Programa principal
│   └── output/
│       └── main.exe        # Executável
├── include/
│   └── func.h
├── build.bat               # Script build (Batch)
├── build.ps1              # Script build (PowerShell)
├── run.bat                # Script para executar
├── raylib-project.code-workspace
└── README.md
```

## Status

✅ **Raylib instalado e configurado**  
✅ **VS Code completamente configurado**  
✅ **Build scripts funcionando**  
✅ **IntelliSense ativo para Raylib**  
✅ **Debug/Run configurado**  

O projeto está pronto para desenvolvimento!
