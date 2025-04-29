// File: src/AdventureState.cpp

#include <States/AdventureState.h>      // Includes GameState, Animation, GameConstants indirectly
#include <Core/Game.h>                // Needed for Game* parameter and get_display()
#include <Platform/PC/pc_display.h>   // Needed for PCDisplay* type

#include <SDL_log.h>
#include <stdexcept>

// --- Include ALL Asset Headers Needed ---
// These provide the _WIDTH, _HEIGHT, _data definitions used in the initializer list
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
    // (Requires members to be declared in AdventureState.h and assets includes above)
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
    // Animation members are default-constructed
{
    // --- Constructor Body ---
    this->game_ptr = game;

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState Constructor: Initializing...");
    // ... (Logging and Sanity Checks) ...

    // --- Define Animations (Populate the animation objects) ---
    // This uses the SpriteFrame members initialized above and Animation members declared in .h
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
        default:            active_anim = &agumon_idle_anim; SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Defaulting to Agumon idle anim");
    }
    if (!active_anim) { throw std::runtime_error("Initial active_anim is NULL."); }
    last_anim_update_time = SDL_GetTicks();
    animation_needs_reset = false; // We've just set it

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState Initialized Successfully.");
} // --- End Constructor Body ---

AdventureState::~AdventureState() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState Destructor called.");
}

// ... other methods (handle_input, update, render, drawClippedTile) are unchanged ...
void AdventureState::handle_input() { /* ... */ }
void AdventureState::update(float delta_time) { /* ... */ }
void AdventureState::render() { /* ... */ }
void AdventureState::drawClippedTile(int dest_x_unclipped, const uint16_t* tile_data, int layer_tile_width, int layer_tile_height) { /* ... */ }