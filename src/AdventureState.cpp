#include "AdventureState.h"
#include "Game.h"           // Include Game to access display etc.
#include "platform/pc/pc_display.h" // Include display definition
#include <SDL2/SDL_log.h>
#include <stdexcept>       // For runtime_error in constructor checks

// --- Include ALL Asset Headers Needed ---
#include "assets/Agumon_Idle_0.h"
#include "assets/Agumon_Idle_1.h"
#include "assets/Agumon_Walk_0.h"
#include "assets/Agumon_Walk_1.h"
#include "assets/Gabumon_Idle_0.h"
#include "assets/Gabumon_Idle_1.h"
#include "assets/Gabumon_Walk_0.h"
#include "assets/Gabumon_Walk_1.h"
#include "assets/Biyomon_Idle_0.h"
#include "assets/Biyomon_Idle_1.h"
#include "assets/Biyomon_Walk_0.h"
#include "assets/Biyomon_Walk_1.h"
#include "assets/Gatomon_Idle_0.h"
#include "assets/Gatomon_Idle_1.h"
#include "assets/Gatomon_Walk_0.h"
#include "assets/Gatomon_Walk_1.h"
#include "assets/Gomamon_Idle_0.h"
#include "assets/Gomamon_Idle_1.h"
#include "assets/Gomamon_Walk_0.h"
#include "assets/Gomamon_Walk_1.h"
#include "assets/Palmon_Idle_0.h"
#include "assets/Palmon_Idle_1.h"
#include "assets/Palmon_Walk_0.h"
#include "assets/Palmon_Walk_1.h"
#include "assets/Tentomon_Idle_0.h"
#include "assets/Tentomon_Idle_1.h"
#include "assets/Tentomon_Walk_0.h"
#include "assets/Tentomon_Walk_1.h"
#include "assets/Patamon_Idle_0.h"
#include "assets/Patamon_Idle_1.h"
#include "assets/Patamon_Walk_0.h"
#include "assets/Patamon_Walk_1.h"
#include "assets/castlebackground0.h"
#include "assets/castlebackground1.h"
#include "assets/castlebackground2.h"


