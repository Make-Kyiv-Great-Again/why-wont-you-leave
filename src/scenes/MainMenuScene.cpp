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

    // Stop gameplay background music if coming from game
    Music gameMusic = ResourceManager::Get().GetMusic("assets/sounds/game_sound.mp3");
    if (gameMusic.ctxData != nullptr && IsMusicStreamPlaying(gameMusic)) {
        StopMusicStream(gameMusic);
    }
}

MainMenuScene::~MainMenuScene() {
    Music menuMusic = ResourceManager::Get().GetMusic("assets/sounds/menu_sound.wav");
    if (menuMusic.ctxData != nullptr && IsMusicStreamPlaying(menuMusic)) {
        StopMusicStream(menuMusic);
    }
}

void MainMenuScene::Update(float dt) {
    // Play/loop menu sound while in Main Menu
    Music menuMusic = ResourceManager::Get().GetMusic("assets/sounds/menu_sound.wav");
    if (menuMusic.ctxData != nullptr) {
        if (!IsMusicStreamPlaying(menuMusic)) {
            PlayMusicStream(menuMusic);
        }
        UpdateMusicStream(menuMusic);
    }

    // Pressing ESC inside Main Menu closes Options if open, or quits the game
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (showOptionsPopup) {
            showOptionsPopup = false;
            return;
        }
        CloseWindow();
        return;
    }

    Vector2 virtualMouse = SceneManager::Get().GetVirtualMousePosition();
    int prevSelected = selectedButtonIndex;

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

    if (selectedButtonIndex != prevSelected) {
        Sound sfx = ResourceManager::Get().GetSound("assets/sounds/select_sound.mp3");
        if (sfx.frameCount > 0) {
            SetSoundVolume(sfx, 2.0f);
            StopSound(sfx);
            PlaySound(sfx);
        }
    }

    bool clickPressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool actionPressed = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_E) || clickPressed;

    if (actionPressed) {
        Sound sfx = ResourceManager::Get().GetSound("assets/sounds/select_sound.mp3");
        if (sfx.frameCount > 0) {
            SetSoundVolume(sfx, 2.0f);
            StopSound(sfx);
            PlaySound(sfx);
        }

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

    // Draw Game Title Sprite
    Texture2D titleTex = ResourceManager::Get().GetTexture("assets/sprites/title.png");
    if (titleTex.id == 0) {
        titleTex = ResourceManager::Get().GetTexture("assets/sprites/WHY WON’T YOU LEAVE_.svg");
    }
    if (titleTex.id != 0) {
        float titleW = 1100.0f;
        float titleH = titleW * ((float)titleTex.height / (float)titleTex.width);
        float titleOffset = sinf(time * 1.5f) * 5.0f;
        Rectangle titleDest = {
            (screenWidth - titleW) / 2.0f,
            190.0f + titleOffset,
            titleW,
            titleH
        };
        DrawTexturePro(
            titleTex,
            Rectangle{ 0, 0, (float)titleTex.width, (float)titleTex.height },
            titleDest,
            Vector2{ 0, 0 },
            0.0f,
            WHITE
        );
    }

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
    int footerW = ResourceManager::MeasureGameText(footerHint, 24.0f);
    ResourceManager::DrawGameTextWithOutline(footerHint, (screenWidth - footerW) / 2.0f, 740.0f, 24.0f, Fade(RAYWHITE, 0.85f), BLACK, 1.5f);

    // Options Modal Overlay (Horizontally Centered & Styled)
    if (showOptionsPopup) {
        float cardWidth = 1020.0f;
        float cardHeight = 500.0f;
        float cardX = (screenWidth - cardWidth) / 2.0f;
        float cardY = (screenHeight - cardHeight) / 2.0f;

        // Dim background backdrop
        DrawRectangle(0, 0, (int)screenWidth, (int)screenHeight, Fade(BLACK, 0.55f));

        // Outer & Inner Modal Frame
        DrawRectangle((int)cardX, (int)cardY, (int)cardWidth, (int)cardHeight, Fade(Color{ 10, 10, 14, 255 }, 0.96f));
        DrawRectangleLinesEx(Rectangle{ cardX, cardY, cardWidth, cardHeight }, 3.0f, Fade(GOLD, 0.85f));
        DrawRectangleLinesEx(Rectangle{ cardX + 6.0f, cardY + 6.0f, cardWidth - 12.0f, cardHeight - 12.0f }, 1.5f, Fade(LIGHTGRAY, 0.30f));

        // Header Title
        const char* optHeader = "✦ SETTINGS & CONTROLS ✦";
        int headerW = ResourceManager::MeasureGameText(optHeader, 36.0f);
        ResourceManager::DrawGameTextWithOutline(optHeader, (screenWidth - headerW) / 2.0f, cardY + 28.0f, 36.0f, GOLD, BLACK, 2.0f);

        DrawLineEx(Vector2{ cardX + 60.0f, cardY + 76.0f }, Vector2{ cardX + cardWidth - 60.0f, cardY + 76.0f }, 2.0f, Fade(GOLD, 0.40f));

        // Controls Grid (2 Columns x 3 Rows)
        struct ControlItem {
            std::string key;
            std::string desc;
        };

        std::vector<ControlItem> col1 = {
            { "A / D  or  <- / ->", "Move Character" },
            { "E", "Inspect Item / Door" },
            { "Q  (Hold)", "Collect Story Memory" }
        };

        std::vector<ControlItem> col2 = {
            { "TAB", "Memory Archive" },
            { "F11  /  Alt+Enter", "Toggle Fullscreen" },
            { "ESC", "Pause / Resume" }
        };

        float startContentY = cardY + 100.0f;
        float rowHeight = 92.0f;
        float colWidth = 430.0f;

        auto DrawControlColumn = [&](const std::vector<ControlItem>& list, float colX) {
            for (size_t i = 0; i < list.size(); i++) {
                float rowY = startContentY + (float)i * rowHeight;
                Rectangle rowBox = { colX, rowY, colWidth, 74.0f };

                DrawRectangleRec(rowBox, Fade(WHITE, 0.04f));
                DrawRectangleLinesEx(rowBox, 1.5f, Fade(LIGHTGRAY, 0.20f));

                // Key Badge
                int keyW = ResourceManager::MeasureGameText(list[i].key.c_str(), 24.0f);
                Rectangle keyRect = { colX + 16.0f, rowY + 18.0f, (float)keyW + 22.0f, 38.0f };
                DrawRectangleRec(keyRect, Fade(BLACK, 0.75f));
                DrawRectangleLinesEx(keyRect, 2.0f, Fade(GOLD, 0.70f));
                ResourceManager::DrawGameTextWithOutline(list[i].key.c_str(), colX + 27.0f, rowY + 24.0f, 24.0f, GOLD, BLACK, 1.5f);

                // Description
                float descX = colX + 16.0f + keyRect.width + 16.0f;
                ResourceManager::DrawGameTextWithOutline(list[i].desc.c_str(), descX, rowY + 26.0f, 22.0f, RAYWHITE, BLACK, 1.5f);
            }
        };

        DrawControlColumn(col1, cardX + 50.0f);
        DrawControlColumn(col2, cardX + 540.0f);

        // Footer Hint
        float alpha = 0.6f + 0.4f * sinf(time * 4.0f);
        const char* closeHint = "Press [Enter], [ESC], or Click to Close";
        int closeW = ResourceManager::MeasureGameText(closeHint, 24.0f);
        ResourceManager::DrawGameTextWithOutline(closeHint, (screenWidth - closeW) / 2.0f, cardY + cardHeight - 48.0f, 24.0f, Fade(GOLD, alpha), BLACK, 1.5f);
    }
}
