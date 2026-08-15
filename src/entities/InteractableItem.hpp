#pragma once
#include "raylib.h"
#include "dialogue/Dialogue.hpp"
#include <string>

class InteractableItem {
public:
    std::string artifactId;
    Rectangle rect;
    Color color;
    Color borderColor;
    std::string name;
    DialogueTree dialogue;

    InteractableItem(const std::string& artifactId, Rectangle rect, Color color, Color borderColor, const std::string& name, const DialogueTree& dialogue);

    bool CheckCollision(const Rectangle& playerRect) const;
    void Interact();
    void Draw() const;
};