// Constructor: Initialize variables, define sprites/animations
AdventureState::AdventureState(Game* game) :
    current_digimon(DIGI_AGUMON),
    current_state(STATE_IDLE),
    active_anim(nullptr),
    current_anim_frame_idx(0),
    last_anim_update_time(0),
    queued_steps(0),
    character_changed_this_frame(false),
    animation_needs_reset(true) // Start needing reset to set initial anim
{
    this->game_ptr = game; // Store the pointer to the Game object

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState Constructor: Initializing...");
    SDL_Log("Expected Tile Dimensions (WxH) for all layers: %d x %d", 1421, 474); // Log expected values
    SDL_Log("Actual Dimensions Layer 0 (WxH): %d x %d", TILE_WIDTH_0, TILE_HEIGHT_0);
    SDL_Log("Actual Dimensions Layer 1 (WxH): %d x %d", TILE_WIDTH_1, TILE_HEIGHT_1);
    SDL_Log("Actual Dimensions Layer 2 (WxH): %d x %d", TILE_WIDTH_2, TILE_HEIGHT_2);
    SDL_Log("Effective Scroll Widths (0, 1, 2): %d, %d, %d", EFFECTIVE_BG_WIDTH_0, EFFECTIVE_BG_WIDTH_1, EFFECTIVE_BG_WIDTH_2);
    SDL_Log("Scroll Speeds (0, 1, 2): %.2f, %.2f, %.2f", SCROLL_SPEED_0, SCROLL_SPEED_1, SCROLL_SPEED_2);

    // --- Sanity Checks ---
     auto checkLayer = [](int layerNum, int width, int height, int effectiveWidth) {
        if (width <= 0 || height <= 0) {
             SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error: Invalid tile dimensions for Layer %d from header (W:%d H:%d)", layerNum, width, height);
             return false;
        }
        if (effectiveWidth <= 0 || effectiveWidth > width) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error: Effective BG Width (%d) is invalid for Layer %d Tile Width (%d)", effectiveWidth, layerNum, width);
             float actual_effective_w = static_cast<float>(width) * (2.0f / 3.0f);
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Consider using effective width: %d based on actual tile width for Layer %d", static_cast<int>(std::round(actual_effective_w)), layerNum);
            return false;
        }
         if (width != 1421 || height != 474) {
              SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Warning: Expected dims (1421x474) differ from header dims for Layer %d (%dx%d)!", layerNum, width, height);
         }
         return true;
    };

    bool config_ok = true;
    config_ok &= checkLayer(0, TILE_WIDTH_0, TILE_HEIGHT_0, EFFECTIVE_BG_WIDTH_0);
    config_ok &= checkLayer(1, TILE_WIDTH_1, TILE_HEIGHT_1, EFFECTIVE_BG_WIDTH_1);
    config_ok &= checkLayer(2, TILE_WIDTH_2, TILE_HEIGHT_2, EFFECTIVE_BG_WIDTH_2);

     if (WINDOW_WIDTH <= 0 || WINDOW_HEIGHT <= 0) {
          SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error: Invalid window dimensions (W:%d H:%d)", WINDOW_WIDTH, WINDOW_HEIGHT);
          config_ok = false;
     }
     if (!config_ok) {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Configuration errors found in AdventureState. Aborting.");
         // Throwing an exception is one way to signal failure from constructor
         throw std::runtime_error("Configuration errors in AdventureState.");
     }
     // --- End Sanity Checks ---


    // --- Define ALL Sprite Frames --- (Copied from old main)
    agumon_idle_0_sf = {AGUMON_IDLE_0_WIDTH, AGUMON_IDLE_0_HEIGHT, Agumon_Idle_0_data};
    agumon_idle_1_sf = {AGUMON_IDLE_1_WIDTH, AGUMON_IDLE_1_HEIGHT, Agumon_Idle_1_data};
    agumon_walk_0_sf = {AGUMON_WALK_0_WIDTH, AGUMON_WALK_0_HEIGHT, Agumon_Walk_0_data};
    agumon_walk_1_sf = {AGUMON_WALK_1_WIDTH, AGUMON_WALK_1_HEIGHT, Agumon_Walk_1_data};
    gabumon_idle_0_sf = {GABUMON_IDLE_0_WIDTH, GABUMON_IDLE_0_HEIGHT, Gabumon_Idle_0_data};
    gabumon_idle_1_sf = {GABUMON_IDLE_1_WIDTH, GABUMON_IDLE_1_HEIGHT, Gabumon_Idle_1_data};
    gabumon_walk_0_sf = {GABUMON_WALK_0_WIDTH, GABUMON_WALK_0_HEIGHT, Gabumon_Walk_0_data};
    gabumon_walk_1_sf = {GABUMON_WALK_1_WIDTH, GABUMON_WALK_1_HEIGHT, Gabumon_Walk_1_data};
    biyomon_idle_0_sf = {BIYOMON_IDLE_0_WIDTH, BIYOMON_IDLE_0_HEIGHT, Biyomon_Idle_0_data};
    biyomon_idle_1_sf = {BIYOMON_IDLE_1_WIDTH, BIYOMON_IDLE_1_HEIGHT, Biyomon_Idle_1_data};
    biyomon_walk_0_sf = {BIYOMON_WALK_0_WIDTH, BIYOMON_WALK_0_HEIGHT, Biyomon_Walk_0_data};
    biyomon_walk_1_sf = {BIYOMON_WALK_1_WIDTH, BIYOMON_WALK_1_HEIGHT, Biyomon_Walk_1_data};
    gatomon_idle_0_sf = {GATOMON_IDLE_0_WIDTH, GATOMON_IDLE_0_HEIGHT, Gatomon_Idle_0_data};
    gatomon_idle_1_sf = {GATOMON_IDLE_1_WIDTH, GATOMON_IDLE_1_HEIGHT, Gatomon_Idle_1_data};
    gatomon_walk_0_sf = {GATOMON_WALK_0_WIDTH, GATOMON_WALK_0_HEIGHT, Gatomon_Walk_0_data};
    gatomon_walk_1_sf = {GATOMON_WALK_1_WIDTH, GATOMON_WALK_1_HEIGHT, Gatomon_Walk_1_data};
    gomamon_idle_0_sf = {GOMAMON_IDLE_0_WIDTH, GOMAMON_IDLE_0_HEIGHT, Gomamon_Idle_0_data};
    gomamon_idle_1_sf = {GOMAMON_IDLE_1_WIDTH, GOMAMON_IDLE_1_HEIGHT, Gomamon_Idle_1_data};
    gomamon_walk_0_sf = {GOMAMON_WALK_0_WIDTH, GOMAMON_WALK_0_HEIGHT, Gomamon_Walk_0_data};
    gomamon_walk_1_sf = {GOMAMON_WALK_1_WIDTH, GOMAMON_WALK_1_HEIGHT, Gomamon_Walk_1_data};
    palmon_idle_0_sf = {PALMON_IDLE_0_WIDTH, PALMON_IDLE_0_HEIGHT, Palmon_Idle_0_data};
    palmon_idle_1_sf = {PALMON_IDLE_1_WIDTH, PALMON_IDLE_1_HEIGHT, Palmon_Idle_1_data};
    palmon_walk_0_sf = {PALMON_WALK_0_WIDTH, PALMON_WALK_0_HEIGHT, Palmon_Walk_0_data};
    palmon_walk_1_sf = {PALMON_WALK_1_WIDTH, PALMON_WALK_1_HEIGHT, Palmon_Walk_1_data};
    tentomon_idle_0_sf = {TENTOMON_IDLE_0_WIDTH, TENTOMON_IDLE_0_HEIGHT, Tentomon_Idle_0_data};
    tentomon_idle_1_sf = {TENTOMON_IDLE_1_WIDTH, TENTOMON_IDLE_1_HEIGHT, Tentomon_Idle_1_data};
    tentomon_walk_0_sf = {TENTOMON_WALK_0_WIDTH, TENTOMON_WALK_0_HEIGHT, Tentomon_Walk_0_data};
    tentomon_walk_1_sf = {TENTOMON_WALK_1_WIDTH, TENTOMON_WALK_1_HEIGHT, Tentomon_Walk_1_data};
    patamon_idle_0_sf = {PATAMON_IDLE_0_WIDTH, PATAMON_IDLE_0_HEIGHT, Patamon_Idle_0_data};
    patamon_idle_1_sf = {PATAMON_IDLE_1_WIDTH, PATAMON_IDLE_1_HEIGHT, Patamon_Idle_1_data};
    patamon_walk_0_sf = {PATAMON_WALK_0_WIDTH, PATAMON_WALK_0_HEIGHT, Patamon_Walk_0_data};
    patamon_walk_1_sf = {PATAMON_WALK_1_WIDTH, PATAMON_WALK_1_HEIGHT, Patamon_Walk_1_data};

    // --- Define Animations for EACH Digimon --- (Copied from old main)
    agumon_idle_anim.addFrame(agumon_idle_0_sf, 1000); agumon_idle_anim.addFrame(agumon_idle_1_sf, 1000); agumon_idle_anim.loops = true;
    agumon_walk_anim.addFrame(agumon_walk_0_sf, 300); agumon_walk_anim.addFrame(agumon_walk_1_sf, 300); agumon_walk_anim.addFrame(agumon_walk_0_sf, 300); agumon_walk_anim.addFrame(agumon_walk_1_sf, 300); agumon_walk_anim.loops = false;
    gabumon_idle_anim.addFrame(gabumon_idle_0_sf, 1100); gabumon_idle_anim.addFrame(gabumon_idle_1_sf, 1100); gabumon_idle_anim.loops = true;
    gabumon_walk_anim.addFrame(gabumon_walk_0_sf, 320); gabumon_walk_anim.addFrame(gabumon_walk_1_sf, 320); gabumon_walk_anim.addFrame(gabumon_walk_0_sf, 320); gabumon_walk_anim.addFrame(gabumon_walk_1_sf, 320); gabumon_walk_anim.loops = false;
    biyomon_idle_anim.addFrame(biyomon_idle_0_sf, 960); biyomon_idle_anim.addFrame(biyomon_idle_1_sf, 960); biyomon_idle_anim.loops = true;
    biyomon_walk_anim.addFrame(biyomon_walk_0_sf, 280); biyomon_walk_anim.addFrame(biyomon_walk_1_sf, 280); biyomon_walk_anim.addFrame(biyomon_walk_0_sf, 280); biyomon_walk_anim.addFrame(biyomon_walk_1_sf, 280); biyomon_walk_anim.loops = false;
    gatomon_idle_anim.addFrame(gatomon_idle_0_sf, 1200); gatomon_idle_anim.addFrame(gatomon_idle_1_sf, 1200); gatomon_idle_anim.loops = true;
    gatomon_walk_anim.addFrame(gatomon_walk_0_sf, 340); gatomon_walk_anim.addFrame(gatomon_walk_1_sf, 340); gatomon_walk_anim.addFrame(gatomon_walk_0_sf, 340); gatomon_walk_anim.addFrame(gatomon_walk_1_sf, 340); gatomon_walk_anim.loops = false;
    gomamon_idle_anim.addFrame(gomamon_idle_0_sf, 1040); gomamon_idle_anim.addFrame(gomamon_idle_1_sf, 1040); gomamon_idle_anim.loops = true;
    gomamon_walk_anim.addFrame(gomamon_walk_0_sf, 310); gomamon_walk_anim.addFrame(gomamon_walk_1_sf, 310); gomamon_walk_anim.addFrame(gomamon_walk_0_sf, 310); gomamon_walk_anim.addFrame(gomamon_walk_1_sf, 310); gomamon_walk_anim.loops = false;
    palmon_idle_anim.addFrame(palmon_idle_0_sf, 1080); palmon_idle_anim.addFrame(palmon_idle_1_sf, 1080); palmon_idle_anim.loops = true;
    palmon_walk_anim.addFrame(palmon_walk_0_sf, 330); palmon_walk_anim.addFrame(palmon_walk_1_sf, 330); palmon_walk_anim.addFrame(palmon_walk_0_sf, 330); palmon_walk_anim.addFrame(palmon_walk_1_sf, 330); palmon_walk_anim.loops = false;
    tentomon_idle_anim.addFrame(tentomon_idle_0_sf, 920); tentomon_idle_anim.addFrame(tentomon_idle_1_sf, 920); tentomon_idle_anim.loops = true;
    tentomon_walk_anim.addFrame(tentomon_walk_0_sf, 290); tentomon_walk_anim.addFrame(tentomon_walk_1_sf, 290); tentomon_walk_anim.addFrame(tentomon_walk_0_sf, 290); tentomon_walk_anim.addFrame(tentomon_walk_1_sf, 290); tentomon_walk_anim.loops = false;
    patamon_idle_anim.addFrame(patamon_idle_0_sf, 1060); patamon_idle_anim.addFrame(patamon_idle_1_sf, 1060); patamon_idle_anim.loops = true;
    patamon_walk_anim.addFrame(patamon_walk_0_sf, 300); patamon_walk_anim.addFrame(patamon_walk_1_sf, 300); patamon_walk_anim.addFrame(patamon_walk_0_sf, 300); patamon_walk_anim.addFrame(patamon_walk_1_sf, 300); patamon_walk_anim.loops = false;

    // --- Background Data Pointers ---
    bg_data_0 = castlebackground0_data;
    bg_data_1 = castlebackground1_data;
    bg_data_2 = castlebackground2_data;

    // Check if pointers are valid
    if (!bg_data_0 || !bg_data_1 || !bg_data_2) {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Background data pointers are NULL after assignment!");
          throw std::runtime_error("Background data pointers are NULL.");
    }


    // Set initial animation based on starting state and digimon
     switch(current_digimon) { /* ... Set Idle Anim ... */
        case DIGI_AGUMON:   active_anim = &agumon_idle_anim; break;
        case DIGI_GABUMON:  active_anim = &gabumon_idle_anim; break;
        case DIGI_BIYOMON:  active_anim = &biyomon_idle_anim; break;
        case DIGI_GATOMON:  active_anim = &gatomon_idle_anim; break;
        case DIGI_GOMAMON:  active_anim = &gomamon_idle_anim; break;
        case DIGI_PALMON:   active_anim = &palmon_idle_anim; break;
        case DIGI_TENTOMON: active_anim = &tentomon_idle_anim; break;
        case DIGI_PATAMON:  active_anim = &patamon_idle_anim; break;
        default:            active_anim = &agumon_idle_anim; SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Defaulting to Agumon idle anim");
    }
    if (!active_anim) {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Initial active_anim is NULL!");
          throw std::runtime_error("Initial active_anim is NULL.");
    }
    last_anim_update_time = SDL_GetTicks(); // Set initial time
    animation_needs_reset = false; // We've just set it

     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState Initialized Successfully.");
}

