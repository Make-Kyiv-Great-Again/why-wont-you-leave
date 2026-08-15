#pragma once
#include "raylib.h"
#include <string>
#include <unordered_set>
#include <vector>

class ActManager {
public:
    static ActManager& Get();

    int GetCurrentAct() const;
    void SetAct(int act);
    void AdvanceAct();
    void Reset();

    // Artifact tracking
    void MarkArtifactRemembered(const std::string& artifactId);
    void UnmarkArtifactRemembered(const std::string& artifactId);
    bool IsArtifactRemembered(const std::string& artifactId) const;
    
    void VanishArtifact(const std::string& artifactId);
    void RestoreArtifact(const std::string& artifactId);
    bool IsArtifactVanished(const std::string& artifactId) const;

    int GetRememberedCountInAct(int act) const;
    bool CanUseExitDoor() const;

    // Visuals & Environment
    Color GetActLightingTint() const;
    std::string GetActTitle() const;
    bool IsMonochromeAct() const;

    // Artifact metadata query
    static int GetArtifactAct(const std::string& artifactId);
    static bool IsArtifactTrue(const std::string& artifactId);

private:
    ActManager();
    ~ActManager() = default;
    ActManager(const ActManager&) = delete;
    ActManager& operator=(const ActManager&) = delete;

    int currentAct = 1;
    std::unordered_set<std::string> rememberedArtifacts;
    std::unordered_set<std::string> vanishedArtifacts;
};
