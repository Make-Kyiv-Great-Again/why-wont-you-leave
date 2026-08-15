#pragma once
#include "raylib.h"
#include <string>
#include <vector>

struct MemoryArtifact {
    std::string id;
    std::string name;
    std::string texturePath;
    Color color;
    Color borderColor;
    bool isRemembered;
    std::string savedChoice;
    int roomId;
};

class MemoryManager {
public:
    static MemoryManager& Get();

    bool IsRemembered(const std::string& id) const;
    void SetRemembered(const std::string& id, bool remembered);
    void RegisterArtifact(const std::string& id, const std::string& name, const std::string& texturePath, Color color, Color borderColor, int roomId);
    void SaveChoice(const std::string& id, const std::string& choice);
    std::string GetSavedChoice(const std::string& id) const;
    int GetRememberedCount() const;
    std::vector<const MemoryArtifact*> GetRememberedArtifacts() const;
    const std::vector<MemoryArtifact>& GetAllArtifacts() const;

    void DrawMemoryInventoryOverlay(float transition);

private:
    MemoryManager();
    ~MemoryManager() = default;
    MemoryManager(const MemoryManager&) = delete;
    MemoryManager& operator=(const MemoryManager&) = delete;

    std::vector<MemoryArtifact> artifacts;
};
