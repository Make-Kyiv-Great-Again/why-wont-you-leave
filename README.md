# Why Won't You Leave?

> A psychological mystery and atmospheric narrative adventure built in C++17 with [Raylib](https://www.raylib.com/).

---

## 🎮 Quick Play (No Installation / No Compilers Needed)

Ready-to-play pre-built packages for Windows and macOS:

### 🪟 Windows
1. Download / extract **`WhyWontYouLeaveWindows.zip`**.
2. Double-click **`RaylibGame.exe`**.
3. *Enjoy the game!*

### 🍏 macOS
1. Download / extract **`WhyWontYouLeaveMac.zip`**.
2. Double-click **`Play_Game.command`** (or open Terminal in the folder and run `./RaylibGame`).
3. *(If macOS Gatekeeper asks, right-click `Play_Game.command` and choose **Open**).*

---

## 🕹️ Controls

| Key | Action |
|---|---|
| **`A` / `D`** or **`←` / `→`** | Move Left / Right |
| **`E`** | Interact / Inspect / Examine / Advance Dialogue / Open Door |
| **`Q` (Hold)** | Archive Memory into TAB inventory (Max 5 memories) |
| **`TAB`** | Open / Close Memory Archive |
| **`ESC`** | Pause Menu |
| **`Space` / `Enter` / `Click`** | Fast-forward text / Confirm selection |

---

## 📖 Story & Features
- **Atmospheric Narrative**: Explore the rooms, reconstruct fragmented memories, and uncover the truth behind Evan and Grace.
- **4 Branching Narrative Endings**: Your choices and the memories you preserve directly determine Evan's fate.
- **Custom Shaders & Audio**: Dynamic lighting, pixelated memory dissolving, corrupted memory shaders, and an original soundtrack.

---

## 🛠️ Building from Source (For Developers)

### Prerequisites
- CMake (3.14+)
- C++17 compliant compiler (`clang`, `gcc`, or `MSVC`)
- Standard build tools

### macOS / Linux
```bash
# 1. Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# 2. Run the game
./build/RaylibGame
```

### Windows (PowerShell / Command Prompt)
```powershell
# 1. Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# 2. Run the game
.\build\Release\RaylibGame.exe
```

### WebAssembly (Play in Browser)
```bash
# Prerequisites: Emscripten SDK (emsdk) activated
emcmake cmake -B build_web
cmake --build build_web

# Serve locally
cd build_web
python3 -m http.server 8080
```
Open `http://localhost:8080/RaylibGame.html` in your browser.
