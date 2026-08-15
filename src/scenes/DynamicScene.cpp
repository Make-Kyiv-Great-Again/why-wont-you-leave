#include "scenes/DynamicScene.hpp"
#include "core/SceneManager.hpp"
#include "dialogue/DialogueManager.hpp"
#include "core/MemoryManager.hpp"
#include "core/EffectManager.hpp"
#include "core/ResourceManager.hpp"
#include "scenes/MainMenuScene.hpp"
#include <nlohmann/json.hpp>
#include <cmath>
#include <iostream>

DynamicScene::DynamicScene(const std::string& sceneId, float spawnPlayerX, const std::string& targetDoorLabel)
    : sceneId(sceneId),
      jsonPath("assets/data/scenes/" + sceneId + ".json"),
      screenWidth(2000.0f),
      screenHeight(800.0f),
      groundY(660.0f),
      backgroundColor(RAYWHITE),
      player(spawnPlayerX >= 0.0f ? spawnPlayerX : 200.0f, 660.0f) {
    LoadFromConfigFile(jsonPath);

    // Pause menu button layout (Options & Exit side-by-side in one row)
    float btnW = 220.0f;
    float btnH = 85.0f;
    float gap = 40.0f;
    float rowW = btnW * 2.0f + gap;
    float startX = (screenWidth - rowW) / 2.0f;
    float startY = 380.0f;

    pauseOptionsRect = Rectangle{ startX, startY, btnW, btnH };
    pauseExitRect    = Rectangle{ startX + btnW + gap, startY, btnW, btnH };

    // Auto-position player centered directly in front of the door matching targetDoorLabel
    bool positionedByDoor = false;
    if (!targetDoorLabel.empty()) {
        for (const auto& door : doors) {
            if (door.label == targetDoorLabel) {
                player.rect.x = door.rect.x + (door.rect.width - player.rect.width) / 2.0f;
                positionedByDoor = true;
                break;
            }
        }
    }

    if (!positionedByDoor && spawnPlayerX >= 0.0f) {
        player.rect.x = spawnPlayerX;
    }

    player.rect.y = groundY - player.rect.height;
}

