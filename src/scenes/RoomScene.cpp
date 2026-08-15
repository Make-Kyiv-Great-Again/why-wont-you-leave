#include "scenes/RoomScene.hpp"
#include "scenes/CorridorScene.hpp"
#include "core/SceneManager.hpp"
#include "dialogue/DialogueManager.hpp"
#include "core/MemoryManager.hpp"
#include "core/EffectManager.hpp"
#include <cmath>

RoomScene::RoomScene(int roomId, float spawnPlayerX)
    : roomId(roomId), screenWidth(2000.0f), screenHeight(800.0f), groundY(660.0f), player(spawnPlayerX, 660.0f) {
    exitDoor = { 100.0f, groundY - 240.0f, 130.0f, 240.0f };

    // Setup room interactable items with artifact IDs
    if (roomId == 1) {
        // Room 1: Sapphire Prism and Emerald Relic
        items.emplace_back(
            "sapphire_prism",
            Rectangle{ 700.0f, groundY - 90.0f, 90.0f, 90.0f },
            SKYBLUE,
            DARKBLUE,
            "Sapphire Prism",
            Dialogues::CreateSapphireCubeDialogue()
        );

        items.emplace_back(
            "emerald_relic",
            Rectangle{ 1450.0f, groundY - 90.0f, 90.0f, 90.0f },
            LIME,
            DARKGREEN,
            "Emerald Relic",
            Dialogues::CreateEmeraldCubeDialogue()
        );
    } else if (roomId == 2) {
        // Room 2: Amethyst Core
        items.emplace_back(
            "amethyst_core",
            Rectangle{ 1050.0f, groundY - 90.0f, 90.0f, 90.0f },
            PURPLE,
            DARKPURPLE,
            "Amethyst Core",
            Dialogues::CreateAmethystCubeDialogue()
        );
    }
}

void RoomScene::Update(float dt) {
    SceneManager::Get().SetTabPressed(IsKeyDown(KEY_TAB));

    // Update effects
    EffectManager::Get().Update(dt);

    // If Tab memory archive is open, pause gameplay actions
    if (IsKeyDown(KEY_TAB)) {
        holdQTimer = 0.0f;
        return;
    }

    // Update dialogue system
    DialogueManager::Get().Update(dt);

    // If dialogue is active, freeze player movement & scene interactions
    if (DialogueManager::Get().IsActive()) {
        promptText = "";
        holdQTimer = 0.0f;
        activeHoverItem = nullptr;
        return;
    }

    // Normal player movement
    player.Update(dt, screenWidth);

    promptText = "";
    activeHoverItem = nullptr;

    // Check interaction with Room Exit Door
    if (CheckCollisionRecs(player.rect, exitDoor)) {
        promptText = "Press [E] to return to Corridor";
        holdQTimer = 0.0f;
        if (IsKeyPressed(KEY_E)) {
            float returnX = (roomId == 1) ? 424.0f : (roomId == 2) ? 960.0f : 1494.0f;
            SceneManager::Get().ChangeScene(std::make_unique<CorridorScene>(returnX));
            return;
        }
    }

    // Check interaction with Room items
    for (auto& item : items) {
        if (item.CheckCollision(player.rect)) {
            activeHoverItem = &item;
            bool isRemembered = MemoryManager::Get().IsRemembered(item.artifactId);

            if (isRemembered) {
                promptText = "[E] Inspect | [Hold Q] Forget Memory";
            } else {
                promptText = "[E] Inspect | [Hold Q] Remember Memory";
            }

            if (IsKeyDown(KEY_Q)) {
                holdQTimer += dt;
                if (holdQTimer >= holdQThreshold) {
                    holdQTimer = 0.0f;
                    if (!isRemembered) {
                        // Remember item: start memory sequence with black screen & centered dialogue
                        MemoryManager::Get().SetRemembered(item.artifactId, true);
                        DialogueManager::Get().StartDialogue(item.dialogue, true /* isMemoryMode */, item.artifactId);
                    } else {
                        // Forget item: play disappearing animation
                        MemoryManager::Get().SetRemembered(item.artifactId, false);
                        EffectManager::Get().SpawnForgettingEffect(item.rect, item.color);
                    }
                    promptText = "";
                    return;
                }
            } else {
                holdQTimer = 0.0f;
                if (IsKeyPressed(KEY_E)) {
                    item.Interact();
                    promptText = "";
                    return;
                }
            }
            break;
        }
    }

    if (!activeHoverItem) {
        holdQTimer = 0.0f;
    }
}

void RoomScene::DrawHoldQGauge(Vector2 centerPos, float progress, bool isRemembering) {
    float radius = 32.0f;
    // Outer shadow ring
    DrawCircleSector(centerPos, radius + 6.0f, 0, 360, 36, Fade(BLACK, 0.7f));
    DrawCircleSectorLines(centerPos, radius + 6.0f, 0, 360, 36, Fade(GRAY, 0.6f));

    // Progress arc
    Color arcColor = isRemembering ? GOLD : RED;
    DrawCircleSector(centerPos, radius, 0, progress * 360.0f, 36, arcColor);
    DrawCircleSectorLines(centerPos, radius, 0, 360.0f, 36, Fade(WHITE, 0.8f));

    // Center "Q" text
    DrawText("Q", (int)centerPos.x - 7, (int)centerPos.y - 12, 24, BLACK);

    // Label below gauge
    const char* actionLabel = isRemembering ? "Remembering..." : "Forgetting...";
    int textW = MeasureText(actionLabel, 20);
    DrawText(actionLabel, (int)centerPos.x - textW / 2, (int)centerPos.y + 40, 20, isRemembering ? GOLD : ORANGE);
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

    // Draw Forgetting / Dissolving Visual Effects
    EffectManager::Get().Draw();

    // Player
    player.Draw();

    // Draw Hold Q Gauge if currently charging
    if (holdQTimer > 0.0f && activeHoverItem) {
        Vector2 gaugePos = { activeHoverItem->rect.x + activeHoverItem->rect.width / 2.0f, activeHoverItem->rect.y - 80.0f };
        bool isRem = !MemoryManager::Get().IsRemembered(activeHoverItem->artifactId);
        DrawHoldQGauge(gaugePos, holdQTimer / holdQThreshold, isRem);
    }

    // Interaction Banner / Controls Guidance (only when dialogue is not active and not holding Tab)
    if (!DialogueManager::Get().IsActive() && !IsKeyDown(KEY_TAB)) {
        if (!promptText.empty()) {
            int textWidth = MeasureText(promptText.c_str(), 36);
            int bannerX = ((int)screenWidth - textWidth - 80) / 2;
            DrawRectangle(bannerX, 690, textWidth + 80, 70, Fade(DARKGRAY, 0.85f));
            DrawText(promptText.c_str(), ((int)screenWidth - textWidth) / 2, 706, 36, WHITE);
        } else {
            DrawText("Controls: [A/D] to Move | [E] Inspect | [Hold Q] Remember/Forget", 40, 736, 30, GRAY);
        }

        // Persistent Tab hint
        const char* tabHint = "[Hold TAB] Memory Archive";
        int tabW = MeasureText(tabHint, 24);
        DrawText(tabHint, (int)screenWidth - tabW - 40, 736, 24, Fade(DARKBLUE, 0.8f));
    }

    // Draw Dialogue Overlay (standard or centered memory mode)
    DialogueManager::Get().Draw();
}
