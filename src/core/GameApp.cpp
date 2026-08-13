#include "core/GameApp.hpp"
#include "core/SceneManager.hpp"
#include "scenes/TestScene.hpp"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

GameApp::GameApp(int width, int height, const char* title)
    : window(width, height, title) {
    SetTargetFPS(60);
    
    // Initialize the starting scene
    SceneManager::Get().ChangeScene(std::make_unique<TestScene>());
}

void GameApp::UpdateDrawFrame() {
    float dt = GetFrameTime();
    
    // Update
    SceneManager::Get().Update(dt);
    
    // Draw
    window.BeginDrawing();
    SceneManager::Get().Draw();
    window.EndDrawing();
}

void GameApp::Run() {
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop_arg([](void* arg) {
        static_cast<GameApp*>(arg)->UpdateDrawFrame();
    }, this, 0, 1);
#else
    while (!window.ShouldClose()) {
        UpdateDrawFrame();
    }
#endif
}
