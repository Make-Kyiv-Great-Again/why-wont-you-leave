#pragma once
#include "scenes/Scene.hpp"
#include "raylib.h"

class IntroScene : public Scene {
public:
    IntroScene();
    ~IntroScene() override;

    void Update(float dt) override;
    void Draw() override;

private:
    Sound introSound;
    float timer = 0.0f;
    float duration = 0.0f;
};