AdventureState::~AdventureState() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState Destructor called.");
    // unique_ptrs for animations would handle cleanup automatically if used
    // No manual cleanup needed for member variables like ints, floats, enums
}


void AdventureState::handle_input() {
    // We don't need the SDL_PollEvent loop here anymore,
    // as Game class handles polling. We just need to check keys.
    // For now, we can use SDL_GetKeyboardState for simplicity.

    const Uint8* keystates = SDL_GetKeyboardState(NULL);

    // Use a flag to prevent multiple step increments per frame if key held
    static bool space_pressed_last_frame = false;
    if (keystates[SDL_SCANCODE_SPACE]) {
        if (!space_pressed_last_frame) {
             if (queued_steps < MAX_QUEUED_STEPS) { queued_steps++; }
        }
        space_pressed_last_frame = true;
    } else {
        space_pressed_last_frame = false;
    }

    // Check number keys for switching (Check only once per press)
    // This requires event polling, let's adapt slightly
    // We'll process events passed from Game later, for now this is simpler
    static bool numkey_pressed_last_frame[DIGI_COUNT] = {false}; // Track each num key
    for (int i = 0; i < DIGI_COUNT; ++i) {
        SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN;
        switch (i) {
            case 0: scancode = SDL_SCANCODE_1; break;
            case 1: scancode = SDL_SCANCODE_2; break;
            case 2: scancode = SDL_SCANCODE_3; break;
            case 3: scancode = SDL_SCANCODE_4; break;
            case 4: scancode = SDL_SCANCODE_5; break;
            case 5: scancode = SDL_SCANCODE_6; break;
            case 6: scancode = SDL_SCANCODE_7; break;
            case 7: scancode = SDL_SCANCODE_8; break;
        }

        if (scancode != SDL_SCANCODE_UNKNOWN && keystates[scancode]) {
            if (!numkey_pressed_last_frame[i]) {
                DigimonType selected_digi = static_cast<DigimonType>(i);
                if (selected_digi != current_digimon) {
                    current_digimon = selected_digi;
                    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Switched character to %d", current_digimon);
                    character_changed_this_frame = true; // Signal change for update()
                    current_state = STATE_IDLE; // Reset to idle on switch
                    queued_steps = 0; // Reset steps on switch
                }
            }
             numkey_pressed_last_frame[i] = true;
        } else {
             numkey_pressed_last_frame[i] = false;
        }
    }


    // Check Escape key to quit
    if (keystates[SDL_SCANCODE_ESCAPE]) {
        if (game_ptr) { game_ptr->quit_game(); }
    }
}


