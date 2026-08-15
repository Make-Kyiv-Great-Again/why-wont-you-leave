#include "core/EffectManager.hpp"
#include <cstdlib>
#include "core/ResourceManager.hpp"

EffectManager& EffectManager::Get() {
    static EffectManager instance;
    return instance;
}

void EffectManager::SpawnForgettingEffect(Rectangle rect, Color color) {
    DissolveEffect eff;
    eff.sourceRect = rect;
    eff.color = color;
    eff.timer = 0.0f;
    eff.maxDuration = 1.3f;

    // Spawn ~30 floating dissolving particles
    for (int i = 0; i < 35; i++) {
        float px = rect.x + (float)(rand() % (int)rect.width);
        float py = rect.y + (float)(rand() % (int)rect.height);
        float vx = ((float)(rand() % 100) - 50.0f) * 1.5f;
        float vy = -((float)(rand() % 100) + 40.0f) * 1.2f;
        float sz = (float)(rand() % 8 + 4);

        eff.particles.push_back({ { px, py }, { vx, vy }, 1.0f, sz, color });
    }

    effects.push_back(eff);
}

void EffectManager::Update(float dt) {
    for (auto it = effects.begin(); it != effects.end(); ) {
        it->timer += dt;
        float progress = it->timer / it->maxDuration;

        for (auto& p : it->particles) {
            p.pos.x += p.vel.x * dt;
            p.pos.y += p.vel.y * dt;
            p.alpha = 1.0f - progress;
        }

        if (it->timer >= it->maxDuration) {
            it = effects.erase(it);
        } else {
            ++it;
        }
    }
}

void EffectManager::Draw() {
    for (const auto& eff : effects) {
        float progress = eff.timer / eff.maxDuration;
        float fade = 1.0f - progress;

        // Expanding dissolving aura ring
        float expansion = progress * 60.0f;
        Rectangle ringRect = {
            eff.sourceRect.x - expansion,
            eff.sourceRect.y - expansion,
            eff.sourceRect.width + expansion * 2.0f,
            eff.sourceRect.height + expansion * 2.0f
        };
        DrawRectangleLinesEx(ringRect, 3.0f * fade, Fade(eff.color, fade * 0.8f));

        // Dissolving ghost cube
        DrawRectangleRec(eff.sourceRect, Fade(eff.color, fade * 0.4f));

        // Particles
        for (const auto& p : eff.particles) {
            DrawCircleV(p.pos, p.size, Fade(p.color, p.alpha));
            DrawCircleV(p.pos, p.size * 0.5f, Fade(WHITE, p.alpha));
        }

        // Floating text: "FORGOTTEN..."
        const char* text = "FORGOTTEN...";
        int tw = ResourceManager::MeasureGameText(text, 22);
        int tx = (int)(eff.sourceRect.x + (eff.sourceRect.width - tw) / 2.0f);
        int ty = (int)(eff.sourceRect.y - 40.0f - progress * 40.0f);
        ResourceManager::DrawGameText(text, (float)tx, (float)ty, 22.0f, Fade(GRAY, fade));
    }
}
