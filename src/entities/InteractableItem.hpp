#pragma once
#include "raylib.h"
#include "dialogue/Dialogue.hpp"
#include <string>

class InteractableItem {
public:
    Rectangle rect;
    Color color;
    Color borderColor;
    std::string name;
    DialogueTree dialogue;

    InteractableItem(Rectangle rect, Color color, Color borderColor, const std::string& name, const DialogueTree& dialogue);

    bool CheckCollision(const Rectangle& playerRect) const;
    void Interact();
    void Draw() const;
};
