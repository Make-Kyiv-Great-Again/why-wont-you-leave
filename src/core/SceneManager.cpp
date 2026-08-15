#include "core/SceneManager.hpp"
#include "core/MemoryManager.hpp"
#include <cmath>

SceneManager& SceneManager::Get() {
    static SceneManager instance;
    return instance;
}

SceneManager::SceneManager() {}

SceneManager::~SceneManager() {
    if (isBufferInitialized) {
        UnloadRenderTexture(sceneBuffer);
        UnloadRenderTexture(blurBuffer);
    }
}

void SceneManager::Init(int width, int height) {
    screenWidth = width;
    screenHeight = height;

    if (isBufferInitialized) {
        UnloadRenderTexture(sceneBuffer);
        UnloadRenderTexture(blurBuffer);
    }

    sceneBuffer = LoadRenderTexture(screenWidth, screenHeight);
    blurBuffer = LoadRenderTexture(screenWidth / 5, screenHeight / 5);
    
    SetTextureFilter(sceneBuffer.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(blurBuffer.texture, TEXTURE_FILTER_BILINEAR);
    
    isBufferInitialized = true;
}

void SceneManager::ChangeScene(std::unique_ptr<Scene> newScene) {
    nextScene = std::move(newScene);
}

void SceneManager::SetTabPressed(bool pressed) {
    tabPressed = pressed;
}

void SceneManager::Update(float dt) {
    // Smooth transition for holding Tab
    if (tabPressed) {
        tabTransition += dt * 4.0f; // Transition in 0.25 seconds
        if (tabTransition > 1.0f) tabTransition = 1.0f;
    } else {
        tabTransition -= dt * 4.0f; // Transition out 0.25 seconds
        if (tabTransition < 0.0f) tabTransition = 0.0f;
    }

    if (nextScene) {
        currentScene = std::move(nextScene);
        nextScene = nullptr;
    }

    if (currentScene) {
        currentScene->Update(dt);
    }
}

void SceneManager::Draw() {
    if (!currentScene || !isBufferInitialized) return;

    // 1. Always render scene to virtual resolution sceneBuffer (2000x800)
    BeginTextureMode(sceneBuffer);
    ClearBackground(RAYWHITE);
    currentScene->Draw();
    EndTextureMode();

    // 2. If holding TAB, render Memory Overlay on top of sceneBuffer
    if (tabTransition > 0.0f) {
        // Downscale sceneBuffer to blurBuffer
        BeginTextureMode(blurBuffer);
        ClearBackground(BLACK);
        DrawTexturePro(
            sceneBuffer.texture,
            Rectangle{ 0, 0, (float)sceneBuffer.texture.width, -(float)sceneBuffer.texture.height },
            Rectangle{ 0, 0, (float)blurBuffer.texture.width, (float)blurBuffer.texture.height },
            Vector2{ 0, 0 },
            0.0f,
            WHITE
        );
        EndTextureMode();

        // Render blur & overlay into sceneBuffer
        BeginTextureMode(sceneBuffer);
        DrawTexturePro(
            blurBuffer.texture,
            Rectangle{ 0, 0, (float)blurBuffer.texture.width, -(float)blurBuffer.texture.height },
            Rectangle{ 0, 0, (float)sceneBuffer.texture.width, (float)sceneBuffer.texture.height },
            Vector2{ 0, 0 },
            0.0f,
            Fade(WHITE, tabTransition)
        );
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, tabTransition * 0.94f));
        MemoryManager::Get().DrawMemoryInventoryOverlay(tabTransition);
        EndTextureMode();
    }

    // 3. Draw sceneBuffer to window screen, automatically scaled & letterboxed for display resolution
    int windowW = GetScreenWidth();
    int windowH = GetScreenHeight();

    float scale = fminf((float)windowW / (float)screenWidth, (float)windowH / (float)screenHeight);
    float destW = screenWidth * scale;
    float destH = screenHeight * scale;
    float destX = (windowW - destW) * 0.5f;
    float destY = (windowH - destH) * 0.5f;

    ClearBackground(BLACK);

    DrawTexturePro(
        sceneBuffer.texture,
        Rectangle{ 0, 0, (float)sceneBuffer.texture.width, -(float)sceneBuffer.texture.height },
        Rectangle{ destX, destY, destW, destH },
        Vector2{ 0, 0 },
        0.0f,
        WHITE
    );
}
