#pragma once
#include "dialogue/Dialogue.hpp"
#include "raylib.h"

class DialogueManager {
public:
    static DialogueManager& Get();

    void StartDialogue(const DialogueTree& tree);
    bool IsActive() const;
    void Update(float dt);
    void Draw();

private:
    DialogueManager() = default;
    ~DialogueManager() = default;
    DialogueManager(const DialogueManager&) = delete;
    DialogueManager& operator=(const DialogueManager&) = delete;

    void DrawWrappedText(const char* text, int posX, int posY, int fontSize, int maxLineWidth, Color color);

    bool isActive = false;
    DialogueTree currentTree;
    const DialogueNode* currentNode = nullptr;
    int selectedOption = 0;
    float pulseTimer = 0.0f;
};
