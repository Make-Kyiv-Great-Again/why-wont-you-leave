#pragma once
#include "raylib.h"
#include <string>

enum class PlayerState {
    Idle,
    Walking
};

class Player {
public:
    Rectangle rect;
    float speed;
    Color color;

    PlayerState state = PlayerState::Idle;
    bool facingRight = true;
    int currentFrame = 0;
    float frameTimer = 0.0f;

    Player(float x = 200.0f, float groundY = 660.0f);
    ~Player();

    void Update(float dt, float screenWidth);
    void ForceMove(float dx, float dt);
    void Draw() const;

private:
    float idleFrameSpeed = 0.12f;
    float walkFrameSpeed = 0.08f;
    int idleFrameCount = 5;
    int walkFrameCount = 8;
    float frameWidth = 32.0f;
    float frameHeight = 48.0f;
};
