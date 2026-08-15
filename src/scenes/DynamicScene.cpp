#include "scenes/DynamicScene.hpp"
#include "core/SceneManager.hpp"
#include "dialogue/DialogueManager.hpp"
#include "core/MemoryManager.hpp"
#include "core/EffectManager.hpp"
#include "core/ResourceManager.hpp"
#include "core/ActManager.hpp"
#include "scenes/MainMenuScene.hpp"
#include "scenes/ActTitleScene.hpp"
#include <nlohmann/json.hpp>
#include <cmath>
#include <iostream>

DynamicScene::DynamicScene(const std::string& sceneId, float spawnPlayerX, const std::string& targetDoorLabel)
    : sceneId(sceneId),
      jsonPath("assets/data/scenes/" + sceneId + ".json"),
      screenWidth(2000.0f),
      screenHeight(800.0f),
      groundY(760.0f),
      backgroundColor(RAYWHITE),
      player(spawnPlayerX >= 0.0f ? spawnPlayerX : 200.0f, 760.0f) {
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

            door.isVisible = doorJson.value("is_visible", true);
            if (doorJson.contains("is_transparent") && doorJson["is_transparent"].get<bool>()) {
                door.isVisible = false;
            }

            doors.push_back(door);
        }
    }

    // Load Items (filtered by vanished status)
    if (j.contains("items") && j["items"].is_array()) {
        for (const auto& itemJson : j["items"]) {
            std::string artifactId = itemJson.value("artifact_id", "item");
            
            // If already remembered/vanished, do not spawn
            if (ActManager::Get().IsArtifactVanished(artifactId)) {
                continue;
            }

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

            bool isVisible = itemJson.value("is_visible", false);
            bool isMemoryArtifact = itemJson.value("is_memory_artifact", true);

            items.emplace_back(artifactId, rect, color, borderColor, name, dialogueFile, itemSprite, isVisible, isMemoryArtifact);

            // Register with MemoryManager only if it is a memory artifact
            if (isMemoryArtifact) {
                int roomId = 0;
                if (sceneId == "bedroom") roomId = 1;
                else if (sceneId == "bathroom") roomId = 2;
                else if (sceneId == "kitchen") roomId = 3;
                else if (sceneId == "corridor") roomId = 0;
                MemoryManager::Get().RegisterArtifact(artifactId, name, itemSprite.texturePath, color, borderColor, roomId);
            }
        }
    }

    // Player Lighting Shader support
    hasPlayerShader = false;
    if (j.contains("player_lighting") && j["player_lighting"].is_object()) {
        auto pl = j["player_lighting"];
        std::string shaderFile = pl.value("shader", "assets/shaders/glsl330/player_lighting.fs");
        playerLightingShader = ResourceManager::Get().GetShader(shaderFile);

        if (playerLightingShader.id != 0) {
            hasPlayerShader = true;
            ambientColorLoc = GetShaderLocation(playerLightingShader, "ambientColor");
            brightnessLoc   = GetShaderLocation(playerLightingShader, "brightness");
            warmthLoc       = GetShaderLocation(playerLightingShader, "warmth");

            Color ambCol = ResourceManager::HexToColor(pl.value("ambient_color", "#FFFFFF"));
            ambientColor = { ambCol.r / 255.0f, ambCol.g / 255.0f, ambCol.b / 255.0f };
            playerBrightness = pl.value("brightness", 1.0f);
            playerWarmth     = pl.value("warmth", 1.0f);

            SetShaderValue(playerLightingShader, ambientColorLoc, &ambientColor, SHADER_UNIFORM_VEC3);
            SetShaderValue(playerLightingShader, brightnessLoc, &playerBrightness, SHADER_UNIFORM_FLOAT);
            SetShaderValue(playerLightingShader, warmthLoc, &playerWarmth, SHADER_UNIFORM_FLOAT);
        }
    }

    // Ambient Dust Particles configuration
    if (j.contains("dust_particles") && j["dust_particles"].is_object()) {
        auto dp = j["dust_particles"];
        bool enabled = dp.value("enabled", true);
        EffectManager::Get().SetDustEnabled(enabled);
        if (enabled) {
            int count = dp.value("count", 60);
            Color dColor = ResourceManager::HexToColor(dp.value("color", "#FFF8E7"));
            EffectManager::Get().InitDustParticles(count, dColor);
        }
    } else {
        EffectManager::Get().SetDustEnabled(true);
    }

    // Scene Shader support
    if (j.contains("shader") && j["shader"].contains("path")) {
        shaderPath = j["shader"]["path"].get<std::string>();
        sceneShader = ResourceManager::Get().GetShader(shaderPath);
    }
}

