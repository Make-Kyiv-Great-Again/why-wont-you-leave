#include "raylib.h"
#include "core/SceneManager.hpp"
#include "scenes/CorridorScene.hpp"

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
    const int screenWidth = 2000;
    const int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "2D Room Navigation Game");

    SceneManager::Get().ChangeScene(std::make_unique<CorridorScene>());

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
