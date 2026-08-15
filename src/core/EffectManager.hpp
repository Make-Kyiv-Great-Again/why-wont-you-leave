#pragma once
#include "raylib.h"
#include <vector>

struct DissolveParticle {
    Vector2 pos;
    Vector2 vel;
    float alpha;
    float size;
    Color color;
};

struct DissolveEffect {
    Rectangle sourceRect;
    Color color;
    float timer;
    float maxDuration;
    std::vector<DissolveParticle> particles;
};

struct DustMote {
    Vector2 pos;
    Vector2 baseVel;
    float phase;
    float size;
    float baseAlpha;
    float currentAlpha;
    Color color;
};

struct ShimmerGlint {
    Vector2 pos;
    float timer;
    float maxDuration;
    float size;
    Color glowColor;
};

class EffectManager {
public:
    static EffectManager& Get();

    void InitDustParticles(int count = 60, Color color = Color{ 255, 245, 220, 180 });
    void ClearDustParticles();
    void SetDustEnabled(bool enabled) { dustEnabled = enabled; }

    void SpawnForgettingEffect(Rectangle rect, Color color);
    void SpawnShimmerGlint(Vector2 pos, float size = 14.0f, Color glowColor = Color{ 220, 240, 255, 255 });

    void Update(float dt);
    void Draw();

private:
    EffectManager();
    ~EffectManager() = default;
    EffectManager(const EffectManager&) = delete;
    EffectManager& operator=(const EffectManager&) = delete;

    std::vector<DissolveEffect> effects;
    std::vector<DustMote> dustParticles;
    std::vector<ShimmerGlint> glints;
    bool dustEnabled = true;
};
