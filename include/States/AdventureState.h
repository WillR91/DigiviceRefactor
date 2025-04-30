// File: include/states/AdventureState.h
#pragma once

// --- Includes with Correct Paths ---
#include "states/GameState.h"       // Correct path to base class
#include "graphics/Animation.h"     // Correct path to Animation struct/class
// #include <Utils/GameConstants.h> // Assuming you have this file/path

// --- Corrected SDL Include ---
#include <SDL.h> // <<< CORRECTED SDL Include >>>

#include <vector>     // Standard
#include <cmath>      // Standard
#include <limits>     // Standard
#include <cstdint>    // Standard
#include <map>        // Needed for animation maps from Phase 2

// Forward declare Game because constructor takes Game*
class Game;

// --- Game Enums --- (Keep these)
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
    // <<< PHASE 2 Refactoring - Remove drawClippedTile for raw pixels >>>
    // void drawClippedTile(int dest_x_unclipped, const uint16_t* tile_data,
    //                      int layer_tile_width, int layer_tile_height);

    // <<< PHASE 2 Refactoring - Data comes from AssetManager / Loaded Animation Data >>>
    // --- REMOVE OLD DATA MEMBERS ---
    // Remove all SpriteFrame member declarations (agumon_idle_0_sf, etc.)
    // Remove all Animation member declarations (agumon_idle_anim, etc.)
    // Remove all const uint16_t* bg_data_ pointers

    // --- ADD NEW DATA MEMBERS from Phase 2 ---
    std::map<DigimonType, Animation> idleAnimations_;
    std::map<DigimonType, Animation> walkAnimations_;
    SDL_Texture* bgTexture0_ = nullptr;
    SDL_Texture* bgTexture1_ = nullptr;
    SDL_Texture* bgTexture2_ = nullptr;
    // --- END NEW DATA ---


    // --- KEEP STATE VARIABLES ---
    DigimonType current_digimon_ = DIGI_AGUMON;
    PlayerState current_state_ = STATE_IDLE;
    Animation* active_anim_ = nullptr; // Pointer to the *current* active animation
    int current_anim_frame_idx_ = 0;
    Uint32 last_anim_update_time_ = 0; // Needs SDL.h
    int queued_steps_ = 0;


    // --- Background Scroll State (Keep these) ---
    float bg_scroll_offset_0_ = 0.0f;
    float bg_scroll_offset_1_ = 0.0f;
    float bg_scroll_offset_2_ = 0.0f;

    // --- Constants (Keep or move later) ---
    const int MAX_QUEUED_STEPS = 2;
    const float SCROLL_SPEED_0 = 3.0f;
    const float SCROLL_SPEED_1 = 1.0f;
    const float SCROLL_SPEED_2 = 0.5f;
    const int WINDOW_WIDTH = 466;
    const int WINDOW_HEIGHT = 466;

     // --- Helper Methods (Needed for Phase 2 refactoring) ---
     void setActiveAnimation();
     void initializeAnimations();
};