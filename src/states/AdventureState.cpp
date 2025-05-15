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
#include "utils/AnimationUtils.h"
#include "core/GameConstants.h"     // Added for game constants
#include "states/BattleState.h"       // Forward declare or include for BattleState later
#include <SDL_log.h>
#include <stdexcept>
#include <fstream>
#include <cstddef>
#include <vector>
#include <string>
#include <cmath>
#include <memory>
#include "core/gameconstants.h"

// Rate Limiting Constants
const float STEP_WINDOW_DURATION = 1.0f; // Time window in seconds
const int MAX_STEPS_PER_WINDOW = 4;      // Max steps allowed per window

// Idle Return Constant
const float TIME_TO_RETURN_TO_IDLE = 1.25f; // Seconds of no STEP input before stopping

// Battle Fade Constants
const float BATTLE_FADE_DURATION_SECONDS = 1.0f; // Duration of fade to battle in seconds

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
    stepsInWindow_(0),
    // Initialize new battle trigger members
    current_area_step_goal_(0),
    total_steps_taken_in_area_(0),
    current_area_enemy_id_("DefaultEnemy"), // Placeholder
    is_fading_to_battle_(false),
    battle_fade_alpha_(0.0f)
{
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Initializing...");
    
    if (!game_ptr || !game_ptr->getAssetManager() || !game_ptr->get_display() || 
        !game_ptr->getPlayerData() || !game_ptr->getAnimationManager()) {
        throw std::runtime_error("AdventureState: Missing required systems!");
    }

    PlayerData* pd = game_ptr->getPlayerData();
    current_digimon_ = pd->currentPartner;

    // Load data from the current map node
    AssetManager* assets = game_ptr->getAssetManager();
    
    // Get node data from player data
    const Digivice::NodeData& nodeData = pd->getCurrentNodeData();
    current_area_step_goal_ = nodeData.totalSteps > 0 ? nodeData.totalSteps : GameConstants::getCurrentChapterStepGoal();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Loaded node '%s' with step goal: %d", 
                nodeData.name.c_str(), current_area_step_goal_);    // Load background textures from the node data
    if (!nodeData.adventureBackgroundLayers.empty()) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
            "DEBUG: NodeData '%s' has %zu background layers defined", 
            nodeData.id.c_str(), nodeData.adventureBackgroundLayers.size());
            
        // Load foreground texture variants (layer 0)
        if (nodeData.adventureBackgroundLayers.size() > 0) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                "DEBUG: Layer 0 has %zu texture paths", 
                nodeData.adventureBackgroundLayers[0].texturePaths.size());
                
            if (!nodeData.adventureBackgroundLayers[0].texturePaths.empty()) {
                std::string baseTexId0 = nodeData.id + "_bg_0";
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                    "DEBUG: Will load Layer 0 variants with base ID '%s'", baseTexId0.c_str());
                    
                loadTextureVariantsForLayer(bgLayer0_, nodeData.adventureBackgroundLayers[0].texturePaths, baseTexId0);
                
                // Set the first texture as the default for backward compatibility with BattleState
                if (!bgLayer0_.textures.empty()) {
                    bgTexture0_ = bgLayer0_.textures[0];
                    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                        "DEBUG: Set legacy bgTexture0_ = first variant of Layer 0");
                } else {
                    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                        "DEBUG: Layer 0 has no loaded textures!");
                }
            }
        }
        
        // Load midground texture variants (layer 1)
        if (nodeData.adventureBackgroundLayers.size() > 1) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                "DEBUG: Layer 1 has %zu texture paths", 
                nodeData.adventureBackgroundLayers[1].texturePaths.size());
                
            if (!nodeData.adventureBackgroundLayers[1].texturePaths.empty()) {
                std::string baseTexId1 = nodeData.id + "_bg_1";
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                    "DEBUG: Will load Layer 1 variants with base ID '%s'", baseTexId1.c_str());
                    
                loadTextureVariantsForLayer(bgLayer1_, nodeData.adventureBackgroundLayers[1].texturePaths, baseTexId1);
                
                // Set the first texture as the default for backward compatibility with BattleState
                if (!bgLayer1_.textures.empty()) {
                    bgTexture1_ = bgLayer1_.textures[0];
                    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                        "DEBUG: Set legacy bgTexture1_ = first variant of Layer 1");
                } else {
                    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                        "DEBUG: Layer 1 has no loaded textures!");
                }
            }
        }
        
        // Load background texture variants (layer 2)
        if (nodeData.adventureBackgroundLayers.size() > 2) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                "DEBUG: Layer 2 has %zu texture paths", 
                nodeData.adventureBackgroundLayers[2].texturePaths.size());
                
            if (!nodeData.adventureBackgroundLayers[2].texturePaths.empty()) {
                std::string baseTexId2 = nodeData.id + "_bg_2";
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                    "DEBUG: Will load Layer 2 variants with base ID '%s'", baseTexId2.c_str());
                    
                loadTextureVariantsForLayer(bgLayer2_, nodeData.adventureBackgroundLayers[2].texturePaths, baseTexId2);
                
                // Set the first texture as the default for backward compatibility with BattleState
                if (!bgLayer2_.textures.empty()) {
                    bgTexture2_ = bgLayer2_.textures[0];
                    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                        "DEBUG: Set legacy bgTexture2_ = first variant of Layer 2");
                } else {
                    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                        "DEBUG: Layer 2 has no loaded textures!");
                }
            }
        }
    } else {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
            "DEBUG: NodeData '%s' has NO background layers defined", 
            nodeData.id.c_str());
    }
      // Fallback to default backgrounds if loading fails
    if (!bgTexture0_) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load foreground from node data, using default");
        bgTexture0_ = assets->getTexture("castle_bg_0");
    }
    if (!bgTexture1_) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load midground from node data, using default");
        bgTexture1_ = assets->getTexture("castle_bg_1");
    }
    if (!bgTexture2_) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load background from node data, using default");
        bgTexture2_ = assets->getTexture("castle_bg_2");
    }
    
    // Load initial animation for the partner Digimon
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
        current_state_ = STATE_IDLE; // Reset state to idle
        queued_steps_ = 0;           // Reset steps

        // Update the animator with the new partner's idle animation
        std::string idleAnimId = AnimationUtils::GetAnimationId(current_digimon_, "Idle");
        AnimationManager* animManager = game_ptr->getAnimationManager();
        const AnimationData* idleAnimData = animManager->getAnimationData(idleAnimId);
        partnerAnimator_.setAnimation(idleAnimData);

        if (!idleAnimData) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AdventureState::enter - Failed to set animation for new partner %d", (int)current_digimon_);
        } else { /* Intentional empty else based on provided code */ }
    }
    firstWalkUpdate_ = true;

    // Reset fade state upon entering AdventureState
    is_fading_to_battle_ = false;
    battle_fade_alpha_ = 0.0f;
}


