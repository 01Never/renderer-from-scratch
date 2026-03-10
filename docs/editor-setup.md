# Editor Setup

## VS Code + clangd (IntelliSense)

### Required Extensions

- **clangd** (`llvm-vs-code-extensions.vscode-clangd`) — C/C++ language server providing autocomplete, go-to-definition, and error diagnostics.

> Disable the built-in **C/C++ IntelliSense** (`ms-vscode.cpptools`) if installed, as it conflicts with clangd.

### How It Works

clangd reads `build/compile_commands.json` to understand exactly how each file is compiled — include paths, defines, language standard, etc. CMake generates this file automatically because `CMakeLists.txt` sets:

```cmake
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
```

### The Problem with This Project

GCC (MinGW64) automatically searches its own prefix's include directories (e.g. `G:\Programs\msys64\mingw64\include`), so the build succeeds without an explicit `-I` flag. However, clangd does **not** get this implicit search path, so it fails to find `<SDL3/SDL.h>` even when the build works fine.

### The Fix — `.clangd`

The `.clangd` file at the project root patches the compile flags that clangd uses:

```yaml
CompileFlags:
  CompilationDatabase: build
  Add:
    - "-IG:/Programs/msys64/mingw64/include"
    - "--target=x86_64-w64-mingw32"
```

| Flag | Purpose |
|------|---------|
| `-IG:/Programs/msys64/mingw64/include` | Tells clangd where to find SDL3 (and all other MinGW64) headers |
| `--target=x86_64-w64-mingw32` | Tells clangd it's compiling for MinGW, not MSVC — ensures Windows types and macros resolve correctly |

### Setup Steps

1. Install the **clangd** extension in VS Code.
2. Run the CMake configure step to generate `compile_commands.json`:
   ```bash
   cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
   ```
3. Open the project folder in VS Code (the `.clangd` file is picked up automatically).
4. If clangd was already running: **Ctrl+Shift+P** → **clangd: Restart language server**.

### If You Move MSYS2

If MSYS2 is installed at a different path, update the `-I` path in `.clangd` to match.
