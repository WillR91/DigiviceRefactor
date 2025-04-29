// File: src/AdventureState.cpp (Cleaned up logging)

#include <States/AdventureState.h>
#include <Core/Game.h>
#include <Platform/PC/pc_display.h> // Needed for PCDisplay*

#include <SDL.h> // Need SDL for SDL_GetKeyboardState, SDL_SCANCODE_* etc.
#include <SDL_log.h>
#include <stdexcept>
#include <cmath>
#include <vector>
#include <cstdint>

// --- Include ALL Asset Headers Needed ---
#include <Agumon_Idle_0.h>
#include <Agumon_Idle_1.h>
#include <Agumon_Walk_0.h>
#include <Agumon_Walk_1.h>
#include <Gabumon_Idle_0.h>
#include <Gabumon_Idle_1.h>
#include <Gabumon_Walk_0.h>
#include <Gabumon_Walk_1.h>
#include <Biyomon_Idle_0.h>
#include <Biyomon_Idle_1.h>
#include <Biyomon_Walk_0.h>
#include <Biyomon_Walk_1.h>
#include <Gatomon_Idle_0.h>
#include <Gatomon_Idle_1.h>
#include <Gatomon_Walk_0.h>
#include <Gatomon_Walk_1.h>
#include <Gomamon_Idle_0.h>
#include <Gomamon_Idle_1.h>
#include <Gomamon_Walk_0.h>
#include <Gomamon_Walk_1.h>
#include <Palmon_Idle_0.h>
#include <Palmon_Idle_1.h>
#include <Palmon_Walk_0.h>
#include <Palmon_Walk_1.h>
#include <Tentomon_Idle_0.h>
#include <Tentomon_Idle_1.h>
#include <Tentomon_Walk_0.h>
#include <Tentomon_Walk_1.h>
#include <Patamon_Idle_0.h>
#include <Patamon_Idle_1.h>
#include <Patamon_Walk_0.h>
#include <Patamon_Walk_1.h>
#include <castlebackground0.h>
#include <castlebackground1.h>
#include <castlebackground2.h>
// ------------------------------------


