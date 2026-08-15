#include "dialogue/DialogueManager.hpp"
#include "core/ResourceManager.hpp"
#include "core/SceneManager.hpp"
#include "core/ActManager.hpp"
#include "core/MemoryManager.hpp"
#include "scenes/ActTitleScene.hpp"
#include "scenes/MainMenuScene.hpp"
#include "scenes/EndingScene.hpp"
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
    if (activeScriptPath == "assets/data/dialogues/final_door_choice.json") {
        std::string choice = MemoryManager::Get().GetSavedChoice("final_door_choice");
        bool isLeave = (choice == "Leave");
        bool hasAllTrue = ActManager::Get().HasAllTrueArtifacts();
        SceneManager::Get().ChangeScene(std::make_unique<EndingScene>(hasAllTrue, isLeave));
        return;
    }

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
    size_t fullLength = currentNode->text.length();
    bool isTypingFinished = (visibleChars >= fullLength);

    if (!isTypingFinished) {
        typeTimer += dt;
        while (typeTimer >= typeSpeed && visibleChars < fullLength) {
            typeTimer -= typeSpeed;
            visibleChars++;
        }
        displayedText = currentNode->text.substr(0, visibleChars);
    } else {
        displayedText = currentNode->text;
    }

    // Fast-forward typing on keypress
    if (!isTypingFinished && (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_E) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))) {
        visibleChars = fullLength;
        displayedText = currentNode->text;
        
        Sound sfx = ResourceManager::Get().GetSound("assets/sounds/ui_typing_sound.mp3");
        if (sfx.frameCount > 0) {
            StopSound(sfx);
        }
        return;
    }

    if (!isTypingFinished) return;

    // Input Handling for choices and line advance
    bool hasOptions = !currentNode->options.empty();
    if (hasOptions) {
        int optCount = (int)currentNode->options.size();

        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            selectedOption = (selectedOption - 1 + optCount) % optCount;
            Sound sfx = ResourceManager::Get().GetSound("assets/sounds/select_sound.mp3");
            if (sfx.frameCount > 0) {
                StopSound(sfx);
                PlaySound(sfx);
            }
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            selectedOption = (selectedOption + 1) % optCount;
            Sound sfx = ResourceManager::Get().GetSound("assets/sounds/select_sound.mp3");
            if (sfx.frameCount > 0) {
                StopSound(sfx);
                PlaySound(sfx);
            }
        }

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            Sound selectSfx = ResourceManager::Get().GetSound("assets/sounds/select_sound.mp3");
            if (selectSfx.frameCount > 0) {
                StopSound(selectSfx);
                PlaySound(selectSfx);
            }

            std::string chosenText = currentNode->options[selectedOption].text;
            if (!activeArtifactId.empty()) {
                MemoryManager::Get().SaveChoice(activeArtifactId, chosenText);
            }
            if (activeScriptPath == "assets/data/dialogues/final_door_choice.json") {
                MemoryManager::Get().SaveChoice("final_door_choice", chosenText);
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
                return;
            } else {
                currentNode = currentTree.GetNode(nextNodeId);
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
    }
}

