#include "dialogue/DialogueManager.hpp"
#include "core/MemoryManager.hpp"
#include "core/ResourceManager.hpp"
#include "core/ActManager.hpp"
#include "core/SceneManager.hpp"
#include "scenes/DynamicScene.hpp"
#include "scenes/ActTitleScene.hpp"
#include "scenes/MainMenuScene.hpp"
#include <sstream>
#include <vector>
#include <cmath>

DialogueManager& DialogueManager::Get() {
    static DialogueManager instance;
    return instance;
}

void DialogueManager::StartDialogue(const DialogueTree& tree, bool isMemory, const std::string& artifactId) {
    currentTree = tree;
    
    int startNode = currentTree.startNodeId;
    bool forceMemory = isMemory;
    
    if (!artifactId.empty() && ActManager::Get().IsArtifactRemembered(artifactId)) {
        if (currentTree.GetNode(100) != nullptr) {
            startNode = 100;
            forceMemory = true;
        }
    }

    currentNode = currentTree.GetNode(startNode);
    selectedOption = 0;
    isActive = (currentNode != nullptr);
    isMemoryMode = forceMemory;
    activeArtifactId = artifactId;
    pulseTimer = 0.0f;
    memoryItemAnimTimer = 0.0f;

    // Reset typing state
    visibleChars = 0;
    typeTimer = 0.0f;
    displayedText = "";

    if (currentNode) {
        if (currentNode->id >= 100) {
            isMemoryMode = true;
        }

        Sound sfx = ResourceManager::Get().GetSound("assets/sounds/ui_typing_sound.mp3");
        if (sfx.frameCount > 0) {
            SetSoundVolume(sfx, 2.0f);
            StopSound(sfx);
            
            float soundDuration = (sfx.stream.sampleRate > 0) ? (float)sfx.frameCount / sfx.stream.sampleRate : 0.0f;
            float defaultSpeed = 0.035f;
            float totalLen = (float)currentNode->text.length();
            
            if (soundDuration > 0.0f && (totalLen * defaultSpeed) > soundDuration) {
                typeSpeed = soundDuration / totalLen;
            } else {
                typeSpeed = defaultSpeed;
            }
            
            PlaySound(sfx);
        } else {
            typeSpeed = 0.035f;
        }
    }
}

void DialogueManager::StartDialogueFile(const std::string& jsonPath, bool isMemory, const std::string& artifactId) {
    activeScriptPath = jsonPath;
    nlohmann::json j = ResourceManager::Get().LoadJson(jsonPath);
    if (!j.is_null()) {
        DialogueTree tree = Dialogues::FromJson(j);
        StartDialogue(tree, isMemory, artifactId);
    }
}

bool DialogueManager::IsActive() const {
    return isActive;
}

bool DialogueManager::IsMemoryMode() const {
    return isMemoryMode;
}

void DialogueManager::OnDialogueFinished() {
    if (activeScriptPath == "assets/data/dialogues/act5_leave_choice.json") {
        SceneManager::Get().ChangeScene(std::make_unique<MainMenuScene>());
        return;
    }
}

