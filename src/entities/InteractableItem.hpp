#pragma once
#include "raylib.h"
#include "dialogue/Dialogue.hpp"
#include "graphics/Sprite.hpp"
#include <string>

class InteractableItem {
public:
    std::string artifactId;
    Rectangle rect;
    Color color;
    Color borderColor;
    std::string name;
    DialogueTree dialogue;
    std::string dialogueFile;
    Sprite sprite;
    bool isVisible = true;

    InteractableItem(const std::string& artifactId, Rectangle rect, Color color, Color borderColor, const std::string& name, const DialogueTree& dialogue, bool isVisible = true);
    InteractableItem(const std::string& artifactId, Rectangle rect, Color color, Color borderColor, const std::string& name, const std::string& dialogueFile, bool isVisible = true);
    InteractableItem(const std::string& artifactId, Rectangle rect, Color color, Color borderColor, const std::string& name, const std::string& dialogueFile, const Sprite& sprite, bool isVisible = true);

    bool CheckCollision(const Rectangle& playerRect) const;
    void Interact();
    void Update(float dt);
    void Draw() const;
};