// --- handle_input ---
void AdventureState::handle_input(InputManager& inputManager, PlayerData* playerData) {
    if (!game_ptr || game_ptr->getCurrentState() != this) return;    // Menu Activation
    if (inputManager.isActionJustPressed(GameAction::CONFIRM)) {
        SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "AdventureState: Confirm action. Requesting fade to MenuState.");
        std::vector<std::string> mainMenuItems = {"DIGIMON", "MAP", "SAVE", "SETTINGS", "EXIT"};
        auto menuState = std::make_unique<MenuState>(game_ptr, mainMenuItems);
        game_ptr->requestFadeToState(std::move(menuState), 0.3f, false); // Fade for 0.3s, pop AdventureState
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
            queued_steps_++; // Actually queue the step to be processed
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

    // If fading to battle, primarily handle fade logic and skip most other updates.
    if (is_fading_to_battle_) {
        battle_fade_alpha_ += (255.0f / BATTLE_FADE_DURATION_SECONDS) * delta_time;
        if (battle_fade_alpha_ >= 255.0f) {
            battle_fade_alpha_ = 255.0f; // Cap alpha at 255
            // Transition to BattleState
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Fade to black complete. Requesting BattleState.");
            
            PlayerData* pd = game_ptr->getPlayerData();
            if (!pd) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: PlayerData is null, cannot start battle.");
                is_fading_to_battle_ = false; // Reset fade as battle cannot start
                battle_fade_alpha_ = 0.0f;
                total_steps_taken_in_area_ = 0; 
                return;
            }

            // Calculate additional scroll distances for battle transition
            float additional_scroll_0 = SCROLL_SPEED_0 * BATTLE_TRANSITION_SCROLL_ADVANCE_SECONDS;
            float additional_scroll_1 = SCROLL_SPEED_1 * BATTLE_TRANSITION_SCROLL_ADVANCE_SECONDS;
            float additional_scroll_2 = SCROLL_SPEED_2 * BATTLE_TRANSITION_SCROLL_ADVANCE_SECONDS;

            // Calculate new battle starting offsets (raw, before wrapping)
            // Subtract because scroll offsets decrease as the player moves "forward" (right)
            float battle_start_offset_0_raw = bg_scroll_offset_0_ - additional_scroll_0;
            float battle_start_offset_1_raw = bg_scroll_offset_1_ - additional_scroll_1;
            float battle_start_offset_2_raw = bg_scroll_offset_2_ - additional_scroll_2;

            // Get effective widths for wrapping
            int effW0 = 0, effW1 = 0, effW2 = 0;
            if (bgTexture0_) { int w; SDL_QueryTexture(bgTexture0_, NULL, NULL, &w, NULL); effW0 = w * 2/3; if (effW0 <= 0) effW0 = w; }
            if (bgTexture1_) { int w; SDL_QueryTexture(bgTexture1_, NULL, NULL, &w, NULL); effW1 = w * 2/3; if (effW1 <= 0) effW1 = w; }
            if (bgTexture2_) { int w; SDL_QueryTexture(bgTexture2_, NULL, NULL, &w, NULL); effW2 = w * 2/3; if (effW2 <= 0) effW2 = w; }

            // Wrap the new offsets
            float battle_start_offset_0 = battle_start_offset_0_raw;
            float battle_start_offset_1 = battle_start_offset_1_raw;
            float battle_start_offset_2 = battle_start_offset_2_raw;

            if (effW0 > 0) battle_start_offset_0 = std::fmod(battle_start_offset_0_raw + effW0, (float)effW0);
            if (effW1 > 0) battle_start_offset_1 = std::fmod(battle_start_offset_1_raw + effW1, (float)effW1);
            if (effW2 > 0) battle_start_offset_2 = std::fmod(battle_start_offset_2_raw + effW2, (float)effW2);

            game_ptr->requestPushState(std::make_unique<BattleState>(
                game_ptr, 
                pd->currentPartner, 
                current_area_enemy_id_,
                bgTexture0_, 
                bgTexture1_, 
                bgTexture2_, 
                battle_start_offset_0, // Pass wrapped offset
                battle_start_offset_1, // Pass wrapped offset
                battle_start_offset_2  // Pass wrapped offset
            ));
            
            total_steps_taken_in_area_ = 0; // Reset steps for the area
            // DO NOT set is_fading_to_battle_ to false here.
            // Let AdventureState::enter() handle resetting the fade status
            // when this state becomes active again.
            // battle_fade_alpha_ remains 255.0f to keep the screen black.
        }
        return; // Don't process other game logic while fading to battle
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
            current_state_ = STATE_IDLE;
            queued_steps_ = 0; // Clear queue when stopping
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Update: Returning to IDLE due to inactivity.");
            needs_new_animation_set = true;
        }
    }

    // Process queued steps
    if (current_state_ == STATE_WALKING && queued_steps_ > 0) {
        // This is where a step is "taken"
        queued_steps_--; // Consume one step from the queue
        if (playerData) {
            playerData->totalSteps++; // Increment global player steps
        }
        total_steps_taken_in_area_++; // Increment steps for this area's battle trigger
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Step taken. Total in area: %d/%d", total_steps_taken_in_area_, current_area_step_goal_);

        // Reset idle timer as a step was just processed
        timeSinceLastStep_ = 0.0f;
        firstWalkUpdate_ = false; // A step has been taken, no longer the "first walk update"
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
    if (playerData && !is_fading_to_battle_ && total_steps_taken_in_area_ >= current_area_step_goal_) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Step goal reached! Initiating fade to battle.");
        is_fading_to_battle_ = true;
        battle_fade_alpha_ = 0.0f; // Start fade from transparent
        // current_state_ = STATE_IDLE; // Optionally stop walking animation
        // queued_steps_ = 0;
    }    // Scroll Background (Only if walking and not fading to battle)
    if (current_state_ == STATE_WALKING && !is_fading_to_battle_) {
        float scrollAmount0 = SCROLL_SPEED_0 * delta_time;
        float scrollAmount1 = SCROLL_SPEED_1 * delta_time;
        float scrollAmount2 = SCROLL_SPEED_2 * delta_time;        // DEBUG: Log scroll status periodically
        static int frameCounter = 0;
        frameCounter = (frameCounter + 1) % 120;  // Log every 120 frames
        bool shouldLog = (frameCounter == 0);
        
        if (shouldLog) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                "DEBUG: Background scroll update - Layer 0: %.2f, Layer 1: %.2f, Layer 2: %.2f",
                bg_scroll_offset_0_, bg_scroll_offset_1_, bg_scroll_offset_2_);
                
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                "DEBUG: Layer variants status - Layer 0: %zu textures, Layer 1: %zu textures, Layer 2: %zu textures",
                bgLayer0_.textures.size(), bgLayer1_.textures.size(), bgLayer2_.textures.size());
                
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                "DEBUG: Current variant indices - Layer 0: %zu/%zu, Layer 1: %zu/%zu, Layer 2: %zu/%zu",
                bgLayer0_.currentVariantIndex, bgLayer0_.textures.size() > 0 ? bgLayer0_.textures.size() : 0,
                bgLayer1_.currentVariantIndex, bgLayer1_.textures.size() > 0 ? bgLayer1_.textures.size() : 0,
                bgLayer2_.currentVariantIndex, bgLayer2_.textures.size() > 0 ? bgLayer2_.textures.size() : 0);
        }
        
        // Update layer 0 (foreground)
        if (!bgLayer0_.textures.empty()) {
            size_t currentIdx = bgLayer0_.currentVariantIndex;
            int effW0 = bgLayer0_.effectiveWidths[currentIdx];
            
            // Update scroll position
            float previousOffset = bg_scroll_offset_0_;
            bg_scroll_offset_0_ -= scrollAmount0;
              // Check if we've scrolled past the edge of the current variant
            // We check if we've scrolled from less than the effective width to more than the effective width
            if ((int)previousOffset % effW0 > (int)bg_scroll_offset_0_ % effW0) {
                // Advance to the next variant
                bgLayer0_.currentVariantIndex = (currentIdx + 1) % bgLayer0_.textures.size();
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, 
                    "Switching to next variant for layer 0: %zu", 
                    bgLayer0_.currentVariantIndex);
            }
            
            // Wrap the scroll offset
            bg_scroll_offset_0_ = std::fmod(bg_scroll_offset_0_ + effW0, (float)effW0);
        } else if (bgTexture0_) {
            // Legacy scrolling for single texture
            int w;
            SDL_QueryTexture(bgTexture0_, nullptr, nullptr, &w, nullptr);
            int effW0 = w * 2/3;
            if (effW0 <= 0) effW0 = w;
            
            bg_scroll_offset_0_ -= scrollAmount0;
            bg_scroll_offset_0_ = std::fmod(bg_scroll_offset_0_ + effW0, (float)effW0);
            
            if (shouldLog) {
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                    "DEBUG: Layer 0 using legacy single texture scrolling (no variants)");
            }
        }
        
        // Update layer 1 (middleground)
        if (!bgLayer1_.textures.empty()) {
            size_t currentIdx = bgLayer1_.currentVariantIndex;
            int effW1 = bgLayer1_.effectiveWidths[currentIdx];
            
            // Update scroll position
            float previousOffset = bg_scroll_offset_1_;
            bg_scroll_offset_1_ -= scrollAmount1;
              // Check if we've scrolled past the edge of the current variant
            if ((int)previousOffset % effW1 > (int)bg_scroll_offset_1_ % effW1) {
                // Advance to the next variant
                bgLayer1_.currentVariantIndex = (currentIdx + 1) % bgLayer1_.textures.size();
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, 
                    "Switching to next variant for layer 1: %zu", 
                    bgLayer1_.currentVariantIndex);
            }
            
            // Wrap the scroll offset
            bg_scroll_offset_1_ = std::fmod(bg_scroll_offset_1_ + effW1, (float)effW1);
        } else if (bgTexture1_) {
            // Legacy scrolling for single texture
            int w;
            SDL_QueryTexture(bgTexture1_, nullptr, nullptr, &w, nullptr);
            int effW1 = w * 2/3;
            if (effW1 <= 0) effW1 = w;
            
            bg_scroll_offset_1_ -= scrollAmount1;
            bg_scroll_offset_1_ = std::fmod(bg_scroll_offset_1_ + effW1, (float)effW1);
            
            if (shouldLog) {
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                    "DEBUG: Layer 1 using legacy single texture scrolling (no variants)");
            }
        }
        
        // Update layer 2 (background)
        if (!bgLayer2_.textures.empty()) {
            size_t currentIdx = bgLayer2_.currentVariantIndex;
            int effW2 = bgLayer2_.effectiveWidths[currentIdx];
            
            // Update scroll position
            float previousOffset = bg_scroll_offset_2_;
            bg_scroll_offset_2_ -= scrollAmount2;
              // Check if we've scrolled past the edge of the current variant
            if ((int)previousOffset % effW2 > (int)bg_scroll_offset_2_ % effW2) {
                // Advance to the next variant
                bgLayer2_.currentVariantIndex = (currentIdx + 1) % bgLayer2_.textures.size();
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, 
                    "Switching to next variant for layer 2: %zu", 
                    bgLayer2_.currentVariantIndex);
            }
            
            // Wrap the scroll offset
            bg_scroll_offset_2_ = std::fmod(bg_scroll_offset_2_ + effW2, (float)effW2);
        } else if (bgTexture2_) {
            // Legacy scrolling for single texture
            int w;
            SDL_QueryTexture(bgTexture2_, nullptr, nullptr, &w, nullptr);
            int effW2 = w * 2/3;
            if (effW2 <= 0) effW2 = w;
            
            bg_scroll_offset_2_ -= scrollAmount2;
            bg_scroll_offset_2_ = std::fmod(bg_scroll_offset_2_ + effW2, (float)effW2);
            
            if (shouldLog) {
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                    "DEBUG: Layer 2 using legacy single texture scrolling (no variants)");
            }
        }
    }

    SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "Update End: State=%d, QueuedSteps=%d", 
                   current_state_, queued_steps_);
}


