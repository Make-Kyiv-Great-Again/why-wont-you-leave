#pragma once
#include "scenes/Scene.hpp"
#include "entities/Player.hpp"
#include "entities/InteractableItem.hpp"
#include "raylib.h"
#include <string>
#include <vector>

struct DoorData {
    Rectangle rect;
    std::string label;
    std::string targetScene;
    float targetSpawnX;
    Color doorColor;
    Color borderColor;
};

class DynamicScene : public Scene {
public:
    DynamicScene(const std::string& sceneId, float spawnPlayerX = 200.0f);
    ~DynamicScene() override = default;

    void LoadFromConfigFile(const std::string& path);
    void Update(float dt) override;
    void Draw() override;

private:
    void DrawHoldQGauge(Vector2 centerPos, float progress, bool isRemembering);

    std::string sceneId;
    std::string jsonPath;
    std::string title;
    float screenWidth;
    float screenHeight;
    float groundY;
    Color backgroundColor;
    std::string controlsHint;
    std::string promptText;

    Player player;
    std::vector<DoorData> doors;
    std::vector<InteractableItem> items;

    InteractableItem* activeHoverItem = nullptr;
    float holdQTimer = 0.0f;
    const float holdQThreshold = 1.0f;

    Shader sceneShader = { 0 };
    std::string shaderPath;
};
