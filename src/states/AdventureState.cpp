// File: src/states/AdventureState.cpp

#include "states/AdventureState.h"
#include "core/Game.h"
#include "core/AssetManager.h"
#include "platform/pc/pc_display.h"
#include "graphics/Animator.h"
#include "graphics/AnimationData.h"
#include "core/AnimationManager.h"
#include "states/MenuState.h"
#include "core/InputManager.h"
#include "core/GameAction.h"
#include "core/PlayerData.h"
#include <SDL_log.h>
#include <stdexcept>
#include <fstream>
#include <cstddef>
#include <vector>
#include <string>
#include <cmath>
#include <memory>


// --- Constructor ---
AdventureState::AdventureState(Game* game) :
    GameState(game),
    // partnerAnimator_ default constructed
    bgTexture0_(nullptr),
    bgTexture1_(nullptr),
    bgTexture2_(nullptr),
    // current_digimon_ initialized below
    current_state_(STATE_IDLE),
    // Removed old animation state initializers
    queued_steps_(0),
    firstWalkUpdate_(true),
    bg_scroll_offset_0_(0.0f),
    bg_scroll_offset_1_(0.0f),
    bg_scroll_offset_2_(0.0f)
{
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "+++ AdventureState Constructor ENTER +++");
    if (!game_ptr || !game_ptr->getAssetManager() || !game_ptr->get_display() || !game_ptr->getPlayerData() || !game_ptr->getAnimationManager()) { // Check AnimManager too
        throw std::runtime_error("AdventureState requires valid Game pointer with initialized systems (Assets, Display, PlayerData, AnimManager)!");
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState Constructor: Initializing...");

    // Set Initial Partner from PlayerData
    PlayerData* pd = game_ptr->getPlayerData();
    current_digimon_ = pd->currentPartner;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Initial partner set to %d from PlayerData.", static_cast<int>(current_digimon_));

    // Get Background Textures
    AssetManager* assets = game_ptr->getAssetManager();
    bgTexture0_ = assets->getTexture("castle_bg_0");
    bgTexture1_ = assets->getTexture("castle_bg_1");
    bgTexture2_ = assets->getTexture("castle_bg_2");
    if (!bgTexture0_ || !bgTexture1_ || !bgTexture2_) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"AdventureState: Background texture(s) missing!"); }

    // Set Initial Animation using Animator (happens here)
    // Note: enter() might reset this if PlayerData changed between construction and enter()
    std::string initialAnimId = getAnimationIdForCurrentState();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,"Constructor: Requesting Anim ID: '%s'", initialAnimId.c_str());
    AnimationManager* animManager = game_ptr->getAnimationManager();
    const AnimationData* initialAnimData = animManager->getAnimationData(initialAnimId);
    partnerAnimator_.setAnimation(initialAnimData);
    if (initialAnimData) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,"Constructor: Animator set with valid data for '%s'. Texture=%p", initialAnimId.c_str(), (void*)partnerAnimator_.getCurrentTexture());
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Constructor: Animator set with NULL data for '%s'!", initialAnimId.c_str());
    }


    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState Initialized Successfully.");
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "--- AdventureState Constructor EXIT ---");
}


