#pragma once
#include "scenes/Scene.hpp"
#include "raylib.h"
#include <string>
#include <vector>

struct EndingDialogueLine {
    std::string speaker;
    std::string text;
    Color speakerColor;
};

class EndingScene : public Scene {
public:
    EndingScene(bool hasAllTrueArtifacts, bool isLeaveChoice);
    ~EndingScene() override = default;

    void Update(float dt) override;
    void Draw() override;

private:
    bool hasAllTrue;
    bool isLeave;

    std::vector<EndingDialogueLine> lines;
    size_t currentLineIndex = 0;

    float displayedCharCount = 0.0f;
    float typingSpeed = 35.0f;
    bool isLineComplete = false;

    // Transition timers
    float whiteAlpha = 0.0f;
    float fadeToBlack = 0.0f;
    bool inCreditsPhase = false;
    float creditsAlpha = 0.0f;
};
