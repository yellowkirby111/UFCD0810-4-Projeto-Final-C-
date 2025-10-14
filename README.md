# UFCD0810-4-Projeto-Final-C- 🚀

**Projeto final de C++ para a UFCD0810 usando Raylib - VERSÃO PORTÁTIL**

## 🎮 Clothing Store Application

Este projeto implementa um menu gráfico para uma loja de roupas usando Raylib em `src/main.cpp`.

### ✨ Funcionalidades:
- Menu principal com View Products / Add Product / Options / Exit
- Navegação por teclado (Up/Down + Enter) e mouse
- Interface gráfica limpa e responsiva
- Sistema completo de estados
- Placeholders para funcionalidades futuras

## 📋 Pré-requisitos (PORTÁTIL - sem instalação!)

**APENAS UM COMPILADOR GCC É NECESSÁRIO:**
- ✅ MinGW-w64, TDM-GCC, ou qualquer distribuição GCC
- ✅ Visual Studio Code (opcional, mas recomendado)
- ❌ **NÃO precisa** instalar Raylib - já incluído!
- ❌ **NÃO precisa** configurar paths específicos!

## 🚀 INSTALAÇÃO RÁPIDA (Portátil)

### Para você e seus amigos:
1. **Baixe/Clone** este repositório
2. **Instale um compilador GCC** (se não tiver):
   - [WinLibs MinGW-w64](https://winlibs.com/) (recomendado)
   - [TDM-GCC](https://jmeubank.github.io/tdm-gcc/)
   - Ou qualquer distribuição MinGW
3. **Execute:** `build.bat` (duplo-clique ou terminal)
4. **Rode:** `run.bat` ou `src\output\main.exe`

### 📝 O script de build detecta automaticamente:
- ✅ GCC no PATH do sistema
- ✅ WinLibs (instalação comum)
- ✅ MinGW em locais padrão
- ✅ TDM-GCC
- 📝 **Guia claro se não encontrar!**

## 💻 Como usar no VS Code

### ✨ Configuração automática já incluída e portátil:
- **IntelliSense** configurado para Raylib (paths relativos)
- **Build tasks** para compilação (detecta compilador automaticamente)
- **Debug/Run** configurado (funciona em qualquer máquina)
- **Problem matcher** para erros

### ⚙️ Comandos VS Code:
1. **Compilar:** `Ctrl+Shift+B`
2. **Executar/Debug:** `F5`
3. **Escolher task:** `Ctrl+Shift+P` → "Tasks: Run Task"

## 🔧 Build Scripts (Portáteis)

### 🎆 Opção 1: Batch Script (Recomendado)
```batch
build.bat
```
**Detecta automaticamente o compilador e usa paths relativos!**

### 🔋 Opção 2: PowerShell Script
```powershell
PowerShell -ExecutionPolicy Bypass -File build.ps1
```
**Versão PowerShell com detecção inteligente de GCC!**

### ⚙️ Opção 3: Compilação Manual (se necessário)
```bash
g++ src/main.cpp -o src/output/main.exe -Iinclude/raylib -Llib -lraylib -lopengl32 -lgdi32 -lwinmm -Wall -Wextra -std=c++17
```

## 🏃‍♂️ Executar o Programa

### Após compilar (build.bat):
```batch
run.bat
```
**ou diretamente:**
```batch
src\output\main.exe
```

## 📱 Estrutura do Projeto (Portátil)

```
UFCD0810-4-Projeto-Final-C-/
├── .vscode/                    # 🔧 Configurações VS Code (portáteis)
│   ├── c_cpp_properties.json    # IntelliSense com paths relativos
│   ├── tasks.json               # Build tasks inteligentes
│   ├── launch.json              # Debug configurado
│   └── settings.json            # Configurações do workspace
├── include/                    # 📚 Headers portáteis
│   ├── raylib/                 # 🎮 Headers Raylib incluídos!
│   │   ├── raylib.h
│   │   ├── raymath.h
│   │   ├── rcamera.h
│   │   └── rlgl.h
│   └── func.h                  # Headers customizados
├── lib/                        # 📦 Bibliotecas portáteis
│   ├── libraylib.a             # 🎮 Biblioteca Raylib!
│   └── pkgconfig/
├── src/
│   ├── main.cpp                # 🎨 Programa principal (C++)
│   └── output/
│       └── main.exe            # 🚀 Executável gerado
├── build.bat                   # 🔨 Script build inteligente
├── build.ps1                  # 💻 Script PowerShell alternativo
├── run.bat                    # ▶️ Script para executar
├── raylib-project.code-workspace  # 📝 Workspace VS Code
└── README.md                  # 📜 Este arquivo!
```

## 🏆 Status - VERSÃO PORTÁTIL COMPLETA!

✅ **Raylib incluído no projeto (não precisa instalar!)**  
✅ **VS Code completamente configurado (portátil)**  
✅ **Build scripts inteligentes (detectam compilador)**  
✅ **IntelliSense ativo para Raylib (paths relativos)**  
✅ **Debug/Run configurado (funciona em qualquer PC)**  
✅ **Pronto para compartilhar com amigos!**  

## 👥 Para Colaboração com Amigos

### 📦 O que seus amigos precisam:
1. **Apenas um compilador GCC** (MinGW, TDM-GCC, etc.)
2. **Este projeto** (clone/download)
3. **Executar:** `build.bat`

### 🚫 O que seus amigos NÃO precisam:
- ❌ Instalar Raylib separadamente
- ❌ Configurar paths específicos
- ❌ Modificar variáveis de ambiente
- ❌ Usar a mesma versão/localização do compilador

### 🎆 **O projeto está pronto para colaboração portátil!**
