// src/Utils/AnimationUtils.cpp
#include "utils/AnimationUtils.h" // Include the header we just created
#include <SDL_log.h>              // For logging warnings on unknown types

// Note: "../entities/Digimon.h" is already included via AnimationUtils.h

namespace AnimationUtils {

    std::string GetDigimonBaseId(DigimonType type) {
        switch (type) {
            case DIGI_AGUMON:   return "agumon_sheet";
            case DIGI_GABUMON:  return "gabumon_sheet";
            case DIGI_BIYOMON:  return "biyomon_sheet";
            case DIGI_GATOMON:  return "gatomon_sheet";
            case DIGI_GOMAMON:  return "gomamon_sheet";
            case DIGI_PALMON:   return "palmon_sheet";
            case DIGI_TENTOMON: return "tentomon_sheet";
            case DIGI_PATAMON:  return "patamon_sheet";
            case DIGI_KUWAGAMON: return "kuwagamon_sheet";
            // Add cases for any other DigimonType values you have
            default:
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AnimationUtils::GetDigimonBaseId - Unknown DigimonType: %d", static_cast<int>(type));
                return "unknown_sheet"; // Return a default/error string
        }
    }

    std::string GetAnimationId(DigimonType type, const std::string& animNameSuffix) {
        std::string baseId = GetDigimonBaseId(type);
        if (baseId == "unknown_sheet" && type != DIGI_COUNT) { // DIGI_COUNT might be a valid enum member but not a drawable Digimon
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AnimationUtils::GetAnimationId - Could not get base ID for DigimonType: %d", static_cast<int>(type));
        }
        if (animNameSuffix.empty()) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AnimationUtils::GetAnimationId - animNameSuffix is empty for baseId: %s", baseId.c_str());
            return baseId; // Or return baseId + "_Default" or some error string
        }
        return baseId + "_" + animNameSuffix;
    }

    std::string GetDigimonBaseId(const Digimon::DigimonDefinition* definition) {
        if (!definition) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AnimationUtils::GetDigimonBaseId (Definition) - Provided definition is null.");
            return "unknown_sheet";
        }
        if (definition->spriteBaseId.empty()) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AnimationUtils::GetDigimonBaseId (Definition) - spriteBaseId is empty for Digimon ID: %s", definition->id.c_str());
            return "unknown_sheet";
        }
        return definition->spriteBaseId;
    }

    std::string GetAnimationId(const Digimon::DigimonDefinition* definition, const std::string& animNameSuffix) {
        if (!definition) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AnimationUtils::GetAnimationId (Definition) - Provided definition is null.");
            return "unknown_sheet" + (animNameSuffix.empty() ? "" : ("_" + animNameSuffix));
        }
        std::string baseId = GetDigimonBaseId(definition); // Use the new GetDigimonBaseId
        
        if (animNameSuffix.empty()) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AnimationUtils::GetAnimationId (Definition) - animNameSuffix is empty for Digimon ID: %s, baseId: %s", definition->id.c_str(), baseId.c_str());
            return baseId; // Or return baseId + "_Default" or some error string
        }
        return baseId + "_" + animNameSuffix;
    }

    std::string GetAnimationId(const std::string& baseId, const std::string& animNameSuffix) {
        if (baseId.empty()) {
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AnimationUtils::GetAnimationId (string overload) - baseId is empty. Returning empty animation ID.");
             return ""; // Return empty string if baseId is empty
        }
        if (animNameSuffix.empty()) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AnimationUtils::GetAnimationId (string overload) - animNameSuffix is empty for baseId: %s. Returning baseId itself.", baseId.c_str());
            return baseId; // Return baseId if only suffix is empty
        }
        return baseId + "_" + animNameSuffix;
    }

} // namespace AnimationUtils