void AdventureState::update(float delta_time) {
    // Use SDL_GetTicks for timing consistency with original code for now
    Uint32 current_time = SDL_GetTicks();

    // --- Update Parallax Scrolling (ONLY when walking) ---
    if (current_state == STATE_WALKING) {
        // Update Layer 0 (Foreground) - Uses SCROLL_SPEED_0
        bg_scroll_offset_0 -= SCROLL_SPEED_0; // Speed is per-frame in original, adjust if needed for delta_time
        while (bg_scroll_offset_0 < 0.0f) { bg_scroll_offset_0 += effectiveW_float_0; }
        bg_scroll_offset_0 = std::fmod(bg_scroll_offset_0, effectiveW_float_0);

        // Update Layer 1 (Middleground) - Uses SCROLL_SPEED_1
        bg_scroll_offset_1 -= SCROLL_SPEED_1;
        while (bg_scroll_offset_1 < 0.0f) { bg_scroll_offset_1 += effectiveW_float_1; }
        bg_scroll_offset_1 = std::fmod(bg_scroll_offset_1, effectiveW_float_1);

        // Update Layer 2 (Background) - Uses SCROLL_SPEED_2
        bg_scroll_offset_2 -= SCROLL_SPEED_2;
        while (bg_scroll_offset_2 < 0.0f) { bg_scroll_offset_2 += effectiveW_float_2; }
        bg_scroll_offset_2 = std::fmod(bg_scroll_offset_2, effectiveW_float_2);
    }

    // --- State & Animation Selection / Update ---
    animation_needs_reset = character_changed_this_frame; // Reset if character just changed
    character_changed_this_frame = false; // Reset flag for next frame

    if (current_state == STATE_IDLE && queued_steps > 0) {
        current_state = STATE_WALKING;
        animation_needs_reset = true;
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "State changed to WALKING");
    }

    // --- Update Animation Pointer if needed ---
    if (animation_needs_reset) {
        if (current_state == STATE_IDLE) {
            switch(current_digimon) {
                case DIGI_AGUMON:   active_anim = &agumon_idle_anim; break;
                case DIGI_GABUMON:  active_anim = &gabumon_idle_anim; break;
                case DIGI_BIYOMON:  active_anim = &biyomon_idle_anim; break;
                case DIGI_GATOMON:  active_anim = &gatomon_idle_anim; break;
                case DIGI_GOMAMON:  active_anim = &gomamon_idle_anim; break;
                case DIGI_PALMON:   active_anim = &palmon_idle_anim; break;
                case DIGI_TENTOMON: active_anim = &tentomon_idle_anim; break;
                case DIGI_PATAMON:  active_anim = &patamon_idle_anim; break;
                default:            active_anim = &agumon_idle_anim;
            }
        } else { // STATE_WALKING
             switch(current_digimon) {
                case DIGI_AGUMON:   active_anim = &agumon_walk_anim; break;
                case DIGI_GABUMON:  active_anim = &gabumon_walk_anim; break;
                case DIGI_BIYOMON:  active_anim = &biyomon_walk_anim; break;
                case DIGI_GATOMON:  active_anim = &gatomon_walk_anim; break;
                case DIGI_GOMAMON:  active_anim = &gomamon_walk_anim; break;
                case DIGI_PALMON:   active_anim = &palmon_walk_anim; break;
                case DIGI_TENTOMON: active_anim = &tentomon_walk_anim; break;
                case DIGI_PATAMON:  active_anim = &patamon_walk_anim; break;
                default:            active_anim = &agumon_walk_anim;
            }
        }
        current_anim_frame_idx = 0; // Reset frame index
        last_anim_update_time = current_time; // Reset timer
        animation_needs_reset = false; // Handled the reset
         if (!active_anim) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Animation became NULL after reset!");}
         else {SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Animation reset. New anim has %zu frames.", active_anim->frames.size());}
    }


    // --- Animation Frame Logic ---
    bool animation_cycle_finished = false;
    if (active_anim && !active_anim->frames.empty() && !active_anim->frame_durations_ms.empty()) {
         // Bounds check before accessing vectors
         if (current_anim_frame_idx >= active_anim->frames.size() || current_anim_frame_idx >= active_anim->frame_durations_ms.size()) {
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Animation frame index %d out of bounds! Resetting.", current_anim_frame_idx);
             current_anim_frame_idx = 0;
             // Re-check if animation is still valid after potential reset
             if (active_anim->frames.empty() || active_anim->frame_durations_ms.empty()) {
                  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Animation has no frames/durations after reset!");
                  active_anim = nullptr; // Prevent further issues this frame
                  return; // Exit update early if anim is broken
             }
         }

        Uint32 current_frame_duration = active_anim->frame_durations_ms[current_anim_frame_idx];
        if (current_time >= last_anim_update_time + current_frame_duration) {
            current_anim_frame_idx++;
            last_anim_update_time = current_time; // Update time only when frame changes

            if (current_anim_frame_idx >= active_anim->frames.size()) {
                animation_cycle_finished = true;
                if (active_anim->loops) {
                    current_anim_frame_idx = 0;
                } else {
                    // Clamp to last frame if not looping
                    current_anim_frame_idx = active_anim->frames.size() - 1;
                    if (current_anim_frame_idx < 0) current_anim_frame_idx = 0; // Safety check
                }
            }
        }
    } else {
        // Handle cases where animation is invalid
        current_anim_frame_idx = 0;
        if (!active_anim) { /* SDL_LogError already happened if it became NULL */ }
        else if (active_anim->frames.empty()) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "active_anim has no frames!"); }
    }


    // --- State Transition AFTER animation check (Walking -> Idle) ---
    if (current_state == STATE_WALKING && animation_cycle_finished && active_anim && !active_anim->loops) {
         queued_steps--;
         SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Walk cycle finished. Steps remaining: %d", queued_steps);
         if (queued_steps > 0) {
             // Start next walk cycle immediately
             current_anim_frame_idx = 0;
             last_anim_update_time = current_time;
             animation_cycle_finished = false; // Reset flag
             SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Starting next queued walk cycle.");
         } else {
             SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Switching to IDLE state.");
             current_state = STATE_IDLE;
             animation_needs_reset = true; // Need to switch back to idle animation next frame
         }
    }
     // Handle the switch back to idle animation if needed (redundant from start?)
     // This ensures idle anim is set correctly AFTER walk finishes
     if (animation_needs_reset && current_state == STATE_IDLE) {
         switch(current_digimon) { /* ... Set Idle Anim ... */
            case DIGI_AGUMON:   active_anim = &agumon_idle_anim; break;
            case DIGI_GABUMON:  active_anim = &gabumon_idle_anim; break;
            case DIGI_BIYOMON:  active_anim = &biyomon_idle_anim; break;
            case DIGI_GATOMON:  active_anim = &gatomon_idle_anim; break;
            case DIGI_GOMAMON:  active_anim = &gomamon_idle_anim; break;
            case DIGI_PALMON:   active_anim = &palmon_idle_anim; break;
            case DIGI_TENTOMON: active_anim = &tentomon_idle_anim; break;
            case DIGI_PATAMON:  active_anim = &patamon_idle_anim; break;
            default:            active_anim = &agumon_idle_anim;
        }
        current_anim_frame_idx = 0;
        last_anim_update_time = current_time;
        animation_needs_reset = false; // Handled
     }
}


