#include "core/SceneManager.hpp"

SceneManager& SceneManager::Get() {
    static SceneManager instance;
    return instance;
}

void SceneManager::ChangeScene(std::unique_ptr<Scene> newScene) {
    nextScene = std::move(newScene);
}

void SceneManager::Update(float dt) {
    if (nextScene) {
        currentScene = std::move(nextScene);
        nextScene = nullptr;
    }
    if (currentScene) {
        currentScene->Update(dt);
    }
}

void SceneManager::Draw() {
    if (currentScene) {
        currentScene->Draw();
    }
}
