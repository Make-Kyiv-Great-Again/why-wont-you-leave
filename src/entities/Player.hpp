#pragma once
#include "raylib.h"

class Player {
public:
    Rectangle rect;
    float speed;
    Color color;

    Player(float x = 200.0f, float groundY = 660.0f)
        : rect{ x, groundY - 200.0f, 80.0f, 200.0f }, speed(700.0f), color(DARKBLUE) {}

    void Update(float dt, float screenWidth) {
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
            rect.x -= speed * dt;
        }
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
            rect.x += speed * dt;
        }

        // Bound within screen limits
        if (rect.x < 0) rect.x = 0;
        if (rect.x + rect.width > screenWidth) {
            rect.x = screenWidth - rect.width;
        }
    }

    void Draw() const {
        DrawRectangleRec(rect, color);
        DrawRectangleLinesEx(rect, 2, BLACK);
    }
};
