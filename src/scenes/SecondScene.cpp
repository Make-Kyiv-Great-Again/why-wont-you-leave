#include "scenes/SecondScene.hpp"
#include "scenes/TestScene.hpp"
#include "core/SceneManager.hpp"
#include <algorithm>

SecondScene::SecondScene() {
    renderTarget = raylib::RenderTexture2D(GetScreenWidth(), GetScreenHeight());
}

void SecondScene::Update(float deltaTime) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        SceneManager::Get().TransitionTo(std::make_unique<TestScene>());
    }
}

void SecondScene::Draw() {
    renderTarget.BeginMode();
    {
        ClearBackground(raylib::Color::RayWhite());
        const char* text = "Menu or something???";
        int fontSize = 50;
        int textWidth = MeasureText(text, fontSize);
        DrawText(text, GetScreenWidth() / 2 - textWidth / 2, GetScreenHeight() / 2 - fontSize / 2, fontSize, BLACK);
    }
    renderTarget.EndMode();
    
    float scale = std::min((float)GetScreenWidth() / renderTarget.texture.width, 
                           (float)GetScreenHeight() / renderTarget.texture.height);
                           
    float destWidth = renderTarget.texture.width * scale;
    float destHeight = renderTarget.texture.height * scale;
    float offsetX = (GetScreenWidth() - destWidth) * 0.5f;
    float offsetY = (GetScreenHeight() - destHeight) * 0.5f;

    ClearBackground(raylib::Color::Black());
    renderTarget.GetTexture().Draw(
        raylib::Rectangle(0, 0, (float)renderTarget.texture.width, (float)-renderTarget.texture.height),
        raylib::Rectangle(offsetX, offsetY, destWidth, destHeight),
        raylib::Vector2(0, 0),
        0.0f,
        WHITE
    );
}
