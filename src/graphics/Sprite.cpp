#include "graphics/Sprite.hpp"
#include "core/ResourceManager.hpp"

void Sprite::Update(float dt) {
    if (!isAnimated || frameCount <= 1) return;

    timer += dt;
    if (timer >= frameTime) {
        timer = 0.0f;
        currentFrame = (currentFrame + 1) % frameCount;
    }
}

void Sprite::Draw(Rectangle destRect) const {
    if (!IsValid()) return;

    Texture2D tex = ResourceManager::Get().GetTexture(texturePath);
    if (tex.id == 0) return;

    Rectangle srcRect = sourceRect;
    if (srcRect.width <= 0.0f || srcRect.height <= 0.0f) {
        srcRect = Rectangle{ 0.0f, 0.0f, (float)tex.width, (float)tex.height };
    }

    if (isAnimated && frameCount > 1) {
        float frameW = srcRect.width / frameCount;
        srcRect.x = currentFrame * frameW;
        srcRect.width = frameW;
    }

    DrawTexturePro(tex, srcRect, destRect, Vector2{ 0.0f, 0.0f }, 0.0f, tint);
}
