#pragma once
#include "scenes/Scene.hpp"
#include "entities/Player.hpp"
#include "entities/InteractableItem.hpp"
#include "graphics/Sprite.hpp"
#include "raylib.h"
#include <string>
#include <vector>

struct DoorData {
    Rectangle rect;
    std::string label;
    std::string targetScene;
    std::string targetDoor;
    float targetSpawnX;
    Color doorColor;
    Color borderColor;
    Sprite sprite;
    bool isVisible = true;
};

class DynamicScene : public Scene {
public:
    DynamicScene(const std::string& sceneId, float spawnPlayerX = -1.0f, const std::string& targetDoorLabel = "");
    ~DynamicScene() override;

    void LoadFromConfigFile(const std::string& path);
    void Update(float dt) override;
    void Draw() override;

private:
    void DrawHoldQGauge(Vector2 centerPos, float progress, bool isRemembering);
    void DrawInteractionPrompt();
    void UpdatePauseMenu(float dt);
    void DrawPauseOverlay();

    std::string sceneId;
    std::string jsonPath;
    std::string title;
    float screenWidth;
    float screenHeight;
    float groundY;
    Color backgroundColor;
    std::string backgroundTexturePath;
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

    // Player Lighting Shader
    Shader playerLightingShader = { 0 };
    int ambientColorLoc = -1;
    int brightnessLoc = -1;
    int warmthLoc = -1;
    Vector3 ambientColor = { 1.0f, 1.0f, 1.0f };
    float playerBrightness = 1.0f;
    float playerWarmth = 1.0f;
    bool hasPlayerShader = false;

    // Sparkle Timer
    float sparkleTimer = 0.0f;

    // Pause system
    bool isPaused = false;
    int pauseSelectedBtn = 0; // 0: Options, 1: Exit
    bool showPauseOptions = false;
    Rectangle pauseOptionsRect;
    Rectangle pauseExitRect;

    // Exit door repulsion & pain distortion sequence
    bool isExitDistortionActive = false;
    float exitDistortionTimer = 0.0f;
    const float exitDistortionMaxTime = 1.8f;
    std::string pendingExitDialogue = "";
    bool pendingExitIsSceneChange = false;
    std::string pendingExitTargetScene = "";

    // 5th memory corridor blackout fade transition
    bool isCorridorTransitioning = false;
    float corridorTransitionTimer = 0.0f;
    float corridorTransitionAlpha = 0.0f;
};
