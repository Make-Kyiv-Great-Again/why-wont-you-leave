#pragma once
#include "scenes/Scene.hpp"
#include "entities/Entity.hpp"
#include "graphics/PostProcessor.hpp"
#include <raylib-cpp.hpp>
#include <memory>

class GlowingSquare : public Entity {
public:
    GlowingSquare();
    void Update(float deltaTime) override;
    void Draw() override;
private:
    raylib::Vector2 position;
    raylib::Color color;
};

class TestScene : public Scene {
public:
    TestScene();
    void Update(float deltaTime) override;
    void Draw() override;
private:
    std::unique_ptr<GlowingSquare> square;
    raylib::RenderTexture2D renderTarget;
    std::unique_ptr<PostProcessor> bloomProcessor;
};
