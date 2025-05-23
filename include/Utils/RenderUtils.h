#pragma once

#include "Utils/GameConstants.h"
#include "graphics/GraphicsConstants.h"
#include <SDL.h>

namespace RenderUtils {

    /**
     * Scales a destination rectangle by the global SPRITE_SCALE_FACTOR.
     * Use this when rendering sprites that are stored at 1x scale.
     * 
     * @param srcRect The source rectangle from the sprite sheet (unmodified)
     * @param x The x-coordinate for rendering on screen
     * @param y The y-coordinate for rendering on screen
     * @param optionalOverrideScale Optional parameter to override the global scale factor
     * @return SDL_Rect The scaled destination rectangle
     */    inline SDL_Rect ScaleDestRect(const SDL_Rect& srcRect, int x, int y, float optionalOverrideScale = 0.0f) {
        float scale = (optionalOverrideScale > 0.0f) ? optionalOverrideScale : Digivice::GraphicsConstants::getAssetScale();
        
        SDL_Rect destRect;
        destRect.x = x;
        destRect.y = y;
        destRect.w = static_cast<int>(srcRect.w * scale);
        destRect.h = static_cast<int>(srcRect.h * scale);
        
        return destRect;
    }

    /**
     * Creates a scaled destination rectangle with specified position and dimensions.
     * 
     * @param x The x-coordinate for rendering on screen
     * @param y The y-coordinate for rendering on screen
     * @param w The unscaled width
     * @param h The unscaled height
     * @param optionalOverrideScale Optional parameter to override the global scale factor
     * @return SDL_Rect The scaled destination rectangle
     */    inline SDL_Rect CreateScaledRect(int x, int y, int w, int h, float optionalOverrideScale = 0.0f) {
        float scale = (optionalOverrideScale > 0.0f) ? optionalOverrideScale : Digivice::GraphicsConstants::getAssetScale();
        
        SDL_Rect destRect;
        destRect.x = x;
        destRect.y = y;
        destRect.w = static_cast<int>(w * scale);
        destRect.h = static_cast<int>(h * scale);
        
        return destRect;
    }

} // namespace RenderUtils
