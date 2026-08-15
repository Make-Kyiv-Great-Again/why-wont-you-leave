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
}

void ActManager::MarkArtifactRemembered(const std::string& artifactId) {
    rememberedArtifacts.insert(artifactId);
    MemoryManager::Get().SetRemembered(artifactId, true);
    VanishArtifact(artifactId);
}

bool ActManager::IsArtifactRemembered(const std::string& artifactId) const {
    return rememberedArtifacts.find(artifactId) != rememberedArtifacts.end();
}

void ActManager::VanishArtifact(const std::string& artifactId) {
    vanishedArtifacts.insert(artifactId);
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
    // In Act 1: allow exit after player has interacted with/remembered at least 2 Act 1 artifacts
    if (currentAct == 1) {
        return GetRememberedCountInAct(1) >= 2;
    }
    // In Act 2: allow after remembering at least 2 Act 2 artifacts
    if (currentAct == 2) {
        return GetRememberedCountInAct(2) >= 2;
    }
    // In Act 3: allow after remembering at least 2 Act 3 artifacts
    if (currentAct == 3) {
        return GetRememberedCountInAct(3) >= 2;
    }
    // In Act 4: exit door is accessed via completing windshield fragment
    if (currentAct == 4) {
        return IsArtifactRemembered("windshield_fragment");
    }
    return true;
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
    if (artifactId == "photo_evan_grace" || artifactId == "travel_bag" || 
        artifactId == "grace_medicine" || artifactId == "car_keys" || artifactId == "torn_envelope") {
        return 1;
    }
    if (artifactId == "phone" || artifactId == "acceptance_letter" || 
        artifactId == "photo_other_man" || artifactId == "bank_receipt" || artifactId == "anniversary_note") {
        return 2;
    }
    if (artifactId == "unsent_letter" || artifactId == "ticket" || 
        artifactId == "spare_key" || artifactId == "hotel_reservation") {
        return 3;
    }
    if (artifactId == "hospital_bracelet" || artifactId == "windshield_fragment") {
        return 4;
    }
    return 1;
}

bool ActManager::IsArtifactTrue(const std::string& artifactId) {
    if (artifactId == "grace_medicine" || artifactId == "photo_other_man" || 
        artifactId == "anniversary_note" || artifactId == "spare_key" || artifactId == "hotel_reservation") {
        return false;
    }
    return true;
}
