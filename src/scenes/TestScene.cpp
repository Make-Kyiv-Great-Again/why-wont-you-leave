#include "scenes/TestScene.hpp"
#include <cmath>

GlowingSquare::GlowingSquare() {
    position = raylib::Vector2(GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f);
    color = raylib::Color(0, 121, 241, 255); // Blue
}

void GlowingSquare::Update(float deltaTime) {
    // Pulse logic
    float pulse = (std::sin(GetTime() * 4.0f) + 1.0f) * 0.5f; 
    color.a = static_cast<unsigned char>(150 + 105 * pulse); // Pulse alpha
}

void GlowingSquare::Draw() {
    raylib::Rectangle rect(position.x - 50.0f, position.y - 50.0f, 100.0f, 100.0f);
    rect.Draw(color);
}

TestScene::TestScene() {
    square = std::make_unique<GlowingSquare>();
    renderTarget = raylib::RenderTexture2D(GetScreenWidth(), GetScreenHeight());
#if defined(PLATFORM_WEB)
    bloomProcessor = std::make_unique<PostProcessor>("assets/shaders/glsl100/bloom.fs");
#else
    bloomProcessor = std::make_unique<PostProcessor>("assets/shaders/glsl330/bloom.fs");
#endif
}

void TestScene::Update(float deltaTime) {
    square->Update(deltaTime);
}

void TestScene::Draw() {
    // 1. Render to Texture
    renderTarget.BeginMode();
    {
        ClearBackground(raylib::Color::Black());
        square->Draw();
    }
    renderTarget.EndMode();

    // 2. Render Texture to Screen with Post-Processing
    ClearBackground(raylib::Color::Black());
    bloomProcessor->BeginPostProcess();
    {
        bloomProcessor->EndPostProcess(renderTarget);
    }
}