void AdventureState::render() {
     if (!game_ptr) return; // Should not happen
     PCDisplay* display = game_ptr->get_display();
     if (!display) return; // Should not happen

    // Clear is handled by Game::run() before calling state render

    // --- Draw Layers and Character in Correct Order ---

    // Layer 2: Background (Slowest)
    int draw2_x1_unclipped = -static_cast<int>(bg_scroll_offset_2);
    int draw2_x2_unclipped = draw2_x1_unclipped + EFFECTIVE_BG_WIDTH_2;
    drawClippedTile(draw2_x1_unclipped, bg_data_2, TILE_WIDTH_2, TILE_HEIGHT_2);
    drawClippedTile(draw2_x2_unclipped, bg_data_2, TILE_WIDTH_2, TILE_HEIGHT_2);

    // Layer 1: Middleground (Medium)
    int draw1_x1_unclipped = -static_cast<int>(bg_scroll_offset_1);
    int draw1_x2_unclipped = draw1_x1_unclipped + EFFECTIVE_BG_WIDTH_1;
    drawClippedTile(draw1_x1_unclipped, bg_data_1, TILE_WIDTH_1, TILE_HEIGHT_1);
    drawClippedTile(draw1_x2_unclipped, bg_data_1, TILE_WIDTH_1, TILE_HEIGHT_1);

    // *** Draw Character Sprite HERE (Before Foreground) ***
    if (active_anim && current_anim_frame_idx < active_anim->frames.size()) {
        const SpriteFrame& current_sprite_frame = active_anim->frames[current_anim_frame_idx];
        if (current_sprite_frame.data) {
            int draw_x = (WINDOW_WIDTH / 2) - (current_sprite_frame.width / 2);
            // Align bottom: Center Y based on window height and sprite height
            // int draw_y = (WINDOW_HEIGHT / 2) - (current_sprite_frame.height / 2);
            // Or align closer to bottom:
             int draw_y = WINDOW_HEIGHT - current_sprite_frame.height - 40; // Adjust 40 pixels offset as needed


            display->drawPixels(draw_x, draw_y, current_sprite_frame.width, current_sprite_frame.height,
                               current_sprite_frame.data, current_sprite_frame.width, current_sprite_frame.height,
                               0, 0);
        } else {
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Attempted to draw sprite with NULL data! Frame %d", current_anim_frame_idx);
        }
    } else if (!active_anim) {
         SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Cannot draw sprite, active_anim is NULL!");
    } else {
         SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Cannot draw sprite, frame index %d out of bounds (size %zu)", current_anim_frame_idx, active_anim->frames.size());
    }


    // Layer 0: Foreground (Fastest) - Drawn last, appears on top of character
    int draw0_x1_unclipped = -static_cast<int>(bg_scroll_offset_0);
    int draw0_x2_unclipped = draw0_x1_unclipped + EFFECTIVE_BG_WIDTH_0;
    drawClippedTile(draw0_x1_unclipped, bg_data_0, TILE_WIDTH_0, TILE_HEIGHT_0);
    drawClippedTile(draw0_x2_unclipped, bg_data_0, TILE_WIDTH_0, TILE_HEIGHT_0);

    // Present is handled by Game::run() after calling state render
}

