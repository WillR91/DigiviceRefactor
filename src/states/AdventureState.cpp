// File: src/states/AdventureState.cpp

#include "states/AdventureState.h"
#include "core/Game.h"
#include "core/AssetManager.h"
#include "platform/pc/pc_display.h"
#include "graphics/Animator.h"
#include "graphics/AnimationData.h"
#include "core/AnimationManager.h"
#include "states/MenuState.h"
#include "states/ProgressState.h"   // Added for ProgressState
#include "core/InputManager.h"
#include "core/GameAction.h"
#include "core/PlayerData.h"
#include "Utils/AnimationUtils.h"
#include "core/GameConstants.h"     // Added for game constants
#include <SDL_log.h>
#include <stdexcept>
#include <fstream>
#include <cstddef>
#include <vector>
#include <string>
#include <cmath>
#include <memory>

// Step Goal Constant
const int CURRENT_CHAPTER_STEP_GOAL = 10;

// Rate Limiting Constants
const float STEP_WINDOW_DURATION = 1.0f; // Time window in seconds
const int MAX_STEPS_PER_WINDOW = 4;      // Max steps allowed per window

// Idle Return Constant
const float TIME_TO_RETURN_TO_IDLE = 1.25f; // Seconds of no STEP input before stopping

// --- Constructor ---
AdventureState::AdventureState(Game* game) :
    GameState(game),
    bgTexture0_(nullptr),
    bgTexture1_(nullptr),
    bgTexture2_(nullptr),
    current_state_(STATE_IDLE),
    queued_steps_(0),
    firstWalkUpdate_(true),
    bg_scroll_offset_0_(0.0f),
    bg_scroll_offset_1_(0.0f),
    bg_scroll_offset_2_(0.0f),
    timeSinceLastStep_(0.0f),
    stepWindowTimer_(0.0f),
    stepsInWindow_(0)
{
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Initializing...");
    
    if (!game_ptr || !game_ptr->getAssetManager() || !game_ptr->get_display() || 
        !game_ptr->getPlayerData() || !game_ptr->getAnimationManager()) {
        throw std::runtime_error("AdventureState: Missing required systems!");
    }

    PlayerData* pd = game_ptr->getPlayerData();
    current_digimon_ = pd->currentPartner;

    AssetManager* assets = game_ptr->getAssetManager();
    bgTexture0_ = assets->getTexture("castle_bg_0");
    bgTexture1_ = assets->getTexture("castle_bg_1");
    bgTexture2_ = assets->getTexture("castle_bg_2");
    
    std::string initialAnimId = AnimationUtils::GetAnimationId(current_digimon_, "Idle");
    AnimationManager* animManager = game_ptr->getAnimationManager();
    const AnimationData* initialAnimData = animManager->getAnimationData(initialAnimId);
    partnerAnimator_.setAnimation(initialAnimData);
    
    if (!initialAnimData) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load initial animation!");
    }
}


// --- Destructor ---
AdventureState::~AdventureState() { SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState Destructor called."); }


// --- enter ---
void AdventureState::enter() {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Entering state");
    if (!game_ptr || !game_ptr->getPlayerData() || !game_ptr->getAnimationManager()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Missing required pointers!");
        return;
    }

    PlayerData* playerData = game_ptr->getPlayerData();
    if (playerData && current_digimon_ != playerData->currentPartner) {
        current_digimon_ = playerData->currentPartner;
        current_state_ = STATE_IDLE;
        queued_steps_ = 0;

        std::string idleAnimId = AnimationUtils::GetAnimationId(current_digimon_, "Idle");
        AnimationManager* animManager = game_ptr->getAnimationManager();
        const AnimationData* idleAnimData = animManager->getAnimationData(idleAnimId);
        partnerAnimator_.setAnimation(idleAnimData);

        if (!idleAnimData) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to set animation: %s", idleAnimId.c_str());
        }
    }
    firstWalkUpdate_ = true;
}