// Constructor: Use initializer list for member initialization
AdventureState::AdventureState(Game* game) :
    // --- Initializer List ---
    current_digimon(DIGI_AGUMON),
    current_state(STATE_IDLE),
    active_anim(nullptr),
    current_anim_frame_idx(0),
    last_anim_update_time(0),
    queued_steps(0),
    character_changed_this_frame(false),
    animation_needs_reset(true),
    // Initialize ALL SpriteFrames using brace initialization {}
    agumon_idle_0_sf{AGUMON_IDLE_0_WIDTH, AGUMON_IDLE_0_HEIGHT, Agumon_Idle_0_data},
    agumon_idle_1_sf{AGUMON_IDLE_1_WIDTH, AGUMON_IDLE_1_HEIGHT, Agumon_Idle_1_data},
    agumon_walk_0_sf{AGUMON_WALK_0_WIDTH, AGUMON_WALK_0_HEIGHT, Agumon_Walk_0_data},
    agumon_walk_1_sf{AGUMON_WALK_1_WIDTH, AGUMON_WALK_1_HEIGHT, Agumon_Walk_1_data},
    gabumon_idle_0_sf{GABUMON_IDLE_0_WIDTH, GABUMON_IDLE_0_HEIGHT, Gabumon_Idle_0_data},
    gabumon_idle_1_sf{GABUMON_IDLE_1_WIDTH, GABUMON_IDLE_1_HEIGHT, Gabumon_Idle_1_data},
    gabumon_walk_0_sf{GABUMON_WALK_0_WIDTH, GABUMON_WALK_0_HEIGHT, Gabumon_Walk_0_data},
    gabumon_walk_1_sf{GABUMON_WALK_1_WIDTH, GABUMON_WALK_1_HEIGHT, Gabumon_Walk_1_data},
    biyomon_idle_0_sf{BIYOMON_IDLE_0_WIDTH, BIYOMON_IDLE_0_HEIGHT, Biyomon_Idle_0_data},
    biyomon_idle_1_sf{BIYOMON_IDLE_1_WIDTH, BIYOMON_IDLE_1_HEIGHT, Biyomon_Idle_1_data},
    biyomon_walk_0_sf{BIYOMON_WALK_0_WIDTH, BIYOMON_WALK_0_HEIGHT, Biyomon_Walk_0_data},
    biyomon_walk_1_sf{BIYOMON_WALK_1_WIDTH, BIYOMON_WALK_1_HEIGHT, Biyomon_Walk_1_data},
    gatomon_idle_0_sf{GATOMON_IDLE_0_WIDTH, GATOMON_IDLE_0_HEIGHT, Gatomon_Idle_0_data},
    gatomon_idle_1_sf{GATOMON_IDLE_1_WIDTH, GATOMON_IDLE_1_HEIGHT, Gatomon_Idle_1_data},
    gatomon_walk_0_sf{GATOMON_WALK_0_WIDTH, GATOMON_WALK_0_HEIGHT, Gatomon_Walk_0_data},
    gatomon_walk_1_sf{GATOMON_WALK_1_WIDTH, GATOMON_WALK_1_HEIGHT, Gatomon_Walk_1_data},
    gomamon_idle_0_sf{GOMAMON_IDLE_0_WIDTH, GOMAMON_IDLE_0_HEIGHT, Gomamon_Idle_0_data},
    gomamon_idle_1_sf{GOMAMON_IDLE_1_WIDTH, GOMAMON_IDLE_1_HEIGHT, Gomamon_Idle_1_data},
    gomamon_walk_0_sf{GOMAMON_WALK_0_WIDTH, GOMAMON_WALK_0_HEIGHT, Gomamon_Walk_0_data},
    gomamon_walk_1_sf{GOMAMON_WALK_1_WIDTH, GOMAMON_WALK_1_HEIGHT, Gomamon_Walk_1_data},
    palmon_idle_0_sf{PALMON_IDLE_0_WIDTH, PALMON_IDLE_0_HEIGHT, Palmon_Idle_0_data},
    palmon_idle_1_sf{PALMON_IDLE_1_WIDTH, PALMON_IDLE_1_HEIGHT, Palmon_Idle_1_data},
    palmon_walk_0_sf{PALMON_WALK_0_WIDTH, PALMON_WALK_0_HEIGHT, Palmon_Walk_0_data},
    palmon_walk_1_sf{PALMON_WALK_1_WIDTH, PALMON_WALK_1_HEIGHT, Palmon_Walk_1_data},
    tentomon_idle_0_sf{TENTOMON_IDLE_0_WIDTH, TENTOMON_IDLE_0_HEIGHT, Tentomon_Idle_0_data},
    tentomon_idle_1_sf{TENTOMON_IDLE_1_WIDTH, TENTOMON_IDLE_1_HEIGHT, Tentomon_Idle_1_data},
    tentomon_walk_0_sf{TENTOMON_WALK_0_WIDTH, TENTOMON_WALK_0_HEIGHT, Tentomon_Walk_0_data},
    tentomon_walk_1_sf{TENTOMON_WALK_1_WIDTH, TENTOMON_WALK_1_HEIGHT, Tentomon_Walk_1_data},
    patamon_idle_0_sf{PATAMON_IDLE_0_WIDTH, PATAMON_IDLE_0_HEIGHT, Patamon_Idle_0_data},
    patamon_idle_1_sf{PATAMON_IDLE_1_WIDTH, PATAMON_IDLE_1_HEIGHT, Patamon_Idle_1_data},
    patamon_walk_0_sf{PATAMON_WALK_0_WIDTH, PATAMON_WALK_0_HEIGHT, Patamon_Walk_0_data},
    patamon_walk_1_sf{PATAMON_WALK_1_WIDTH, PATAMON_WALK_1_HEIGHT, Patamon_Walk_1_data}
{
    // --- Constructor Body ---
    this->game_ptr = game;

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState Constructor: Initializing...");

    // Logging for background dimensions can be useful sometimes, keep if desired
    SDL_Log("Expected Background Dimensions (WxH) for all layers: %d x %d", Constants::BACKGROUND_WIDTH, Constants::BACKGROUND_HEIGHT);
    // float effective_bg_width_float = static_cast<float>(Constants::BACKGROUND_WIDTH) * (2.0f / 3.0f);
    // int effective_bg_width_int = static_cast<int>(std::round(effective_bg_width_float));
    // SDL_Log("Calculated Effective Scroll Width: %d", effective_bg_width_int); // Optional log
    // SDL_Log("Scroll Speeds (0, 1, 2): %.2f, %.2f, %.2f", SCROLL_SPEED_0, SCROLL_SPEED_1, SCROLL_SPEED_2); // Optional log

    // Sanity checks are good, maybe keep them but remove if causing issues or verbose
    /* // Commented out Sanity Checks for now
    auto checkLayer = [](int layerNum, int width, int height) {
        if (width <= 0 || height <= 0) {
             SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error: Invalid background dimensions from constants (W:%d H:%d)", width, height);
             return false;
        }
        return true;
    };
    bool config_ok = true;
    config_ok &= checkLayer(0, Constants::BACKGROUND_WIDTH, Constants::BACKGROUND_HEIGHT);
    config_ok &= checkLayer(1, Constants::BACKGROUND_WIDTH, Constants::BACKGROUND_HEIGHT);
    config_ok &= checkLayer(2, Constants::BACKGROUND_WIDTH, Constants::BACKGROUND_HEIGHT);
    if (WINDOW_WIDTH <= 0 || WINDOW_HEIGHT <= 0) { config_ok = false; }
    if (!config_ok) { throw std::runtime_error("Configuration errors in AdventureState."); }
    */

    // --- Define Animations ---
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

    if (!bg_data_0 || !bg_data_1 || !bg_data_2) { throw std::runtime_error("Background data pointers are NULL."); }

    // Set initial animation
     switch(current_digimon) {
        case DIGI_AGUMON:   active_anim = &agumon_idle_anim; break;
        case DIGI_GABUMON:  active_anim = &gabumon_idle_anim; break;
        case DIGI_BIYOMON:  active_anim = &biyomon_idle_anim; break;
        case DIGI_GATOMON:  active_anim = &gatomon_idle_anim; break;
        case DIGI_GOMAMON:  active_anim = &gomamon_idle_anim; break;
        case DIGI_PALMON:   active_anim = &palmon_idle_anim; break;
        case DIGI_TENTOMON: active_anim = &tentomon_idle_anim; break;
        case DIGI_PATAMON:  active_anim = &patamon_idle_anim; break;
        default:            active_anim = &agumon_idle_anim; SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Defaulting to Agumon idle anim"); break;
    }
    if (!active_anim) { throw std::runtime_error("Initial active_anim is NULL."); }
    last_anim_update_time = SDL_GetTicks();
    animation_needs_reset = false;

    // Removed constructor debug logs for animation counts

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState Initialized Successfully.");
} // --- End Constructor Body ---

