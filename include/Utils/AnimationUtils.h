// include/Utils/AnimationUtils.h
#ifndef ANIMATIONUTILS_H
#define ANIMATIONUTILS_H

#include <string>
#include "../entities/Digimon.h" // For DigimonType enum (adjust path if needed)
#include "../entities/DigimonDefinition.h" // Add DigimonDefinition
// No need to include PlayerState.h if we pass animNameSuffix as a string

// Forward declarations
namespace Digimon {
    struct DigimonDefinition;
}

namespace AnimationUtils {

    // Legacy function - Gets the base string ID for a Digimon's assets (e.g., "agumon_sheet")
    std::string GetDigimonBaseId(DigimonType type);
    
    // New function - Gets the base string ID from a DigimonDefinition
    std::string GetDigimonBaseId(const Digimon::DigimonDefinition* definition);

    // Legacy function - Constructs the full animation ID used by AnimationManager.
    // Example: GetAnimationId(DIGI_AGUMON, "Idle") -> "agumon_sheet_Idle"
    // Example: GetAnimationId(DIGI_GABUMON, "Walk") -> "gabumon_sheet_Walk"
    std::string GetAnimationId(DigimonType type, const std::string& animNameSuffix);
    
    // New function - Constructs the animation ID from a DigimonDefinition
    std::string GetAnimationId(const Digimon::DigimonDefinition* definition, const std::string& animNameSuffix);

    // Overload that takes a pre-resolved base ID
    std::string GetAnimationId(const std::string& baseId, const std::string& animNameSuffix);

} // namespace AnimationUtils

#endif // ANIMATIONUTILS_H