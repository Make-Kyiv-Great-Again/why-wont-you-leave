#include "scenes/IntroScene.hpp"
#include "core/SceneManager.hpp"
#include "core/ResourceManager.hpp"
#include "scenes/DynamicScene.hpp"

IntroScene::IntroScene() {
    introSound = ResourceManager::Get().GetSound("assets/sounds/intro_sound.MP3");
    if (introSound.frameCount > 0) {
        PlaySound(introSound);
        duration = (float)introSound.frameCount / (float)introSound.stream.sampleRate;
    } else {
        duration = 5.0f; // fallback if sound fails
    }
}

IntroScene::~IntroScene() {
    if (introSound.frameCount > 0) {
        StopSound(introSound);
    }
}

void IntroScene::Update(float dt) {
    timer += dt;
    
    bool skipPressed = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_E);
    bool soundFinished = false;
    
    if (introSound.frameCount > 0) {
        soundFinished = !IsSoundPlaying(introSound) && (timer > 0.5f);
    } else {
        soundFinished = (timer >= duration);
    }

    if (skipPressed || soundFinished) {
        if (introSound.frameCount > 0) {
            StopSound(introSound);
        }
        SceneManager::Get().ChangeScene(std::make_unique<DynamicScene>("bedroom"));
    }
}

void IntroScene::Draw() {
    ClearBackground(BLACK);

    const char* skipHint = "Press [SPACE / ENTER] to Skip";
    int fontSize = 28;
    int textW = ResourceManager::MeasureGameText(skipHint, fontSize);

    float x = 2000.0f - textW - 40.0f;
    float y = 800.0f - fontSize - 40.0f;

    float pulse = (sinf(timer * 4.0f) + 1.0f) * 0.5f;
    Color hintColor = Fade(GOLD, 0.65f + pulse * 0.35f);

    ResourceManager::DrawGameText(skipHint, x, y, fontSize, hintColor);
}
