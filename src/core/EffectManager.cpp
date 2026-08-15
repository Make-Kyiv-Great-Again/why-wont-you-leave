#include "core/EffectManager.hpp"
#include <cstdlib>
#include <cmath>
#include "core/ResourceManager.hpp"

EffectManager& EffectManager::Get() {
    static EffectManager instance;
    return instance;
}

EffectManager::EffectManager() {
    InitDustParticles(85);
}

void EffectManager::InitDustParticles(int count, Color color) {
    dustParticles.clear();
    for (int i = 0; i < count; i++) {
        DustMote mote;
        mote.pos = { (float)(rand() % 2000), (float)(rand() % 800) };
        mote.baseVel = { ((float)(rand() % 40) - 18.0f) * 0.7f, ((float)(rand() % 30) - 15.0f) * 0.5f };
        mote.phase = ((float)(rand() % 360)) * (PI / 180.0f);
        mote.size = (float)(rand() % 4 + 3); // 3px, 4px, 5px, 6px square pixels (+50% larger)
        mote.baseAlpha = (float)(rand() % 40 + 45) / 100.0f; // 0.45 to 0.85 bright visibility
        mote.currentAlpha = mote.baseAlpha;
        mote.color = color;
        dustParticles.push_back(mote);
    }
}

void EffectManager::ClearDustParticles() {
    dustParticles.clear();
}

void EffectManager::SpawnForgettingEffect(Rectangle rect, Color color) {
    DissolveEffect eff;
    eff.sourceRect = rect;
    eff.color = color;
    eff.timer = 0.0f;
    eff.maxDuration = 0.95f;

    // Spawn floating square pixel particles (3px - 6px)
    for (int i = 0; i < 32; i++) {
        float px = rect.x + (float)(rand() % (int)fmaxf(rect.width, 10.0f));
        float py = rect.y + (float)(rand() % (int)fmaxf(rect.height, 10.0f));
        float vx = ((float)(rand() % 120) - 60.0f) * 1.1f;
        float vy = -((float)(rand() % 110) + 40.0f) * 1.1f;
        float sz = (float)(rand() % 4 + 3); // 3px to 6px crisp square pixel motes

        eff.particles.push_back({ { px, py }, { vx, vy }, 1.0f, sz, color });
    }

    effects.push_back(eff);
}

void EffectManager::SpawnShimmerGlint(Vector2 pos, float size, Color glowColor) {
    ShimmerGlint g;
    g.pos = pos;
    g.timer = 0.0f;
    g.maxDuration = 0.55f;
    g.size = size;
    g.glowColor = glowColor;
    glints.push_back(g);
}

void EffectManager::Update(float dt) {
    float time = (float)GetTime();

    // 1. Update Dust Motes (lively drift across entire room 2000x800)
    if (dustEnabled) {
        for (auto& mote : dustParticles) {
            mote.pos.x += (mote.baseVel.x + sinf(time * 0.9f + mote.phase) * 16.0f) * dt;
            mote.pos.y += (mote.baseVel.y + cosf(time * 0.7f + mote.phase) * 12.0f) * dt;

            // Alpha pulsation in light beams
            mote.currentAlpha = mote.baseAlpha * (0.6f + 0.4f * sinf(time * 1.8f + mote.phase));

            // Wrap smoothly around virtual room 2000x800
            if (mote.pos.x < 0.0f) mote.pos.x += 2000.0f;
            if (mote.pos.x > 2000.0f) mote.pos.x -= 2000.0f;
            if (mote.pos.y < 0.0f) mote.pos.y += 800.0f;
            if (mote.pos.y > 800.0f) mote.pos.y -= 800.0f;
        }
    }

    // 2. Update Shimmer Glints
    for (auto it = glints.begin(); it != glints.end(); ) {
        it->timer += dt;
        if (it->timer >= it->maxDuration) {
            it = glints.erase(it);
        } else {
            ++it;
        }
    }

    // 3. Update Dissolve Effects
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
    // 1. Draw Dust Motes as crisp floating square pixels
    if (dustEnabled) {
        for (const auto& mote : dustParticles) {
            int px = (int)mote.pos.x;
            int py = (int)mote.pos.y;
            int sz = (int)mote.size;

            // Soft 1px ambient border
            DrawRectangle(px - 1, py - 1, sz + 2, sz + 2, Fade(mote.color, mote.currentAlpha * 0.40f));
            // Bright illuminated pixel core
            DrawRectangle(px, py, sz, sz, Fade(WHITE, mote.currentAlpha * 0.90f));
        }
    }

    // 2. Draw Soft, Subtle Silver/Glass Shimmer Glints (delicate ~55% opacity)
    for (const auto& g : glints) {
        float progress = g.timer / g.maxDuration;
        float scale = sinf(progress * PI);
        float alpha = scale * 0.55f; // Soft, delicate, non-intrusive opacity
        float r = g.size * scale;
        Vector2 c = g.pos;

        // Outer soft halo
        DrawCircleV(c, r * 1.6f, Fade(g.glowColor, alpha * 0.25f));

        // 4-point primary flare rays (Thin, delicate lines)
        Color rayCol = Fade(WHITE, alpha * 0.75f);
        DrawLineEx(Vector2{ c.x - r * 2.4f, c.y }, Vector2{ c.x + r * 2.4f, c.y }, 1.8f, rayCol);
        DrawLineEx(Vector2{ c.x, c.y - r * 2.4f }, Vector2{ c.x, c.y + r * 2.4f }, 1.8f, rayCol);

        // Diagonal secondary rays
        Color diagCol = Fade(g.glowColor, alpha * 0.45f);
        DrawLineEx(Vector2{ c.x - r * 1.1f, c.y - r * 1.1f }, Vector2{ c.x + r * 1.1f, c.y + r * 1.1f }, 1.0f, diagCol);
        DrawLineEx(Vector2{ c.x - r * 1.1f, c.y + r * 1.1f }, Vector2{ c.x + r * 1.1f, c.y - r * 1.1f }, 1.0f, diagCol);

        // Core soft bright center
        DrawCircleV(c, r * 0.45f, Fade(WHITE, alpha * 0.85f));
    }

    // 3. Draw Crisp Pixelated Memory Pick-up Particle Burst
    for (const auto& eff : effects) {
        for (const auto& p : eff.particles) {
            int sz = (int)p.size;
            int px = (int)(p.pos.x - sz / 2.0f);
            int py = (int)(p.pos.y - sz / 2.0f);

            // Outer colored pixel border
            DrawRectangle(px - 1, py - 1, sz + 2, sz + 2, Fade(p.color, p.alpha * 0.75f));
            // Inner crisp white pixel
            DrawRectangle(px, py, sz, sz, Fade(WHITE, p.alpha * 0.95f));
        }
    }
}
