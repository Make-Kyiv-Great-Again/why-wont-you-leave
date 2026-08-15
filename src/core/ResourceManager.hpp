#pragma once
#include "raylib.h"
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

class ResourceManager {
public:
    static ResourceManager& Get();

    // Color conversion utility (#RRGGBB -> Color)
    static Color HexToColor(const std::string& hexStr);

    // JSON file loading & caching
    nlohmann::json LoadJson(const std::string& filePath);

    // Texture caching
    Texture2D GetTexture(const std::string& filePath);

    // Shader caching
    Shader GetShader(const std::string& filePath);

    // Unload cached assets
    void ClearCache();

private:
    ResourceManager() = default;
    ~ResourceManager();
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    std::unordered_map<std::string, nlohmann::json> jsonCache;
    std::unordered_map<std::string, Texture2D> textureCache;
    std::unordered_map<std::string, Shader> shaderCache;
};
