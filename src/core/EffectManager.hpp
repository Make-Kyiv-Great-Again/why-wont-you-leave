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

class EffectManager {
public:
    static EffectManager& Get();

    void SpawnForgettingEffect(Rectangle rect, Color color);
    void Update(float dt);
    void Draw();

private:
    EffectManager() = default;
    ~EffectManager() = default;
    EffectManager(const EffectManager&) = delete;
    EffectManager& operator=(const EffectManager&) = delete;

    std::vector<DissolveEffect> effects;
};
