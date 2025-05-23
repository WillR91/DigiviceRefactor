#pragma once
#include "utils/ConfigManager.h"
#include <SDL_log.h>

namespace Digivice {

class GraphicsConstants {
public:
    // Update scale from config - call during initialization and when config changes
    static void updateFromConfig() {
        if (ConfigManager::isInitialized()) {
            float newScale = ConfigManager::getValue<float>("graphics.assetScale", 1.0f);
            if (newScale != ASSET_SCALE) {
                ASSET_SCALE = newScale;
                SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "GraphicsConstants: Asset scale updated to %.2f", ASSET_SCALE);
            }
        }
    }

    // Apply scale to a dimension
    static int scaleSize(int baseSize) {
        return static_cast<int>(baseSize * ASSET_SCALE);
    }

    // Get current scale factor
    static float getAssetScale() {
        return ASSET_SCALE;
    }

private:
    static inline float ASSET_SCALE = 1.0f;
};

} // namespace Digivice
