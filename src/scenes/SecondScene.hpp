#pragma once
#include "scenes/Scene.hpp"
#include <raylib-cpp.hpp>

class SecondScene : public Scene {
public:
    SecondScene();
    void Update(float deltaTime) override;
    void Draw() override;

private:
    raylib::RenderTexture2D renderTarget;
};
