// File: include/States/AdventureState.h
#pragma once

// --- Includes with Correct Paths ---
#include <States/GameState.h>   // Include base class
#include <Graphics/Animation.h> // Include Animation definition (defines SpriteFrame too)
#include <Utils/GameConstants.h>  // Include Constants
// ---------------------------------

#include <SDL.h>      // For SDL types (like Uint32 used below) - OK
#include <vector>     // Standard - OK
#include <cmath>      // Standard - OK
#include <limits>     // Standard - OK
#include <cstdint>    // Standard - OK


// Forward declare Game because constructor takes Game*
class Game;

// --- Game Enums ---
enum PlayerState { STATE_IDLE, STATE_WALKING };
enum DigimonType { DIGI_AGUMON, DIGI_GABUMON, DIGI_BIYOMON, DIGI_GATOMON, DIGI_GOMAMON, DIGI_PALMON, DIGI_TENTOMON, DIGI_PATAMON, DIGI_COUNT };

class AdventureState : public GameState {
public:
    AdventureState(Game* game);
    ~AdventureState() override;

    void handle_input() override;
    void update(float delta_time) override;
    void render() override;

private:
    // Helper
    void drawClippedTile(int dest_x_unclipped, const uint16_t* tile_data,
                         int layer_tile_width, int layer_tile_height);

    // --- Constants ---
    const int MAX_QUEUED_STEPS = 2;
    const float SCROLL_SPEED_0 = 3.0f;
    const float SCROLL_SPEED_1 = 1.0f;
    const float SCROLL_SPEED_2 = 0.5f;
    const int WINDOW_WIDTH = 466; // Consider moving to Constants
    const int WINDOW_HEIGHT = 466; // Consider moving to Constants


    // --- Sprite Frames (DECLARE ALL MEMBERS) --- // <-- Declarations are needed here
    SpriteFrame agumon_idle_0_sf, agumon_idle_1_sf, agumon_walk_0_sf, agumon_walk_1_sf;
    SpriteFrame gabumon_idle_0_sf, gabumon_idle_1_sf, gabumon_walk_0_sf, gabumon_walk_1_sf;
    SpriteFrame biyomon_idle_0_sf, biyomon_idle_1_sf, biyomon_walk_0_sf, biyomon_walk_1_sf;
    SpriteFrame gatomon_idle_0_sf, gatomon_idle_1_sf, gatomon_walk_0_sf, gatomon_walk_1_sf;
    SpriteFrame gomamon_idle_0_sf, gomamon_idle_1_sf, gomamon_walk_0_sf, gomamon_walk_1_sf;
    SpriteFrame palmon_idle_0_sf, palmon_idle_1_sf, palmon_walk_0_sf, palmon_walk_1_sf;
    SpriteFrame tentomon_idle_0_sf, tentomon_idle_1_sf, tentomon_walk_0_sf, tentomon_walk_1_sf;
    SpriteFrame patamon_idle_0_sf, patamon_idle_1_sf, patamon_walk_0_sf, patamon_walk_1_sf;
    // --- END Sprite Frames ---


    // --- Animations (DECLARE ALL MEMBERS) --- // <-- Declarations are needed here
    Animation agumon_idle_anim, agumon_walk_anim;
    Animation gabumon_idle_anim, gabumon_walk_anim;
    Animation biyomon_idle_anim, biyomon_walk_anim;
    Animation gatomon_idle_anim, gatomon_walk_anim;
    Animation gomamon_idle_anim, gomamon_walk_anim;
    Animation palmon_idle_anim, palmon_walk_anim;
    Animation tentomon_idle_anim, tentomon_walk_anim;
    Animation patamon_idle_anim, patamon_walk_anim;
    // --- END Animations ---

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
    Animation* active_anim = nullptr;
    int current_anim_frame_idx = 0;
    Uint32 last_anim_update_time = 0;
    int queued_steps = 0;
    bool character_changed_this_frame = false;
    bool animation_needs_reset = false;
};