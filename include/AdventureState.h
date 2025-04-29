#pragma once

#include "GameState.h"
#include "Animation.h" // Include your Animation header
#include <SDL.h>  // For SDL types like Uint32, SDL_Event
#include <vector>
#include <cmath> // For std::fmod, std::round
#include <limits>
#include <cstdint> // For uint16_t

// --- Game Enums (Moved here from main.cpp) ---
enum PlayerState { STATE_IDLE, STATE_WALKING };
enum DigimonType { DIGI_AGUMON, DIGI_GABUMON, DIGI_BIYOMON, DIGI_GATOMON, DIGI_GOMAMON, DIGI_PALMON, DIGI_TENTOMON, DIGI_PATAMON, DIGI_COUNT };

class AdventureState : public GameState {
public:
    AdventureState(Game* game); // Constructor needs Game pointer
    ~AdventureState() override; // Destructor (optional override if needed)

    void handle_input() override;
    void update(float delta_time) override; // delta_time is now passed
    void render() override;

private:
    // Helper to draw background tiles (copied logic)
    void drawClippedTile(int dest_x_unclipped, const uint16_t* tile_data,
                         int layer_tile_width, int layer_tile_height);

    // --- Constants (Moved from main.cpp) ---
    const int MAX_QUEUED_STEPS = 2;

    // Background constants - Ensure these match your asset headers
    // Assuming asset headers define CASTLEBACKGROUND*_WIDTH/HEIGHT correctly
    const int TILE_WIDTH_0 = CASTLEBACKGROUND0_WIDTH;
    const int TILE_HEIGHT_0 = CASTLEBACKGROUND0_HEIGHT;
    const int EFFECTIVE_BG_WIDTH_0 = 947; // Adjust if needed (TILE_WIDTH_0 * 2/3)
    const float effectiveW_float_0 = static_cast<float>(EFFECTIVE_BG_WIDTH_0);
    const float SCROLL_SPEED_0 = 3.0f;

    const int TILE_WIDTH_1 = CASTLEBACKGROUND1_WIDTH;
    const int TILE_HEIGHT_1 = CASTLEBACKGROUND1_HEIGHT;
    const int EFFECTIVE_BG_WIDTH_1 = 947; // Adjust if needed
    const float effectiveW_float_1 = static_cast<float>(EFFECTIVE_BG_WIDTH_1);
    const float SCROLL_SPEED_1 = 1.0f;

    const int TILE_WIDTH_2 = CASTLEBACKGROUND2_WIDTH;
    const int TILE_HEIGHT_2 = CASTLEBACKGROUND2_HEIGHT;
    const int EFFECTIVE_BG_WIDTH_2 = 947; // Adjust if needed
    const float effectiveW_float_2 = static_cast<float>(EFFECTIVE_BG_WIDTH_2);
    const float SCROLL_SPEED_2 = 0.5f;

    // Game Dimensions needed for drawing calculations
    const int WINDOW_WIDTH = 466;  // Consider getting these from Game obj later
    const int WINDOW_HEIGHT = 466;


    // --- Sprite Frames (Defined in constructor or loaded) ---
    SpriteFrame agumon_idle_0_sf, agumon_idle_1_sf, agumon_walk_0_sf, agumon_walk_1_sf;
    SpriteFrame gabumon_idle_0_sf, gabumon_idle_1_sf, gabumon_walk_0_sf, gabumon_walk_1_sf;
    // ... (Declare all other SpriteFrames) ...
    SpriteFrame patamon_idle_0_sf, patamon_idle_1_sf, patamon_walk_0_sf, patamon_walk_1_sf;


    // --- Animations (Defined in constructor or loaded) ---
    Animation agumon_idle_anim, agumon_walk_anim;
    Animation gabumon_idle_anim, gabumon_walk_anim;
    // ... (Declare all other Animations) ...
    Animation patamon_idle_anim, patamon_walk_anim;

    // --- Background Data Pointers ---
    const uint16_t* bg_data_0 = nullptr;
    const uint16_t* bg_data_1 = nullptr;
    const uint16_t* bg_data_2 = nullptr;

    // --- Background Scroll State ---
    float bg_scroll_offset_0 = 0.0f;
    float bg_scroll_offset_1 = 0.0f;
    float bg_scroll_offset_2 = 0.0f;

    // --- Game State Variables ---
    DigimonType current_digimon = DIGI_AGUMON;
    PlayerState current_state = STATE_IDLE;
    Animation* active_anim = nullptr; // Pointer to the current animation
    int current_anim_frame_idx = 0;
    Uint32 last_anim_update_time = 0;
    int queued_steps = 0;
    bool character_changed_this_frame = false; // Flag for animation reset
    bool animation_needs_reset = false; // Helper flag
};