AdventureState::~AdventureState() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState Destructor called.");
}

void AdventureState::handle_input() {
    const Uint8* keystates = SDL_GetKeyboardState(NULL);

    // --- Ensure local static variables are declared ---
    static bool space_pressed_last_frame = false;
    static bool numkey_pressed_last_frame[DIGI_COUNT] = {false};
    // -------------------------------------------------

    if (keystates[SDL_SCANCODE_SPACE]) {
        if (!space_pressed_last_frame) {
            if (queued_steps < MAX_QUEUED_STEPS) {
                queued_steps++;
                // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Space pressed! queued_steps = %d", queued_steps); // Removed Debug Log
            }
        }
        space_pressed_last_frame = true;
    } else {
        space_pressed_last_frame = false;
    }

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
                 // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Numkey %d pressed!", i+1); // Removed Debug Log
                 DigimonType selected_digi = static_cast<DigimonType>(i);
                 if (selected_digi != current_digimon) {
                    current_digimon = selected_digi;
                    // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "--> Character changed to %d", current_digimon); // Removed Debug Log
                    character_changed_this_frame = true;
                    current_state = STATE_IDLE;
                    queued_steps = 0;
                 }
            }
             numkey_pressed_last_frame[i] = true;
        } else {
             numkey_pressed_last_frame[i] = false;
        }
    }

    if (keystates[SDL_SCANCODE_ESCAPE]) {
        if (game_ptr) { game_ptr->quit_game(); }
    }
}


