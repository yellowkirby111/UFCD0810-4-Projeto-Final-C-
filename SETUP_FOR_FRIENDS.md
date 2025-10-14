# 🚀 Setup Rápido para Amigos

## ⚡ Instalação em 3 passos (2 minutos!)

### 1️⃣ Baixe este projeto
- **Clone/Download** este repositório para seu PC
- Extraia se necessário

### 2️⃣ Instale um compilador GCC (se não tiver)
Escolha **UMA** opção:

#### 🌟 **WinLibs MinGW-w64** (Recomendado - mais fácil)
1. Vá em: https://winlibs.com/
2. Baixe a versão **UCRT** mais recente
3. Extraia em `C:\mingw64\` (ou qualquer pasta)
4. Adicione `C:\mingw64\bin` ao PATH do Windows

#### 🔧 **TDM-GCC** (Alternativa simples)
1. Vá em: https://jmeubank.github.io/tdm-gcc/
2. Baixe e instale (instala automaticamente)

#### 📋 **Outras opções MinGW**
- MSYS2: https://www.msys2.org/
- MinGW original: http://www.mingw.org/

### 3️⃣ Compile e rode!
1. **Duplo-clique** em `build.bat` OU abra terminal na pasta e digite `build.bat`
2. Se compilar com sucesso, **duplo-clique** em `run.bat`
3. **Pronto!** A aplicação deve abrir! 🎮

## 🆘 Se der erro?

### ❌ "Could not find g++ compiler!"
- Você precisa instalar um compilador GCC (passo 2 acima)
- Verifique se adicionou ao PATH (para WinLibs)

### ❌ "Build failed!"
- Verifique se o compilador está funcionando: abra CMD e digite `g++ --version`
- Se não funcionar, reinstale o compilador ou adicione ao PATH

### ✅ **Compilou mas não roda?**
- Verifique se há antivírus bloqueando o `.exe`
- Tente executar `src\output\main.exe` diretamente

## 💡 Dicas

### Para VS Code (opcional):
1. Abra a pasta do projeto no VS Code
2. Instale a extensão "C/C++" da Microsoft
3. Use `Ctrl+Shift+B` para compilar
4. Use `F5` para debug/executar

### Para desenvolvimento:
- ✨ Tudo já está configurado e portátil!
- ✨ Raylib headers em `include/raylib/`
- ✨ Biblioteca em `lib/libraylib.a`
- ✨ Código principal em `src/main.cpp`

## 🎯 **É isso! Agora vocês podem colaborar facilmente!**

### 🔄 Para colaborar:
1. Modifique o código em `src/main.cpp`
2. Execute `build.bat` para compilar
3. Execute `run.bat` para testar
4. Commit/push para GitHub para compartilhar

**Nenhuma configuração específica necessária! 🚀**