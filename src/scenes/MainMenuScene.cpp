#include "scenes/MainMenuScene.hpp"
#include "core/SceneManager.hpp"
#include "core/ResourceManager.hpp"
#include "core/ActManager.hpp"
#include "dialogue/DialogueManager.hpp"
#include "scenes/DynamicScene.hpp"
#include "scenes/IntroScene.hpp"
#include <cmath>

MainMenuScene::MainMenuScene() {
    float screenWidth = 2000.0f;

    // Top button: New Game (large centered, matching 89x14 aspect ratio)
    float mainBtnW = 440.0f;
    float mainBtnH = 70.0f;
    newGameBtnRect = Rectangle{ (screenWidth - mainBtnW) / 2.0f, 420.0f, mainBtnW, mainBtnH };

    // Second row: Options & Exit side-by-side in one row (matching 37x13 and 28x13 aspect ratio)
    float subBtnW = 200.0f;
    float subBtnH = 70.0f;
    float gap = 40.0f;
    float rowWidth = subBtnW * 2.0f + gap;
    float startX = (screenWidth - rowWidth) / 2.0f;
    float subRowY = 530.0f;

    optionsBtnRect = Rectangle{ startX, subRowY, subBtnW, subBtnH };
    exitBtnRect    = Rectangle{ startX + subBtnW + gap, subRowY, subBtnW, subBtnH };
}

void MainMenuScene::Update(float dt) {
    // Pressing ESC inside Main Menu quits the game
    if (IsKeyPressed(KEY_ESCAPE)) {
        CloseWindow();
        return;
    }

    Vector2 virtualMouse = SceneManager::Get().GetVirtualMousePosition();

    // Check mouse hover
    if (CheckCollisionPointRec(virtualMouse, newGameBtnRect)) {
        selectedButtonIndex = 0;
    } else if (CheckCollisionPointRec(virtualMouse, optionsBtnRect)) {
        selectedButtonIndex = 1;
    } else if (CheckCollisionPointRec(virtualMouse, exitBtnRect)) {
        selectedButtonIndex = 2;
    }

    // Grid Keyboard navigation
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        selectedButtonIndex = 0;
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        if (selectedButtonIndex == 0) selectedButtonIndex = 1;
    }
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        if (selectedButtonIndex == 2) selectedButtonIndex = 1;
    }
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
        if (selectedButtonIndex == 1) selectedButtonIndex = 2;
    }

    bool clickPressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool actionPressed = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_E) || clickPressed;

    if (actionPressed) {
        if (selectedButtonIndex == 0) {
            // New Game or Resume Game
            if (SceneManager::Get().HasSavedGameplayScene()) {
                SceneManager::Get().ResumeGame();
            } else {
                ActManager::Get().Reset();
                SceneManager::Get().ChangeScene(std::make_unique<IntroScene>());
            }
        } else if (selectedButtonIndex == 1) {
            // Options toggle
            showOptionsPopup = !showOptionsPopup;
        } else if (selectedButtonIndex == 2) {
            // Exit
            CloseWindow();
        }
    }
}

void MainMenuScene::Draw() {
    float screenWidth = 2000.0f;
    float screenHeight = 800.0f;

    // Draw Main Menu Background Sprite
    Texture2D bgTex = ResourceManager::Get().GetTexture("assets/sprites/main-menu.png");
    if (bgTex.id == 0) {
        bgTex = ResourceManager::Get().GetTexture("assets/sprites/mian-menu.png");
    }
    if (bgTex.id != 0) {
        DrawTexturePro(
            bgTex,
            Rectangle{ 0, 0, (float)bgTex.width, (float)bgTex.height },
            Rectangle{ 0, 0, screenWidth, screenHeight },
            Vector2{ 0, 0 },
            0.0f,
            WHITE
        );
    } else {
        ClearBackground(DARKBLUE);
    }

    // Helper Lambda to draw user's custom sprite button with hover highlight (NO text overlays)
    float time = (float)GetTime();
    auto DrawMenuButton = [&](int index, Rectangle rect, const char* texturePath) {
        bool isSelected = (selectedButtonIndex == index);
        Texture2D btnTex = ResourceManager::Get().GetTexture(texturePath);

        float scalePulse = isSelected ? (sinf(time * 6.0f) * 3.0f) : 0.0f;
        Rectangle drawRect = {
            rect.x - scalePulse,
            rect.y - scalePulse,
            rect.width + scalePulse * 2.0f,
            rect.height + scalePulse * 2.0f
        };

        if (btnTex.id != 0) {
            Color tintColor = isSelected ? WHITE : Fade(LIGHTGRAY, 0.85f);
            DrawTexturePro(
                btnTex,
                Rectangle{ 0, 0, (float)btnTex.width, (float)btnTex.height },
                drawRect,
                Vector2{ 0, 0 },
                0.0f,
                tintColor
            );
        } else {
            // Fallback rectangle if texture fails
            Color boxColor = isSelected ? GOLD : DARKGRAY;
            DrawRectangleRec(drawRect, boxColor);
            DrawRectangleLinesEx(drawRect, 3.0f, WHITE);
        }
    };

    // Draw User's Sprite Buttons
    DrawMenuButton(0, newGameBtnRect, "assets/sprites/New game.png");
    DrawMenuButton(1, optionsBtnRect, "assets/sprites/optoins.png");
    DrawMenuButton(2, exitBtnRect, "assets/sprites/exit.png");

    // Guidance footer
    const char* footerHint = "Controls: [WASD / Arrows / Mouse] to Select | [Enter/Click] to Confirm | [ESC] Exit";
    int footerW = MeasureText(footerHint, 24);
    DrawText(footerHint, (int)(screenWidth - footerW) / 2, 740, 24, Fade(WHITE, 0.8f));

    // Options Popup Overlay
    if (showOptionsPopup) {
        DrawRectangle(400, 200, 1200, 400, Fade(BLACK, 0.9f));
        DrawRectangleLinesEx(Rectangle{ 400, 200, 1200, 400 }, 4.0f, GOLD);
        
        DrawText("OPTIONS / SETTINGS", 760, 250, 40, GOLD);
        DrawText("- Fullscreen Mode: Press F11 or Alt+Enter at any time", 500, 340, 28, WHITE);
        DrawText("- Hot Reload Scenes: Press R during gameplay", 500, 390, 28, WHITE);
        DrawText("Press [Enter] or click Options again to close", 650, 520, 24, GRAY);
    }
}