// --- Destructor ---
AdventureState::~AdventureState() { SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState Destructor called."); }


// <<< --- ADDED enter() Implementation --- >>>
void AdventureState::enter() {
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "--- AdventureState enter() CALLED ---");
     // Ensure game_ptr and necessary managers are available
     if (!game_ptr || !game_ptr->getPlayerData() || !game_ptr->getAnimationManager()) {
          SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AdventureState::enter() - Missing required pointers!");
          return;
     }

     // Immediately check if PlayerData's partner matches our current partner
     PlayerData* playerData = game_ptr->getPlayerData();
     if (playerData && current_digimon_ != playerData->currentPartner) {
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState::enter() detected partner change in PlayerData (from %d to %d). Updating.",
                       static_cast<int>(current_digimon_), static_cast<int>(playerData->currentPartner));

         // Update internal state
         current_digimon_ = playerData->currentPartner;
         current_state_ = STATE_IDLE; // Reset to idle
         queued_steps_ = 0;           // Clear steps

         // Set the correct IDLE animation for the new partner immediately
         std::string idleAnimId = getAnimationIdForCurrentState(); // Uses the now updated current_digimon_ and current_state_
         AnimationManager* animManager = game_ptr->getAnimationManager();
         const AnimationData* idleAnimData = animManager->getAnimationData(idleAnimId);
         partnerAnimator_.setAnimation(idleAnimData); // Update the animator

         if (!idleAnimData) {
             SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Enter: Failed to set idle animation for %s", idleAnimId.c_str());
         } else {
             SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Enter: Set animation to '%s'", idleAnimId.c_str());
         }
     } else if (playerData) {
         SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "AdventureState::enter() - Partner matches PlayerData (%d). No change needed.", static_cast<int>(current_digimon_));
         // Ensure the animator is set to the *current* correct animation, in case it wasn't
         // (e.g., if returning from a state that didn't exist before)
         std::string currentAnimId = getAnimationIdForCurrentState();
         AnimationManager* animManager = game_ptr->getAnimationManager(); // Need manager again
         const AnimationData* currentAnimData = nullptr;
         if(animManager) { // Check manager pointer
            currentAnimData = animManager->getAnimationData(currentAnimId);
         } else {
             SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AdventureState::enter() - AnimationManager null!");
         }

         // Only reset if the animator isn't already playing this exact data
         if (partnerAnimator_.getCurrentAnimationData() != currentAnimData) {
             partnerAnimator_.setAnimation(currentAnimData);
             if (currentAnimData) {
                 SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Enter: Refreshed animation to '%s'", currentAnimId.c_str());
             } else {
                 SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Enter: Refreshed animation but data for '%s' is NULL!", currentAnimId.c_str());
             }
         }
     }
     // Reset flag for jitter debugging if kept
     firstWalkUpdate_ = true;
}