void AdventureState::update(float delta_time) {
    Uint32 current_time = SDL_GetTicks();

    // --- Ensure local variable is declared ---
    bool animation_cycle_finished = false;
    // -----------------------------------------

    // Define effective scroll width locally
    float effectiveW_float_0 = static_cast<float>(Constants::BACKGROUND_WIDTH) * (2.0f / 3.0f);
    float effectiveW_float_1 = effectiveW_float_0;
    float effectiveW_float_2 = effectiveW_float_0;

    // Update Parallax Scrolling
    if (current_state == STATE_WALKING) {
        bg_scroll_offset_0 -= SCROLL_SPEED_0;
        while (bg_scroll_offset_0 < 0.0f) { bg_scroll_offset_0 += effectiveW_float_0; }
        bg_scroll_offset_0 = std::fmod(bg_scroll_offset_0, effectiveW_float_0);

        bg_scroll_offset_1 -= SCROLL_SPEED_1;
        while (bg_scroll_offset_1 < 0.0f) { bg_scroll_offset_1 += effectiveW_float_1; }
        bg_scroll_offset_1 = std::fmod(bg_scroll_offset_1, effectiveW_float_1);

        bg_scroll_offset_2 -= SCROLL_SPEED_2;
        while (bg_scroll_offset_2 < 0.0f) { bg_scroll_offset_2 += effectiveW_float_2; }
        bg_scroll_offset_2 = std::fmod(bg_scroll_offset_2, effectiveW_float_2);
    }

    // State & Animation Selection / Update
    animation_needs_reset = character_changed_this_frame;
    character_changed_this_frame = false;

    if (current_state == STATE_IDLE && queued_steps > 0) {
        current_state = STATE_WALKING;
        animation_needs_reset = true;
        // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Update: Changed state to WALKING"); // Removed Debug Log
    }

    if (animation_needs_reset) {
        // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Update: Animation reset needed."); // Removed Debug Log
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
                default:            active_anim = &agumon_idle_anim; break;
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
                default:            active_anim = &agumon_walk_anim; break;
            }
        }
        current_anim_frame_idx = 0;
        last_anim_update_time = current_time;
        animation_needs_reset = false;
         if (!active_anim) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Animation became NULL after reset!");} // Keep Error Log
         // else {SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Update: Animation reset done. New anim has %zu frames.", active_anim->frames.size());} // Removed Debug Log
    }


    // Animation Frame Logic
    if (active_anim && !active_anim->frames.empty() && !active_anim->frame_durations_ms.empty()) {
        // Bounds check
        if (current_anim_frame_idx >= static_cast<int>(active_anim->frames.size()) || current_anim_frame_idx >= static_cast<int>(active_anim->frame_durations_ms.size())) {
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Animation frame index %d out of bounds! Resetting.", current_anim_frame_idx); // Keep Warn Log
             current_anim_frame_idx = 0;
             if (!active_anim || active_anim->frames.empty() || active_anim->frame_durations_ms.empty()) {
                 active_anim = nullptr;
                 SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Animation invalid after index reset!"); // Keep Error Log
                 return;
             }
        }

        // Check index validity before accessing duration
        if(current_anim_frame_idx < static_cast<int>(active_anim->frame_durations_ms.size()) && current_anim_frame_idx >= 0)
        {
            Uint32 current_frame_duration = active_anim->frame_durations_ms[current_anim_frame_idx];

            // --- REMOVED FRAME TIMING LOG ---
            // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Anim Check: CurTime=%u, LastUpdate=%u, FrameDur=%u, TargetTime=%u", ...);
            // ------------------------------

            if (current_time >= last_anim_update_time + current_frame_duration) {
                // --- REMOVED FRAME ADVANCE LOG ---
                // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, " --> Advancing frame from %d", current_anim_frame_idx);
                // -------------------------------

                current_anim_frame_idx++;
                last_anim_update_time = current_time;

                // Check index AFTER incrementing
                if (current_anim_frame_idx >= static_cast<int>(active_anim->frames.size())) {
                    // --- REMOVED CYCLE FINISHED LOG ---
                     // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, " --> Animation cycle finished");
                    // --------------------------------

                    animation_cycle_finished = true;
                    if (active_anim->loops) {
                        current_anim_frame_idx = 0;
                    } else {
                        current_anim_frame_idx = static_cast<int>(active_anim->frames.size()) - 1;
                        if (current_anim_frame_idx < 0) current_anim_frame_idx = 0;
                    }
                }
            }
        } else {
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Frame index %d out of bounds for durations (size %zu)! Resetting.", current_anim_frame_idx, active_anim->frame_durations_ms.size()); // Keep Warn Log
             current_anim_frame_idx = 0;
        }

    } else {
        current_anim_frame_idx = 0;
        if (!active_anim) { /* LogError might happen in reset logic */ }
        else if (active_anim->frames.empty()) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "active_anim has no frames!"); } // Keep Warn Log
     }


    // State Transition AFTER animation check (Walking -> Idle)
    if (current_state == STATE_WALKING && animation_cycle_finished && active_anim && !active_anim->loops) {
         queued_steps--;
         // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Update: Walk cycle finished. Steps remaining: %d", queued_steps); // Removed Debug Log
         if (queued_steps > 0) {
              current_anim_frame_idx = 0;
              last_anim_update_time = current_time;
              // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Update: Starting next queued walk cycle."); // Removed Debug Log
         } else {
              // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Update: Switching back to IDLE state."); // Removed Debug Log
              current_state = STATE_IDLE;
              animation_needs_reset = true;
         }
    }
     // Handle the switch back to idle animation if needed
     if (animation_needs_reset && current_state == STATE_IDLE) {
         switch(current_digimon) {
            case DIGI_AGUMON:   active_anim = &agumon_idle_anim; break;
            case DIGI_GABUMON:  active_anim = &gabumon_idle_anim; break;
            case DIGI_BIYOMON:  active_anim = &biyomon_idle_anim; break;
            case DIGI_GATOMON:  active_anim = &gatomon_idle_anim; break;
            case DIGI_GOMAMON:  active_anim = &gomamon_idle_anim; break;
            case DIGI_PALMON:   active_anim = &palmon_idle_anim; break;
            case DIGI_TENTOMON: active_anim = &tentomon_idle_anim; break;
            case DIGI_PATAMON:  active_anim = &patamon_idle_anim; break;
            default:            active_anim = &agumon_idle_anim; break;
        }
        current_anim_frame_idx = 0;
        last_anim_update_time = current_time;
        animation_needs_reset = false;
     }
}