void DynamicScene::UpdatePauseMenu(float dt) {
    Vector2 virtualMouse = SceneManager::Get().GetVirtualMousePosition();
    int prevSelected = pauseSelectedBtn;

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

    if (pauseSelectedBtn != prevSelected) {
        Sound sfx = ResourceManager::Get().GetSound("assets/sounds/select_sound.mp3");
        if (sfx.frameCount > 0) {
            StopSound(sfx);
            PlaySound(sfx);
        }
    }

    bool clickPressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool actionPressed = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_E) || clickPressed;

    if (actionPressed) {
        Sound sfx = ResourceManager::Get().GetSound("assets/sounds/select_sound.mp3");
        if (sfx.frameCount > 0) {
            StopSound(sfx);
            PlaySound(sfx);
        }

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
        Sound walkingSfx = ResourceManager::Get().GetSound("assets/sounds/walking_sound.mp3");
        if (walkingSfx.frameCount > 0 && IsSoundPlaying(walkingSfx)) {
            StopSound(walkingSfx);
        }
        UpdatePauseMenu(dt);
        return;
    }

    SceneManager::Get().SetTabPressed(IsKeyDown(KEY_TAB));

    // Hot-reload scene JSON when pressing R
    if (IsKeyPressed(KEY_R)) {
        LoadFromConfigFile(jsonPath);
    }

    // Update sprite animations (items are never erased from scene)
    for (auto& item : items) {
        item.Update(dt);
    }
    for (auto& door : doors) {
        door.sprite.Update(dt);
    }

    // Update effects
    EffectManager::Get().Update(dt);

    // Shimmer glint on interactable doors and active items (every ~1.2 seconds)
    sparkleTimer += dt;
    if (sparkleTimer >= 1.2f) {
        sparkleTimer = 0.0f;
        for (const auto& door : doors) {
            Vector2 handlePos = {
                door.rect.x + (door.isVisible ? (door.rect.width - 20.0f) : (door.rect.width * 0.45f)),
                door.rect.y + (door.isVisible ? 120.0f : 240.0f)
            };
            EffectManager::Get().SpawnShimmerGlint(handlePos, 14.0f, Color{ 220, 240, 255, 255 });
        }
        for (const auto& item : items) {
            // Ambient items always shimmer; story memory items shimmer only when not in TAB
            if (!item.isMemoryArtifact || !ActManager::Get().IsArtifactRemembered(item.artifactId)) {
                Vector2 itemCorner = { item.rect.x + item.rect.width * 0.5f, item.rect.y + item.rect.height * 0.4f };
                EffectManager::Get().SpawnShimmerGlint(itemCorner, 14.0f, Color{ 240, 248, 255, 255 });
            }
        }
    }

    // Pause if Tab memory archive is open
    if (IsKeyDown(KEY_TAB)) {
        holdQTimer = 0.0f;
        player.state = PlayerState::Idle;
        Sound walkingSfx = ResourceManager::Get().GetSound("assets/sounds/walking_sound.mp3");
        if (walkingSfx.frameCount > 0 && IsSoundPlaying(walkingSfx)) {
            StopSound(walkingSfx);
        }
        return;
    }

    // Update dialogue system
    DialogueManager::Get().Update(dt);

    // Freeze player if dialogue is active
    if (DialogueManager::Get().IsActive()) {
        promptText = "";
        holdQTimer = 0.0f;
        activeHoverItem = nullptr;
        player.state = PlayerState::Idle;
        Sound walkingSfx = ResourceManager::Get().GetSound("assets/sounds/walking_sound.mp3");
        if (walkingSfx.frameCount > 0 && IsSoundPlaying(walkingSfx)) {
            StopSound(walkingSfx);
        }
        return;
    }

    // Handle Exit Door repulsion and pain distortion sequence
    if (isExitDistortionActive) {
        exitDistortionTimer += dt;

        // Step 1: Force player to step backward away from the exit door (moving right away from x=100)
        if (exitDistortionTimer <= 0.8f) {
            player.ForceMove(220.0f * dt, dt);
        } else {
            player.state = PlayerState::Idle;
            Sound walkingSfx = ResourceManager::Get().GetSound("assets/sounds/walking_sound.mp3");
            if (walkingSfx.frameCount > 0 && IsSoundPlaying(walkingSfx)) {
                StopSound(walkingSfx);
            }
        }

        // Step 2: Once repulsion & pain sequence ends, trigger dialogue or act transition
        if (exitDistortionTimer >= exitDistortionMaxTime) {
            isExitDistortionActive = false;
            exitDistortionTimer = 0.0f;
            if (pendingExitIsSceneChange) {
                SceneManager::Get().ChangeScene(std::make_unique<ActTitleScene>(5, pendingExitTargetScene));
            } else if (!pendingExitDialogue.empty()) {
                DialogueManager::Get().StartDialogueFile(pendingExitDialogue);
            }
        }
        return; // Lock player input during exit repulsion sequence
    }

    // Normal player movement
    player.Update(dt, screenWidth);

    promptText = "";
    activeHoverItem = nullptr;

    // Check interaction with Doors
    for (const auto& door : doors) {
        if (CheckCollisionRecs(player.rect, door.rect)) {
            if (door.targetScene == "exit_door") {
                if (ActManager::Get().CanUseExitDoor()) {
                    promptText = "Press [E] to Step Through Exit";
                    if (IsKeyPressed(KEY_E)) {
                        isExitDistortionActive = true;
                        exitDistortionTimer = 0.0f;
                        pendingExitIsSceneChange = true;
                        pendingExitTargetScene = "the_door";
                        return;
                    }
                } else {
                    promptText = "Exit is locked (Explore more memories first)";
                }
            } else if (door.targetScene == "act5_choice") {
                promptText = "Press [E] to open THE DOOR";
                if (IsKeyPressed(KEY_E)) {
                    DialogueManager::Get().StartDialogueFile("assets/data/dialogues/act5_leave_choice.json");
                    return;
                }
            } else {
                promptText = "Press [E] to enter " + door.label;
                if (IsKeyPressed(KEY_E)) {
                    SceneManager::Get().ChangeScene(std::make_unique<DynamicScene>(door.targetScene, door.targetSpawnX, door.targetDoor));
                    return;
                }
            }
            break;
        }
    }

    // Check interaction with Room items (only if not hovering a door)
    bool hoveringAnyItem = false;
    if (promptText.empty()) {
        for (auto it = items.begin(); it != items.end(); ++it) {
            // If item is already in TAB memory, skip interaction
            if (ActManager::Get().IsArtifactRemembered(it->artifactId)) continue;

            if (it->CheckCollision(player.rect)) {
                hoveringAnyItem = true;
                activeHoverItem = &(*it);

                bool isArtifact = (it->artifactId != "bedroom_mirror" && it->artifactId != "corridor_locked_door" && 
                                   it->artifactId != "kitchen_oven" && it->artifactId != "bathroom_toilet");

                if (it->isMemoryArtifact) {
                    promptText = "Press [E] to Inspect " + it->name + " | Hold [Q] to Take Memory";

                    if (IsKeyDown(KEY_Q)) {
                        holdQTimer += dt;
                        if (holdQTimer >= 0.85f) {
                            if (MemoryManager::Get().GetRememberedCount() >= 5) {
                                promptText = "Memory Archive Full (Max 5 items)!";
                            } else {
                                ActManager::Get().MarkArtifactRemembered(it->artifactId);
                                holdQTimer = 0.0f;
                                EffectManager::Get().SpawnForgettingEffect(it->rect, GOLD);
                                Sound selectSfx = ResourceManager::Get().GetSound("assets/sounds/select_sound.mp3");
                                if (selectSfx.frameCount > 0) PlaySound(selectSfx);
                                promptText = "";
                                return;
                            }
                        }
                    } else {
                        holdQTimer = 0.0f;
                    }
                } else {
                    promptText = "Press [E] to Examine " + it->name;
                    holdQTimer = 0.0f;
                }

                if (IsKeyPressed(KEY_E)) {
                    it->Interact();
                    promptText = "";
                    return;
                }
                break;
            }
        }
    }

    if (!hoveringAnyItem) {
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
    int titleW = ResourceManager::MeasureGameText(pauseTitle, 50);
    ResourceManager::DrawGameText(pauseTitle, (int)(screenWidth - titleW) / 2, 240, 50, GOLD);

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
    int hintW = ResourceManager::MeasureGameText(pauseHint, 24);
    ResourceManager::DrawGameText(pauseHint, (int)(screenWidth - hintW) / 2, 700, 24, Fade(WHITE, 0.85f));

    // Options Popup Overlay inside Pause
    if (showPauseOptions) {
        DrawRectangle(400, 200, 1200, 400, Fade(BLACK, 0.92f));
        DrawRectangleLinesEx(Rectangle{ 400, 200, 1200, 400 }, 4.0f, GOLD);
        
        ResourceManager::DrawGameText("OPTIONS / SETTINGS", 760, 250, 40, GOLD);
        ResourceManager::DrawGameText("- Fullscreen Mode: Press F11 or Alt+Enter at any time", 500, 340, 28, WHITE);
        ResourceManager::DrawGameText("- Hot Reload Scenes: Press R during gameplay", 500, 390, 28, WHITE);
        ResourceManager::DrawGameText("Press [Enter] or click Options again to close", 650, 520, 24, GRAY);
    }
}