void DialogueManager::Update(float dt) {
    bool shouldBlackout = isActive && currentNode && (currentNode->isBlackout || currentNode->isMemory || isMemoryMode);
    float targetAlpha = shouldBlackout ? 0.98f : 0.0f;
    blackoutAlpha += (targetAlpha - blackoutAlpha) * fminf(dt * 7.0f, 1.0f);

    if (shouldBlackout || blackoutAlpha > 0.05f) {
        memoryItemAnimTimer += dt;
    } else {
        memoryItemAnimTimer = 0.0f;
    }

    if (!isActive || !currentNode) return;

    pulseTimer += dt * 4.0f;

    // Handle typing animation progress
    bool isTypingFinished = (visibleChars >= currentNode->text.length());
    if (!isTypingFinished) {
        typeTimer += dt;
        if (typeTimer >= typeSpeed) {
            typeTimer = 0.0f;
            visibleChars++;
            displayedText = currentNode->text.substr(0, visibleChars);
        }

        // Allow skipping typing animation with keypresses
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_E)) {
            visibleChars = currentNode->text.length();
            displayedText = currentNode->text;
            
            Sound sfx = ResourceManager::Get().GetSound("assets/sounds/ui_typing_sound.mp3");
            if (sfx.frameCount > 0) {
                StopSound(sfx);
            }
        }
        return; // Lock choice inputs and next dialogue triggers during typing
    } else {
        // Stop sound once typing finishes naturally
        Sound sfx = ResourceManager::Get().GetSound("assets/sounds/ui_typing_sound.mp3");
        if (sfx.frameCount > 0 && IsSoundPlaying(sfx)) {
            StopSound(sfx);
        }
    }

    bool hasOptions = !currentNode->options.empty();

    if (hasOptions) {
        int optCount = (int)currentNode->options.size();
        int prevSelected = selectedOption;

        // Arrow navigation
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            selectedOption = (selectedOption - 1 + optCount) % optCount;
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            selectedOption = (selectedOption + 1) % optCount;
        }

        // Direct number keys (1-4)
        if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1)) if (optCount >= 1) { selectedOption = 0; }
        if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2)) if (optCount >= 2) { selectedOption = 1; }
        if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3)) if (optCount >= 3) { selectedOption = 2; }
        if (IsKeyPressed(KEY_FOUR) || IsKeyPressed(KEY_KP_4)) if (optCount >= 4) { selectedOption = 3; }

        if (selectedOption != prevSelected) {
            Sound sfx = ResourceManager::Get().GetSound("assets/sounds/select_sound.mp3");
            if (sfx.frameCount > 0) {
                SetSoundVolume(sfx, 2.0f);
                StopSound(sfx);
                PlaySound(sfx);
            }
        }

        // Confirm choice
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            Sound selectSfx = ResourceManager::Get().GetSound("assets/sounds/select_sound.mp3");
            if (selectSfx.frameCount > 0) {
                SetSoundVolume(selectSfx, 2.0f);
                StopSound(selectSfx);
                PlaySound(selectSfx);
            }

            std::string chosenText = currentNode->options[selectedOption].text;
            if (!activeArtifactId.empty()) {
                MemoryManager::Get().SaveChoice(activeArtifactId, chosenText);
            }

            if (chosenText == "Remember") {
                isMemoryMode = true;
                memoryItemAnimTimer = 0.0f;
            }

            int nextId = currentNode->options[selectedOption].targetNodeId;
            
            Sound sfx = ResourceManager::Get().GetSound("assets/sounds/ui_typing_sound.mp3");
            if (sfx.frameCount > 0) {
                StopSound(sfx);
            }

            if (nextId == -1) {
                isActive = false;
                currentNode = nullptr;
                OnDialogueFinished();
                return;
            } else {
                currentNode = currentTree.GetNode(nextId);
                selectedOption = 0;
                visibleChars = 0;
                displayedText = "";
                if (currentNode) {
                    if (currentNode->id >= 100) {
                        isMemoryMode = true;
                    }
                    if (sfx.frameCount > 0) {
                        SetSoundVolume(sfx, 2.0f);
                        float soundDuration = (sfx.stream.sampleRate > 0) ? (float)sfx.frameCount / sfx.stream.sampleRate : 0.0f;
                        float defaultSpeed = 0.035f;
                        float totalLen = (float)currentNode->text.length();
                        if (soundDuration > 0.0f && (totalLen * defaultSpeed) > soundDuration) {
                            typeSpeed = soundDuration / totalLen;
                        } else {
                            typeSpeed = defaultSpeed;
                        }
                        PlaySound(sfx);
                    } else {
                        typeSpeed = 0.035f;
                    }
                } else {
                    isActive = false;
                }
            }
        }
    } else {
        // Linear advance
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_E)) {
            int nextNodeId = currentNode->nextNodeId;
            
            Sound sfx = ResourceManager::Get().GetSound("assets/sounds/ui_typing_sound.mp3");
            if (sfx.frameCount > 0) {
                StopSound(sfx);
            }

            if (nextNodeId == -1) {
                isActive = false;
                currentNode = nullptr;
                OnDialogueFinished();
            } else {
                currentNode = currentTree.GetNode(nextNodeId);
                selectedOption = 0;
                visibleChars = 0;
                displayedText = "";
                if (currentNode) {
                    if (currentNode->id >= 100) {
                        isMemoryMode = true;
                    }
                    if (sfx.frameCount > 0) {
                        float soundDuration = (sfx.stream.sampleRate > 0) ? (float)sfx.frameCount / sfx.stream.sampleRate : 0.0f;
                        float defaultSpeed = 0.035f;
                        float totalLen = (float)currentNode->text.length();
                        if (soundDuration > 0.0f && (totalLen * defaultSpeed) > soundDuration) {
                            typeSpeed = soundDuration / totalLen;
                        } else {
                            typeSpeed = defaultSpeed;
                        }
                        PlaySound(sfx);
                    } else {
                        typeSpeed = 0.035f;
                    }
                } else {
                    isActive = false;
                    OnDialogueFinished();
                }
            }
        }
    }
}

