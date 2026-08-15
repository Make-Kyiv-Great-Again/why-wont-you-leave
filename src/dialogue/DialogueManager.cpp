#include "dialogue/DialogueManager.hpp"
#include "core/MemoryManager.hpp"
#include "core/ResourceManager.hpp"
#include <sstream>
#include <vector>
#include <cmath>

DialogueManager& DialogueManager::Get() {
    static DialogueManager instance;
    return instance;
}

void DialogueManager::StartDialogue(const DialogueTree& tree, bool isMemory, const std::string& artifactId) {
    currentTree = tree;
    currentNode = currentTree.GetNode(currentTree.startNodeId);
    selectedOption = 0;
    isActive = (currentNode != nullptr);
    isMemoryMode = isMemory;
    activeArtifactId = artifactId;
    pulseTimer = 0.0f;
}

void DialogueManager::StartDialogueFile(const std::string& jsonPath, bool isMemory, const std::string& artifactId) {
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

void DialogueManager::Update(float dt) {
    if (!isActive || !currentNode) return;

    pulseTimer += dt * 4.0f;

    bool hasOptions = !currentNode->options.empty();

    if (hasOptions) {
        int optCount = (int)currentNode->options.size();

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

        // Confirm choice
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            if (!activeArtifactId.empty()) {
                MemoryManager::Get().SaveChoice(activeArtifactId, currentNode->options[selectedOption].text);
            }

            int nextId = currentNode->options[selectedOption].targetNodeId;
            if (nextId == -1) {
                isActive = false;
                currentNode = nullptr;
            } else {
                currentNode = currentTree.GetNode(nextId);
                selectedOption = 0;
                if (!currentNode) isActive = false;
            }
        }
    } else {
        // Linear advance
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_E)) {
            if (currentNode->nextNodeId == -1) {
                isActive = false;
                currentNode = nullptr;
            } else {
                currentNode = currentTree.GetNode(currentNode->nextNodeId);
                selectedOption = 0;
                if (!currentNode) isActive = false;
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
        int textWidth = MeasureText(testLine.c_str(), fontSize);
        if (textWidth > maxLineWidth && !currentLine.empty()) {
            DrawText(currentLine.c_str(), posX, currentY, fontSize, color);
            currentLine = word;
            currentY += lineSpacing;
        } else {
            currentLine = testLine;
        }
    }
    if (!currentLine.empty()) {
        DrawText(currentLine.c_str(), posX, currentY, fontSize, color);
    }
}

