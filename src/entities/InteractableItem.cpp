#include "entities/InteractableItem.hpp"
#include "dialogue/DialogueManager.hpp"

InteractableItem::InteractableItem(Rectangle rect, Color color, Color borderColor, const std::string& name, const DialogueTree& dialogue)
    : rect(rect), color(color), borderColor(borderColor), name(name), dialogue(dialogue) {}

bool InteractableItem::CheckCollision(const Rectangle& playerRect) const {
    return CheckCollisionRecs(playerRect, rect);
}

void InteractableItem::Interact() {
    DialogueManager::Get().StartDialogue(dialogue);
}

void InteractableItem::Draw() const {
    // Draw the cube
    DrawRectangleRec(rect, color);
    DrawRectangleLinesEx(rect, 3, borderColor);

    // Draw item name label floating above
    int labelWidth = MeasureText(name.c_str(), 22);
    DrawText(name.c_str(), (int)(rect.x + (rect.width - labelWidth) / 2.0f), (int)(rect.y - 32), 22, DARKGRAY);
}
