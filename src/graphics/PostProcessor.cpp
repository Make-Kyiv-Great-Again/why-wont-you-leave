#include "graphics/PostProcessor.hpp"

PostProcessor::PostProcessor(const char* shaderPath) {
    shader = raylib::Shader::Load(0, shaderPath);
}

void PostProcessor::BeginPostProcess() {
    shader.BeginMode();
}

void PostProcessor::EndPostProcess(const raylib::RenderTexture2D& target) {

    // Draw the passed render texture
    target.GetTexture().Draw(
        raylib::Rectangle(0, 0, (float)target.texture.width, (float)-target.texture.height),
        raylib::Rectangle(0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()),
        raylib::Vector2(0, 0),
        0.0f,
        WHITE
    );
    shader.EndMode();
}
