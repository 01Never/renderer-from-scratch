# Development Environment

## Overview

This project is a 3D software renderer written in C, built for learning purposes.

## Platform

- **OS:** Windows (x86_64)
- **Shell:** MSYS2 (MinGW64 environment)

## Toolchain

| Tool    | Version | Package (pacman)                  |
|---------|---------|-----------------------------------|
| GCC     | 15.2.0  | `mingw-w64-x86_64-gcc`            |
| CMake   | 4.2.3   | `mingw-w64-x86_64-cmake`          |
| Ninja   | 1.13.2  | (installed as CMake dependency)   |
| SDL3    | 3.4.2   | `mingw-w64-x86_64-sdl3`           |

All packages are installed under `G:\Programs\msys64\mingw64\`.

## Installing Dependencies

Open an **MSYS2 MinGW64** terminal and run:

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-sdl3
```

## Windows PATH

Add `G:\Programs\msys64\mingw64\bin` to your Windows user `PATH` so that `cmake`, `gcc`, and `ninja` are available in any terminal (PowerShell, cmd, VS Code integrated terminal) without needing to open MSYS2.

**Settings → System → Advanced system settings → Environment Variables → Path → Edit → New**

## Project Structure

```
3DRenderingLessons/
├── .clangd             # clangd configuration (IntelliSense)
├── CMakeLists.txt      # Build configuration
├── build/
│   └── compile_commands.json  # Generated — used by clangd
├── docs/
│   ├── dev-environment.md
│   ├── build-system.md
│   └── editor-setup.md
└── src/
    └── main.c          # Entry point
```

## Building

From the project root in an MSYS2 MinGW64 terminal:

```bash
# Configure (first time or after CMakeLists changes)
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build

# Run
./build/renderer.exe
```

## Language Standard

- **C11** (`-std=c11` via `CMAKE_C_STANDARD 11`)

## Libraries

- **SDL3** — window creation, input handling, and 2D rendering surface used as the display target for the software renderer.
  - Linked via CMake: `SDL3::SDL3`
  - Headers: `<SDL3/SDL.h>`
