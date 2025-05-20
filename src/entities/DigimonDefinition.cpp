#include "entities/DigimonDefinition.h"
#include "utils/AnimationUtils.h"
#include <SDL_log.h>

namespace Digimon {

// Static method to convert legacy enum to new definition format
DigimonDefinition DigimonDefinition::fromLegacyEnum(DigimonType type) {
    DigimonDefinition def;
    
    // Set the legacy enum value for backward compatibility
    def.legacyEnum = type;
    
    // Set up default values based on enum type
    switch (type) {        case DIGI_AGUMON:
            def.id = "agumon";
            def.displayName = "Agumon";
            def.digimonClass = DigimonClass::PlayerRookie;
            def.stats = {150, 15, 10, 8};
            def.spriteBaseId = "agumon"; // Updated to use new sprite ID
            def.description = "A Reptile Digimon with an appearance resembling a small dinosaur.";
            def.animations = {"Idle", "Walk", "Run", "Attack"};
            break;
              case DIGI_GABUMON:
            def.id = "gabumon";
            def.displayName = "Gabumon";
            def.digimonClass = DigimonClass::PlayerRookie;
            def.stats = {130, 13, 12, 7};
            def.spriteBaseId = "gabumon";
            def.description = "A Reptile Digimon with a fur pelt.";
            def.animations = {"Idle", "Walk", "Run", "Attack"};
            break;
              case DIGI_BIYOMON:
            def.id = "biyomon";
            def.displayName = "Biyomon";
            def.digimonClass = DigimonClass::PlayerRookie;
            def.stats = {120, 12, 8, 12};
            def.spriteBaseId = "biyomon";
            def.description = "A Bird Digimon with pink feathers.";
            def.animations = {"Idle", "Walk", "Run", "Attack"};
            break;
              case DIGI_GATOMON:
            def.id = "gatomon";
            def.displayName = "Gatomon";
            def.digimonClass = DigimonClass::PlayerRookie;
            def.stats = {125, 14, 9, 13};
            def.spriteBaseId = "gatomon";
            def.description = "A feline Digimon with Holy Ring on its tail.";
            def.animations = {"Idle", "Walk", "Run", "Attack"};
            break;
              case DIGI_GOMAMON:
            def.id = "gomamon";
            def.displayName = "Gomamon";
            def.digimonClass = DigimonClass::PlayerRookie;
            def.stats = {140, 11, 11, 9};
            def.spriteBaseId = "gomamon";
            def.description = "A Sea Animal Digimon with purple markings.";
            def.animations = {"Idle", "Walk", "Run", "Attack"};
            break;
              case DIGI_PALMON:
            def.id = "palmon";
            def.displayName = "Palmon";
            def.digimonClass = DigimonClass::PlayerRookie;
            def.stats = {135, 12, 10, 7};
            def.spriteBaseId = "palmon";
            def.description = "A Plant Digimon with a flower on its head.";
            def.animations = {"Idle", "Walk", "Run", "Attack"};
            break;
              case DIGI_TENTOMON:
            def.id = "tentomon";
            def.displayName = "Tentomon";
            def.digimonClass = DigimonClass::PlayerRookie;
            def.stats = {130, 13, 11, 8};
            def.spriteBaseId = "tentomon";
            def.description = "An Insectoid Digimon with a hard shell.";
            def.animations = {"Idle", "Walk", "Run", "Attack"};
            break;
              case DIGI_PATAMON:
            def.id = "patamon";
            def.displayName = "Patamon";
            def.digimonClass = DigimonClass::PlayerRookie;
            def.stats = {120, 12, 9, 11};
            def.spriteBaseId = "patamon";
            def.description = "A Mammal Digimon with bat-like wings.";
            def.animations = {"Idle", "Walk", "Run", "Attack"};
            break;
              case DIGI_KUWAGAMON:
            def.id = "kuwagamon";
            def.displayName = "Kuwagamon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {180, 18, 12, 9};
            def.spriteBaseId = "kuwagamon";
            def.description = "An Insectoid Digimon with large pincers and red shell.";
            def.animations = {"Idle", "Attack"};
            break;
            
        default:
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "DigimonDefinition::fromLegacyEnum: Unknown DigimonType: %d", static_cast<int>(type));            def.id = "unknown";
            def.displayName = "Unknown Digimon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.spriteBaseId = "unknown";
            def.description = "An unidentified Digimon.";
            break;
    }
    
    return def;
}

} // namespace Digimon
