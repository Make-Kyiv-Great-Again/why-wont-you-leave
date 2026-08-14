#include "scenes/RoomScene.hpp"
#include "scenes/CorridorScene.hpp"
#include "core/SceneManager.hpp"
#include "dialogue/DialogueManager.hpp"

RoomScene::RoomScene(int roomId, float spawnPlayerX)
    : roomId(roomId), screenWidth(2000.0f), screenHeight(800.0f), groundY(660.0f), player(spawnPlayerX, 660.0f) {
    exitDoor = { 100.0f, groundY - 240.0f, 130.0f, 240.0f };

    // Setup room interactable items
    if (roomId == 1) {
        // Room 1 has 2 cubes
        items.emplace_back(
            Rectangle{ 700.0f, groundY - 90.0f, 90.0f, 90.0f },
            SKYBLUE,
            DARKBLUE,
            "Sapphire Prism",
            Dialogues::CreateSapphireCubeDialogue()
        );

        items.emplace_back(
            Rectangle{ 1450.0f, groundY - 90.0f, 90.0f, 90.0f },
            LIME,
            DARKGREEN,
            "Emerald Relic",
            Dialogues::CreateEmeraldCubeDialogue()
        );
    } else if (roomId == 2) {
        // Room 2 has 1 cube
        items.emplace_back(
            Rectangle{ 1050.0f, groundY - 90.0f, 90.0f, 90.0f },
            PURPLE,
            DARKPURPLE,
            "Amethyst Core",
            Dialogues::CreateAmethystCubeDialogue()
        );
    }
}

void RoomScene::Update(float dt) {
    // Update dialogue system
    DialogueManager::Get().Update(dt);

    // If dialogue is active, freeze player movement & scene interactions
    if (DialogueManager::Get().IsActive()) {
        promptText = "";
        return;
    }

    // Normal player movement
    player.Update(dt, screenWidth);

    promptText = "";

    // Check interaction with Room Exit Door
    if (CheckCollisionRecs(player.rect, exitDoor)) {
        promptText = "Press [E] to return to Corridor";
        if (IsKeyPressed(KEY_E)) {
            float returnX = (roomId == 1) ? 424.0f : (roomId == 2) ? 960.0f : 1494.0f;
            SceneManager::Get().ChangeScene(std::make_unique<CorridorScene>(returnX));
            return;
        }
    }

    // Check interaction with Room items
    for (auto& item : items) {
        if (item.CheckCollision(player.rect)) {
            promptText = "Press [E] to inspect " + item.name;
            if (IsKeyPressed(KEY_E)) {
                item.Interact();
                promptText = "";
                return;
            }
        }
    }
}

void RoomScene::Draw() {
    ClearBackground(RAYWHITE);

    // Floor Line
    DrawLine(0, (int)groundY, (int)screenWidth, (int)groundY, DARKGRAY);

    // Room Header
    std::string roomTitle = "ROOM " + std::to_string(roomId);
    DrawText(roomTitle.c_str(), 40, 40, 48, DARKGRAY);

    // Draw Room Exit Door on left
    DrawRectangleRec(exitDoor, BROWN);
    DrawRectangleLinesEx(exitDoor, 4, DARKBROWN);
    DrawCircle((int)(exitDoor.x + exitDoor.width - 20), (int)(exitDoor.y + 120), 8, GOLD);

    int labelWidth = MeasureText("Corridor", 32);
    DrawText("Corridor",
             (int)(exitDoor.x + (exitDoor.width - labelWidth) / 2.0f),
             (int)(exitDoor.y - 50), 32, DARKBROWN);

    // Draw Interactable Items
    for (const auto& item : items) {
        item.Draw();
    }

    // Player
    player.Draw();

    // Interaction Banner / Controls Guidance (only when dialogue is not active)
    if (!DialogueManager::Get().IsActive()) {
        if (!promptText.empty()) {
            int textWidth = MeasureText(promptText.c_str(), 40);
            int bannerX = ((int)screenWidth - textWidth - 80) / 2;
            DrawRectangle(bannerX, 690, textWidth + 80, 70, Fade(DARKGRAY, 0.85f));
            DrawText(promptText.c_str(), ((int)screenWidth - textWidth) / 2, 704, 40, WHITE);
        } else {
            DrawText("Controls: [A/D] or [Left/Right] to Move | [E] to Interact", 40, 736, 32, GRAY);
        }
    }

    // Draw Dialogue Overlay on top of everything
    DialogueManager::Get().Draw();
}