// --- render ---
void AdventureState::render(PCDisplay& display) {
    // Add debugging to verify rendering is happening
    // static int frameCount = 0; // Commented out
    // if (frameCount % 60 == 0) { // Log every 60 frames to avoid spamming // Commented out
    //     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState rendering frame: %d", frameCount); // Commented out
    // } // Commented out
    // frameCount++; // Commented out

    SDL_Renderer* renderer = display.getRenderer();
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AdventureState::render - Renderer is NULL!");
        return;
    }

    // Clear the screen (this should ideally be a common color or handled by a specific background)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Clear to black
    SDL_RenderClear(renderer);

    const int windowW = GameConstants::WINDOW_WIDTH; 
    const int windowH = GameConstants::WINDOW_HEIGHT; 

    auto drawTiledBg = [&](SDL_Texture* tex, float offset, int texW, int texH, int effectiveWidth, const char* layerName) { 
        if (!tex || texW <= 0 || effectiveWidth <= 0) { 
            return; 
        } 
        int drawX1 = -static_cast<int>(std::fmod(offset, (float)effectiveWidth)); 
        if (drawX1 > 0) drawX1 -= effectiveWidth; 
        SDL_Rect dst1 = { drawX1, 0, texW, texH }; 
        display.drawTexture(tex, NULL, &dst1); 
        
        int drawX2 = drawX1 + effectiveWidth; 
        SDL_Rect dst2 = { drawX2, 0, texW, texH }; 
        display.drawTexture(tex, NULL, &dst2); 
        
        if (drawX2 + texW < windowW) { 
            int drawX3 = drawX2 + effectiveWidth; 
            SDL_Rect dst3 = { drawX3, 0, texW, texH }; 
            display.drawTexture(tex, NULL, &dst3); 
        } 
    };     // DEBUG: Add static counter to limit logging frequency in render
    static int renderFrameCounter = 0;
    renderFrameCounter = (renderFrameCounter + 1) % 60;  // Log every 60 frames
    bool shouldLogRender = (renderFrameCounter == 0);
    
    // Get dimensions for legacy textures if needed
    int bgW0=0, bgH0=0, effW0=0, bgW1=0, bgH1=0, effW1=0, bgW2=0, bgH2=0, effW2=0;
    if(bgTexture0_) { SDL_QueryTexture(bgTexture0_, nullptr, nullptr, &bgW0, &bgH0); effW0=bgW0*2/3; if(effW0<=0) effW0=bgW0; }
    if(bgTexture1_) { SDL_QueryTexture(bgTexture1_, nullptr, nullptr, &bgW1, &bgH1); effW1=bgW1*2/3; if(effW1<=0) effW1=bgW1; }
    if(bgTexture2_) { SDL_QueryTexture(bgTexture2_, nullptr, nullptr, &bgW2, &bgH2); effW2=bgW2*2/3; if(effW2<=0) effW2=bgW2; }

    if (shouldLogRender) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
            "DEBUG: RENDER - Layer variants count - L0: %zu, L1: %zu, L2: %zu", 
            bgLayer0_.textures.size(), bgLayer1_.textures.size(), bgLayer2_.textures.size());
    }

    // Background layer (layer 2 - furthest back)
    if (!bgLayer2_.textures.empty()) {
        if (shouldLogRender) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                "DEBUG: Rendering Layer 2 with variant system (%zu textures, current: %zu)", 
                bgLayer2_.textures.size(), bgLayer2_.currentVariantIndex);
        }
        renderBackgroundLayerVariants(display, bgLayer2_, bg_scroll_offset_2_);
    } else if (bgTexture2_) {
        if (shouldLogRender) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                "DEBUG: Rendering Layer 2 with legacy system (single texture)");
        }
        drawTiledBg(bgTexture2_, bg_scroll_offset_2_, bgW2, bgH2, effW2, "Layer 2");
    } else {
        if (shouldLogRender) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "DEBUG: No texture for Layer 2");
        }
    }
    
    // Midground layer (layer 1)
    if (!bgLayer1_.textures.empty()) {
        if (shouldLogRender) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                "DEBUG: Rendering Layer 1 with variant system (%zu textures, current: %zu)", 
                bgLayer1_.textures.size(), bgLayer1_.currentVariantIndex);
        }
        renderBackgroundLayerVariants(display, bgLayer1_, bg_scroll_offset_1_);
    } else if (bgTexture1_) {
        if (shouldLogRender) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                "DEBUG: Rendering Layer 1 with legacy system (single texture)");
        }
        drawTiledBg(bgTexture1_, bg_scroll_offset_1_, bgW1, bgH1, effW1, "Layer 1");
    } else {
        if (shouldLogRender) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "DEBUG: No texture for Layer 1");
        }
    }

    // Draw the character
    SDL_Texture* currentTexture = partnerAnimator_.getCurrentTexture();
    SDL_Rect currentSourceRect = partnerAnimator_.getCurrentFrameRect();

    if (currentTexture && currentSourceRect.w > 0 && currentSourceRect.h > 0) {
        int drawX = (windowW / 2) - (currentSourceRect.w / 2);
        int verticalOffset = 7; // This might need to be a constant or configurable
        int drawY = (windowH / 2) - (currentSourceRect.h / 2) - verticalOffset;
        SDL_Rect dstRect = { drawX, drawY, currentSourceRect.w, currentSourceRect.h };

        display.drawTexture(currentTexture, &currentSourceRect, &dstRect);
    }
    
    // Foreground layer (layer 0 - closest to the camera)
    if (!bgLayer0_.textures.empty()) {
        if (shouldLogRender) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                "DEBUG: Rendering Layer 0 with variant system (%zu textures, current: %zu)", 
                bgLayer0_.textures.size(), bgLayer0_.currentVariantIndex);
        }
        renderBackgroundLayerVariants(display, bgLayer0_, bg_scroll_offset_0_);
    } else if (bgTexture0_) {
        if (shouldLogRender) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                "DEBUG: Rendering Layer 0 with legacy system (single texture)");
        }
        drawTiledBg(bgTexture0_, bg_scroll_offset_0_, bgW0, bgH0, effW0, "Layer 0");
    } else {
        if (shouldLogRender) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "DEBUG: No texture for Layer 0");
        }
    }

    // Render fade_to_battle overlay if active
    if (is_fading_to_battle_ && battle_fade_alpha_ > 0.0f) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, static_cast<Uint8>(battle_fade_alpha_));
        SDL_Rect fullscreen_rect = {0, 0, GameConstants::WINDOW_WIDTH, GameConstants::WINDOW_HEIGHT};
        SDL_RenderFillRect(renderer, &fullscreen_rect);
        SDL_LogVerbose(SDL_LOG_CATEGORY_RENDER, "AdventureState: Rendering fade overlay with alpha: %.2f", battle_fade_alpha_);
    }
}

