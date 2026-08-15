#include "core/MemoryManager.hpp"
#include <cmath>

MemoryManager& MemoryManager::Get() {
    static MemoryManager instance;
    return instance;
}

MemoryManager::MemoryManager() {
    artifacts.push_back({ "sapphire_prism", "Sapphire Prism", SKYBLUE, DARKBLUE, false, "", 1 });
    artifacts.push_back({ "emerald_relic", "Emerald Relic", LIME, DARKGREEN, false, "", 1 });
    artifacts.push_back({ "amethyst_core", "Amethyst Core", PURPLE, DARKPURPLE, false, "", 2 });
}

bool MemoryManager::IsRemembered(const std::string& id) const {
    for (const auto& art : artifacts) {
        if (art.id == id) {
            return art.isRemembered;
        }
    }
    return false;
}

void MemoryManager::SetRemembered(const std::string& id, bool remembered) {
    for (auto& art : artifacts) {
        if (art.id == id) {
            art.isRemembered = remembered;
            if (!remembered) {
                art.savedChoice = ""; // Clear choice when forgotten
            }
            return;
        }
    }
}

void MemoryManager::RegisterArtifact(const std::string& id, const std::string& name, Color color, Color borderColor, int roomId) {
    for (const auto& art : artifacts) {
        if (art.id == id) return; // Already registered
    }
    artifacts.push_back({ id, name, color, borderColor, false, "", roomId });
}

void MemoryManager::SaveChoice(const std::string& id, const std::string& choice) {
    for (auto& art : artifacts) {
        if (art.id == id) {
            art.savedChoice = choice;
            return;
        }
    }
}

std::string MemoryManager::GetSavedChoice(const std::string& id) const {
    for (const auto& art : artifacts) {
        if (art.id == id) {
            return art.savedChoice;
        }
    }
    return "";
}

const std::vector<MemoryArtifact>& MemoryManager::GetAllArtifacts() const {
    return artifacts;
}

void MemoryManager::DrawMemoryInventoryOverlay(float transition) {
    if (transition <= 0.0f) return;

    const int totalSlots = 5;
    float screenWidth = 2000.0f;
    float screenHeight = 800.0f;

    // Distribute exactly 5 slots across the screen width
    float spacing = screenWidth / (totalSlots + 1);
    float time = (float)GetTime();

    for (int i = 0; i < totalSlots; i++) {
        float slotX = spacing * (i + 1);
        float centerY = screenHeight / 2.0f;

        // Ethereal sinusoidal floating bobbing
        float floatOffset = sinf(time * 2.2f + i * 1.6f) * 22.0f;

        float cubeSize = 130.0f;
        float cubeX = slotX - cubeSize / 2.0f;
        float cubeY = centerY - cubeSize / 2.0f + floatOffset;
        Rectangle cubeRect = { cubeX, cubeY, cubeSize, cubeSize };

        bool isSlotRemembered = (i < (int)artifacts.size() && artifacts[i].isRemembered);

        if (isSlotRemembered) {
            const auto& art = artifacts[i];
            // Draw soft radial light circles behind the remembered cube
            // Outer soft aura
            DrawCircle((int)slotX, (int)(centerY + floatOffset), 150.0f, Fade(art.color, 0.12f * transition));
            // Mid glow
            DrawCircle((int)slotX, (int)(centerY + floatOffset), 110.0f, Fade(art.color, 0.22f * transition));
            // Inner light
            DrawCircle((int)slotX, (int)(centerY + floatOffset), 70.0f, Fade(WHITE, 0.35f * transition));

            // Ethereal particles around remembered cube
            float pulseAngle = time * 2.0f + i * 3.0f;
            float px1 = slotX + cosf(pulseAngle) * 90.0f;
            float py1 = centerY + floatOffset + sinf(pulseAngle) * 90.0f;
            float px2 = slotX - cosf(pulseAngle) * 90.0f;
            float py2 = centerY + floatOffset - sinf(pulseAngle) * 90.0f;
            DrawCircle((int)px1, (int)py1, 4.0f, Fade(GOLD, 0.8f * transition));
            DrawCircle((int)px2, (int)py2, 4.0f, Fade(GOLD, 0.8f * transition));

            // Solid vibrant cube
            DrawRectangleRec(cubeRect, Fade(art.color, transition));
            DrawRectangleLinesEx(cubeRect, 4.0f, Fade(GOLD, transition));
        } else {
            // Spirit-like texture (semi-transparent gray with outline)
            DrawRectangleRec(cubeRect, Fade(GRAY, 0.20f * transition));
            DrawRectangleLinesEx(cubeRect, 2.5f, Fade(LIGHTGRAY, 0.45f * transition));

            // Faint outer spirit ring
            float ringPulse = (sinf(time * 2.8f + i) + 1.0f) * 0.5f;
            float ringSize = cubeSize + 20.0f * ringPulse;
            Rectangle ringRect = {
                slotX - ringSize / 2.0f,
                centerY - ringSize / 2.0f + floatOffset,
                ringSize,
                ringSize
            };
            DrawRectangleLinesEx(ringRect, 1.5f, Fade(LIGHTGRAY, 0.25f * (1.0f - ringPulse) * transition));
        }
    }
}
