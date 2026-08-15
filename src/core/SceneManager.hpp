#pragma once
#include <memory>
#include "scenes/Scene.hpp"
#include "raylib.h"

class SceneManager {
public:
    static SceneManager& Get();

    void Init(int width, int height);
    void ChangeScene(std::unique_ptr<Scene> newScene);
    void OpenMainMenu();
    void ResumeGame();
    bool HasSavedGameplayScene() const { return savedGameplayScene != nullptr; }
    void ClearSavedGameplayScene() { savedGameplayScene = nullptr; }

    void Update(float dt);
    void Draw();

    void SetTabPressed(bool pressed);
    float GetTabTransition() const { return tabTransition; }
    Vector2 GetVirtualMousePosition() const;

private:
    SceneManager();
    ~SceneManager();
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    std::unique_ptr<Scene> currentScene;
    std::unique_ptr<Scene> nextScene;
    std::unique_ptr<Scene> savedGameplayScene;

    RenderTexture2D sceneBuffer;
    RenderTexture2D blurBuffer;
    bool isBufferInitialized = false;
    bool tabPressed = false;
    float tabTransition = 0.0f;
    int screenWidth = 2000;
    int screenHeight = 800;
};