StateType AdventureState::getType() const {
    return StateType::Adventure;
}

// Helper method to load texture variants for a layer
void AdventureState::loadTextureVariantsForLayer(LayerVariants& layer, 
                                              const std::vector<std::string>& texturePaths, 
                                              const std::string& baseTextureId) {
    // Clear any existing data
    layer.textures.clear();
    layer.widths.clear();
    layer.effectiveWidths.clear();
    layer.currentVariantIndex = 0;
    
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
        "DEBUG: loadTextureVariantsForLayer - Loading %zu textures for layer %s", 
        texturePaths.size(), baseTextureId.c_str());
    
    if (texturePaths.empty()) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
            "DEBUG: loadTextureVariantsForLayer - No textures to load for %s", 
            baseTextureId.c_str());
        return;
    }
    
    AssetManager* assets = game_ptr->getAssetManager();
    if (!assets) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AdventureState::loadTextureVariantsForLayer - AssetManager is NULL!");
        return;
    }
    
    // Loop through each texture path and log what we're trying to load
    for (size_t i = 0; i < texturePaths.size(); i++) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
            "DEBUG: Attempting to load variant %zu for %s: '%s'", 
            i, baseTextureId.c_str(), texturePaths[i].c_str());
    }
    
    // Loop through each texture path and load it
    for (size_t i = 0; i < texturePaths.size(); i++) {
        const std::string& texturePath = texturePaths[i];
        std::string variantTexId = baseTextureId;
        
        // For variant textures (after the first one), append the variant index to the ID
        if (i > 0) {
            variantTexId += "_" + std::to_string(i);
        }
        
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
            "DEBUG: Loading texture variant %zu for %s - ID: '%s', Path: '%s'", 
            i, baseTextureId.c_str(), variantTexId.c_str(), texturePath.c_str());
            
        // Check if texture is already loaded
        SDL_Texture* tex = assets->getTexture(variantTexId);
        if (tex) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                "DEBUG: Texture '%s' already loaded in AssetManager", variantTexId.c_str());
        } else {
            // Try to load the texture
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                "DEBUG: Attempting to load texture '%s' from path '%s'", 
                variantTexId.c_str(), texturePath.c_str());
                
            if (assets->loadTexture(variantTexId, texturePath)) {
                tex = assets->getTexture(variantTexId);
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                    "DEBUG: Successfully loaded texture '%s'", variantTexId.c_str());
            } else {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                    "DEBUG: Failed to load texture '%s' from path '%s'", 
                    variantTexId.c_str(), texturePath.c_str());
            }
        }
        
        if (tex) {
            // Add texture to the layer's variants
            layer.textures.push_back(tex);
            
            // Get texture width and calculate effective width
            int width = 0, height = 0;
            SDL_QueryTexture(tex, nullptr, nullptr, &width, &height);
            layer.widths.push_back(width);
            
            // Set effective width to 2/3 of the actual width (for better scrolling effect)
            int effectiveWidth = width * 2/3;
            if (effectiveWidth <= 0) effectiveWidth = width;
            layer.effectiveWidths.push_back(effectiveWidth);
            
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, 
                "Loaded texture variant %d for layer %s: %dx%d (effective width: %d)", 
                (int)i, baseTextureId.c_str(), width, height, effectiveWidth);
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                "Failed to load texture variant %d for layer %s: %s", 
                (int)i, baseTextureId.c_str(), texturePath.c_str());
        }
    }
    
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, 
        "Loaded %d texture variants for layer %s", 
        (int)layer.textures.size(), baseTextureId.c_str());
}