void DynamicScene::LoadFromConfigFile(const std::string& path) {
    nlohmann::json j = ResourceManager::Get().LoadJson(path);
    if (j.is_null()) {
        TraceLog(LOG_WARNING, "DYNAMIC_SCENE: Could not load scene from %s", path.c_str());
        return;
    }

    doors.clear();
    items.clear();

    title = j.value("title", "SCENE");
    groundY = j.value("ground_y", 660.0f);
    backgroundColor = ResourceManager::HexToColor(j.value("background_color", "#F5F5F5"));
    controlsHint = j.value("controls_hint", "Controls: [A/D] to Move | [E] Interact | [ESC] Pause");

    // Load background texture path
    backgroundTexturePath = "";
    if (j.contains("background_texture")) {
        backgroundTexturePath = j["background_texture"].get<std::string>();
    } else if (j.contains("background") && j["background"].is_object() && j["background"].contains("texture")) {
        backgroundTexturePath = j["background"]["texture"].get<std::string>();
    }

    // Load Doors
    if (j.contains("doors") && j["doors"].is_array()) {
        for (const auto& doorJson : j["doors"]) {
            DoorData door;
            door.label = doorJson.value("label", "Door");
            door.targetScene = doorJson.value("target_scene", "corridor");
            door.targetDoor = doorJson.value("target_door", "");
            door.targetSpawnX = doorJson.value("target_spawn_x", -1.0f);
            
            if (doorJson.contains("rect")) {
                auto r = doorJson["rect"];
                door.rect = Rectangle{
                    r.value("x", 0.0f),
                    r.value("y", 0.0f),
                    r.value("width", 130.0f),
                    r.value("height", 240.0f)
                };
            } else {
                door.rect = Rectangle{ 0.0f, groundY - 240.0f, 130.0f, 240.0f };
            }

            door.doorColor = ResourceManager::HexToColor(doorJson.value("door_color", "#8B4513"));
            door.borderColor = ResourceManager::HexToColor(doorJson.value("border_color", "#5C4033"));

            // Door Sprite parsing
            if (doorJson.contains("texture")) {
                door.sprite.texturePath = doorJson["texture"].get<std::string>();
            } else if (doorJson.contains("sprite") && doorJson["sprite"].is_object() && doorJson["sprite"].contains("texture")) {
                door.sprite.texturePath = doorJson["sprite"]["texture"].get<std::string>();
            }

            doors.push_back(door);
        }
    }

    // Load Items
    if (j.contains("items") && j["items"].is_array()) {
        for (const auto& itemJson : j["items"]) {
            std::string artifactId = itemJson.value("artifact_id", "item");
            std::string name = itemJson.value("name", "Item");
            
            Rectangle rect;
            if (itemJson.contains("rect")) {
                auto r = itemJson["rect"];
                rect = Rectangle{
                    r.value("x", 0.0f),
                    r.value("y", 0.0f),
                    r.value("width", 90.0f),
                    r.value("height", 90.0f)
                };
            } else {
                rect = Rectangle{ 500.0f, groundY - 90.0f, 90.0f, 90.0f };
            }

            Color color = ResourceManager::HexToColor(itemJson.value("color", "#87CEEB"));
            Color borderColor = ResourceManager::HexToColor(itemJson.value("border_color", "#00008B"));
            std::string dialogueFile = itemJson.value("dialogue_file", "");

            // Item Sprite parsing
            Sprite itemSprite;
            if (itemJson.contains("texture")) {
                itemSprite.texturePath = itemJson["texture"].get<std::string>();
            } else if (itemJson.contains("visual") && itemJson["visual"].is_object() && itemJson["visual"].contains("texture") && !itemJson["visual"]["texture"].is_null()) {
                itemSprite.texturePath = itemJson["visual"]["texture"].get<std::string>();
            }

            if (itemJson.contains("is_animated")) {
                itemSprite.isAnimated = itemJson.value("is_animated", false);
                itemSprite.frameCount = itemJson.value("frame_count", 1);
                itemSprite.frameTime = itemJson.value("frame_time", 0.1f);
            }

            items.emplace_back(artifactId, rect, color, borderColor, name, dialogueFile, itemSprite);

            // Register with MemoryManager
            int roomId = 0;
            if (sceneId.rfind("room", 0) == 0 && sceneId.length() > 4) {
                try { roomId = std::stoi(sceneId.substr(4)); } catch (...) {}
            }
            MemoryManager::Get().RegisterArtifact(artifactId, name, color, borderColor, roomId);
        }
    }

    // Shader support
    if (j.contains("shader") && j["shader"].contains("path")) {
        shaderPath = j["shader"]["path"].get<std::string>();
        sceneShader = ResourceManager::Get().GetShader(shaderPath);
    }
}

void DynamicScene::UpdatePauseMenu(float dt) {
    Vector2 virtualMouse = SceneManager::Get().GetVirtualMousePosition();

    // Mouse hover check
    if (CheckCollisionPointRec(virtualMouse, pauseOptionsRect)) {
        pauseSelectedBtn = 0;
    } else if (CheckCollisionPointRec(virtualMouse, pauseExitRect)) {
        pauseSelectedBtn = 1;
    }

    // Keyboard navigation (Left/Right)
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        pauseSelectedBtn = 0;
    }
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
        pauseSelectedBtn = 1;
    }

    bool clickPressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool actionPressed = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_E) || clickPressed;

    if (actionPressed) {
        if (pauseSelectedBtn == 0) {
            // Options
            showPauseOptions = !showPauseOptions;
        } else if (pauseSelectedBtn == 1) {
            // Exit to Main Menu
            isPaused = false;
            SceneManager::Get().ChangeScene(std::make_unique<MainMenuScene>());
        }
    }
}

