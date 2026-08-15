#include "core/ActManager.hpp"
#include "core/MemoryManager.hpp"

ActManager& ActManager::Get() {
    static ActManager instance;
    return instance;
}

ActManager::ActManager() {
    currentAct = 1;
}

int ActManager::GetCurrentAct() const {
    return currentAct;
}

void ActManager::SetAct(int act) {
    if (act >= 1 && act <= 5) {
        currentAct = act;
    }
}

void ActManager::AdvanceAct() {
    if (currentAct < 5) {
        currentAct++;
    }
}

void ActManager::Reset() {
    currentAct = 1;
    rememberedArtifacts.clear();
    vanishedArtifacts.clear();
    MemoryManager::Get().Reset();
}

void ActManager::MarkArtifactRemembered(const std::string& artifactId) {
    rememberedArtifacts.insert(artifactId);
    MemoryManager::Get().SetRemembered(artifactId, true);
    VanishArtifact(artifactId);
}

void ActManager::UnmarkArtifactRemembered(const std::string& artifactId) {
    rememberedArtifacts.erase(artifactId);
    vanishedArtifacts.erase(artifactId);
    MemoryManager::Get().SetRemembered(artifactId, false);
}

bool ActManager::IsArtifactRemembered(const std::string& artifactId) const {
    return rememberedArtifacts.find(artifactId) != rememberedArtifacts.end();
}

void ActManager::VanishArtifact(const std::string& artifactId) {
    vanishedArtifacts.insert(artifactId);
}

void ActManager::RestoreArtifact(const std::string& artifactId) {
    vanishedArtifacts.erase(artifactId);
}

bool ActManager::IsArtifactVanished(const std::string& artifactId) const {
    return vanishedArtifacts.find(artifactId) != vanishedArtifacts.end();
}

int ActManager::GetRememberedCountInAct(int act) const {
    int count = 0;
    for (const auto& id : rememberedArtifacts) {
        if (GetArtifactAct(id) == act) {
            count++;
        }
    }
    return count;
}

bool ActManager::CanUseExitDoor() const {
    return rememberedArtifacts.size() >= 5;
}

bool ActManager::HasAllTrueArtifacts() const {
    return IsArtifactRemembered("travel_bag") &&
           IsArtifactRemembered("job_letter") &&
           IsArtifactRemembered("car_keys") &&
           IsArtifactRemembered("diary") &&
           IsArtifactRemembered("accident_info");
}

Color ActManager::GetActLightingTint() const {
    switch (currentAct) {
        case 1:
            return WHITE; // Normal apartment
        case 2:
            return Color{ 175, 195, 235, 255 }; // Colder blue tint, heavier shadows
        case 3:
            return Color{ 150, 155, 170, 255 }; // Near monochrome, moody
        case 4:
            return Color{ 120, 120, 130, 255 }; // Stark monochrome
        case 5:
            return BLACK; // Endless void
        default:
            return WHITE;
    }
}

std::string ActManager::GetActTitle() const {
    switch (currentAct) {
        case 1: return "ACT I — THE APARTMENT";
        case 2: return "ACT II — THE THINGS SHE HID";
        case 3: return "ACT III — THE CHOICE";
        case 4: return "ACT IV — THE CRASH";
        case 5: return "ACT V — THE DOOR";
        default: return "ACT";
    }
}

bool ActManager::IsMonochromeAct() const {
    return currentAct >= 3;
}

int ActManager::GetArtifactAct(const std::string& artifactId) {
    if (artifactId == "photo_of_couple" || artifactId == "travel_bag" || 
        artifactId == "medicine" || artifactId == "car_keys" || artifactId == "diary" ||
        artifactId == "accident_info" || artifactId == "broken_plate" || artifactId == "guitar" ||
        artifactId == "toothbrushes" || artifactId == "job_letter") {
        return 1;
    }
    return 1;
}

bool ActManager::IsArtifactTrue(const std::string& artifactId) {
    if (artifactId == "photo_of_couple" || artifactId == "broken_plate" || 
        artifactId == "guitar" || artifactId == "toothbrushes" || artifactId == "medicine") {
        return false; // Optional / Unnecessary artifacts
    }
    return true; // Required artifacts (travel_bag, job_letter, car_keys, diary, accident_info)
}
