#include "scenes/ActTitleScene.hpp"
#include "core/SceneManager.hpp"
#include "core/ActManager.hpp"
#include "core/ResourceManager.hpp"
#include "scenes/DynamicScene.hpp"
#include <cmath>

ActTitleScene::ActTitleScene(int actNumber, const std::string& targetScene)
    : actNumber(actNumber), targetScene(targetScene), timer(0.0f) {}

void ActTitleScene::Update(float dt) {
    timer += dt;

    bool skipPressed = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_E);
    if (timer >= duration || skipPressed) {
        SceneManager::Get().ChangeScene(std::make_unique<DynamicScene>(targetScene));
    }
}

void ActTitleScene::Draw() {
    ClearBackground(BLACK);

    float screenWidth = 2000.0f;
    float screenHeight = 800.0f;

    std::string titleText = ActManager::Get().GetActTitle();

    // Fade in during first 0.5s, fade out in last 0.5s
    float alpha = 1.0f;
    if (timer < 0.5f) {
        alpha = timer / 0.5f;
    } else if (timer > duration - 0.5f) {
        alpha = (duration - timer) / 0.5f;
    }
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;

    int fontSize = 54;
    int titleW = ResourceManager::MeasureGameText(titleText.c_str(), fontSize);
    float textX = (screenWidth - titleW) / 2.0f;
    float textY = (screenHeight - fontSize) / 2.0f;

    // Outer subtle black shadow and golden glowing title text
    ResourceManager::DrawGameText(titleText.c_str(), textX + 2.0f, textY + 2.0f, fontSize, Fade(BLACK, alpha));
    ResourceManager::DrawGameText(titleText.c_str(), textX, textY, fontSize, Fade(GOLD, alpha));
}
