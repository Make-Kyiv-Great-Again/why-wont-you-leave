#pragma once
#include "dialogue/Dialogue.hpp"
#include "raylib.h"
#include <string>

class DialogueManager {
public:
    static DialogueManager& Get();

    void StartDialogue(const DialogueTree& tree, bool isMemory = false, const std::string& artifactId = "");
    void StartDialogueFile(const std::string& jsonPath, bool isMemory = false, const std::string& artifactId = "");
    bool IsActive() const;
    bool IsMemoryMode() const;
    void Update(float dt);
    void Draw();

private:
    DialogueManager() = default;
    ~DialogueManager() = default;
    DialogueManager(const DialogueManager&) = delete;
    DialogueManager& operator=(const DialogueManager&) = delete;

    void DrawWrappedText(const char* text, int posX, int posY, int fontSize, int maxLineWidth, Color color);
    void OnDialogueFinished();

    bool isActive = false;
    bool isMemoryMode = false;
    float blackoutAlpha = 0.0f;
    std::string activeArtifactId = "";
    std::string activeScriptPath = "";
    DialogueTree currentTree;
    const DialogueNode* currentNode = nullptr;
    int selectedOption = 0;
    float pulseTimer = 0.0f;

    // Typing animation variables
    size_t visibleChars = 0;
    float typeTimer = 0.0f;
    float typeSpeed = 0.02f;
    std::string displayedText = "";
};
