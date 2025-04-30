// File: src/states/AdventureState.cpp

#include "states/AdventureState.h"  // Include own header
#include "core/Game.h"              // To access Game methods/members
#include "core/AssetManager.h"      // To get assets
#include "platform/pc/pc_display.h" // To draw
#include "graphics/Animation.h"     // Uses Animation/SpriteFrame
#include <SDL_log.h>                // <<< CORRECTED SDL Include >>>
#include <stdexcept>                // Standard


// --- Constructor (Needs Phase 2 Implementation) ---
AdventureState::AdventureState(Game* game) :
    // Initialize members based on Phase 2 design
    current_digimon_(DIGI_AGUMON),
    current_state_(STATE_IDLE),
    active_anim_(nullptr),
    current_anim_frame_idx_(0),
    last_anim_update_time_(0),
    queued_steps_(0),
    bgTexture0_(nullptr), // Init pointers to null
    bgTexture1_(nullptr),
    bgTexture2_(nullptr)
{
    this->game_ptr = game;
    if (!game_ptr || !game_ptr->getAssetManager() || !game_ptr->get_display()) {
        throw std::runtime_error("AdventureState requires valid Game pointer with initialized systems!");
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState Constructor: Initializing...");

    // Get background textures
    AssetManager* assets = game_ptr->getAssetManager();
    bgTexture0_ = assets->getTexture("castle_bg_0");
    bgTexture1_ = assets->getTexture("castle_bg_1");
    bgTexture2_ = assets->getTexture("castle_bg_2");
    // TODO: Add error checking for textures

    // Initialize animations (this will define frame rects, etc.)
    initializeAnimations();

    // Set initial animation
    setActiveAnimation();
    if (!active_anim_) {
         throw std::runtime_error("Failed to set initial active animation!");
    }

    last_anim_update_time_ = SDL_GetTicks();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState Initialized.");
}

AdventureState::~AdventureState() {
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState Destructor called.");
}

// --- Initialize Animations (Phase 2 Implementation needed here) ---
void AdventureState::initializeAnimations() {
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState::initializeAnimations - TODO: Implement loading from AssetManager and defining frame Rects.");
     // <<< This is where you'll get textures from AssetManager >>>
     // <<< and define the SDL_Rects for each frame based on your JSON/sheet data >>>
     // Example (replace with actual data loading):
     AssetManager* assets = game_ptr->getAssetManager();
     SDL_Texture* texAgumon = assets->getTexture("agumon_sheet");
     if (texAgumon) {
         SDL_Rect aguIdle0 = { 0, 0, 32, 32 };   // EXAMPLE ONLY
         SDL_Rect aguIdle1 = { 32, 0, 32, 32 };  // EXAMPLE ONLY
         SDL_Rect aguWalk0 = { 0, 32, 32, 32 };  // EXAMPLE ONLY
         SDL_Rect aguWalk1 = { 32, 32, 32, 32 }; // EXAMPLE ONLY

         Animation idle; idle.addFrame({texAgumon, aguIdle0}, 1000); idle.addFrame({texAgumon, aguIdle1}, 1000); idle.loops = true; idleAnimations_[DIGI_AGUMON] = idle;
         Animation walk; walk.addFrame({texAgumon, aguWalk0}, 300); walk.addFrame({texAgumon, aguWalk1}, 300); /*...*/ walk.loops = false; walkAnimations_[DIGI_AGUMON] = walk;
     }
      // ... Load and define for other Digimon ...
}

// --- Set Active Animation (Phase 2 Implementation needed here) ---
void AdventureState::setActiveAnimation() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState::setActiveAnimation - TODO: Implement setting active_anim_ based on maps.");
     // <<< This logic looks okay from the previous example, uses the maps >>>
      if (current_state_ == STATE_IDLE) {
         auto it = idleAnimations_.find(current_digimon_);
         active_anim_ = (it != idleAnimations_.end()) ? &(it->second) : nullptr;
     } else { // STATE_WALKING
         auto it = walkAnimations_.find(current_digimon_);
         active_anim_ = (it != walkAnimations_.end()) ? &(it->second) : nullptr;
     }
     current_anim_frame_idx_ = 0;
     last_anim_update_time_ = SDL_GetTicks();
     if (!active_anim_) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Failed to find active animation for state %d, digi %d", current_state_, current_digimon_);
}