// --- handle_input ---
void AdventureState::handle_input(InputManager& inputManager, PlayerData* playerData) {
    SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "--- AdventureState handle_input CALLED (Start State: %d, QueuedSteps: %d) ---", current_state_, queued_steps_);
    if (!game_ptr || game_ptr->getCurrentState() != this) { return; }
    if (!playerData) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "HandleInput: PlayerData null"); }

    bool stateOrDigiChanged = false;

    // Menu Activation
    if (inputManager.isActionJustPressed(GameAction::CONFIRM)) {
        SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Confirm action in AdventureState, pushing MenuState.");
        std::vector<std::string> mainMenuItems = {"DIGIMON", "MAP", "ITEMS", "SAVE", "EXIT"};
        game_ptr->requestPushState(std::make_unique<MenuState>(game_ptr, mainMenuItems));
        SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "--- AdventureState handle_input EXITING (Pushing Menu) ---");
        return;
     }

    // Step Input
    if (inputManager.isActionJustPressed(GameAction::STEP)) {
         if (queued_steps_ < MAX_QUEUED_STEPS) {
            queued_steps_++;
            SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "Step action detected. Queued: %d", queued_steps_);
            if (playerData) {
                playerData->stepsTakenThisChapter++;
                playerData->totalSteps++;
                SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "PlayerData updated: ChapterSteps=%d, TotalSteps=%d", playerData->stepsTakenThisChapter, playerData->totalSteps);
            }
        } else {
             SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "Step action detected, but queue is full (%d).", queued_steps_);
        }
     }

    // Debug Key Check
    DigimonType previousDigimon = current_digimon_;
    DigimonType keySelectedDigimon = previousDigimon;
    if (inputManager.isActionJustPressed(GameAction::SELECT_DIGI_1)) { keySelectedDigimon = DIGI_AGUMON; }
    else if (inputManager.isActionJustPressed(GameAction::SELECT_DIGI_2)) { keySelectedDigimon = DIGI_GABUMON; }
    else if (inputManager.isActionJustPressed(GameAction::SELECT_DIGI_3)) { keySelectedDigimon = DIGI_BIYOMON; }
    else if (inputManager.isActionJustPressed(GameAction::SELECT_DIGI_4)) { keySelectedDigimon = DIGI_GATOMON; }
    else if (inputManager.isActionJustPressed(GameAction::SELECT_DIGI_5)) { keySelectedDigimon = DIGI_GOMAMON; }
    else if (inputManager.isActionJustPressed(GameAction::SELECT_DIGI_6)) { keySelectedDigimon = DIGI_PALMON; }
    else if (inputManager.isActionJustPressed(GameAction::SELECT_DIGI_7)) { keySelectedDigimon = DIGI_TENTOMON; }
    else if (inputManager.isActionJustPressed(GameAction::SELECT_DIGI_8)) { keySelectedDigimon = DIGI_PATAMON; }

    if (keySelectedDigimon != previousDigimon) {
         current_digimon_ = keySelectedDigimon;
         stateOrDigiChanged = true;
         if (playerData) { playerData->currentPartner = current_digimon_; }
    }

    // If Digimon was changed (by debug key), reset state AND set new IDLE animation
    if (stateOrDigiChanged) {
        current_state_ = STATE_IDLE;
        queued_steps_ = 0;
        SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Switched character to %d via debug key.", static_cast<int>(current_digimon_));

        // Set new Idle animation via Animator
        std::string newIdleAnimId = getAnimationIdForCurrentState();
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,"HandleInput Switch: Requesting Anim ID: '%s'", newIdleAnimId.c_str());
        AnimationManager* animManager = game_ptr->getAnimationManager();
        if(animManager){
            const AnimationData* idleAnimData = animManager->getAnimationData(newIdleAnimId);
            partnerAnimator_.setAnimation(idleAnimData);
            if (idleAnimData) {
                 SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,"HandleInput: Animator set with valid data for '%s'. Texture=%p", newIdleAnimId.c_str(), (void*)partnerAnimator_.getCurrentTexture());
            } else {
                 SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"HandleInput: Animator set with NULL data for '%s'!", newIdleAnimId.c_str());
            }
        } else {
             SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"AnimationManager pointer is null in handle_input!");
        }
    }

    SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "--- AdventureState handle_input EXITING (State: %d, QueuedSteps: %d) ---", current_state_, queued_steps_);
}


