# Raylib C++ Boilerplate

A boilerplate C++ project using `raylib` and `raylib-cpp`.

## Features
- Terminal-only CMake workflow
- Fetches `raylib` and `raylib-cpp` automatically via `FetchContent`
- Object-Oriented C++ design (Scene, Entity, GameApp)
- Layering and Post-Processing support (Bloom shader example)

## Build Instructions

### Prerequisites
- CMake (3.14+)
- C++17 compliant compiler
- Standard build tools (make, gcc/clang/msvc)

### macOS & Windows (Terminal)

```bash
# 1. Configure the project
cmake -B build

# 2. Build the project
cmake --build build

# 3. Run the executable (The executable is usually created inside the build directory)
# macOS / Linux
./build/RaylibGame
# Windows
.\build\Debug\RaylibGame.exe
```

### WebAssembly (Emscripten)x

**Prerequisites:** You must have the [Emscripten SDK (emsdk)](https://emscripten.org/docs/getting_started/downloads.html) installed and activated in your terminal environment.

```bash
# 1. Configure the project for WebAssembly using emcmake
emcmake cmake -B build_web

# 2. Build the project
cmake --build build_web

# 3. Serve the generated files
# Browsers prevent loading local files via XHR, so you must use a local web server
cd build_web
python3 -m http.server 8080
```
Then, open your web browser and navigate to `http://localhost:8080/RaylibGame.html` to play the game!