void DialogueManager::DrawWrappedText(const char* text, int posX, int posY, int fontSize, int maxLineWidth, Color color) {
    std::string textStr(text);
    std::istringstream words(textStr);
    std::string word;
    std::string currentLine = "";
    int currentY = posY;
    int lineSpacing = fontSize + 8;

    while (words >> word) {
        std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
        int textWidth = ResourceManager::MeasureGameText(testLine.c_str(), fontSize);
        if (textWidth > maxLineWidth && !currentLine.empty()) {
            ResourceManager::DrawGameText(currentLine.c_str(), posX, currentY, fontSize, color);
            currentLine = word;
            currentY += lineSpacing;
        } else {
            currentLine = testLine;
        }
    }
    if (!currentLine.empty()) {
        ResourceManager::DrawGameText(currentLine.c_str(), posX, currentY, fontSize, color);
    }
}

void DialogueManager::Draw() {
    if (!isActive || !currentNode) return;

    Vector2 mousePos = SceneManager::Get().GetVirtualMousePosition();
    bool hasOptions = !currentNode->options.empty();
    bool isTypingFinished = (visibleChars >= currentNode->text.length());
    bool isMemoryActive = (blackoutAlpha > 0.02f) && (isMemoryMode || (currentNode && (currentNode->isMemory || currentNode->isBlackout)));

    // 1. Memory Mode / Blackout Dark Backdrop & Header
    if (blackoutAlpha > 0.02f) {
        DrawRectangle(0, 0, 2000, 800, Fade(BLACK, blackoutAlpha));

        if (currentNode && (currentNode->isMemory || currentNode->isBlackout)) {
            const char* memTitle = "✦ MEMORY ✦";
            int mtw = ResourceManager::MeasureGameText(memTitle, 36);
            ResourceManager::DrawGameText(memTitle, (2000 - mtw) / 2, 45, 36, Fade(GOLD, blackoutAlpha * 0.85f));
        }
    }

    // Floating Animated Memory Item in Center/Upper Screen (no background circles)
    const MemoryArtifact* memArt = nullptr;
    if (!activeArtifactId.empty()) {
        memArt = MemoryManager::Get().GetArtifact(activeArtifactId);
    }
    if (memArt == nullptr && !activeScriptPath.empty()) {
        for (const auto& a : MemoryManager::Get().GetAllArtifacts()) {
            if (activeScriptPath.find(a.id) != std::string::npos) {
                memArt = &a;
                break;
            }
        }
    }

    bool hasItemToDraw = (isMemoryActive && memArt != nullptr && !memArt->texturePath.empty());

    if (hasItemToDraw) {
        float animDuration = 0.65f;
        float progress = fminf(memoryItemAnimTimer / animDuration, 1.0f);
        // Smooth cubic ease-out
        float ease = 1.0f - powf(1.0f - progress, 3.0f);
        float spriteAlpha = ease * (blackoutAlpha / 0.98f);

        float centerX = 1000.0f;
        float centerY = (isTypingFinished && hasOptions) ? 220.0f : 275.0f;
        float floatOffset = sinf((float)GetTime() * 2.2f) * 6.0f;

        float baseSize = (isTypingFinished && hasOptions) ? 150.0f : 180.0f;
        float drawSize = baseSize * (0.80f + 0.20f * ease);

        // Draw Item Texture with fading transparency animation
        Texture2D itemTex = ResourceManager::Get().GetTexture(memArt->texturePath);
        if (itemTex.id != 0) {
            Rectangle srcRect = { 0.0f, 0.0f, (float)itemTex.width, (float)itemTex.height };
            Rectangle destRect = {
                centerX - drawSize / 2.0f,
                centerY - drawSize / 2.0f + floatOffset,
                drawSize,
                drawSize
            };
            DrawTexturePro(itemTex, srcRect, destRect, Vector2{ 0, 0 }, 0.0f, Fade(WHITE, spriteAlpha));
        }

        // Draw Item Name underneath
        int nameWidth = ResourceManager::MeasureGameText(memArt->name.c_str(), 24);
        ResourceManager::DrawGameText(memArt->name.c_str(), (int)(centerX - nameWidth / 2.0f), (int)(centerY + drawSize / 2.0f + 8.0f + floatOffset), 24, Fade(memArt->color, spriteAlpha * 0.90f));
    }

    // 2. Response Options (Pale Vertical Buttons)
    if (isTypingFinished && hasOptions) {
        int optCount = (int)currentNode->options.size();
        int cardWidth = 720;
        int cardHeight = hasItemToDraw ? 44 : 50;
        int spacing = hasItemToDraw ? 10 : 14;
        int totalHeight = optCount * cardHeight + (optCount - 1) * spacing;
        
        int cardX = (2000 - cardWidth) / 2;
        int startY = hasItemToDraw ? 360 : ((800 - totalHeight) / 2 - 50);

        for (int i = 0; i < optCount; i++) {
            int cardY = startY + i * (cardHeight + spacing);
            Rectangle cardRect = { (float)cardX, (float)cardY, (float)cardWidth, (float)cardHeight };

            bool isHovered = CheckCollisionPointRec(mousePos, cardRect);
            if (isHovered) {
                if (selectedOption != i) {
                    selectedOption = i;
                    Sound sfx = ResourceManager::Get().GetSound("assets/sounds/select_sound.mp3");
                    if (sfx.frameCount > 0) {
                        StopSound(sfx);
                        PlaySound(sfx);
                    }
                }
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    Sound selectSfx = ResourceManager::Get().GetSound("assets/sounds/select_sound.mp3");
                    if (selectSfx.frameCount > 0) {
                        StopSound(selectSfx);
                        PlaySound(selectSfx);
                    }

                    std::string chosenText = currentNode->options[i].text;
                    if (!activeArtifactId.empty()) {
                        MemoryManager::Get().SaveChoice(activeArtifactId, chosenText);
                    }

                    if (chosenText == "Remember") {
                        isMemoryMode = true;
                        memoryItemAnimTimer = 0.0f;
                    }

                    int nextId = currentNode->options[i].targetNodeId;
                    
                    Sound sfx = ResourceManager::Get().GetSound("assets/sounds/ui_typing_sound.mp3");
                    if (sfx.frameCount > 0) {
                        StopSound(sfx);
                    }

                    if (nextId == -1) {
                        isActive = false;
                        currentNode = nullptr;
                        OnDialogueFinished();
                        return;
                    } else {
                        currentNode = currentTree.GetNode(nextId);
                        selectedOption = 0;
                        visibleChars = 0;
                        displayedText = "";
                        if (currentNode) {
                            if (currentNode->id >= 100) {
                                isMemoryMode = true;
                            }
                            if (sfx.frameCount > 0) {
                                float soundDuration = (sfx.stream.sampleRate > 0) ? (float)sfx.frameCount / sfx.stream.sampleRate : 0.0f;
                                float defaultSpeed = 0.035f;
                                float totalLen = (float)currentNode->text.length();
                                if (soundDuration > 0.0f && (totalLen * defaultSpeed) > soundDuration) {
                                    typeSpeed = soundDuration / totalLen;
                                } else {
                                    typeSpeed = defaultSpeed;
                                }
                                PlaySound(sfx);
                            } else {
                                typeSpeed = 0.035f;
                            }
                        } else {
                            isActive = false;
                            OnDialogueFinished();
                        }
                        return;
                    }
                }
            }

            bool isSelected = (selectedOption == i);

            // Small pale vertical buttons styling
            Color bgColor = isSelected ? Fade(WHITE, 0.22f) : Fade(BLACK, 0.70f);
            Color borderColor = isSelected ? Fade(WHITE, 0.90f) : Fade(LIGHTGRAY, 0.35f);
            Color textColor = isSelected ? WHITE : Fade(RAYWHITE, 0.85f);

            DrawRectangleRec(cardRect, bgColor);
            DrawRectangleLinesEx(cardRect, isSelected ? 2.5f : 1.5f, borderColor);

            // Subtle gold accent dot on selection
            if (isSelected) {
                DrawCircle((int)cardRect.x + 22, (int)cardRect.y + cardHeight / 2, 4.5f, GOLD);
            }

            std::string optText = currentNode->options[i].text;
            ResourceManager::DrawGameText(optText.c_str(), (int)cardRect.x + (isSelected ? 40 : 26), (int)cardRect.y + (cardHeight - 24) / 2, 24, textColor);
        }
    }

    // 3. Dialogue Box at the Bottom (Black Semi-Transparent Rectangle)
    int boxWidth = 1760;
    int boxHeight = 190;
    int boxX = (2000 - boxWidth) / 2;
    int boxY = 800 - boxHeight - 30; // Positioned cleanly at the bottom

    // Black semi-transparent background
    DrawRectangle(boxX, boxY, boxWidth, boxHeight, Fade(BLACK, 0.88f));
    DrawRectangleLinesEx({ (float)boxX, (float)boxY, (float)boxWidth, (float)boxHeight }, 2.0f, Fade(LIGHTGRAY, 0.40f));

    // Speaker Name Tag (Top Left of Bottom Box)
    if (!currentNode->speaker.empty()) {
        DrawRectangle(boxX + 25, boxY - 20, 240, 40, Fade(BLACK, 0.95f));
        DrawRectangleLinesEx({ (float)boxX + 25, (float)boxY - 20, 240.0f, 40.0f }, 2.0f, Fade(LIGHTGRAY, 0.50f));
        ResourceManager::DrawGameText(currentNode->speaker.c_str(), boxX + 40, boxY - 14, 26, GOLD);
    }

    // Dialogue Text (Animated visible letters)
    DrawWrappedText(displayedText.c_str(), boxX + 40, boxY + 32, 28, boxWidth - 80, RAYWHITE);

    // Continue Hint
    if (isTypingFinished && !hasOptions) {
        float alpha = 0.6f + 0.4f * sinf(pulseTimer);
        const char* continueMsg = "Press [Space] or [Enter] ▶";
        int msgWidth = ResourceManager::MeasureGameText(continueMsg, 24);
        ResourceManager::DrawGameText(continueMsg, boxX + boxWidth - msgWidth - 40, boxY + boxHeight - 38, 24, Fade(GOLD, alpha));
    }
}