// Helper to render a background layer with its variants
void AdventureState::renderBackgroundLayerVariants(PCDisplay& display, 
                                                const LayerVariants& layer, 
                                                float scrollOffset) {
    if (layer.textures.empty()) return;
    
    const int windowW = WINDOW_WIDTH;
    const int windowH = WINDOW_HEIGHT;
    
    // Get the current variant
    size_t variantIndex = layer.currentVariantIndex % layer.textures.size();
    SDL_Texture* currentTexture = layer.textures[variantIndex];
    if (!currentTexture) return;
    
    int texW = layer.widths[variantIndex];
    int texH = windowH; // Use window height for texture height
    int effectiveWidth = layer.effectiveWidths[variantIndex];
    
    // Calculate where to draw the current texture
    int drawX = -static_cast<int>(std::fmod(scrollOffset, (float)effectiveWidth));
    if (drawX > 0) drawX -= effectiveWidth;
    
    // Draw the current texture
    SDL_Rect dst = { drawX, 0, texW, texH };
    display.drawTexture(currentTexture, nullptr, &dst);
    
    // Draw the next variant (or wrap back to the first if this is the last)
    size_t nextVariantIndex = (variantIndex + 1) % layer.textures.size();
    SDL_Texture* nextTexture = layer.textures[nextVariantIndex];
    if (!nextTexture) return;
    
    int nextTexW = layer.widths[nextVariantIndex];
    int nextEffectiveWidth = layer.effectiveWidths[nextVariantIndex];
    
    // Draw the next texture after the current one
    int drawNextX = drawX + effectiveWidth;
    SDL_Rect dstNext = { drawNextX, 0, nextTexW, texH };
    display.drawTexture(nextTexture, nullptr, &dstNext);
    
    // Draw a third texture if needed (could be the first variant again)
    if (drawNextX + nextTexW < windowW) {
        size_t thirdVariantIndex = (nextVariantIndex + 1) % layer.textures.size();
        SDL_Texture* thirdTexture = layer.textures[thirdVariantIndex];
        if (!thirdTexture) return;
        
        int thirdTexW = layer.widths[thirdVariantIndex];
        int drawThirdX = drawNextX + nextEffectiveWidth;
        SDL_Rect dstThird = { drawThirdX, 0, thirdTexW, texH };
        display.drawTexture(thirdTexture, nullptr, &dstThird);
    }
}