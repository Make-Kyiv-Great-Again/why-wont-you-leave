#pragma once
#include <raylib-cpp.hpp>

class PostProcessor {
public:
    PostProcessor(const char* shaderPath);
    ~PostProcessor() = default;
    
    void BeginPostProcess();
    void EndPostProcess(const raylib::RenderTexture2D& target);

private:
    raylib::Shader shader;
};
