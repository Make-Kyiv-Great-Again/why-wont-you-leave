#include "core/SceneManager.hpp"

SceneManager& SceneManager::Get() {
    static SceneManager instance;
    return instance;
}

void SceneManager::ChangeScene(std::unique_ptr<Scene> newScene) {
    nextScene = std::move(newScene);
}

void SceneManager::Update(float deltaTime) {
    if (nextScene) {
        currentScene = std::move(nextScene);
    }
    if (currentScene) {
        currentScene->Update(deltaTime);
    }
}

void SceneManager::Draw() {
    if (currentScene) {
        currentScene->Draw();
    }
}
