#include "scenes/TestScene.hpp"
#include "scenes/SecondScene.hpp"
#include "core/SceneManager.hpp"
#include <cmath>

GlowingSquare::GlowingSquare() {
    position = raylib::Vector2(GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f);
    color = raylib::Color(255, 255, 0, 255);
}

void GlowingSquare::Update(float deltaTime) {
    float pulse = (std::sin(GetTime() * 4.0f) + 1.0f) * 0.5f;
    position = raylib::Vector2(
        GetScreenWidth() / (2.0f + std::sin(GetTime() * 1.0f) * 0.9),
        GetScreenHeight() / (2.0f + std::cos(GetTime() * 2.7f) * 0.9)
    );
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
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        SceneManager::Get().TransitionTo(std::make_unique<SecondScene>());
    }
}

void TestScene::Draw() {
    // 1. Render to Texture
    renderTarget.BeginMode();
    {
        ClearBackground(raylib::Color(10, 10, 20));
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
