#pragma once
#include <memory>
#include "scenes/Scene.hpp"

class SceneManager {
public:
    static SceneManager& Get();

    void ChangeScene(std::unique_ptr<Scene> newScene);
    void Update(float dt);
    void Draw();

private:
    SceneManager() = default;
    ~SceneManager() = default;
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    std::unique_ptr<Scene> currentScene;
    std::unique_ptr<Scene> nextScene;
};