// --- handle_input ---
void AdventureState::handle_input(InputManager& inputManager, PlayerData* playerData) {
    if (!game_ptr || game_ptr->getCurrentState() != this) return;

    // Menu Activation
    if (inputManager.isActionJustPressed(GameAction::CONFIRM)) {
        SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "AdventureState: Confirm action. Requesting fade to MenuState.");
        std::vector<std::string> mainMenuItems = {"DIGIMON", "MAP", "ITEMS", "SAVE", "EXIT"};
        auto menuState = std::make_unique<MenuState>(game_ptr, mainMenuItems);
        game_ptr->requestFadeToState(std::move(menuState), 0.5f, false); // Fade for 0.5s, pop AdventureState
        return;
    }
    if (inputManager.isActionJustPressed(GameAction::CANCEL)) {
        SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "AdventureState: Cancel action. Pushing ProgressState.");
        game_ptr->requestPushState(std::make_unique<ProgressState>(game_ptr));
        return;
    }

    // Step Input with Rate Limiting
    if (inputManager.isActionJustPressed(GameAction::STEP)) {
        SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "Input: STEP pressed");
        if (stepsInWindow_ < MAX_STEPS_PER_WINDOW) {
            stepsInWindow_++; // Consume a slot in the window
            if (playerData) {
                playerData->stepsTakenThisChapter++;
                playerData->totalSteps++;
                SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, 
                    "Step Counted (Rate Limit OK). Steps in Window: %d. Chapter Steps: %d", 
                    stepsInWindow_, playerData->stepsTakenThisChapter);
            }
            current_state_ = STATE_WALKING;
            timeSinceLastStep_ = 0.0f;
        } else {
            SDL_LogVerbose(SDL_LOG_CATEGORY_INPUT, 
                "Step press ignored due to rate limit (StepsInWindow: %d)", stepsInWindow_);
            if(current_state_ == STATE_WALKING) {
                timeSinceLastStep_ = 0.0f;
            }
            current_state_ = STATE_WALKING;
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
         if (playerData) { playerData->currentPartner = current_digimon_; }

         current_state_ = STATE_IDLE;
         queued_steps_ = 0;

         std::string newIdleAnimId = AnimationUtils::GetAnimationId(current_digimon_, "Idle");
         AnimationManager* animManager = game_ptr->getAnimationManager();
         if(animManager){
             const AnimationData* idleAnimData = animManager->getAnimationData(newIdleAnimId);
             partnerAnimator_.setAnimation(idleAnimData);
             if (!idleAnimData) {
                 SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"HandleInput: Animator set with NULL data for '%s'!", newIdleAnimId.c_str());
             }
         } else {
             SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"AnimationManager pointer is null in handle_input!");
         }
    }
}


// --- update ---
void AdventureState::update(float delta_time, PlayerData* playerData) {
    if (!playerData) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AdventureState::update - PlayerData is null!");
    }

    PlayerState stateBeforeChanges = current_state_;
    bool needs_new_animation_set = false;

    // Update Rate Limiting Window Timer
    stepWindowTimer_ += delta_time;
    if (stepWindowTimer_ >= STEP_WINDOW_DURATION) {
        stepWindowTimer_ = std::fmod(stepWindowTimer_, STEP_WINDOW_DURATION);
        SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, 
            "Resetting step window. Steps counted in last window: %d", stepsInWindow_);
        stepsInWindow_ = 0;
    }

    // Update Idle Timer and Check for Return to Idle
    if (current_state_ == STATE_WALKING) {
        timeSinceLastStep_ += delta_time;
        if (timeSinceLastStep_ >= TIME_TO_RETURN_TO_IDLE) {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, 
                "Update: No step input for %.2fs. Transitioning to IDLE.", timeSinceLastStep_);
            current_state_ = STATE_IDLE;
            needs_new_animation_set = true;
        }
    }

    // Update Animator
    partnerAnimator_.update(delta_time);

    // Determine required animation based on current state
    std::string requiredAnimSuffix;
    if (current_state_ == STATE_IDLE) {
        requiredAnimSuffix = "Idle";
    } else { // current_state_ == STATE_WALKING
        requiredAnimSuffix = "WalkLoop"; // Use the looping version for walking
    }
    
    std::string requiredAnimId = AnimationUtils::GetAnimationId(current_digimon_, requiredAnimSuffix);
    
    // Check if we need to set/reset the animation
    bool needsNewAnimation = needs_new_animation_set || 
                           !partnerAnimator_.getCurrentAnimationData() ||
                           (partnerAnimator_.getCurrentAnimationData()->id != requiredAnimId);

    if (needsNewAnimation) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, 
            "Update: Setting animation to '%s' (State: %d)", requiredAnimId.c_str(), current_state_);
        
        const AnimationData* requiredAnimData = game_ptr->getAnimationManager()->getAnimationData(requiredAnimId);
        partnerAnimator_.setAnimation(requiredAnimData);
        
        if (!requiredAnimData) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                "Update: Failed to get animation data for '%s'", requiredAnimId.c_str());
        }
    }

    // Goal/Encounter Checks
    if (playerData && current_state_ == STATE_WALKING) {
        if (playerData->stepsTakenThisChapter >= GameConstants::CURRENT_CHAPTER_STEP_GOAL) {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, 
                "!!! CHAPTER GOAL REACHED (%d steps) !!!", GameConstants::CURRENT_CHAPTER_STEP_GOAL);
            playerData->stepsTakenThisChapter = 0;
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Chapter steps reset to 0.");
        }
    }

    // Scroll Background (Only if walking)
    if (current_state_ == STATE_WALKING) {
        float scrollAmount0 = SCROLL_SPEED_0 * delta_time;
        float scrollAmount1 = SCROLL_SPEED_1 * delta_time;
        float scrollAmount2 = SCROLL_SPEED_2 * delta_time;
        int effW0 = 0, effW1 = 0, effW2 = 0;

        if (bgTexture0_) {
            int w;
            SDL_QueryTexture(bgTexture0_, 0, 0, &w, 0);
            effW0 = w * 2/3;
            if (effW0 <= 0) effW0 = w;
        }
        if (bgTexture1_) {
            int w;
            SDL_QueryTexture(bgTexture1_, 0, 0, &w, 0);
            effW1 = w * 2/3;
            if (effW1 <= 0) effW1 = w;
        }
        if (bgTexture2_) {
            int w;
            SDL_QueryTexture(bgTexture2_, 0, 0, &w, 0);
            effW2 = w * 2/3;
            if (effW2 <= 0) effW2 = w;
        }

        if (effW0 > 0) {
            bg_scroll_offset_0_ -= scrollAmount0;
            bg_scroll_offset_0_ = std::fmod(bg_scroll_offset_0_ + effW0, (float)effW0);
        }
        if (effW1 > 0) {
            bg_scroll_offset_1_ -= scrollAmount1;
            bg_scroll_offset_1_ = std::fmod(bg_scroll_offset_1_ + effW1, (float)effW1);
        }
        if (effW2 > 0) {
            bg_scroll_offset_2_ -= scrollAmount2;
            bg_scroll_offset_2_ = std::fmod(bg_scroll_offset_2_ + effW2, (float)effW2);
        }
    }

    SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "Update End: State=%d, QueuedSteps=%d", 
                   current_state_, queued_steps_);
}