void DynamicScene::Draw() {
    Color actTint = ActManager::Get().GetActLightingTint();
    ClearBackground(sceneId == "the_door" ? BLACK : backgroundColor);

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
                actTint
            );
        }
    }

    // Scene Header Title
    if (sceneId != "the_door") {
        ResourceManager::DrawGameText(title.c_str(), 40, 40, 44, DARKGRAY);
    }

    // Draw Doors
    float time = (float)GetTime();
    for (const auto& door : doors) {
        if (!door.isVisible) {
            continue; // Skip rendering for invisible/background-integrated doors
        }

        if (sceneId == "the_door") {
            // White Door glowing effect
            float glowPulse = (sinf(time * 3.0f) + 1.0f) * 0.5f;
            Rectangle glowRect = { door.rect.x - 12.0f - glowPulse * 6.0f, door.rect.y - 12.0f - glowPulse * 6.0f,
                                   door.rect.width + 24.0f + glowPulse * 12.0f, door.rect.height + 24.0f + glowPulse * 12.0f };
            DrawRectangleRec(glowRect, Fade(WHITE, 0.25f + glowPulse * 0.2f));
            DrawRectangleRec(door.rect, WHITE);
            DrawRectangleLinesEx(door.rect, 4.0f, GOLD);
        } else if (door.sprite.IsValid()) {
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
                 (int)(door.rect.y - 50), 32, (sceneId == "the_door") ? WHITE : door.borderColor);
    }

    // Draw Items
    for (const auto& item : items) {
        item.Draw();
    }

    // Draw Visual Effects
    EffectManager::Get().Draw();

    // Player with room-specific lighting shader
    if (hasPlayerShader && playerLightingShader.id != 0) {
        BeginShaderMode(playerLightingShader);
        player.Draw();
        EndShaderMode();
    } else {
        player.Draw();
    }

    // Draw Pain Vignette and Screen Darkness overlay during exit repulsion
    if (isExitDistortionActive) {
        float pulse = (sinf(exitDistortionTimer * 16.0f) + 1.0f) * 0.5f;

        // Heavy red & black pain pulse
        DrawRectangle(0, 0, (int)screenWidth, (int)screenHeight, Fade(BLACK, 0.45f + pulse * 0.25f));
        DrawRectangle(0, 0, (int)screenWidth, (int)screenHeight, Fade(RED, 0.15f + pulse * 0.2f));

        // Dark vignette edges
        int borderThickness = 120 + (int)(pulse * 40.0f);
        DrawRectangle(0, 0, (int)screenWidth, borderThickness, Fade(BLACK, 0.85f));
        DrawRectangle(0, (int)screenHeight - borderThickness, (int)screenWidth, borderThickness, Fade(BLACK, 0.85f));
        DrawRectangle(0, 0, borderThickness, (int)screenHeight, Fade(BLACK, 0.85f));
        DrawRectangle((int)screenWidth - borderThickness, 0, borderThickness, (int)screenHeight, Fade(BLACK, 0.85f));
    }

    if (sceneShader.id != 0) {
        EndShaderMode();
    }

    // Interaction Banner
    if (!DialogueManager::Get().IsActive() && !IsKeyDown(KEY_TAB) && !isPaused) {
        if (holdQTimer > 0.05f) {
            Vector2 gaugePos = { player.rect.x + player.rect.width / 2.0f, player.rect.y - 45.0f };
            DrawHoldQGauge(gaugePos, fminf(holdQTimer / 0.85f, 1.0f), true);
        }

        if (!promptText.empty()) {
            int textWidth = ResourceManager::MeasureGameText(promptText.c_str(), 32);
            int bannerX = ((int)screenWidth - textWidth - 80) / 2;
            DrawRectangle(bannerX, 690, textWidth + 80, 70, Fade(DARKGRAY, 0.85f));
            ResourceManager::DrawGameText(promptText.c_str(), ((int)screenWidth - textWidth) / 2, 708, 32, WHITE);
        }
    }

    // Draw Dialogue Overlay
    DialogueManager::Get().Draw();

    // Draw Pause Menu Overlay if paused
    if (isPaused) {
        DrawPauseOverlay();
    }
}