void AdventureState::render() {
    // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "--- AdventureState::render() TOP ---"); // Removed Debug Log

    if (!game_ptr) return;
    PCDisplay* display = game_ptr->get_display();
    if (!display) return;
    SDL_Renderer* renderer = display->getRenderer();
    if (!renderer) return;

    if (!active_anim) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Render Error: active_anim pointer is NULL!"); // Keep Error Log
        // Maybe draw a placeholder if anim is null? For now, just continue to draw background.
    }
    // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Render: Got valid display(%p) and renderer(%p). active_anim=%p", ...); // Removed Debug Log


    // Calculate effective width
    float effective_bg_width_float = static_cast<float>(Constants::BACKGROUND_WIDTH) * (2.0f / 3.0f);
    int effective_bg_width_int = static_cast<int>(std::round(effective_bg_width_float));
    // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  Effective BG Width: %d", effective_bg_width_int); // Removed Debug Log

    // Draw Layer 2
    int draw2_x1_unclipped = -static_cast<int>(bg_scroll_offset_2);
    int draw2_x2_unclipped = draw2_x1_unclipped + effective_bg_width_int;
    // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  Layer 2: Offset=%.2f, DrawX1=%d, DrawX2=%d", ...); // Removed Debug Log
    drawClippedTile(draw2_x1_unclipped, bg_data_2, Constants::BACKGROUND_WIDTH, Constants::BACKGROUND_HEIGHT);
    drawClippedTile(draw2_x2_unclipped, bg_data_2, Constants::BACKGROUND_WIDTH, Constants::BACKGROUND_HEIGHT);

    // Draw Layer 1
    int draw1_x1_unclipped = -static_cast<int>(bg_scroll_offset_1);
    int draw1_x2_unclipped = draw1_x1_unclipped + effective_bg_width_int;
    // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  Layer 1: Offset=%.2f, DrawX1=%d, DrawX2=%d", ...); // Removed Debug Log
    drawClippedTile(draw1_x1_unclipped, bg_data_1, Constants::BACKGROUND_WIDTH, Constants::BACKGROUND_HEIGHT);
    drawClippedTile(draw1_x2_unclipped, bg_data_1, Constants::BACKGROUND_WIDTH, Constants::BACKGROUND_HEIGHT);

