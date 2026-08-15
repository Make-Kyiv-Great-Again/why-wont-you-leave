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
    float btnW = 200.0f;
    float btnH = 70.0f;
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

    // Stop main menu sound when entering gameplay
    Music menuMusic = ResourceManager::Get().GetMusic("assets/sounds/menu_sound.wav");
    if (menuMusic.ctxData != nullptr && IsMusicStreamPlaying(menuMusic)) {
        StopMusicStream(menuMusic);
    }
}

DynamicScene::~DynamicScene() {
    Sound walkingSfx = ResourceManager::Get().GetSound("assets/sounds/walking_sound.mp3");
    if (walkingSfx.frameCount > 0 && IsSoundPlaying(walkingSfx)) {
        StopSound(walkingSfx);
    }
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
    // Play/loop game background music stream during gameplay
    Music gameMusic = ResourceManager::Get().GetMusic("assets/sounds/game_sound.mp3");
    if (gameMusic.ctxData != nullptr) {
        if (!IsMusicStreamPlaying(gameMusic)) {
            PlayMusicStream(gameMusic);
        }
        UpdateMusicStream(gameMusic);
    }

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

    // Handle 5th memory corridor blackout transition
    if (isCorridorTransitioning) {
        corridorTransitionTimer += dt;
        if (corridorTransitionTimer <= 0.45f) {
            corridorTransitionAlpha = corridorTransitionTimer / 0.45f;
        } else if (corridorTransitionTimer <= 0.95f) {
            corridorTransitionAlpha = 1.0f - (corridorTransitionTimer - 0.45f) / 0.50f;
        } else {
            corridorTransitionAlpha = 0.0f;
            isCorridorTransitioning = false;
        }
    }

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
            auto PlayDoorSfx = []() {
                Sound doorSfx = ResourceManager::Get().GetSound("assets/sounds/door_sound.mp3");
                if (doorSfx.frameCount > 0) {
                    StopSound(doorSfx);
                    PlaySound(doorSfx);
                }
            };

            if (door.targetScene == "exit_door") {
                if (ActManager::Get().CanUseExitDoor()) {
                    promptText = "Press [E] to Step Through Exit";
                    if (IsKeyPressed(KEY_E)) {
                        PlayDoorSfx();
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
                    PlayDoorSfx();
                    DialogueManager::Get().StartDialogueFile("assets/data/dialogues/act5_leave_choice.json");
                    return;
                }
            } else {
                promptText = "Press [E] to enter " + door.label;
                if (IsKeyPressed(KEY_E)) {
                    PlayDoorSfx();
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
                                Sound addSound = ResourceManager::Get().GetSound("assets/sounds/add_item_sound.mp3");
                                if (addSound.frameCount > 0) {
                                    StopSound(addSound);
                                    PlaySound(addSound);
                                } else {
                                    Sound selectSfx = ResourceManager::Get().GetSound("assets/sounds/select_sound.mp3");
                                    if (selectSfx.frameCount > 0) PlaySound(selectSfx);
                                }

                                if (sceneId == "corridor" && MemoryManager::Get().GetRememberedCount() == 5) {
                                    isCorridorTransitioning = true;
                                    corridorTransitionTimer = 0.0f;
                                    corridorTransitionAlpha = 0.0f;
                                    Sound doorSfx = ResourceManager::Get().GetSound("assets/sounds/door_sound.mp3");
                                    if (doorSfx.frameCount > 0) PlaySound(doorSfx);
                                }
                                promptText = "";
                                return;
                            }
                        }
                    } else {
                        holdQTimer = 0.0f;
                    }
                } else if (it->artifactId == "corridor_locked_door") {
                    if (MemoryManager::Get().GetRememberedCount() == 5) {
                        promptText = "Press [E] to Approach the Open Door";
                    } else {
                        promptText = "Press [E] to Examine " + it->name;
                    }
                    holdQTimer = 0.0f;
                } else {
                    promptText = "Press [E] to Examine " + it->name;
                    holdQTimer = 0.0f;
                }

                if (IsKeyPressed(KEY_E)) {
                    if (it->artifactId == "corridor_locked_door" && MemoryManager::Get().GetRememberedCount() == 5) {
                        DialogueManager::Get().StartDialogueFile("assets/data/dialogues/final_door_choice.json");
                    } else {
                        it->Interact();
                    }
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

void DynamicScene::DrawInteractionPrompt() {
    Texture2D btnETex = ResourceManager::Get().GetTexture("assets/sprites/button_E.png");
    Texture2D btnQTex = ResourceManager::Get().GetTexture("assets/sprites/button_Q.png");

    float btnSize = 42.0f;
    int fontSize = 30;
    float gap = 14.0f;
    float midGap = 20.0f;
    float centerY = 735.0f;

    if (activeHoverItem != nullptr) {
        if (activeHoverItem->isMemoryArtifact) {
            std::string qDesc = "Take Memory";
            std::string eDesc = "Inspect " + activeHoverItem->name;

            int qW = ResourceManager::MeasureGameText(qDesc.c_str(), (float)fontSize);
            int eW = ResourceManager::MeasureGameText(eDesc.c_str(), (float)fontSize);

            float totalW = qW + gap + btnSize + midGap + btnSize + gap + eW;
            float startX = (screenWidth - totalW) / 2.0f;

            // 1. Button Q Description (White with Black Outline)
            Color qColor = (holdQTimer > 0.05f) ? GOLD : WHITE;
            ResourceManager::DrawGameTextWithOutline(qDesc.c_str(), startX, centerY - fontSize / 2.0f, (float)fontSize, qColor, BLACK, 2.0f);

            // 2. Button Q Sprite
            float btnQX = startX + qW + gap;
            float btnQY = centerY - btnSize / 2.0f;
            if (btnQTex.id != 0) {
                float pulseQ = (holdQTimer > 0.05f) ? 3.0f : 0.0f;
                DrawTexturePro(btnQTex, Rectangle{ 0, 0, (float)btnQTex.width, (float)btnQTex.height },
                               Rectangle{ btnQX - pulseQ, btnQY - pulseQ, btnSize + pulseQ * 2.0f, btnSize + pulseQ * 2.0f },
                               Vector2{ 0, 0 }, 0.0f, WHITE);
            }

            // 3. Button E Sprite
            float btnEX = btnQX + btnSize + midGap;
            float btnEY = centerY - btnSize / 2.0f;
            if (btnETex.id != 0) {
                DrawTexturePro(btnETex, Rectangle{ 0, 0, (float)btnETex.width, (float)btnETex.height },
                               Rectangle{ btnEX, btnEY, btnSize, btnSize },
                               Vector2{ 0, 0 }, 0.0f, WHITE);
            }

            // 4. Button E Description (White with Black Outline)
            float eTextX = btnEX + btnSize + gap;
            ResourceManager::DrawGameTextWithOutline(eDesc.c_str(), eTextX, centerY - fontSize / 2.0f, (float)fontSize, WHITE, BLACK, 2.0f);
        } else {
            // Non-memory artifact item (e.g. mirror, oven, toilet, open door)
            std::string eDesc = "Examine " + activeHoverItem->name;
            if (activeHoverItem->artifactId == "corridor_locked_door" && MemoryManager::Get().GetRememberedCount() == 5) {
                eDesc = "Approach the Open Door";
            }
            int eW = ResourceManager::MeasureGameText(eDesc.c_str(), (float)fontSize);
            float totalW = btnSize + gap + eW;
            float startX = (screenWidth - totalW) / 2.0f;

            if (btnETex.id != 0) {
                DrawTexturePro(btnETex, Rectangle{ 0, 0, (float)btnETex.width, (float)btnETex.height },
                               Rectangle{ startX, centerY - btnSize / 2.0f, btnSize, btnSize },
                               Vector2{ 0, 0 }, 0.0f, WHITE);
            }
            ResourceManager::DrawGameTextWithOutline(eDesc.c_str(), startX + btnSize + gap, centerY - fontSize / 2.0f, (float)fontSize, WHITE, BLACK, 2.0f);
        }
    } else if (!promptText.empty()) {
        const std::string prefix = "Press [E] to ";
        if (promptText.rfind(prefix, 0) == 0) {
            std::string actionDesc = promptText.substr(prefix.length());
            if (!actionDesc.empty()) actionDesc[0] = (char)toupper(actionDesc[0]);
            
            int textW = ResourceManager::MeasureGameText(actionDesc.c_str(), (float)fontSize);
            float totalW = btnSize + gap + textW;
            float startX = (screenWidth - totalW) / 2.0f;

            if (btnETex.id != 0) {
                DrawTexturePro(btnETex, Rectangle{ 0, 0, (float)btnETex.width, (float)btnETex.height },
                               Rectangle{ startX, centerY - btnSize / 2.0f, btnSize, btnSize },
                               Vector2{ 0, 0 }, 0.0f, WHITE);
            }
            ResourceManager::DrawGameTextWithOutline(actionDesc.c_str(), startX + btnSize + gap, centerY - fontSize / 2.0f, (float)fontSize, WHITE, BLACK, 2.0f);
        } else {
            int textW = ResourceManager::MeasureGameText(promptText.c_str(), (float)fontSize);
            float startX = (screenWidth - textW) / 2.0f;
            ResourceManager::DrawGameTextWithOutline(promptText.c_str(), startX, centerY - fontSize / 2.0f, (float)fontSize, GOLD, BLACK, 2.0f);
        }
    }
}

void DynamicScene::DrawPauseOverlay() {
    // 1. Darken the game screen slightly
    DrawRectangle(0, 0, (int)screenWidth, (int)screenHeight, Fade(BLACK, 0.70f));

    float time = (float)GetTime();

    // 2. Pause Header (Custom sprite game_paused.png / game_paused.svg with fallback)
    Texture2D pauseTitleTex = ResourceManager::Get().GetTexture("assets/sprites/game_paused.png");
    if (pauseTitleTex.id == 0) {
        pauseTitleTex = ResourceManager::Get().GetTexture("assets/sprites/game_paused.svg");
    }

    if (pauseTitleTex.id != 0) {
        float titleW = 550.0f;
        float titleH = titleW * ((float)pauseTitleTex.height / (float)pauseTitleTex.width);
        float titleOffset = sinf(time * 1.5f) * 4.0f;
        Rectangle titleDest = {
            (screenWidth - titleW) / 2.0f,
            215.0f + titleOffset,
            titleW,
            titleH
        };
        DrawTexturePro(
            pauseTitleTex,
            Rectangle{ 0, 0, (float)pauseTitleTex.width, (float)pauseTitleTex.height },
            titleDest,
            Vector2{ 0, 0 },
            0.0f,
            WHITE
        );
    } else {
        const char* pauseTitle = "GAME PAUSED";
        int titleW = ResourceManager::MeasureGameText(pauseTitle, 48.0f);
        ResourceManager::DrawGameTextWithOutline(pauseTitle, (screenWidth - titleW) / 2.0f, 225.0f, 48.0f, Color{ 255, 245, 241, 255 }, Color{ 103, 90, 79, 255 }, 2.5f);
    }

    // 3. Helper Lambda to draw SVG pause buttons (NO text overlays)
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
    int hintW = ResourceManager::MeasureGameText(pauseHint, 24.0f);
    ResourceManager::DrawGameTextWithOutline(pauseHint, (screenWidth - hintW) / 2.0f, 700.0f, 24.0f, Fade(RAYWHITE, 0.85f), BLACK, 1.5f);

    // 4. Options Modal Overlay inside Pause (Horizontally Centered & Styled)
    if (showPauseOptions) {
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

void DynamicScene::Draw() {
    Color actTint = ActManager::Get().GetActLightingTint();
    ClearBackground(sceneId == "the_door" ? BLACK : backgroundColor);

    if (sceneShader.id != 0) {
        BeginShaderMode(sceneShader);
    }

    // Render Background Sprite Texture if specified
    if (!backgroundTexturePath.empty()) {
        std::string activeBgPath = backgroundTexturePath;
        if (sceneId == "corridor" && MemoryManager::Get().GetRememberedCount() == 5) {
            activeBgPath = "assets/sprites/coridor-final.png";
        }
        Texture2D bgTex = ResourceManager::Get().GetTexture(activeBgPath);
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

    // Interaction Prompt (Custom Button Sprites & Text Alignment)
    if (!DialogueManager::Get().IsActive() && !IsKeyDown(KEY_TAB) && !isPaused) {
        if (holdQTimer > 0.05f) {
            Vector2 gaugePos = { player.rect.x + player.rect.width / 2.0f, player.rect.y - 45.0f };
            DrawHoldQGauge(gaugePos, fminf(holdQTimer / 0.85f, 1.0f), true);
        }

        DrawInteractionPrompt();
    }

    // Draw 5th memory corridor blackout transition overlay
    if (corridorTransitionAlpha > 0.001f) {
        DrawRectangle(0, 0, (int)screenWidth, (int)screenHeight, Fade(BLACK, corridorTransitionAlpha));
    }

    // Draw Dialogue Overlay
    DialogueManager::Get().Draw();

    // Draw Pause Menu Overlay if paused
    if (isPaused) {
        DrawPauseOverlay();
    }
}
