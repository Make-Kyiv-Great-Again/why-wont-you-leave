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
    if (!currentScene) return;

    if (tabTransition > 0.0f && isBufferInitialized) {
        // 1. Draw scene to full-size sceneBuffer
        BeginTextureMode(sceneBuffer);
        ClearBackground(RAYWHITE);
        currentScene->Draw();
        EndTextureMode();

        // 2. Draw sceneBuffer downscaled to blurBuffer
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

        // 3. Draw full-size sharp scene on screen
        DrawTexturePro(
            sceneBuffer.texture,
            Rectangle{ 0, 0, (float)sceneBuffer.texture.width, -(float)sceneBuffer.texture.height },
            Rectangle{ 0, 0, (float)screenWidth, (float)screenHeight },
            Vector2{ 0, 0 },
            0.0f,
            WHITE
        );

        // 4. Draw blurry blurBuffer scaled up and blended over screen
        DrawTexturePro(
            blurBuffer.texture,
            Rectangle{ 0, 0, (float)blurBuffer.texture.width, -(float)blurBuffer.texture.height },
            Rectangle{ 0, 0, (float)screenWidth, (float)screenHeight },
            Vector2{ 0, 0 },
            0.0f,
            Fade(WHITE, tabTransition)
        );

        // 5. Draw dark dimming overlay
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, tabTransition * 0.94f));

        // 6. Draw mysterious Memory Overlay directly to screen space (sharp & colorful)
        MemoryManager::Get().DrawMemoryInventoryOverlay(tabTransition);
    } else {
        // Tab not pressed: Draw normally directly to the screen
        currentScene->Draw();
    }
}