void DialogueManager::Draw() {
    if (!isActive || !currentNode) return;

    Vector2 mousePos = GetMousePosition();
    bool hasOptions = !currentNode->options.empty();

    if (isMemoryMode) {
        // ==========================================
        // MEMORY MODE: Full Black Screen with Centered Dialogue
        // ==========================================
        DrawRectangle(0, 0, 2000, 800, Fade(BLACK, 0.96f));

        // Memory Header
        const char* memTitle = "✦ MEMORY RECALLED ✦";
        int mtw = MeasureText(memTitle, 36);
        DrawText(memTitle, (2000 - mtw) / 2, 70, 36, Fade(GOLD, 0.9f));

        // Centered Dialogue Box
        int boxWidth = 1400;
        int boxHeight = 220;
        int boxX = (2000 - boxWidth) / 2;
        int boxY = 140;

        DrawRectangle(boxX, boxY, boxWidth, boxHeight, Fade(DARKGRAY, 0.25f));
        DrawRectangleLinesEx({ (float)boxX, (float)boxY, (float)boxWidth, (float)boxHeight }, 3.0f, GOLD);

        // Speaker Name
        DrawRectangle(boxX + 30, boxY - 20, 240, 40, GOLD);
        DrawText(currentNode->speaker.c_str(), boxX + 45, boxY - 14, 26, BLACK);

        // Text
        DrawWrappedText(currentNode->text.c_str(), boxX + 40, boxY + 45, 30, boxWidth - 80, RAYWHITE);

        if (!hasOptions) {
            float alpha = 0.6f + 0.4f * sinf(pulseTimer);
            const char* continueMsg = "Press [Space] or [Enter] to Continue ▶";
            int msgWidth = MeasureText(continueMsg, 24);
            DrawText(continueMsg, boxX + boxWidth - msgWidth - 40, boxY + boxHeight - 40, 24, Fade(GOLD, alpha));
        } else {
            // Centered Reply Options
            int optCount = (int)currentNode->options.size();
            int panelY = 410;
            int cardHeight = 65;
            int spacing = 16;
            int cardWidth = 1300;
            int cardX = (2000 - cardWidth) / 2;

            for (int i = 0; i < optCount; i++) {
                int cardY = panelY + i * (cardHeight + spacing);
                Rectangle cardRect = { (float)cardX, (float)cardY, (float)cardWidth, (float)cardHeight };

                bool isHovered = CheckCollisionPointRec(mousePos, cardRect);
                if (isHovered) {
                    selectedOption = i;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        if (!activeArtifactId.empty()) {
                            MemoryManager::Get().SaveChoice(activeArtifactId, currentNode->options[i].text);
                        }

                        int nextId = currentNode->options[i].targetNodeId;
                        if (nextId == -1) {
                            isActive = false;
                            currentNode = nullptr;
                            return;
                        } else {
                            currentNode = currentTree.GetNode(nextId);
                            selectedOption = 0;
                            if (!currentNode) isActive = false;
                            return;
                        }
                    }
                }

                bool isSelected = (selectedOption == i);
                Color bgColor = isSelected ? Fade(DARKBLUE, 0.90f) : Fade(BLACK, 0.70f);
                Color borderColor = isSelected ? GOLD : Fade(GRAY, 0.6f);
                Color textColor = isSelected ? YELLOW : RAYWHITE;

                DrawRectangleRec(cardRect, bgColor);
                DrawRectangleLinesEx(cardRect, isSelected ? 3.0f : 1.5f, borderColor);

                std::string optText = "[" + std::to_string(i + 1) + "]  " + currentNode->options[i].text;
                DrawText(optText.c_str(), (int)cardRect.x + 30, (int)cardRect.y + 18, 26, textColor);
            }
        }
    } else {
        // ==========================================
        // STANDARD MODE: Top Voice Banner
        // ==========================================
        int boxX = 160;
        int boxY = 30;
        int boxWidth = 1680;
        int boxHeight = 220;

        DrawRectangle(boxX, boxY, boxWidth, boxHeight, Fade(BLACK, 0.90f));
        DrawRectangleLinesEx({ (float)boxX, (float)boxY, (float)boxWidth, (float)boxHeight }, 4.0f, GOLD);

        DrawRectangle(boxX + 25, boxY - 20, 260, 42, GOLD);
        DrawText(currentNode->speaker.c_str(), boxX + 45, boxY - 14, 28, BLACK);

        DrawWrappedText(currentNode->text.c_str(), boxX + 40, boxY + 45, 30, boxWidth - 80, RAYWHITE);

        if (!hasOptions) {
            float alpha = 0.6f + 0.4f * sinf(pulseTimer);
            const char* continueMsg = "Press [Space] or [Enter] to Continue ▶";
            int msgWidth = MeasureText(continueMsg, 24);
            DrawText(continueMsg, boxX + boxWidth - msgWidth - 40, boxY + boxHeight - 40, 24, Fade(GOLD, alpha));
        } else {
            int optCount = (int)currentNode->options.size();
            int panelY = 500;
            int cardHeight = 55;
            int spacing = 12;

            for (int i = 0; i < optCount; i++) {
                int cardY = panelY + i * (cardHeight + spacing);
                Rectangle cardRect = { 250.0f, (float)cardY, 1500.0f, (float)cardHeight };

                bool isHovered = CheckCollisionPointRec(mousePos, cardRect);
                if (isHovered) {
                    selectedOption = i;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        int nextId = currentNode->options[i].targetNodeId;
                        if (nextId == -1) {
                            isActive = false;
                            currentNode = nullptr;
                            return;
                        } else {
                            currentNode = currentTree.GetNode(nextId);
                            selectedOption = 0;
                            if (!currentNode) isActive = false;
                            return;
                        }
                    }
                }

                bool isSelected = (selectedOption == i);
                Color bgColor = isSelected ? Fade(DARKBLUE, 0.95f) : Fade(BLACK, 0.85f);
                Color borderColor = isSelected ? GOLD : GRAY;
                Color textColor = isSelected ? YELLOW : RAYWHITE;

                DrawRectangleRec(cardRect, bgColor);
                DrawRectangleLinesEx(cardRect, isSelected ? 3.0f : 2.0f, borderColor);

                std::string optText = "[" + std::to_string(i + 1) + "]  " + currentNode->options[i].text;
                DrawText(optText.c_str(), (int)cardRect.x + 30, (int)cardRect.y + 14, 26, textColor);
            }
        }
    }
}
