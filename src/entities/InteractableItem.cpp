#include "entities/InteractableItem.hpp"
#include "dialogue/DialogueManager.hpp"
#include "core/MemoryManager.hpp"
#include "core/ResourceManager.hpp"
#include <cmath>

InteractableItem::InteractableItem(const std::string& artifactId, Rectangle rect, Color color, Color borderColor, const std::string& name, const DialogueTree& dialogue, bool isVisible, bool isMemoryArtifact)
    : artifactId(artifactId), rect(rect), color(color), borderColor(borderColor), name(name), dialogue(dialogue), isVisible(isVisible), isMemoryArtifact(isMemoryArtifact) {}

InteractableItem::InteractableItem(const std::string& artifactId, Rectangle rect, Color color, Color borderColor, const std::string& name, const std::string& dialogueFile, bool isVisible, bool isMemoryArtifact)
    : artifactId(artifactId), rect(rect), color(color), borderColor(borderColor), name(name), dialogueFile(dialogueFile), isVisible(isVisible), isMemoryArtifact(isMemoryArtifact) {}

InteractableItem::InteractableItem(const std::string& artifactId, Rectangle rect, Color color, Color borderColor, const std::string& name, const std::string& dialogueFile, const Sprite& sprite, bool isVisible, bool isMemoryArtifact)
    : artifactId(artifactId), rect(rect), color(color), borderColor(borderColor), name(name), dialogueFile(dialogueFile), sprite(sprite), isVisible(isVisible), isMemoryArtifact(isMemoryArtifact) {}

bool InteractableItem::CheckCollision(const Rectangle& playerRect) const {
    return CheckCollisionRecs(playerRect, rect);
}

void InteractableItem::Interact() {
    if (!dialogueFile.empty()) {
        DialogueManager::Get().StartDialogueFile(dialogueFile, false, artifactId);
    } else {
        DialogueManager::Get().StartDialogue(dialogue, false, artifactId);
    }
}

void InteractableItem::Update(float dt) {
    if (sprite.IsValid()) {
        sprite.Update(dt);
    }
}

void InteractableItem::Draw() const {
    if (!isVisible) return; // Clean background art for ambient inspection objects

    bool remembered = MemoryManager::Get().IsRemembered(artifactId);
    float time = (float)GetTime();

    if (remembered) {
        // Glowing halo
        float haloPulse = (sinf(time * 3.0f) + 1.0f) * 0.5f;
        Rectangle haloRect = { rect.x - 8.0f - haloPulse * 4.0f, rect.y - 8.0f - haloPulse * 4.0f,
                               rect.width + 16.0f + haloPulse * 8.0f, rect.height + 16.0f + haloPulse * 8.0f };
        DrawRectangleRec(haloRect, Fade(color, 0.25f + haloPulse * 0.15f));
        DrawRectangleLinesEx(haloRect, 2.0f, Fade(GOLD, 0.6f));

        // Draw Sprite or Solid Color
        if (sprite.IsValid()) {
            sprite.Draw(rect);
            DrawRectangleLinesEx(rect, 3.0f, GOLD);
        } else {
            DrawRectangleRec(rect, color);
            DrawRectangleLinesEx(rect, 3.0f, GOLD);
        }

        // Label above
        std::string labelText = name + " ✦";
        int labelWidth = ResourceManager::MeasureGameText(labelText.c_str(), 22);
        ResourceManager::DrawGameText(labelText.c_str(), (int)(rect.x + (rect.width - labelWidth) / 2.0f), (int)(rect.y - 34), 22, GOLD);
    } else {
        // Normal state
        if (sprite.IsValid()) {
            sprite.Draw(rect);
            DrawRectangleLinesEx(rect, 3.0f, borderColor);
        } else {
            DrawRectangleRec(rect, color);
            DrawRectangleLinesEx(rect, 3.0f, borderColor);
        }

        // Label above
        int labelWidth = ResourceManager::MeasureGameText(name.c_str(), 22);
        ResourceManager::DrawGameText(name.c_str(), (int)(rect.x + (rect.width - labelWidth) / 2.0f), (int)(rect.y - 32), 22, DARKGRAY);
    }
}
