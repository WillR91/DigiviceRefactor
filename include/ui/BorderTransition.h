// File: include/ui/BorderTransition.h
#pragma once

#include <SDL.h>
#include <memory>

// Forward declarations
class Game;
class AssetManager;
class PCDisplay;

/**
 * @brief BorderTransition manages a 4-border animation system where borders slide in from screen edges
 * 
 * The BorderTransition creates a cinematic effect where 4 border elements slide in from the screen edges
 * (top, bottom, left, right) to frame the content in a central square viewport. This is used when
 * transitioning to MenuState to create an elegant framing effect.
 */
class BorderTransition {
public:
    enum class AnimationState {
        BORDERS_OUT,        // Borders are off-screen
        BORDERS_ANIMATING,  // Borders are sliding in
        BORDERS_IN          // Borders are in position
    };    /**
     * @brief Construct a new BorderTransition
     * @param game Pointer to the Game instance for asset management
     * @param animationDuration Duration of the slide-in animation in seconds
     * @param inwardDistance How far inward the borders should move from screen edges (in pixels)
     */
    BorderTransition(Game* game, float animationDuration = 0.8f, int inwardDistance = 93);
    ~BorderTransition();

    /**
     * @brief Start the border animation
     */
    void startAnimation();

    /**
     * @brief Update the border animation
     * @param deltaTime Time elapsed since last update
     */
    void update(float deltaTime);

    /**
     * @brief Render the borders
     * @param display Reference to the display system
     */
    void render(PCDisplay& display);

    /**
     * @brief Check if the animation is complete
     * @return true if borders are fully in position
     */
    bool isAnimationComplete() const;

    /**
     * @brief Get the current animation state
     * @return Current AnimationState
     */
    AnimationState getAnimationState() const;

    /**
     * @brief Reset the animation to start position
     */
    void reset();

private:
    // Core data
    Game* game_;
    float animationDuration_;
    float animationTimer_;
    AnimationState currentState_;
    int inwardDistance_; // How far inward borders move from screen edges

    // Border textures
    SDL_Texture* borderTop_;
    SDL_Texture* borderBottom_;
    SDL_Texture* borderLeft_;
    SDL_Texture* borderRight_;

    // Border dimensions (loaded from textures)
    int topHeight_;
    int bottomHeight_;
    int leftWidth_;
    int rightWidth_;

    // Animation helper methods
    void loadBorderTextures();
    float easeInOutCubic(float t) const;
    void calculateBorderPositions(SDL_Rect& topRect, SDL_Rect& bottomRect, 
                                  SDL_Rect& leftRect, SDL_Rect& rightRect) const;
};