void DialogueManager::DrawWrappedText(const char* text, int posX, int posY, int fontSize, int maxLineWidth, Color color) {
    std::string textStr = text;
    std::istringstream words(textStr);
    std::string word;
    std::string currentLine = "";
    int currentY = posY;
    int lineSpacing = (int)(fontSize * 1.35f);

    while (words >> word) {
        std::string testLine = currentLine.empty() ? word : (currentLine + " " + word);
        int lineWidth = ResourceManager::MeasureGameText(testLine.c_str(), fontSize);

        if (lineWidth > maxLineWidth && !currentLine.empty()) {
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
    if (blackoutAlpha > 0.001f) {
        DrawRectangle(0, 0, 2000, 800, Fade(BLACK, blackoutAlpha));
    }

    if (!isActive || !currentNode) return;

    float screenWidth = 2000.0f;
    float screenHeight = 800.0f;
    Vector2 mousePos = SceneManager::Get().GetVirtualMousePosition();

    // 1. Floating Memory Item Sprite in Blackout Memory Mode
    const MemoryArtifact* memArt = nullptr;
    if (!activeArtifactId.empty()) {
        const auto& allArts = MemoryManager::Get().GetAllArtifacts();
        for (const auto& a : allArts) {
            if (a.id == activeArtifactId) {
                memArt = &a;
                break;
            }
        }
    }

    bool hasItemToDraw = (blackoutAlpha > 0.35f && memArt != nullptr);
    if (hasItemToDraw) {
        float centerX = screenWidth / 2.0f;
        float centerY = 190.0f;
        float floatOffset = sinf(memoryItemAnimTimer * 2.2f) * 12.0f;
        float drawSize = 140.0f;
        float spriteAlpha = fminf(blackoutAlpha * 1.1f, 1.0f);

        // Radiant glow
        DrawCircle((int)centerX, (int)(centerY + floatOffset), drawSize * 0.95f, Fade(memArt->color, 0.18f * spriteAlpha));
        DrawCircle((int)centerX, (int)(centerY + floatOffset), drawSize * 0.55f, Fade(WHITE, 0.28f * spriteAlpha));

        Texture2D artTex = ResourceManager::Get().GetTexture(memArt->texturePath);
        if (artTex.id != 0) {
            float scale = fminf(drawSize / (float)artTex.width, drawSize / (float)artTex.height);
            float dw = (float)artTex.width * scale;
            float dh = (float)artTex.height * scale;
            Rectangle destRect = {
                centerX - dw / 2.0f,
                centerY + floatOffset - dh / 2.0f,
                dw,
                dh
            };
            DrawTexturePro(artTex, Rectangle{ 0, 0, (float)artTex.width, (float)artTex.height }, destRect, Vector2{ 0, 0 }, 0.0f, Fade(WHITE, spriteAlpha));
        }

        int nameWidth = ResourceManager::MeasureGameText(memArt->name.c_str(), 24);
        ResourceManager::DrawGameText(memArt->name.c_str(), (int)(centerX - nameWidth / 2.0f), (int)(centerY + drawSize / 2.0f + 8.0f + floatOffset), 24, Fade(memArt->color, spriteAlpha * 0.90f));
    }

    // 2. Response Options (Pale Vertical Buttons)
    size_t fullLength = currentNode->text.length();
    bool isTypingFinished = (visibleChars >= fullLength);
    bool hasOptions = !currentNode->options.empty();

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
                    if (activeScriptPath == "assets/data/dialogues/final_door_choice.json") {
                        MemoryManager::Get().SaveChoice("final_door_choice", chosenText);
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
                    return;
                }
            }

            bool isSelected = (selectedOption == i);
            Color fillCol = isSelected ? Fade(WHITE, 0.95f) : Fade(Color{ 18, 16, 22, 255 }, 0.88f);
            Color borderCol = isSelected ? GOLD : Fade(Color{ 160, 150, 165, 255 }, 0.65f);
            Color textCol = isSelected ? Color{ 20, 18, 24, 255 } : Fade(Color{ 230, 225, 235, 255 }, 0.92f);

            DrawRectangleRec(cardRect, fillCol);
            DrawRectangleLinesEx(cardRect, isSelected ? 3.0f : 1.5f, borderCol);

            int optFontSize = hasItemToDraw ? 22 : 24;
            int textW = ResourceManager::MeasureGameText(currentNode->options[i].text.c_str(), optFontSize);
            ResourceManager::DrawGameText(currentNode->options[i].text.c_str(), cardX + (cardWidth - textW) / 2, cardY + (cardHeight - optFontSize) / 2, optFontSize, textCol);
        }
    }

    // 3. Dialogue Box UI (TCOAAL Style)
    int boxX = 220;
    int boxY = hasItemToDraw ? 560 : 540;
    int boxW = 1560;
    int boxH = hasItemToDraw ? 210 : 230;

    DrawRectangle(boxX, boxY, boxW, boxH, Fade(Color{ 10, 8, 14, 255 }, 0.92f));
    DrawRectangleLinesEx(Rectangle{ (float)boxX, (float)boxY, (float)boxW, (float)boxH }, 2.5f, Fade(Color{ 200, 195, 210, 255 }, 0.85f));

    // Speaker Name Plate
    if (!currentNode->speaker.empty()) {
        int namePlateW = 320;
        int namePlateH = 46;
        int namePlateX = boxX + 30;
        int namePlateY = boxY - 34;

        DrawRectangle(namePlateX, namePlateY, namePlateW, namePlateH, Fade(Color{ 15, 12, 20, 255 }, 0.96f));
        DrawRectangleLinesEx(Rectangle{ (float)namePlateX, (float)namePlateY, (float)namePlateW, (float)namePlateH }, 2.0f, Fade(GOLD, 0.90f));

        Color spkColor = (currentNode->speaker == "Evan") ? GOLD : Color{ 180, 210, 255, 255 };
        ResourceManager::DrawGameText(currentNode->speaker.c_str(), namePlateX + 24, namePlateY + 10, 26, spkColor);
    }

    // Typewritten / Animated Dialogue Text
    int textPaddingX = 50;
    int textPaddingY = 40;
    DrawWrappedText(displayedText.c_str(), boxX + textPaddingX, boxY + textPaddingY, 28, boxW - (textPaddingX * 2), Fade(Color{ 245, 242, 248, 255 }, 0.96f));

    // Blinking continuation indicator
    if (isTypingFinished && !hasOptions) {
        float blinkAlpha = (sinf(pulseTimer) + 1.0f) * 0.5f;
        const char* prompt = "[ Space / E / Click ▶ ]";
        int promptW = ResourceManager::MeasureGameText(prompt, 20);
        ResourceManager::DrawGameText(prompt, boxX + boxW - promptW - 35, boxY + boxH - 35, 20, Fade(GOLD, blinkAlpha));
    }
}
