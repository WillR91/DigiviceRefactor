// File: include/states/TransitionState.h
#pragma once

#include "states/GameState.h"
#include <SDL.h>
#include <string>
#include "vendor/nlohmann/json.hpp" // <<<< INCLUDE FULL HEADER

// Forward declarations
class Game;
class InputManager;
class PlayerData;
class PCDisplay;

// Enum to define different transition effect types
enum class TransitionEffectType {
    BORDER_WIPE,    // Existing border wipe effect
    FADE_TO_COLOR,  // Fade the screen to a solid color (e.g., black)
    FADE_FROM_COLOR // Fade from a solid color to reveal the underlying screen
};

class TransitionState : public GameState {
public:
    // --- Constructors ---
    // Constructor for BORDER_WIPE
    // Uses const nlohmann::json& which is fine with a forward declaration
    TransitionState(Game* game, float duration, SDL_Texture* borderTexture, const nlohmann::json& transitionData);

    // Constructor for FADE_TO_COLOR and FADE_FROM_COLOR
    TransitionState(Game* game, float duration, TransitionEffectType effectType, SDL_Color fadeColor = {0, 0, 0, 255});

    ~TransitionState() override;

    // Lifecycle methods
    void enter() override;
    void exit() override {}; // No specific exit logic needed for now

    // Core state functions
    void handle_input(InputManager& inputManager, PlayerData* playerData) override;
    void update(float delta_time, PlayerData* playerData) override;
    void render(PCDisplay& display) override;
    StateType getType() const override;

    TransitionEffectType getEffectType() const; // <<< --- ADDED DECLARATION --- >>>

private:
    // --- Data Members ---
    float duration_;
    float timer_;
    TransitionEffectType effectType_;

    // For BORDER_WIPE
    SDL_Texture* borderAtlasTexture_; 
    SDL_Rect borderTopSrcRect_;
    SDL_Rect borderBottomSrcRect_;
    SDL_Rect borderLeftSrcRect_;
    SDL_Rect borderRightSrcRect_;

    // For FADE effects
    SDL_Color fadeColor_;            

    // Common
    bool transitionComplete_;        

    // --- Private Helper Methods ---
    // Only relevant for BORDER_WIPE
    // Uses const nlohmann::json& which is fine with a forward declaration
    bool loadBorderRectsFromJson(const nlohmann::json& transitionData); 

}; // End TransitionState class