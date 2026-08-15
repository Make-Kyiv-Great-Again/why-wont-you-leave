#include "core/MemoryManager.hpp"
#include "core/ResourceManager.hpp"
#include "core/SceneManager.hpp"
#include "core/ActManager.hpp"
#include <cmath>

MemoryManager& MemoryManager::Get() {
    static MemoryManager instance;
    return instance;
}

MemoryManager::MemoryManager() {
    // 5 Required / True Story Artifacts
    artifacts.push_back({ "travel_bag", "Grace's Suitcase", "assets/sprites/items/bedroom/travel_bag.png", GOLD, DARKBROWN, false, "", 1 });
    artifacts.push_back({ "job_letter", "Job Offer Letter", "assets/sprites/items/kitchen/job_letter.png", SKYBLUE, DARKBLUE, false, "", 3 });
    artifacts.push_back({ "car_keys", "Car Keys", "assets/sprites/items/hall/19_key.png", LIGHTGRAY, DARKGRAY, false, "", 0 });
    artifacts.push_back({ "diary", "Grace's Diary", "assets/sprites/items/bedroom/20_diary.png", PINK, MAROON, false, "", 1 });
    artifacts.push_back({ "accident_info", "Accident Report", "assets/sprites/items/bedroom/accident_info.png", RED, DARKGRAY, false, "", 1 });

    // 5 Optional / Redundant Artifacts (Holding onto past / Guilt)
    artifacts.push_back({ "photo_of_couple", "Photograph", "assets/sprites/items/kitchen/photo_of_couple.png", BEIGE, BROWN, false, "", 3 });
    artifacts.push_back({ "broken_plate", "Broken Plate", "assets/sprites/items/kitchen/broken_plate.png", LIGHTGRAY, DARKGRAY, false, "", 3 });
    artifacts.push_back({ "guitar", "Guitar", "assets/sprites/items/hall/guitar.png", ORANGE, DARKBROWN, false, "", 0 });
    artifacts.push_back({ "toothbrushes", "Two Toothbrushes", "assets/sprites/items/bathroom/toothbrushes.png", SKYBLUE, BLUE, false, "", 2 });
    artifacts.push_back({ "medicine", "Sleeping Pills", "assets/sprites/items/bathroom/medicine.png", VIOLET, PURPLE, false, "", 2 });
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
                art.savedChoice = "";
            }
            return;
        }
    }
}

void MemoryManager::RegisterArtifact(const std::string& id, const std::string& name, const std::string& texturePath, Color color, Color borderColor, int roomId) {
    for (auto& art : artifacts) {
        if (art.id == id) {
            art.name = name;
            if (!texturePath.empty()) art.texturePath = texturePath;
            art.color = color;
            art.borderColor = borderColor;
            art.roomId = roomId;
            return;
        }
    }
    artifacts.push_back({ id, name, texturePath, color, borderColor, false, "", roomId });
}

void MemoryManager::SaveChoice(const std::string& id, const std::string& choice) {
    genericChoices[id] = choice;
    for (auto& art : artifacts) {
        if (art.id == id) {
            art.savedChoice = choice;
            return;
        }
    }
}

std::string MemoryManager::GetSavedChoice(const std::string& id) const {
    auto it = genericChoices.find(id);
    if (it != genericChoices.end()) {
        return it->second;
    }
    for (const auto& art : artifacts) {
        if (art.id == id) {
            return art.savedChoice;
        }
    }
    return "";
}

int MemoryManager::GetRememberedCount() const {
    int count = 0;
    for (const auto& art : artifacts) {
        if (art.isRemembered) count++;
    }
    return count;
}

std::vector<const MemoryArtifact*> MemoryManager::GetRememberedArtifacts() const {
    std::vector<const MemoryArtifact*> list;
    for (const auto& art : artifacts) {
        if (art.isRemembered) {
            list.push_back(&art);
        }
    }
    return list;
}

const MemoryArtifact* MemoryManager::GetArtifact(const std::string& id) const {
    for (const auto& art : artifacts) {
        if (art.id == id) {
            return &art;
        }
    }
    return nullptr;
}

const std::vector<MemoryArtifact>& MemoryManager::GetAllArtifacts() const {
    return artifacts;
}

void MemoryManager::Reset() {
    genericChoices.clear();
    for (auto& art : artifacts) {
        art.isRemembered = false;
        art.savedChoice = "";
    }
}