// --- render ---
void AdventureState::render(PCDisplay& display) {
    const int windowW = WINDOW_WIDTH; const int windowH = WINDOW_HEIGHT; auto drawTiledBg = [&](SDL_Texture* tex, float offset, int texW, int texH, int effectiveWidth, const char* layerName) { if (!tex || texW <= 0 || effectiveWidth <= 0) { return; } int drawX1 = -static_cast<int>(std::fmod(offset, (float)effectiveWidth)); if (drawX1 > 0) drawX1 -= effectiveWidth; SDL_Rect dst1 = { drawX1, 0, texW, texH }; display.drawTexture(tex, NULL, &dst1); int drawX2 = drawX1 + effectiveWidth; SDL_Rect dst2 = { drawX2, 0, texW, texH }; display.drawTexture(tex, NULL, &dst2); if (drawX2 + texW < windowW) { int drawX3 = drawX2 + effectiveWidth; SDL_Rect dst3 = { drawX3, 0, texW, texH }; display.drawTexture(tex, NULL, &dst3); } }; int bgW0=0,bgH0=0,effW0=0, bgW1=0,bgH1=0,effW1=0, bgW2=0,bgH2=0,effW2=0; if(bgTexture0_) { SDL_QueryTexture(bgTexture0_,0,0,&bgW0,&bgH0); effW0=bgW0*2/3; if(effW0<=0)effW0=bgW0;} if(bgTexture1_) { SDL_QueryTexture(bgTexture1_,0,0,&bgW1,&bgH1); effW1=bgW1*2/3; if(effW1<=0)effW1=bgW1;} if(bgTexture2_) { SDL_QueryTexture(bgTexture2_,0,0,&bgW2,&bgH2); effW2=bgW2*2/3; if(effW2<=0)effW2=bgW2;}
    drawTiledBg(bgTexture2_, bg_scroll_offset_2_, bgW2, bgH2, effW2, "Layer 2");
    drawTiledBg(bgTexture1_, bg_scroll_offset_1_, bgW1, bgH1, effW1, "Layer 1");

    SDL_Texture* currentTexture = partnerAnimator_.getCurrentTexture();
    SDL_Rect currentSourceRect = partnerAnimator_.getCurrentFrameRect();

    if (currentTexture && currentSourceRect.w > 0 && currentSourceRect.h > 0) {
        int drawX = (windowW / 2) - (currentSourceRect.w / 2);
        int verticalOffset = 7;
        int drawY = (windowH / 2) - (currentSourceRect.h / 2) - verticalOffset;
        SDL_Rect dstRect = { drawX, drawY, currentSourceRect.w, currentSourceRect.h };

        display.drawTexture(currentTexture, &currentSourceRect, &dstRect);
    }

    drawTiledBg(bgTexture0_, bg_scroll_offset_0_, bgW0, bgH0, effW0, "Layer 0");
}

StateType AdventureState::getType() const {
    return StateType::Adventure;
}