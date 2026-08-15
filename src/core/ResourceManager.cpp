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

Font ResourceManager::GetFont(const std::string& filePath) {
    if (filePath.empty()) return GetFontDefault();

    auto it = fontCache.find(filePath);
    if (it != fontCache.end()) {
        return it->second;
    }

    Font font = LoadFont(filePath.c_str());
    if (font.texture.id != 0) {
        fontCache[filePath] = font;
    } else {
        TraceLog(LOG_WARNING, "RESOURCE: Failed to load Font: %s", filePath.c_str());
    }
    return font;
}

Sound ResourceManager::GetSound(const std::string& filePath) {
    if (filePath.empty()) return Sound{ 0 };

    auto it = soundCache.find(filePath);
    if (it != soundCache.end()) {
        return it->second;
    }

    Sound sound = LoadSound(filePath.c_str());
    if (sound.frameCount != 0) {
        soundCache[filePath] = sound;
    } else {
        TraceLog(LOG_WARNING, "RESOURCE: Failed to load Sound: %s", filePath.c_str());
    }
    return sound;
}

void ResourceManager::DrawGameText(const char* text, float posX, float posY, float fontSize, Color color) {
    Font font = ResourceManager::Get().GetFont("assets/fonts/Jersey10-Regular.ttf");
    DrawTextEx(font, text, Vector2{ posX, posY }, fontSize, 2.0f, color);
}

int ResourceManager::MeasureGameText(const char* text, float fontSize) {
    Font font = ResourceManager::Get().GetFont("assets/fonts/Jersey10-Regular.ttf");
    Vector2 size = MeasureTextEx(font, text, fontSize, 2.0f);
    return (int)size.x;
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

    for (auto& pair : fontCache) {
        UnloadFont(pair.second);
    }
    fontCache.clear();

    for (auto& pair : soundCache) {
        UnloadSound(pair.second);
    }
    soundCache.clear();
    
    jsonCache.clear();
}
