// File: include/states/AdventureState.h
#pragma once

#include "states/GameState.h"       // Base class
#include "graphics/Animation.h"     // Animation definition
#include "../entities/Digimon.h"    // Include DigimonType enum
#include <SDL.h>                    // SDL types (SDL_Texture*, Uint32 etc.)
#include <vector>                   // Standard library container
#include <cmath>                    // Standard library math functions
#include <cstdint>                  // Standard library integer types
#include <map>                      // Standard library map container
#include <cstddef>                  // For size_t type

// Forward declarations
class Game;
class InputManager; // <<< ADDED
class PlayerData;   // <<< ADDED
class PCDisplay;    // <<< ADDED

// Enums specific to AdventureState logic
enum PlayerState { STATE_IDLE, STATE_WALKING };

class AdventureState : public GameState {
public:
    // Constructor & Destructor
    AdventureState(Game* game); // Declaration OK
    ~AdventureState() override; // Declaration OK

    // Core state functions override (with NEW signatures)
    void handle_input(InputManager& inputManager, PlayerData* playerData) override; // <<< MODIFIED
    void update(float delta_time, PlayerData* playerData) override;                // <<< MODIFIED
    void render(PCDisplay& display) override;                                     // <<< MODIFIED

private:
    // --- Data Members ---
    // (Remain unchanged)
    std::map<DigimonType, Animation> idleAnimations_;
    std::map<DigimonType, Animation> walkAnimations_;
    SDL_Texture* bgTexture0_;
    SDL_Texture* bgTexture1_;
    SDL_Texture* bgTexture2_;
    DigimonType current_digimon_;
    PlayerState current_state_;
    Animation* active_anim_;
    size_t current_anim_frame_idx_;
    float current_frame_elapsed_time_;
    int queued_steps_;
    float bg_scroll_offset_0_;
    float bg_scroll_offset_1_;
    float bg_scroll_offset_2_;

    // --- Constants ---
    // (Remain unchanged)
    const int MAX_QUEUED_STEPS = 2;
    const float SCROLL_SPEED_0 = 3.0f * 60.0f;
    const float SCROLL_SPEED_1 = 1.0f * 60.0f;
    const float SCROLL_SPEED_2 = 0.5f * 60.0f;
    const int WINDOW_WIDTH = 466;
    const int WINDOW_HEIGHT = 466;

    // --- Private Helper Methods ---
    // (Remain unchanged)
    void setActiveAnimation();
    void initializeAnimations();

}; // End of AdventureState class definition