// --- Handle Input (Needs adapting for Phase 2) ---
void AdventureState::handle_input() {
     // <<< Placeholder using direct input checking >>>
     const Uint8* keystates = SDL_GetKeyboardState(NULL);
     bool stateOrDigiChanged = false;
     // Step Input
     static bool space_pressed = false;
     if(keystates[SDL_SCANCODE_SPACE] && !space_pressed) { if(queued_steps_ < MAX_QUEUED_STEPS) queued_steps_++; space_pressed = true; }
     if(!keystates[SDL_SCANCODE_SPACE]) space_pressed = false;
     // Digi Switch Input
     static bool num_pressed[DIGI_COUNT] = {false};
     for(int i=0; i<DIGI_COUNT; ++i) { /*...*/ } // Add scancode lookup and logic
     // Quit
     if(keystates[SDL_SCANCODE_ESCAPE] && game_ptr) game_ptr->quit_game();
     // Update anim if needed
     // if(stateOrDigiChanged) setActiveAnimation(); // Call if digi/state changes
}

// --- Update (Needs adapting for Phase 2) ---
void AdventureState::update(float delta_time) {
     // SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState::update - TODO: Implement frame-independent movement and Phase 2 logic.");
     // <<< Placeholder using logic similar to previous version >>>
     Uint32 current_time = SDL_GetTicks();
     bool stateChanged = false;
     // Scrolling (Needs frame independence)
     if (current_state_ == STATE_WALKING) { /* ... update offsets ... */ }
     // State Change Idle->Walk
     if (current_state_ == STATE_IDLE && queued_steps_ > 0) { current_state_ = STATE_WALKING; stateChanged = true; }
     // Animation update
     bool anim_finished = false;
     if(active_anim_) { /* ... update current_anim_frame_idx_ based on time/duration ... */ }
     // State Change Walk->Idle
     if (current_state_ == STATE_WALKING && anim_finished && active_anim_ && !active_anim_->loops) { /* ... decrement steps or switch to idle ... */ stateChanged = true;}
     // Set anim if state changed
     if(stateChanged) setActiveAnimation();
}

// --- Render (Needs adapting for Phase 2) ---
void AdventureState::render() {
    // SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState::render - TODO: Implement using texture rendering from AssetManager.");
    if (!game_ptr) return;
    PCDisplay* display = game_ptr->get_display();
    if (!display) return;

    // <<< Placeholder using texture drawing logic from previous example >>>
    // Draw Backgrounds
    auto drawTiledBg = [&](SDL_Texture* tex, float offset, int winW) { /* ... lambda using display->drawTexture ... */ };
    // Calculate effective widths based on actual texture sizes
    int effW0=0, effW1=0, effW2=0;
    if(bgTexture0_) {int w; SDL_QueryTexture(bgTexture0_,0,0,&w,0); effW0 = w*2/3;}
    if(bgTexture1_) {int w; SDL_QueryTexture(bgTexture1_,0,0,&w,0); effW1 = w*2/3;}
    if(bgTexture2_) {int w; SDL_QueryTexture(bgTexture2_,0,0,&w,0); effW2 = w*2/3;}

    drawTiledBg(bgTexture2_, bg_scroll_offset_2_, effW2);
    drawTiledBg(bgTexture1_, bg_scroll_offset_1_, effW1);

    // Draw Character
    if (active_anim_) {
        const SpriteFrame* frame = active_anim_->getFrame(current_anim_frame_idx_);
        if (frame && frame->texturePtr) {
            SDL_Rect dst = { (WINDOW_WIDTH - frame->sourceRect.w)/2, WINDOW_HEIGHT - frame->sourceRect.h - 40, frame->sourceRect.w, frame->sourceRect.h };
            display->drawTexture(frame->texturePtr, &frame->sourceRect, &dst);
        }
    }

    drawTiledBg(bgTexture0_, bg_scroll_offset_0_, effW0);
}