// File: include/states/AdventureState.h
#pragma once

#include "states/GameState.h"       // Base class
#include "graphics/Animation.h"     // Animation definition
#include <SDL.h>                    // SDL types (SDL_Texture*, Uint32 etc.)
#include <vector>                   // Standard library container
#include <cmath>                    // Standard library math functions
#include <cstdint>                  // Standard library integer types
#include <map>                      // Standard library map container
#include <cstddef>                  // For size_t type
// Removed forward declaration for TransitionState as it's no longer directly pushed

// Forward declaration for Game pointer
class Game;

// Enums specific to AdventureState logic
enum PlayerState { STATE_IDLE, STATE_WALKING };
enum DigimonType {
    DIGI_AGUMON, DIGI_GABUMON, DIGI_BIYOMON, DIGI_GATOMON,
    DIGI_GOMAMON, DIGI_PALMON, DIGI_TENTOMON, DIGI_PATAMON,
    DIGI_COUNT // Helper to get the number of digimon types
};

class AdventureState : public GameState {
public:
    // Constructor & Destructor
    AdventureState(Game* game);
    ~AdventureState() override;

    // Core state functions override
    void handle_input() override;
    void update(float delta_time) override;
    void render() override;

private:
    // --- Data Members ---

    // Animation Storage
    std::map<DigimonType, Animation> idleAnimations_;
    std::map<DigimonType, Animation> walkAnimations_;
    // Add maps for other animations (attack, etc.) here later

    // Background Textures (Non-owning pointers)
    SDL_Texture* bgTexture0_ = nullptr; // Foreground
    SDL_Texture* bgTexture1_ = nullptr; // Middleground
    SDL_Texture* bgTexture2_ = nullptr; // Background

    // Current State Tracking
    DigimonType current_digimon_ = DIGI_AGUMON; // Currently selected partner
    PlayerState current_state_ = STATE_IDLE;    // Current player state (idle/walking)
    Animation* active_anim_ = nullptr;          // Pointer to the currently playing animation object
    size_t current_anim_frame_idx_ = 0;         // Index of the current frame within active_anim_
    float current_frame_elapsed_time_ = 0.0f;   // Time accumulator for current frame (seconds)
    int queued_steps_ = 0;                      // Steps waiting for walk animation cycles

    // Background Scrolling
    float bg_scroll_offset_0_ = 0.0f;
    float bg_scroll_offset_1_ = 0.0f;
    float bg_scroll_offset_2_ = 0.0f;

    // --- Constants --- (Consider moving to a separate constants file/namespace later)
    const int MAX_QUEUED_STEPS = 2;
    // Scroll speeds defined in Pixels Per Second
    const float SCROLL_SPEED_0 = 3.0f * 60.0f; // ~180 pixels/sec
    const float SCROLL_SPEED_1 = 1.0f * 60.0f; // ~60 pixels/sec
    const float SCROLL_SPEED_2 = 0.5f * 60.0f; // ~30 pixels/sec
    // Window dimensions (Temporary - get from Game/Display later)
    // NOTE: These should ideally come from the display/game config
    const int WINDOW_WIDTH = 466;
    const int WINDOW_HEIGHT = 466;
    // --- End Constants ---


    // --- Private Helper Methods ---
    void setActiveAnimation();      // Sets active_anim_ based on state/digimon
    void initializeAnimations();    // Loads animation definitions (called by constructor)

}; // End of AdventureState class definition