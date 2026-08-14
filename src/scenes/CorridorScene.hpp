#pragma once
#include "scenes/Scene.hpp"
#include "entities/Player.hpp"
#include "raylib.h"
#include <string>

struct CorridorDoor {
    Rectangle rect;
    std::string label;
    int targetRoomId;
    float returnPlayerX;
};

class CorridorScene : public Scene {
public:
    CorridorScene(float spawnPlayerX = 200.0f);

    void Update(float dt) override;
    void Draw() override;

private:
    Player player;
    CorridorDoor doors[3];
    std::string promptText;
    float screenWidth;
    float screenHeight;
    float groundY;
};
