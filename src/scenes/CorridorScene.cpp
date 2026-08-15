#include "scenes/CorridorScene.hpp"
#include "scenes/RoomScene.hpp"
#include "core/SceneManager.hpp"
#include "dialogue/DialogueManager.hpp"
#include "core/MemoryManager.hpp"

CorridorScene::CorridorScene(float spawnPlayerX)
    : screenWidth(2000.0f), screenHeight(800.0f), groundY(660.0f), player(spawnPlayerX, 660.0f) {
    doors[0] = { { 400.0f, groundY - 240.0f, 130.0f, 240.0f }, "Room 1", 1, 424.0f };
    doors[1] = { { 934.0f, groundY - 240.0f, 130.0f, 240.0f }, "Room 2", 2, 960.0f };
    doors[2] = { { 1470.0f, groundY - 240.0f, 130.0f, 240.0f }, "Room 3", 3, 1494.0f };
}

void CorridorScene::Update(float dt) {
    SceneManager::Get().SetTabPressed(IsKeyDown(KEY_TAB));

    if (IsKeyDown(KEY_TAB)) {
        return;
    }

    DialogueManager::Get().Update(dt);
    if (DialogueManager::Get().IsActive()) {
        promptText = "";
        return;
    }

    player.Update(dt, screenWidth);

    promptText = "";
    for (int i = 0; i < 3; i++) {
        if (CheckCollisionRecs(player.rect, doors[i].rect)) {
            promptText = "Press [E] to enter " + doors[i].label;
            if (IsKeyPressed(KEY_E)) {
                SceneManager::Get().ChangeScene(std::make_unique<RoomScene>(doors[i].targetRoomId));
                return;
            }
        }
    }
}

void CorridorScene::Draw() {
    ClearBackground(RAYWHITE);

    // Floor Line
    DrawLine(0, (int)groundY, (int)screenWidth, (int)groundY, DARKGRAY);

    // Header
    DrawText("CORRIDOR", 40, 40, 48, DARKGRAY);

    // Draw Corridor Doors
    for (int i = 0; i < 3; i++) {
        DrawRectangleRec(doors[i].rect, BROWN);
        DrawRectangleLinesEx(doors[i].rect, 4, DARKBROWN);
        DrawCircle((int)(doors[i].rect.x + 20), (int)(doors[i].rect.y + 120), 8, GOLD);

        int labelWidth = MeasureText(doors[i].label.c_str(), 32);
        DrawText(doors[i].label.c_str(),
                 (int)(doors[i].rect.x + (doors[i].rect.width - labelWidth) / 2.0f),
                 (int)(doors[i].rect.y - 50), 32, DARKBROWN);
    }

    // Player
    player.Draw();

    // Interaction Banner / Controls Guidance
    if (!DialogueManager::Get().IsActive() && !IsKeyDown(KEY_TAB)) {
        if (!promptText.empty()) {
            int textWidth = MeasureText(promptText.c_str(), 40);
            int bannerX = ((int)screenWidth - textWidth - 80) / 2;
            DrawRectangle(bannerX, 690, textWidth + 80, 70, Fade(DARKGRAY, 0.85f));
            DrawText(promptText.c_str(), ((int)screenWidth - textWidth) / 2, 704, 40, WHITE);
        } else {
            DrawText("Controls: [A/D] or [Left/Right] to Move | [E] to Enter Doors", 40, 736, 30, GRAY);
        }

        // Persistent Tab hint
        const char* tabHint = "[Hold TAB] Memory Archive";
        int tabW = MeasureText(tabHint, 24);
        DrawText(tabHint, (int)screenWidth - tabW - 40, 736, 24, Fade(DARKBLUE, 0.8f));
    }

    // Draw Dialogue Overlay
    DialogueManager::Get().Draw();
}
