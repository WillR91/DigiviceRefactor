#include "utils/ScalingUtils.h"
#include "utils/ConfigManager.h"
#include <SDL_log.h>
#include <unordered_map>

namespace ScalingUtils {

    // Cache scaling factors to avoid repeated config lookups
    static std::unordered_map<ElementType, float> scalingFactors;
    static bool isInitialized = false;

    void initialize() {
        updateFromConfig();
        isInitialized = true;
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ScalingUtils: Initialized with config values");
    }

    void updateFromConfig() {
        if (!ConfigManager::isInitialized()) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ScalingUtils: ConfigManager not initialized, using default scaling factors");
            // Set default values
            scalingFactors[ElementType::SPRITES] = 1.0f;
            scalingFactors[ElementType::UI] = 1.0f;
            scalingFactors[ElementType::TEXT] = 1.0f;
            scalingFactors[ElementType::ENVIRONMENTS] = 1.0f;
            scalingFactors[ElementType::EFFECTS] = 1.0f;
            return;
        }

        // Load scaling factors from config
        scalingFactors[ElementType::SPRITES] = ConfigManager::getValue<float>("display.scaling.sprites", 1.0f);
        scalingFactors[ElementType::UI] = ConfigManager::getValue<float>("display.scaling.ui", 1.0f);
        scalingFactors[ElementType::TEXT] = ConfigManager::getValue<float>("display.scaling.text", 1.0f);
        scalingFactors[ElementType::ENVIRONMENTS] = ConfigManager::getValue<float>("display.scaling.environments", 1.0f);
        scalingFactors[ElementType::EFFECTS] = ConfigManager::getValue<float>("display.scaling.effects", 1.0f);

        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, 
                   "ScalingUtils: Updated scaling factors - Sprites: %.2f, UI: %.2f, Text: %.2f, Environments: %.2f, Effects: %.2f",
                   scalingFactors[ElementType::SPRITES], scalingFactors[ElementType::UI], scalingFactors[ElementType::TEXT],
                   scalingFactors[ElementType::ENVIRONMENTS], scalingFactors[ElementType::EFFECTS]);
    }

    float getScalingFactor(ElementType type) {
        if (!isInitialized) {
            initialize();
        }
        
        auto it = scalingFactors.find(type);
        if (it != scalingFactors.end()) {
            return it->second;
        }
        
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ScalingUtils: Unknown element type, returning default scale 1.0f");
        return 1.0f;
    }

    SDL_Rect createScaledDestRect(const SDL_Rect& srcRect, int x, int y, ElementType type) {
        float scale = getScalingFactor(type);
        
        SDL_Rect destRect;
        destRect.x = x;
        destRect.y = y;
        destRect.w = static_cast<int>(static_cast<float>(srcRect.w) * scale);
        destRect.h = static_cast<int>(static_cast<float>(srcRect.h) * scale);
        
        return destRect;
    }

    SDL_Point applyScaling(int width, int height, ElementType type) {
        float scale = getScalingFactor(type);
        
        SDL_Point result;
        result.x = static_cast<int>(static_cast<float>(width) * scale);
        result.y = static_cast<int>(static_cast<float>(height) * scale);
        
        return result;
    }

    int applyScaling(int value, ElementType type) {
        float scale = getScalingFactor(type);
        return static_cast<int>(static_cast<float>(value) * scale);
    }

    float applyScaling(float value, ElementType type) {
        float scale = getScalingFactor(type);
        return value * scale;
    }    SDL_Rect createScaledRect(int x, int y, int width, int height, ElementType type) {
        float scale = getScalingFactor(type);
        
        SDL_Rect destRect;
        destRect.x = x;
        destRect.y = y;
        destRect.w = static_cast<int>(static_cast<float>(width) * scale);
        destRect.h = static_cast<int>(static_cast<float>(height) * scale);
        
        return destRect;
    }

    SDL_Rect createCenteredScaledRect(int width, int height, ElementType type) {
        float scale = getScalingFactor(type);
        
        // Calculate scaled dimensions
        int scaledWidth = static_cast<int>(static_cast<float>(width) * scale);
        int scaledHeight = static_cast<int>(static_cast<float>(height) * scale);
        
        // Center on logical screen (466x466)
        const int LOGICAL_SIZE = 466;
        SDL_Rect destRect;
        destRect.x = (LOGICAL_SIZE / 2) - (scaledWidth / 2);
        destRect.y = (LOGICAL_SIZE / 2) - (scaledHeight / 2);
        destRect.w = scaledWidth;
        destRect.h = scaledHeight;
        
        return destRect;
    }

    SDL_Rect createVisualCenteredRect(int width, int height, ElementType type) {
        // Start with normal centered positioning
        SDL_Rect rect = createCenteredScaledRect(width, height, type);
        
        // Apply visual centering adjustments for specific element types
        if (type == ElementType::SPRITES) {
            // Digimon sprites typically have padding at top - adjust upward
            float scaleFactor = getScalingFactor(type);
            rect.y -= static_cast<int>(5 * scaleFactor); // Scales with sprite size
        }
        // Other element types (ENVIRONMENTS, UI, TEXT, etc.) use default centering
        
        return rect;
    }

} // namespace ScalingUtils
