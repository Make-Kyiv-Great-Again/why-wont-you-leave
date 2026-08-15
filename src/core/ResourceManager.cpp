#include "core/ResourceManager.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

ResourceManager& ResourceManager::Get() {
    static ResourceManager instance;
    return instance;
}

ResourceManager::~ResourceManager() {
    ClearCache();
}

Color ResourceManager::HexToColor(const std::string& hexStr) {
    std::string hex = hexStr;
    if (!hex.empty() && hex[0] == '#') {
        hex = hex.substr(1);
    }
    if (hex.length() != 6) {
        return WHITE;
    }

    unsigned int r, g, b;
    std::stringstream ss;
    ss << std::hex << hex.substr(0, 2);
    ss >> r;
    ss.clear();
    ss << std::hex << hex.substr(2, 2);
    ss >> g;
    ss.clear();
    ss << std::hex << hex.substr(4, 2);
    ss >> b;

    return Color{ (unsigned char)r, (unsigned char)g, (unsigned char)b, 255 };
}

nlohmann::json ResourceManager::LoadJson(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        TraceLog(LOG_ERROR, "RESOURCE: Failed to open JSON file: %s", filePath.c_str());
        return nlohmann::json();
    }
    nlohmann::json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        TraceLog(LOG_ERROR, "RESOURCE: Error parsing JSON in %s: %s", filePath.c_str(), e.what());
    }
    return j;
}

Texture2D ResourceManager::GetTexture(const std::string& filePath) {
    if (filePath.empty()) return Texture2D{ 0 };

    auto it = textureCache.find(filePath);
    if (it != textureCache.end()) {
        return it->second;
    }

    std::string pathToLoad = filePath;
    if (filePath.length() >= 4 && filePath.substr(filePath.length() - 4) == ".svg") {
        std::string pngPath = filePath.substr(0, filePath.length() - 4) + ".png";
        if (FileExists(pngPath.c_str())) {
            pathToLoad = pngPath;
        }
    }

    Texture2D tex = LoadTexture(pathToLoad.c_str());
    if (tex.id != 0) {
        SetTextureFilter(tex, TEXTURE_FILTER_POINT);
        textureCache[filePath] = tex;
    }
    return tex;
}

Shader ResourceManager::GetShader(const std::string& filePath) {
    if (filePath.empty()) return Shader{ 0 };

    auto it = shaderCache.find(filePath);
    if (it != shaderCache.end()) {
        return it->second;
    }

    Shader shader = LoadShader(nullptr, filePath.c_str());
    if (shader.id != 0) {
        shaderCache[filePath] = shader;
    }
    return shader;
}

void ResourceManager::ClearCache() {
    for (auto& pair : textureCache) {
        UnloadTexture(pair.second);
    }
    textureCache.clear();

    for (auto& pair : shaderCache) {
        UnloadShader(pair.second);
    }
    shaderCache.clear();
    jsonCache.clear();
}
