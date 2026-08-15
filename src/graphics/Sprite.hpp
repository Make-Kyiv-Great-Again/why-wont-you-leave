#pragma once
#include "raylib.h"
#include <string>

struct Sprite {
    std::string texturePath;
    Rectangle sourceRect = { 0.0f, 0.0f, 0.0f, 0.0f };
    Color tint = WHITE;

    // Animation ready fields
    bool isAnimated = false;
    int frameCount = 1;
    int currentFrame = 0;
    float frameTime = 0.1f;
    float timer = 0.0f;

    bool IsValid() const { return !texturePath.empty(); }

    void Update(float dt);
    void Draw(Rectangle destRect) const;
};
