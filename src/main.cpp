#include "raylib.h"
#include "core/SceneManager.hpp"
#include "scenes/DynamicScene.hpp"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

void UpdateDrawFrame() {
    float dt = GetFrameTime();

    // Toggle Fullscreen / Windowed with F11 or Alt+Enter
    if (IsKeyPressed(KEY_F11) || ((IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) && IsKeyPressed(KEY_ENTER))) {
        ToggleBorderlessWindowed();
    }

    SceneManager::Get().Update(dt);

    BeginDrawing();
    SceneManager::Get().Draw();
    EndDrawing();
}

int main() {
    // Enable High-DPI support and Resizable window flags before window initialization
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);

    const int virtualWidth = 2000;
    const int virtualHeight = 800;

    // Fallback window initialization
    InitWindow(1280, 512, "2D Room Navigation Game");

    // Query active monitor dimensions & launch in Fullscreen Borderless Mode
    int monitor = GetCurrentMonitor();
    int monitorWidth = GetMonitorWidth(monitor);
    int monitorHeight = GetMonitorHeight(monitor);

    if (monitorWidth > 0 && monitorHeight > 0) {
        SetWindowSize(monitorWidth, monitorHeight);
        SetWindowPosition(0, 0);
    }

    // Toggle into Fullscreen Borderless Mode
    ToggleBorderlessWindowed();

    InitAudioDevice(); // Initialize audio for typing sfx

    SceneManager::Get().Init(virtualWidth, virtualHeight);
    SceneManager::Get().ChangeScene(std::make_unique<DynamicScene>("corridor"));

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        UpdateDrawFrame();
    }
#endif

    CloseAudioDevice(); // Close audio device
    CloseWindow();
    return 0;
}
