#include "core/SceneManager.hpp"
#include <raylib.h>

SceneManager::SceneManager() {
}

SceneManager& SceneManager::Get() {
    static SceneManager instance;
    return instance;
}

void SceneManager::ChangeScene(std::unique_ptr<Scene> newScene) {
    nextScene = std::move(newScene);
}

void SceneManager::TransitionTo(std::unique_ptr<Scene> newScene) {
    nextScene = std::move(newScene);
    isTransitioning = true;
    transitionProgress = 0.0f;
    sceneSwapped = false;
}

void SceneManager::Update(float deltaTime) {
    if (isTransitioning) {
        transitionProgress += deltaTime / transitionDuration;
        
        if (transitionProgress >= 0.5f && !sceneSwapped) {
            currentScene = std::move(nextScene);
            sceneSwapped = true;
        }
        
        if (transitionProgress >= 1.0f) {
            transitionProgress = 1.0f;
            isTransitioning = false;
        }
    } else {
        if (nextScene) {
            currentScene = std::move(nextScene);
        }
    }

    if (currentScene) {
        currentScene->Update(deltaTime);
    }
}

void SceneManager::Draw() {
    if (currentScene) {
        currentScene->Draw();
    }
    
    if (isTransitioning) {
        float p = transitionProgress < 0.5f ? (transitionProgress * 2.0f) : (1.0f - (transitionProgress - 0.5f) * 2.0f);
        
        // Easing for punchy effect
        p = p * p * (3.0f - 2.0f * p); 
        
        int columns = 30;
        int rows = 20;
        float cellWidth = (float)GetScreenWidth() / columns;
        float cellHeight = (float)GetScreenHeight() / rows;
        
        for (int y = 0; y < rows; y++) {
            for (int x = 0; x < columns; x++) {
                // To avoid floating point gaps, add slightly to width/height
                float blockW = cellWidth * p + 1.0f;
                float blockH = cellHeight * p + 1.0f;
                
                float centerX = x * cellWidth + cellWidth / 2.0f;
                float centerY = y * cellHeight + cellHeight / 2.0f;
                
                DrawRectangle(
                    (int)(centerX - blockW / 2.0f),
                    (int)(centerY - blockH / 2.0f),
                    (int)blockW,
                    (int)blockH,
                    raylib::Color::Black()
                );
            }
        }
    }
}
