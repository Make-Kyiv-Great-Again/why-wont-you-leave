#pragma once
#include "scenes/Scene.hpp"
#include "entities/Player.hpp"
#include "entities/InteractableItem.hpp"
#include "raylib.h"
#include <string>
#include <vector>

class RoomScene : public Scene {
public:
    RoomScene(int roomId, float spawnPlayerX = 270.0f);

    void Update(float dt) override;
    void Draw() override;

private:
    void DrawHoldQGauge(Vector2 centerPos, float progress, bool isRemembering);

    int roomId;
    Player player;
    Rectangle exitDoor;
    std::vector<InteractableItem> items;
    std::string promptText;
    float holdQTimer = 0.0f;
    const float holdQThreshold = 1.2f;
    InteractableItem* activeHoverItem = nullptr;
    float screenWidth;
    float screenHeight;
    float groundY;
};