// --- update ---
void AdventureState::update(float delta_time, PlayerData* playerData) {
    SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "--- AdventureState update CALLED (Start State: %d, QueuedSteps: %d, DT: %.4f) ---", current_state_, queued_steps_, delta_time);

    if (!playerData) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AdventureState::update - PlayerData pointer is null!"); }

    // <<< --- DELETED THE PARTNER CHANGE CHECK BLOCK --- >>>

    // Reset first walk flag if not walking
    if (current_state_ != STATE_WALKING) { firstWalkUpdate_ = true; }

    PlayerState stateBeforeChanges = current_state_;
    bool stateNeedsAnimUpdate = false;     // Flag to trigger setAnimation call later
    bool animationChangedThisFrame = false; // Auxiliary flag, used when resetting walk cycle

    // --- State Change: Idle -> Walking ---
    if (current_state_ == STATE_IDLE && queued_steps_ > 0) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "*** CONDITION MET: Changing state IDLE -> WALKING ***");
        current_state_ = STATE_WALKING;
        firstWalkUpdate_ = true;
        stateNeedsAnimUpdate = true; // Flag to set walk animation
    }

    // --- Update the Animator ---
    partnerAnimator_.update(delta_time); // Update animator state first

    // --- State Change: Walking -> Idle ---
    if (current_state_ == STATE_WALKING && partnerAnimator_.isFinished()) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "*** Walk cycle finished (reported by Animator). Decrementing steps. ***");
        queued_steps_--;
        if (queued_steps_ <= 0) {
            // Last step finished, transition back to IDLE
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, ">>> State Change: WALKING -> IDLE (Queued Steps <= 0) <<<");
            current_state_ = STATE_IDLE;
            stateNeedsAnimUpdate = true; // Flag to set idle animation
            queued_steps_ = 0; // Ensure it's 0
        } else {
            // Still steps queued, reset walk animation frame/timer for next cycle
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Walk cycle finished, starting next. Steps remaining: %d", queued_steps_);
            partnerAnimator_.setAnimation(partnerAnimator_.getCurrentAnimationData());
            animationChangedThisFrame = true;
        }
    }

    // Log if state changed
    if(current_state_ != stateBeforeChanges) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, ">>> State Changed This Frame: %d -> %d <<<", stateBeforeChanges, current_state_);
        stateNeedsAnimUpdate = true;
    }


    // --- Scroll Background ---
    if (current_state_ == STATE_WALKING) {
        // scroll logic (unchanged)
        float scrollAmount0 = SCROLL_SPEED_0 * delta_time; float scrollAmount1 = SCROLL_SPEED_1 * delta_time; float scrollAmount2 = SCROLL_SPEED_2 * delta_time; int effW0=0, effW1=0, effW2=0; if(bgTexture0_) {int w; SDL_QueryTexture(bgTexture0_,0,0,&w,0); effW0=w*2/3; if(effW0<=0)effW0=w;} if(bgTexture1_) {int w; SDL_QueryTexture(bgTexture1_,0,0,&w,0); effW1=w*2/3; if(effW1<=0)effW1=w;} if(bgTexture2_) {int w; SDL_QueryTexture(bgTexture2_,0,0,&w,0); effW2=w*2/3; if(effW2<=0)effW2=w;} if(effW0 > 0) { bg_scroll_offset_0_ -= scrollAmount0; bg_scroll_offset_0_ = std::fmod(bg_scroll_offset_0_ + effW0, (float)effW0); } if(effW1 > 0) { bg_scroll_offset_1_ -= scrollAmount1; bg_scroll_offset_1_ = std::fmod(bg_scroll_offset_1_ + effW1, (float)effW1); } if(effW2 > 0) { bg_scroll_offset_2_ -= scrollAmount2; bg_scroll_offset_2_ = std::fmod(bg_scroll_offset_2_ + effW2, (float)effW2); }
    }

    // --- Set active animation ---
    if (stateNeedsAnimUpdate) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, ">>> Calling setAnimation on Animator (State is now %d) <<<", current_state_);
        std::string nextAnimId = getAnimationIdForCurrentState();
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,"Update StateChange: Requesting Anim ID: '%s' for state %d", nextAnimId.c_str(), current_state_);
        AnimationManager* animManager = game_ptr->getAnimationManager();
        if (animManager) {
             const AnimationData* nextAnimData = animManager->getAnimationData(nextAnimId);
             partnerAnimator_.setAnimation(nextAnimData);
            if (nextAnimData) {
                 SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,"Update: Animator set with valid data for '%s'. Texture=%p", nextAnimId.c_str(), (void*)partnerAnimator_.getCurrentTexture());
             } else {
                 SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Update: Animator set with NULL data for '%s'!", nextAnimId.c_str());
             }
        } else {
             SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"AnimationManager pointer is null in update when trying to set animation!");
        }
    }

    // Clear first walk flag
    if (current_state_ == STATE_WALKING && firstWalkUpdate_) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, ">>> First Walk Update Frame Processing Completed <<<");
        firstWalkUpdate_ = false;
    }

    SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "Update End: State=%d, QueuedSteps=%d", current_state_, queued_steps_);
}