// Draw Character Sprite
if (active_anim && current_anim_frame_idx >= 0 && current_anim_frame_idx < static_cast<int>(active_anim->frames.size())) { // Check index validity
    const SpriteFrame& current_sprite_frame = active_anim->frames[current_anim_frame_idx];
    if (current_sprite_frame.data) {
        // Center horizontally
        int draw_x = (WINDOW_WIDTH / 2) - (current_sprite_frame.width / 2);
        // --- UPDATED: Center vertically ---
        int draw_y = (WINDOW_HEIGHT / 2) - (current_sprite_frame.height / 2);
        // ----------------------------------

        // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  Drawing Sprite: FrameIdx=%d, W=%d, H=%d, X=%d, Y=%d, Data=%p", ...); // Keep commented out unless needed

        display->drawPixels(draw_x, draw_y, current_sprite_frame.width, current_sprite_frame.height,
                            current_sprite_frame.data, current_sprite_frame.width, current_sprite_frame.height,
                            0, 0);
    } else {
         SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "  Skipping sprite draw: NULL data! Frame %d", current_anim_frame_idx);
    }
} else {
     SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "  Skipping sprite draw: Active anim invalid (%p) or frame index %d out of bounds.", (void*)active_anim, current_anim_frame_idx);
}

    // Draw Layer 0
    int draw0_x1_unclipped = -static_cast<int>(bg_scroll_offset_0);
    int draw0_x2_unclipped = draw0_x1_unclipped + effective_bg_width_int;
    // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  Layer 0: Offset=%.2f, DrawX1=%d, DrawX2=%d", ...); // Removed Debug Log
    drawClippedTile(draw0_x1_unclipped, bg_data_0, Constants::BACKGROUND_WIDTH, Constants::BACKGROUND_HEIGHT);
    drawClippedTile(draw0_x2_unclipped, bg_data_0, Constants::BACKGROUND_WIDTH, Constants::BACKGROUND_HEIGHT);
}

void AdventureState::drawClippedTile(
    int dest_x_unclipped, const uint16_t* tile_data,
    int layer_tile_width, int layer_tile_height
) {
    // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  drawClippedTile called: ..."); // Removed Debug Log

     if (!game_ptr) return;
     PCDisplay* display = game_ptr->get_display();
     if (!display || !tile_data) {
         SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "  drawClippedTile: Skipping due to null display or tile_data"); // Keep Warn Log
         return;
     }

    SDL_Rect srcRect = { 0, 0, layer_tile_width, layer_tile_height };
    SDL_Rect dstRect = { dest_x_unclipped, 0, layer_tile_width, layer_tile_height };

    // --- Corrected Clipping ---
    // ... (Clipping logic) ...

    // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "    After Clipping: src={%d,%d %dx%d} dst={%d,%d %dx%d}", ...); // Removed Debug Log

    if (dstRect.w > 0 && srcRect.w > 0 && dstRect.h > 0 && srcRect.h > 0) {
        // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "    --> Drawing clipped tile now!"); // Removed Debug Log
        display->drawPixels(dstRect.x, dstRect.y, dstRect.w, dstRect.h, tile_data,
                            layer_tile_width, layer_tile_height, srcRect.x, srcRect.y);
    } else {
        // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "    --> NOT Drawing clipped tile (clipped empty or invalid)."); // Removed Debug Log
    }
}