void MemoryManager::DrawMemoryInventoryOverlay(float transition) {
    if (transition <= 0.0f) return;

    Vector2 mousePos = SceneManager::Get().GetVirtualMousePosition();
    const int totalSlots = 5;
    float screenWidth = 2000.0f;
    float screenHeight = 800.0f;

    // Distribute exactly 5 slots across the screen width
    float spacing = screenWidth / (totalSlots + 1);
    float time = (float)GetTime();

    auto rememberedList = GetRememberedArtifacts();

    // Load Corrupted/Redundant shader
    Shader corruptedShader = ResourceManager::Get().GetShader("assets/shaders/glsl330/corrupted_memory.fs");
    if (corruptedShader.id != 0) {
        int timeLoc = GetShaderLocation(corruptedShader, "time");
        if (timeLoc != -1) {
            SetShaderValue(corruptedShader, timeLoc, &time, SHADER_UNIFORM_FLOAT);
        }
    }

    // Overlay title
    const char* tabTitle = "✦ MEMORY ARCHIVE (TAB) ✦";
    int ttw = ResourceManager::MeasureGameText(tabTitle, 32);
    ResourceManager::DrawGameText(tabTitle, (int)(screenWidth - ttw) / 2, 70, 32, Fade(GOLD, 0.9f * transition));

    char countBuf[64];
    snprintf(countBuf, sizeof(countBuf), "Stored Memories: %d / 5", (int)rememberedList.size());
    int cw = ResourceManager::MeasureGameText(countBuf, 22);
    ResourceManager::DrawGameText(countBuf, (int)(screenWidth - cw) / 2, 115, 22, Fade(LIGHTGRAY, 0.8f * transition));

    for (int i = 0; i < totalSlots; i++) {
        float slotX = spacing * (i + 1);
        float centerY = screenHeight / 2.0f + 20.0f;

        // Ethereal sinusoidal floating bobbing
        float floatOffset = sinf(time * 2.2f + i * 1.6f) * 16.0f;

        float cubeSize = 140.0f;
        float cubeX = slotX - cubeSize / 2.0f;
        float cubeY = centerY - cubeSize / 2.0f + floatOffset;
        Rectangle cubeRect = { cubeX, cubeY, cubeSize, cubeSize };

        bool isOccupied = (i < (int)rememberedList.size());

        if (isOccupied) {
            const auto* art = rememberedList[i];
            bool isTrue = ActManager::IsArtifactTrue(art->id);

            if (isTrue) {
                // 1. Required / True Story Artifact: Radiant Golden Light
                DrawCircle((int)slotX, (int)(centerY + floatOffset), 130.0f, Fade(art->color, 0.14f * transition));
                DrawCircle((int)slotX, (int)(centerY + floatOffset), 90.0f, Fade(art->color, 0.24f * transition));
                DrawCircle((int)slotX, (int)(centerY + floatOffset), 50.0f, Fade(WHITE, 0.35f * transition));

                // Ethereal orbiting gold particles
                float pulseAngle = time * 2.0f + i * 3.0f;
                float px1 = slotX + cosf(pulseAngle) * 80.0f;
                float py1 = centerY + floatOffset + sinf(pulseAngle) * 80.0f;
                float px2 = slotX - cosf(pulseAngle) * 80.0f;
                float py2 = centerY + floatOffset - sinf(pulseAngle) * 80.0f;
                DrawCircle((int)px1, (int)py1, 3.5f, Fade(GOLD, 0.8f * transition));
                DrawCircle((int)px2, (int)py2, 3.5f, Fade(GOLD, 0.8f * transition));

                // Slot Card Background
                DrawRectangleRec(cubeRect, Fade(BLACK, 0.85f * transition));
                DrawRectangleLinesEx(cubeRect, 3.0f, Fade(GOLD, transition));

                // Draw Texture in True/Clean state
                Texture2D tex = ResourceManager::Get().GetTexture(art->texturePath);
                if (tex.id != 0) {
                    float spriteScale = fminf((cubeSize - 30.0f) / (float)tex.width, (cubeSize - 30.0f) / (float)tex.height);
                    float dw = (float)tex.width * spriteScale;
                    float dh = (float)tex.height * spriteScale;
                    Rectangle destRect = { slotX - dw / 2.0f, (centerY + floatOffset) - dh / 2.0f, dw, dh };
                    DrawTexturePro(tex, Rectangle{ 0, 0, (float)tex.width, (float)tex.height }, destRect, Vector2{ 0, 0 }, 0.0f, Fade(WHITE, transition));
                }

                // Name in GOLD
                int nameW = ResourceManager::MeasureGameText(art->name.c_str(), 20);
                ResourceManager::DrawGameText(art->name.c_str(), (int)(slotX - nameW / 2.0f), (int)(cubeY + cubeSize + 14), 20, Fade(GOLD, transition));
            } else {
                // 2. Optional / Redundant Artifact: Dark Charred Shadow & Corrupted Shader
                DrawCircle((int)slotX, (int)(centerY + floatOffset), 120.0f, Fade(BLACK, 0.60f * transition));
                DrawCircle((int)slotX, (int)(centerY + floatOffset), 80.0f, Fade(Color{ 50, 20, 35, 255 }, 0.45f * transition));

                // Dark ash pixel particles
                float pulseAngle = time * 1.5f + i * 2.5f;
                float px1 = slotX + cosf(pulseAngle) * 75.0f;
                float py1 = centerY + floatOffset + sinf(pulseAngle) * 75.0f;
                DrawRectangle((int)px1 - 2, (int)py1 - 2, 4, 4, Fade(DARKGRAY, 0.65f * transition));

                // Dark Corrupted Slot Card
                DrawRectangleRec(cubeRect, Fade(Color{ 14, 10, 16, 255 }, 0.94f * transition));
                DrawRectangleLinesEx(cubeRect, 2.5f, Fade(Color{ 85, 60, 75, 255 }, 0.85f * transition));

                // Draw Texture through Corrupted/Dark Shadow Shader
                Texture2D tex = ResourceManager::Get().GetTexture(art->texturePath);
                if (tex.id != 0) {
                    float spriteScale = fminf((cubeSize - 30.0f) / (float)tex.width, (cubeSize - 30.0f) / (float)tex.height);
                    float dw = (float)tex.width * spriteScale;
                    float dh = (float)tex.height * spriteScale;
                    Rectangle destRect = { slotX - dw / 2.0f, (centerY + floatOffset) - dh / 2.0f, dw, dh };

                    if (corruptedShader.id != 0) {
                        BeginShaderMode(corruptedShader);
                    }
                    DrawTexturePro(tex, Rectangle{ 0, 0, (float)tex.width, (float)tex.height }, destRect, Vector2{ 0, 0 }, 0.0f, Fade(WHITE, transition));
                    if (corruptedShader.id != 0) {
                        EndShaderMode();
                    }
                }

                // Name in Muted Ash
                int nameW = ResourceManager::MeasureGameText(art->name.c_str(), 18);
                ResourceManager::DrawGameText(art->name.c_str(), (int)(slotX - nameW / 2.0f), (int)(cubeY + cubeSize + 14), 18, Fade(Color{ 160, 140, 150, 255 }, transition));
            }

            // Check Mouse Hover & Remove Button
            bool isHovered = CheckCollisionPointRec(mousePos, cubeRect);
            if (isHovered) {
                DrawRectangleLinesEx(cubeRect, 3.5f, Fade(RED, transition));

                // Small [X] remove badge
                Rectangle closeBtn = { cubeX + cubeSize - 26.0f, cubeY + 4.0f, 22.0f, 22.0f };
                DrawRectangleRec(closeBtn, Fade(MAROON, 0.9f * transition));
                DrawRectangleLinesEx(closeBtn, 1.5f, Fade(RED, transition));
                ResourceManager::DrawGameText("✕", (int)closeBtn.x + 5, (int)closeBtn.y + 2, 18, WHITE);

                const char* removeHint = "Click to Release";
                int rhw = ResourceManager::MeasureGameText(removeHint, 18);
                ResourceManager::DrawGameText(removeHint, (int)(slotX - rhw / 2.0f), (int)(cubeY - 24), 18, Fade(RED, 0.95f * transition));

                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    ActManager::Get().UnmarkArtifactRemembered(art->id);
                    Sound sfx = ResourceManager::Get().GetSound("assets/sounds/select_sound.mp3");
                    if (sfx.frameCount > 0) PlaySound(sfx);
                }
            }
        } else {
            // Empty ethereal spirit slot
            DrawRectangleRec(cubeRect, Fade(BLACK, 0.50f * transition));
            DrawRectangleLinesEx(cubeRect, 2.0f, Fade(LIGHTGRAY, 0.35f * transition));

            // Faint outer spirit ring
            float ringPulse = (sinf(time * 2.8f + i) + 1.0f) * 0.5f;
            float ringSize = cubeSize + 16.0f * ringPulse;
            Rectangle ringRect = {
                slotX - ringSize / 2.0f,
                centerY - ringSize / 2.0f + floatOffset,
                ringSize,
                ringSize
            };
            DrawRectangleLinesEx(ringRect, 1.5f, Fade(LIGHTGRAY, 0.20f * (1.0f - ringPulse) * transition));

            // Empty slot text
            const char* emptyLabel = "[ Empty ]";
            int ew = ResourceManager::MeasureGameText(emptyLabel, 18);
            ResourceManager::DrawGameText(emptyLabel, (int)(slotX - ew / 2.0f), (int)(centerY + floatOffset - 9), 18, Fade(GRAY, 0.45f * transition));
        }
    }
}