void DynamicScene::Update(float dt) {
    // Pressing ESC toggles in-game pause
    if (IsKeyPressed(KEY_ESCAPE)) {
        isPaused = !isPaused;
        showPauseOptions = false;
        return;
    }

    // If game is paused, only update pause menu
    if (isPaused) {
        UpdatePauseMenu(dt);
        return;
    }

    SceneManager::Get().SetTabPressed(IsKeyDown(KEY_TAB));

    // Hot-reload scene JSON when pressing R
    if (IsKeyPressed(KEY_R)) {
        LoadFromConfigFile(jsonPath);
    }

    // Update sprite animations
    for (auto& item : items) {
        item.Update(dt);
    }
    for (auto& door : doors) {
        door.sprite.Update(dt);
    }

    // Update effects
    EffectManager::Get().Update(dt);

    // Pause if Tab memory archive is open
    if (IsKeyDown(KEY_TAB)) {
        holdQTimer = 0.0f;
        return;
    }

    // Update dialogue system
    DialogueManager::Get().Update(dt);

    // Freeze player if dialogue is active
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

    // Check interaction with Doors
    for (const auto& door : doors) {
        if (CheckCollisionRecs(player.rect, door.rect)) {
            promptText = "Press [E] to enter " + door.label;
            holdQTimer = 0.0f;
            if (IsKeyPressed(KEY_E)) {
                SceneManager::Get().ChangeScene(std::make_unique<DynamicScene>(door.targetScene, door.targetSpawnX, door.targetDoor));
                return;
            }
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
                        MemoryManager::Get().SetRemembered(item.artifactId, true);
                        if (!item.dialogueFile.empty()) {
                            DialogueManager::Get().StartDialogueFile(item.dialogueFile, true /* isMemoryMode */, item.artifactId);
                        }
                    } else {
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

void DynamicScene::DrawHoldQGauge(Vector2 centerPos, float progress, bool isRemembering) {
    float radius = 32.0f;
    DrawCircleSector(centerPos, radius + 6.0f, 0, 360, 36, Fade(BLACK, 0.7f));
    DrawCircleSectorLines(centerPos, radius + 6.0f, 0, 360, 36, Fade(GRAY, 0.6f));

    Color arcColor = isRemembering ? GOLD : RED;
    DrawCircleSector(centerPos, radius, 0, progress * 360.0f, 36, arcColor);
    DrawCircleSectorLines(centerPos, radius, 0, 360.0f, 36, Fade(WHITE, 0.8f));

    ResourceManager::DrawGameText("Q", centerPos.x - 7, centerPos.y - 12, 24, BLACK);

    const char* actionLabel = isRemembering ? "Remembering..." : "Forgetting...";
    int textW = ResourceManager::MeasureGameText(actionLabel, 20);
    ResourceManager::DrawGameText(actionLabel, centerPos.x - textW / 2, centerPos.y + 40, 20, isRemembering ? GOLD : ORANGE);
}

void DynamicScene::DrawPauseOverlay() {
    // 1. Darken the game screen slightly
    DrawRectangle(0, 0, (int)screenWidth, (int)screenHeight, Fade(BLACK, 0.65f));

    // 2. Pause Header
    const char* pauseTitle = "GAME PAUSED";
    int titleW = MeasureText(pauseTitle, 50);
    DrawText(pauseTitle, (int)(screenWidth - titleW) / 2, 240, 50, GOLD);

    // 3. Helper Lambda to draw SVG pause buttons (NO text overlays)
    float time = (float)GetTime();
    auto DrawPauseBtn = [&](int index, Rectangle rect, const char* texturePath) {
        bool isSelected = (pauseSelectedBtn == index);
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

            if (isSelected) {
                DrawRectangleLinesEx(drawRect, 3.0f, GOLD);
            }
        } else {
            Color boxColor = isSelected ? GOLD : DARKGRAY;
            DrawRectangleRec(drawRect, boxColor);
            DrawRectangleLinesEx(drawRect, 3.0f, WHITE);
        }
    };

    // Draw Options and Exit buttons side-by-side in one row
    DrawPauseBtn(0, pauseOptionsRect, "assets/sprites/optoins.png");
    DrawPauseBtn(1, pauseExitRect, "assets/sprites/exit.png");

    // Guidance footer
    const char* pauseHint = "Press [ESC] to Resume Gameplay | [A/D / Mouse] to Select";
    int hintW = MeasureText(pauseHint, 24);
    DrawText(pauseHint, (int)(screenWidth - hintW) / 2, 700, 24, Fade(WHITE, 0.85f));

    // Options Popup Overlay inside Pause
    if (showPauseOptions) {
        DrawRectangle(400, 200, 1200, 400, Fade(BLACK, 0.92f));
        DrawRectangleLinesEx(Rectangle{ 400, 200, 1200, 400 }, 4.0f, GOLD);
        
        DrawText("OPTIONS / SETTINGS", 760, 250, 40, GOLD);
        DrawText("- Fullscreen Mode: Press F11 or Alt+Enter at any time", 500, 340, 28, WHITE);
        DrawText("- Hot Reload Scenes: Press R during gameplay", 500, 390, 28, WHITE);
        DrawText("Press [Enter] or click Options again to close", 650, 520, 24, GRAY);
    }
}

void DynamicScene::Draw() {
    ClearBackground(backgroundColor);

    if (sceneShader.id != 0) {
        BeginShaderMode(sceneShader);
    }

    // Render Background Sprite Texture if specified
    if (!backgroundTexturePath.empty()) {
        Texture2D bgTex = ResourceManager::Get().GetTexture(backgroundTexturePath);
        if (bgTex.id != 0) {
            DrawTexturePro(
                bgTex,
                Rectangle{ 0, 0, (float)bgTex.width, (float)bgTex.height },
                Rectangle{ 0, 0, screenWidth, screenHeight },
                Vector2{ 0, 0 },
                0.0f,
                WHITE
            );
        }
    }

    // Floor Line
    DrawLine(0, (int)groundY, (int)screenWidth, (int)groundY, DARKGRAY);

    // Scene Header Title
    ResourceManager::DrawGameText(title.c_str(), 40, 40, 48, DARKGRAY);

    // Draw Doors
    for (const auto& door : doors) {
        if (door.sprite.IsValid()) {
            door.sprite.Draw(door.rect);
            DrawRectangleLinesEx(door.rect, 4, door.borderColor);
        } else {
            DrawRectangleRec(door.rect, door.doorColor);
            DrawRectangleLinesEx(door.rect, 4, door.borderColor);
            DrawCircle((int)(door.rect.x + door.rect.width - 20), (int)(door.rect.y + 120), 8, GOLD);
        }

        int labelWidth = ResourceManager::MeasureGameText(door.label.c_str(), 32);
        ResourceManager::DrawGameText(door.label.c_str(),
                 (int)(door.rect.x + (door.rect.width - labelWidth) / 2.0f),
                 (int)(door.rect.y - 50), 32, door.borderColor);
    }

    // Draw Items
    for (const auto& item : items) {
        item.Draw();
    }

    // Draw Visual Effects
    EffectManager::Get().Draw();

    // Player
    player.Draw();

    if (sceneShader.id != 0) {
        EndShaderMode();
    }

    // Draw Hold Q Gauge if currently charging
    if (holdQTimer > 0.0f && activeHoverItem) {
        Vector2 gaugePos = { activeHoverItem->rect.x + activeHoverItem->rect.width / 2.0f, activeHoverItem->rect.y - 80.0f };
        bool isRem = !MemoryManager::Get().IsRemembered(activeHoverItem->artifactId);
        DrawHoldQGauge(gaugePos, holdQTimer / holdQThreshold, isRem);
    }

    // Interaction Banner / Controls Guidance
    if (!DialogueManager::Get().IsActive() && !IsKeyDown(KEY_TAB) && !isPaused) {
        if (!promptText.empty()) {
            int textWidth = ResourceManager::MeasureGameText(promptText.c_str(), 36);
            int bannerX = ((int)screenWidth - textWidth - 80) / 2;
            DrawRectangle(bannerX, 690, textWidth + 80, 70, Fade(DARKGRAY, 0.85f));
            ResourceManager::DrawGameText(promptText.c_str(), ((int)screenWidth - textWidth) / 2, 706, 36, WHITE);
        } else {
            ResourceManager::DrawGameText(controlsHint.c_str(), 40, 736, 30, GRAY);
        }

        const char* tabHint = "[Hold TAB] Memory Archive | [ESC] Pause | [R] Reload";
        int tabW = ResourceManager::MeasureGameText(tabHint, 24);
        ResourceManager::DrawGameText(tabHint, (int)screenWidth - tabW - 40, 736, 24, Fade(DARKBLUE, 0.8f));
    }

    // Draw Dialogue Overlay
    DialogueManager::Get().Draw();

    // Draw Pause Menu Overlay if paused
    if (isPaused) {
        DrawPauseOverlay();
    }
}
