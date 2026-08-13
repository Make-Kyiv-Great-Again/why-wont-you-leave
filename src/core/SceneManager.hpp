#pragma once
#include <memory>
#include <raylib-cpp.hpp>
#include "scenes/Scene.hpp"

class SceneManager {
public:
    static SceneManager& Get();
    void ChangeScene(std::unique_ptr<Scene> newScene);
    void TransitionTo(std::unique_ptr<Scene> newScene);
    void Update(float deltaTime);
    void Draw();

private:
    SceneManager();
    std::unique_ptr<Scene> currentScene;
    std::unique_ptr<Scene> nextScene;
    
    bool isTransitioning = false;
    float transitionProgress = 0.0f;
    float transitionDuration = 0.5f; // half second transition
    bool sceneSwapped = false;
};
