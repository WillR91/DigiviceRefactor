#pragma once

#include <SDL_rect.h>

/**
 * ScalingUtils - Utility functions for applying granular scaling from config
 * 
 * This system provides helper functions to apply appropriate scaling factors
 * based on element type (sprites, UI, text, environments, effects) from
 * the display.scaling config values.
 */
namespace ScalingUtils {

    /**
     * Element types that can have different scaling factors
     */
    enum class ElementType {
        SPRITES,      // Digimon, character sprites
        UI,           // Buttons, panels, UI elements  
        TEXT,         // All text rendering (handled by TextRenderer)
        ENVIRONMENTS, // Background layers, environments
        EFFECTS       // Visual effects, particles
    };

    /**
     * Initialize scaling system by loading config values
     * Should be called during game initialization
     */
    void initialize();

    /**
     * Update scaling values from config
     * Should be called when config changes
     */
    void updateFromConfig();

    /**
     * Get the scaling factor for a specific element type
     * @param type The type of element to get scaling for
     * @return The scaling factor from config (default 1.0f if not found)
     */
    float getScalingFactor(ElementType type);

    /**
     * Apply scaling to a destination rectangle for sprite rendering
     * @param srcRect The source rectangle from sprite sheet
     * @param x The x-coordinate for rendering on screen
     * @param y The y-coordinate for rendering on screen
     * @param type The element type (defaults to SPRITES)
     * @return SDL_Rect The destination rectangle with appropriate scaling applied
     */
    SDL_Rect createScaledDestRect(const SDL_Rect& srcRect, int x, int y, ElementType type = ElementType::SPRITES);

    /**
     * Apply scaling to dimensions for a specific element type
     * @param width Original width
     * @param height Original height
     * @param type The element type
     * @return SDL_Point with scaled width and height
     */
    SDL_Point applyScaling(int width, int height, ElementType type);

    /**
     * Apply scaling to a single dimension
     * @param value Original value
     * @param type The element type
     * @return Scaled value
     */
    int applyScaling(int value, ElementType type);

    /**
     * Apply scaling to a float dimension
     * @param value Original value
     * @param type The element type
     * @return Scaled value
     */
    float applyScaling(float value, ElementType type);    /**
     * Create a scaled rectangle with custom dimensions
     * @param x The x-coordinate for rendering
     * @param y The y-coordinate for rendering  
     * @param width The width
     * @param height The height
     * @param type The element type (defaults to SPRITES)
     * @return SDL_Rect The destination rectangle with appropriate scaling applied
     */
    SDL_Rect createScaledRect(int x, int y, int width, int height, ElementType type = ElementType::SPRITES);

    /**
     * Create a scaled rectangle centered on screen
     * @param width The original width
     * @param height The original height
     * @param type The element type (defaults to SPRITES)
     * @return SDL_Rect The destination rectangle scaled and centered on screen
     */
    SDL_Rect createCenteredScaledRect(int width, int height, ElementType type = ElementType::SPRITES);

} // namespace ScalingUtils