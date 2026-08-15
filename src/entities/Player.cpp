#include "entities/Player.hpp"
#include "core/ResourceManager.hpp"

Player::Player(float x, float groundY)
    : rect{ x, groundY - 390.0f, 260.0f, 390.0f },
      speed(580.0f),
      color(DARKBLUE) {}

void Player::Update(float dt, float screenWidth) {
    PlayerState previousState = state;
    bool isMoving = false;

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
        rect.x -= speed * dt;
        facingRight = false;
        isMoving = true;
    }
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
        rect.x += speed * dt;
        facingRight = true;
        isMoving = true;
    }

    // Bound within screen limits
    if (rect.x < 0) rect.x = 0;
    if (rect.x + rect.width > screenWidth) {
        rect.x = screenWidth - rect.width;
    }

    state = isMoving ? PlayerState::Walking : PlayerState::Idle;

    // Reset animation frame on state transition
    if (state != previousState) {
        currentFrame = 0;
        frameTimer = 0.0f;
    }

    // Advance frame timer
    frameTimer += dt;
    float currentSpeed = (state == PlayerState::Walking) ? walkFrameSpeed : idleFrameSpeed;
    int maxFrames = (state == PlayerState::Walking) ? walkFrameCount : idleFrameCount;

    if (frameTimer >= currentSpeed) {
        frameTimer = 0.0f;
        currentFrame = (currentFrame + 1) % maxFrames;
    }
}

void Player::ForceMove(float dx, float dt) {
    rect.x += dx;
    facingRight = (dx > 0);
    state = PlayerState::Walking;

    frameTimer += dt;
    if (frameTimer >= walkFrameSpeed) {
        frameTimer = 0.0f;
        currentFrame = (currentFrame + 1) % walkFrameCount;
    }
}

void Player::Draw() const {
    std::string texPath = (state == PlayerState::Walking)
        ? "assets/sprites/playerWalkRightAnimation.png"
        : "assets/sprites/playerIdleAnimation.png";

    Texture2D tex = ResourceManager::Get().GetTexture(texPath);

    if (tex.id != 0) {
        // Negative width flips texture horizontally when facing left
        Rectangle sourceRect = {
            currentFrame * frameWidth,
            0.0f,
            facingRight ? frameWidth : -frameWidth,
            frameHeight
        };

        DrawTexturePro(
            tex,
            sourceRect,
            rect,
            Vector2{ 0, 0 },
            0.0f,
            WHITE
        );
    } else {
        // Fallback vector player
        DrawRectangleRec(rect, color);
        DrawRectangleLinesEx(rect, 2, BLACK);
    }
}
