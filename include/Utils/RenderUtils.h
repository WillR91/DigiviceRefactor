#pragma once

#include <SDL.h>

namespace RenderUtils {

    /**
     * Creates a destination rectangle without any manual scaling.
     * SDL logical scaling handles all scaling automatically.
     * 
     * @param srcRect The source rectangle from the sprite sheet (unmodified)
     * @param x The x-coordinate for rendering on screen
     * @param y The y-coordinate for rendering on screen
     * @return SDL_Rect The destination rectangle (no manual scaling applied)
     */
    inline SDL_Rect ScaleDestRect(const SDL_Rect& srcRect, int x, int y, float optionalOverrideScale = 0.0f) {
        // Note: optionalOverrideScale is ignored since SDL logical scaling handles all scaling
        SDL_Rect destRect;
        destRect.x = x;
        destRect.y = y;
        destRect.w = srcRect.w;  // Use original dimensions, SDL will scale automatically
        destRect.h = srcRect.h;
        
        return destRect;
    }

    /**
     * Creates a destination rectangle with specified position and dimensions.
     * SDL logical scaling handles all scaling automatically.
     * 
     * @param x The x-coordinate for rendering on screen
     * @param y The y-coordinate for rendering on screen
     * @param w The width
     * @param h The height
     * @return SDL_Rect The destination rectangle (no manual scaling applied)
     */
    inline SDL_Rect CreateScaledRect(int x, int y, int w, int h, float optionalOverrideScale = 0.0f) {
        // Note: optionalOverrideScale is ignored since SDL logical scaling handles all scaling
        SDL_Rect destRect;
        destRect.x = x;
        destRect.y = y;
        destRect.w = w;  // Use provided dimensions, SDL will scale automatically
        destRect.h = h;
        
        return destRect;
    }

} // namespace RenderUtils