// Helper function moved into the class
void AdventureState::drawClippedTile(
    int dest_x_unclipped, const uint16_t* tile_data,
    int layer_tile_width, int layer_tile_height
) {
     if (!game_ptr) return;
     PCDisplay* display = game_ptr->get_display();
     if (!display || !tile_data) return;

    int src_x = 0, src_y = 0;
    int src_w = layer_tile_width, src_h = layer_tile_height;
    int dest_x = dest_x_unclipped, dest_y = 0; // Assuming Y=0 for background
    int dest_w = layer_tile_width, dest_h = layer_tile_height;

    // --- Clipping ---
    // Clip Left
    if (dest_x < 0) {
        int clip = -dest_x;
        if (clip >= layer_tile_width) return; // Fully clipped
        src_x += clip; src_w -= clip; dest_w -= clip; dest_x = 0;
    }
    // Clip Right
    if (dest_x + dest_w > WINDOW_WIDTH) {
        int clip = (dest_x + dest_w) - WINDOW_WIDTH;
        if (clip >= src_w) return; // Fully clipped (shouldn't happen if width > window?)
        src_w -= clip; dest_w -= clip;
    }
    // Clip Bottom (adjust if background isn't aligned to top)
    if (dest_y + dest_h > WINDOW_HEIGHT) {
         int clip = (dest_y + dest_h) - WINDOW_HEIGHT;
         if (clip >= src_h) return;
         src_h -= clip; dest_h -= clip;
    }
     // Clip Top (if dest_y could be < 0)
     if (dest_y < 0) {
        int clip = -dest_y;
        if (clip >= layer_tile_height) return;
        src_y += clip; src_h -= clip; dest_h -= clip; dest_y = 0;
     }

    // --- Draw if visible ---
    if (dest_w > 0 && src_w > 0 && dest_h > 0 && src_h > 0) {
        // Ensure source width used in drawPixels matches the original stride of the data
        display->drawPixels(dest_x, dest_y, dest_w, dest_h, tile_data,
                           layer_tile_width, // Pass original width for stride calculation
                           layer_tile_height, src_x, src_y);
    }
}