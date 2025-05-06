// include/Utils/AnimationUtils.h
#ifndef ANIMATIONUTILS_H
#define ANIMATIONUTILS_H

#include <string>
#include "../entities/Digimon.h" // For DigimonType enum (adjust path if needed)
// No need to include PlayerState.h if we pass animNameSuffix as a string

// Forward declarations if needed by function signatures (not in this case for free functions)

namespace AnimationUtils {

    // Gets the base string ID for a Digimon's assets (e.g., "agumon_sheet")
    std::string GetDigimonBaseId(DigimonType type);

    // Constructs the full animation ID used by AnimationManager.
    // Example: GetAnimationId(DIGI_AGUMON, "Idle") -> "agumon_sheet_Idle"
    // Example: GetAnimationId(DIGI_GABUMON, "Walk") -> "gabumon_sheet_Walk"
    std::string GetAnimationId(DigimonType type, const std::string& animNameSuffix);

    // Overload that takes a pre-resolved base ID
    std::string GetAnimationId(const std::string& baseId, const std::string& animNameSuffix);

} // namespace AnimationUtils

#endif // ANIMATIONUTILS_H