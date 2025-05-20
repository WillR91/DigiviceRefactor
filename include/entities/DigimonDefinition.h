#pragma once

#include <string>
#include <vector>
#include <map>
#include "entities/Digimon.h" // Include for backward compatibility during transition

namespace Digimon {

// Classification of Digimon types for game logic
enum class DigimonClass {
    PlayerRookie,     // Player's starting Digimon (Agumon, Gabumon, etc.)
    PlayerChampion,   // Player's Digivolved forms (not implemented yet)
    PlayerUltimate    // Player's Digivolved forms (not implemented yet)
    PlayerMega        // Player's Final forms (not implemented yet)
    PlayerUnlockables
    StandardEnemy,    // Regular enemy Digimon (Kuwagamon, etc.)
    Boss,             // Boss Digimon with special abilities
    NPC               // Non-combat Digimon for story/dialog
};

// Stats for a Digimon
struct DigimonStats {
    int hp;
    int attack;
    int defense;
    int speed;
};

// Definition of a Digimon
struct DigimonDefinition {
    std::string id;                 // Unique identifier (e.g., "agumon")
    std::string displayName;        // Name to display (e.g., "Agumon")
    DigimonClass digimonClass;      // Classification of the Digimon
    DigimonStats stats;             // Base stats
    std::string spriteBaseId;       // Base ID for sprite sheets (e.g., "agumon_sheet")
    std::string description;        // Short description
    std::vector<std::string> animations; // List of supported animation types (e.g., "Idle", "Walk")
    std::map<std::string, std::string> attributes; // Additional attributes for flexibility
    DigimonType legacyEnum;         // Legacy enum value for backward compatibility
    
    // Constructor with basic initialization
    DigimonDefinition(
        const std::string& id = "",
        const std::string& displayName = "",
        DigimonClass digimonClass = DigimonClass::StandardEnemy,
        const DigimonStats& stats = {100, 10, 5, 5},
        const std::string& spriteBaseId = "",
        const std::string& description = "",
        DigimonType legacyEnum = DigimonType::NONE
    ) : id(id),
        displayName(displayName),
        digimonClass(digimonClass),
        stats(stats),
        spriteBaseId(spriteBaseId),
        description(description),
        legacyEnum(legacyEnum) {}
        
    // Helper to convert old enum to definition
    static DigimonDefinition fromLegacyEnum(DigimonType type);
};

// String conversion utilities
std::string digimonClassToString(DigimonClass digimonClass);
DigimonClass stringToDigimonClass(const std::string& classStr);

} // namespace Digimon
