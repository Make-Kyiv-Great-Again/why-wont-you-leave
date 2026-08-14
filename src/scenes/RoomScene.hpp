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
    int roomId;
    Player player;
    Rectangle exitDoor;
    std::vector<InteractableItem> items;
    std::string promptText;
    float screenWidth;
    float screenHeight;
    float groundY;
};
