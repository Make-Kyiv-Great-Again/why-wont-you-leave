#include "raylib.h"
#include "core/SceneManager.hpp"
#include "scenes/DynamicScene.hpp"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

void UpdateDrawFrame() {
    float dt = GetFrameTime();

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

    // Fallback default size
    InitWindow(1280, 512, "2D Room Navigation Game");

    // Automatically adapt initial window size to fit user's monitor screen
    int monitor = GetCurrentMonitor();
    int monitorWidth = GetMonitorWidth(monitor);
    int monitorHeight = GetMonitorHeight(monitor);

    if (monitorWidth > 0 && monitorHeight > 0) {
        int winWidth = (int)(monitorWidth * 0.75f);
        int winHeight = (int)(winWidth * ((float)virtualHeight / (float)virtualWidth));

        // Ensure window height fits comfortably within screen boundaries (e.g. 80% max)
        if (winHeight > (int)(monitorHeight * 0.80f)) {
            winHeight = (int)(monitorHeight * 0.80f);
            winWidth = (int)(winHeight * ((float)virtualWidth / (float)virtualHeight));
        }

        SetWindowSize(winWidth, winHeight);
        SetWindowPosition((monitorWidth - winWidth) / 2, (monitorHeight - winHeight) / 2);
    }

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

    CloseWindow();
    return 0;
}