// --- render ---
void AdventureState::render(PCDisplay& display) {
    const int windowW = WINDOW_WIDTH; const int windowH = WINDOW_HEIGHT; auto drawTiledBg = [&](SDL_Texture* tex, float offset, int texW, int texH, int effectiveWidth, const char* layerName) { if (!tex || texW <= 0 || effectiveWidth <= 0) { return; } int drawX1 = -static_cast<int>(std::fmod(offset, (float)effectiveWidth)); if (drawX1 > 0) drawX1 -= effectiveWidth; SDL_Rect dst1 = { drawX1, 0, texW, texH }; display.drawTexture(tex, NULL, &dst1); int drawX2 = drawX1 + effectiveWidth; SDL_Rect dst2 = { drawX2, 0, texW, texH }; display.drawTexture(tex, NULL, &dst2); if (drawX2 + texW < windowW) { int drawX3 = drawX2 + effectiveWidth; SDL_Rect dst3 = { drawX3, 0, texW, texH }; display.drawTexture(tex, NULL, &dst3); } }; int bgW0=0,bgH0=0,effW0=0, bgW1=0,bgH1=0,effW1=0, bgW2=0,bgH2=0,effW2=0; if(bgTexture0_) { SDL_QueryTexture(bgTexture0_,0,0,&bgW0,&bgH0); effW0=bgW0*2/3; if(effW0<=0)effW0=bgW0;} if(bgTexture1_) { SDL_QueryTexture(bgTexture1_,0,0,&bgW1,&bgH1); effW1=bgW1*2/3; if(effW1<=0)effW1=bgW1;} if(bgTexture2_) { SDL_QueryTexture(bgTexture2_,0,0,&bgW2,&bgH2); effW2=bgW2*2/3; if(effW2<=0)effW2=bgW2;}
    drawTiledBg(bgTexture2_, bg_scroll_offset_2_, bgW2, bgH2, effW2, "Layer 2");
    drawTiledBg(bgTexture1_, bg_scroll_offset_1_, bgW1, bgH1, effW1, "Layer 1");

    // <<< --- Draw Character using Animator --- >>>
    SDL_Texture* currentTexture = partnerAnimator_.getCurrentTexture();
    SDL_Rect currentSourceRect = partnerAnimator_.getCurrentFrameRect();

    SDL_LogVerbose(SDL_LOG_CATEGORY_RENDER, "Render: AnimTexture=%p, AnimSrcRect={%d,%d,%d,%d}",
                     (void*)currentTexture, currentSourceRect.x, currentSourceRect.y, currentSourceRect.w, currentSourceRect.h);


    if (currentTexture && currentSourceRect.w > 0 && currentSourceRect.h > 0) {
        // Calculate dstRect
        int drawX = (windowW / 2) - (currentSourceRect.w / 2);
        int verticalOffset = 7;
        int drawY = (windowH / 2) - (currentSourceRect.h / 2) - verticalOffset;
        SDL_Rect dstRect = { drawX, drawY, currentSourceRect.w, currentSourceRect.h };

        display.drawTexture(currentTexture, &currentSourceRect, &dstRect);
    } else {
        SDL_LogWarn(SDL_LOG_CATEGORY_RENDER,"AS Render: Animator returned invalid texture (%p) or frame rect (%d,%d,%d,%d) for partner %d!",
                     (void*)currentTexture, currentSourceRect.x, currentSourceRect.y, currentSourceRect.w, currentSourceRect.h,
                     static_cast<int>(current_digimon_));
    }
    // <<< --- End Character Drawing --- >>>


    // Draw Foreground
    drawTiledBg(bgTexture0_, bg_scroll_offset_0_, bgW0, bgH0, effW0, "Layer 0");
}

// --- Private Helper Definitions ---

std::string AdventureState::getDigimonTextureId(DigimonType type) const {
     switch(type) {
         case DIGI_AGUMON: return "agumon_sheet";
         case DIGI_GABUMON: return "gabumon_sheet";
         case DIGI_BIYOMON: return "biyomon_sheet";
         case DIGI_GATOMON: return "gatomon_sheet";
         case DIGI_GOMAMON: return "gomamon_sheet";
         case DIGI_PALMON: return "palmon_sheet";
         case DIGI_TENTOMON: return "tentomon_sheet";
         case DIGI_PATAMON: return "patamon_sheet";
         default: return "unknown_sheet";
     }
}

std::string AdventureState::getAnimationIdForCurrentState() const {
    std::string baseId = getDigimonTextureId(current_digimon_);
    std::string stateSuffix = "";
    switch (current_state_) {
        case STATE_IDLE:    stateSuffix = "_Idle";    break;
        case STATE_WALKING: stateSuffix = "_Walk";    break;
        default:            stateSuffix = "_Idle";    break;
    }
    return baseId + stateSuffix;
}