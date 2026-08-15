#pragma once
#include "scenes/Scene.hpp"
#include "raylib.h"
#include <string>

class ActTitleScene : public Scene {
public:
    ActTitleScene(int actNumber, const std::string& targetScene = "corridor");
    ~ActTitleScene() override = default;

    void Update(float dt) override;
    void Draw() override;

private:
    int actNumber;
    std::string targetScene;
    float timer = 0.0f;
    const float duration = 3.0f;
};
