#pragma once
#include <memory>
#include "scenes/Scene.hpp"

class SceneManager {
public:
    static SceneManager& Get();
    void ChangeScene(std::unique_ptr<Scene> newScene);
    void Update(float deltaTime);
    void Draw();

private:
    SceneManager() = default;
    std::unique_ptr<Scene> currentScene;
    std::unique_ptr<Scene> nextScene;
};
