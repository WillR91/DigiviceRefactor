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
#include "Utils/RenderUtils.h"        // Added for sprite scaling utilities
#include "entities/DigimonRegistry.h" // <<< ADDED for DigimonRegistry access
#include "Core/BackgroundVariantManager.h" // Added for new variant background system
#include <SDL_log.h>
#include <stdexcept>
#include <fstream>
#include <cstddef>
#include <vector>
#include <string>
#include <iostream>
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
    current_state_(STATE_IDLE),
    queued_steps_(0),
    firstWalkUpdate_(true),
    bg_scroll_offset_0_(0.0f),
    bg_scroll_offset_1_(0.0f),
    bg_scroll_offset_2_(0.0f),
    timeSinceLastStep_(0.0f),
    stepWindowTimer_(0.0f),
    stepsInWindow_(0),
    // Initialize battle trigger members
    current_area_step_goal_(0),
    total_steps_taken_in_area_(0),
    current_area_enemy_id_("DefaultEnemy"), // Placeholder
    is_fading_to_battle_(false),
    battle_fade_alpha_(0.0f),
    bgTexture0_(nullptr),
    bgTexture1_(nullptr),
    bgTexture2_(nullptr),
    current_partner_definition_(nullptr)
{
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Initializing...");
    
    if (!game_ptr || !game_ptr->getAssetManager() || !game_ptr->get_display() || 
        !game_ptr->getPlayerData() || !game_ptr->getAnimationManager() || !game_ptr->getDigimonRegistry()) { // <<< ADDED getDigimonRegistry() check
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Missing required game systems!");
        // Consider throwing an exception or setting an error state
        return;
    }

    PlayerData* pd = game_ptr->getPlayerData();
    Digimon::DigimonRegistry* registry = game_ptr->getDigimonRegistry(); // <<< GET REGISTRY
    const Digimon::DigimonDefinition* partnerDef = pd->getCurrentPartnerDefinition(registry); // <<< GET PARTNER DEFINITION

    if (!partnerDef) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Could not get partner definition for ID: %s", pd->getCurrentPartnerId().c_str());
        // Handle error: maybe default to a known Digimon or throw
        return;
    }
    current_partner_definition_ = partnerDef;

    // Load data from the current map node
    AssetManager* assets = game_ptr->getAssetManager();
    
    // Get node data from player data
    const Digivice::NodeData& nodeData = pd->getCurrentNodeData();
    current_area_step_goal_ = nodeData.totalSteps > 0 ? nodeData.totalSteps : GameConstants::getCurrentChapterStepGoal();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Loaded node '%s' with step goal: %d", 
                nodeData.name.c_str(), current_area_step_goal_);
    
    // Load background textures from the node data
    if (!nodeData.adventureBackgroundLayers.empty()) {
        // Load foreground texture (layer 0)
        if (nodeData.adventureBackgroundLayers.size() > 0 && !nodeData.adventureBackgroundLayers[0].texturePaths.empty()) {
            std::string bgTexId0 = nodeData.id + "_bg_0";
            if (!assets->getTexture(bgTexId0)) {
                if (assets->loadTexture(bgTexId0, nodeData.adventureBackgroundLayers[0].texturePaths[0])) {
                    bgTexture0_ = assets->getTexture(bgTexId0);
                }
            } else {
                bgTexture0_ = assets->getTexture(bgTexId0);
            }
        }
        
        // Load midground texture (layer 1)
        if (nodeData.adventureBackgroundLayers.size() > 1 && !nodeData.adventureBackgroundLayers[1].texturePaths.empty()) {
            std::string bgTexId1 = nodeData.id + "_bg_1";
            if (!assets->getTexture(bgTexId1)) {
                if (assets->loadTexture(bgTexId1, nodeData.adventureBackgroundLayers[1].texturePaths[0])) {
                    bgTexture1_ = assets->getTexture(bgTexId1);
                }
            } else {
                bgTexture1_ = assets->getTexture(bgTexId1);
            }
        }
        
        // Load background texture (layer 2)
        if (nodeData.adventureBackgroundLayers.size() > 2 && !nodeData.adventureBackgroundLayers[2].texturePaths.empty()) {
            std::string bgTexId2 = nodeData.id + "_bg_2";
            if (!assets->getTexture(bgTexId2)) {
                if (assets->loadTexture(bgTexId2, nodeData.adventureBackgroundLayers[2].texturePaths[0])) {
                    bgTexture2_ = assets->getTexture(bgTexId2);
                }
            } else {
                bgTexture2_ = assets->getTexture(bgTexId2);
            }
        }
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
    // Use partnerDef->spriteBaseId and "Idle"
    std::string initialAnimId = AnimationUtils::GetAnimationId(partnerDef->spriteBaseId, "Idle");
    AnimationManager* animManager = game_ptr->getAnimationManager();
    const AnimationData* initialAnimData = animManager->getAnimationData(initialAnimId);
    partnerAnimator_.setAnimation(initialAnimData);
    
    // Try loading backgrounds using the new variant system
    // Extract environment path from the first background layer
    if (!nodeData.adventureBackgroundLayers.empty() && 
        !nodeData.adventureBackgroundLayers[0].texturePaths.empty()) {
        loadBackgroundVariants(nodeData.adventureBackgroundLayers[0].texturePaths[0]);
    }
}


// --- Destructor ---
AdventureState::~AdventureState() { SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState Destructor called."); }


// --- enter ---
void AdventureState::enter() {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Entering state");
    if (!game_ptr || !game_ptr->getPlayerData() || !game_ptr->getAnimationManager() || !game_ptr->getDigimonRegistry()) { // <<< ADDED getDigimonRegistry() check
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AdventureState::enter() - Missing required game systems!");
        return;
    }

    PlayerData* playerData = game_ptr->getPlayerData();
    Digimon::DigimonRegistry* registry = game_ptr->getDigimonRegistry(); // <<< GET REGISTRY
    const Digimon::DigimonDefinition* partnerDef = playerData->getCurrentPartnerDefinition(registry); // <<< GET PARTNER DEFINITION

    if (!partnerDef) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AdventureState::enter(): Could not get partner definition for ID: %s", playerData->getCurrentPartnerId().c_str());
        return;
    }    current_partner_definition_ = partnerDef;
    
    // Ensure animation is set based on current state (e.g., Idle)
    std::string currentAnimSuffix = getAnimationIdForCurrentState(); // Assuming this returns "Idle" or "Walk"
    std::string animId = AnimationUtils::GetAnimationId(partnerDef->spriteBaseId, currentAnimSuffix);
    const AnimationData* animData = game_ptr->getAnimationManager()->getAnimationData(animId);
    partnerAnimator_.setAnimation(animData);


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
        std::vector<std::string> mainMenuItems = {"DIGIMON", "MAP", "SAVE", "SETTINGS", "DEBUG", "EXIT"};
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
        if (queued_steps_ < MAX_QUEUED_STEPS && stepsInWindow_ < MAX_STEPS_PER_WINDOW) {
            queued_steps_++;
            stepsInWindow_++;
            timeSinceLastStep_ = 0.0f; // Reset idle timer on step input
            SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "AdventureState: Step registered. Queued: %d, Window: %d", queued_steps_, stepsInWindow_);
        } else {
            SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "AdventureState: Step NOT registered. Queued: %d (Max: %d), Window: %d (Max: %d)", 
                         queued_steps_, MAX_QUEUED_STEPS, stepsInWindow_, MAX_STEPS_PER_WINDOW);
        }    }    

    // Debug Key Check - removed - for changing Digimon to use IDs now
    // This functionality can be reimplemented using DigimonDefinition IDs
    // if needed in the future
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
            battle_fade_alpha_ = 255.0f;
            
            // --- Transition to BattleState ---
            PlayerData* pd = game_ptr->getPlayerData();
            Digimon::DigimonRegistry* registry = game_ptr->getDigimonRegistry();
            const Digimon::DigimonDefinition* playerDef = pd->getCurrentPartnerDefinition(registry);

            if (!playerDef) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Cannot start battle, player definition is null!");
                is_fading_to_battle_ = false; // Abort fade
                battle_fade_alpha_ = 0.0f;
                return;
            }

            // For now, enemy ID is hardcoded, later this will come from map data or encounter logic
            std::string enemyId = current_area_enemy_id_; 
            const Digimon::DigimonDefinition* enemyDef = registry->getDefinitionById(enemyId);
            if (!enemyDef) {
                 SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Enemy ID '%s' not found in registry for battle. Defaulting to Kuwagamon.", enemyId.c_str());
                 enemyDef = registry->getDefinitionById("kuwagamon"); // Fallback
                 if (!enemyDef) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Fallback enemy 'kuwagamon' also not found. Cannot start battle.");
                    is_fading_to_battle_ = false; // Abort fade
                    battle_fade_alpha_ = 0.0f;
                    return;
                 }            }
            
            // Pass the legacy DigimonType for now, or update BattleState to take IDs/Definitions            
            game_ptr->requestPushState(std::make_unique<BattleState>(
                game_ptr, 
                playerDef->legacyEnum, // Need to pass the legacy enum since BattleState still uses it
                enemyDef->id,          // Pass enemy ID
                bgTexture0_, bgTexture1_, bgTexture2_,
                bg_scroll_offset_0_, bg_scroll_offset_1_, bg_scroll_offset_2_
            ));
            // The fade out is complete, BattleState will handle its own fade in or entry.
            // No need to reset is_fading_to_battle_ here as the state will change.
            return; // Important: return to avoid further updates in this state
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
            needs_new_animation_set = true;
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Returned to IDLE due to inactivity.");
        }
    }

    // Process queued steps
    if (queued_steps_ > 0) {
        if (current_state_ == STATE_IDLE) {
            current_state_ = STATE_WALKING;
            needs_new_animation_set = true;
            firstWalkUpdate_ = true; // Reset for the new walking sequence
        }
        
        // Update player data for steps taken
        playerData->stepsTakenThisChapter++;
        playerData->totalSteps++;
        total_steps_taken_in_area_++; // Increment steps in current area

        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Step taken. Chapter: %d, Total: %d, Area: %d (Goal: %d)", 
            playerData->stepsTakenThisChapter, playerData->totalSteps, total_steps_taken_in_area_, current_area_step_goal_);

        // Check for battle trigger
        if (total_steps_taken_in_area_ >= current_area_step_goal_ && current_area_step_goal_ > 0) {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Battle triggered! Steps: %d, Goal: %d", 
                        total_steps_taken_in_area_, current_area_step_goal_);
            is_fading_to_battle_ = true; // Start fading to battle
            battle_fade_alpha_ = 0.0f;   // Reset fade alpha
            total_steps_taken_in_area_ = 0; // Reset steps for the area after triggering
            // The actual transition to BattleState will happen when fade is complete.
        }

        queued_steps_--;
    }    // Update Animator
    partnerAnimator_.update(delta_time);

    // Determine required animation based on current state    
    std::string requiredAnimSuffix;
    if (current_state_ == STATE_IDLE) {
        requiredAnimSuffix = "Idle";
    } else { // current_state_ == STATE_WALKING
        requiredAnimSuffix = "WalkLoop"; // Use the looping version for walking
    }
    
    // Get the current partner definition and use its spriteBaseId for animations
    Digimon::DigimonRegistry* registry = game_ptr->getDigimonRegistry();
    const Digimon::DigimonDefinition* partnerDef = playerData->getCurrentPartnerDefinition(registry);
    if (!partnerDef) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AdventureState::update - Cannot get partner definition!");
        return;
    }
    
    std::string requiredAnimId = AnimationUtils::GetAnimationId(partnerDef->spriteBaseId, requiredAnimSuffix);
    
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
    }

    // Scroll Background (Only if walking and not fading to battle)
    if (current_state_ == STATE_WALKING && !is_fading_to_battle_) {
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

    // Get screen dimensions
    int windowW = 0, windowH = 0;
    display.getWindowSize(windowW, windowH);

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
    }; 

    // Query texture dimensions for background rendering
    int bgW0=0,bgH0=0,effW0=0, bgW1=0,bgH1=0,effW1=0, bgW2=0,bgH2=0,effW2=0;
    if(bgTexture0_) { SDL_QueryTexture(bgTexture0_,0,0,&bgW0,&bgH0); effW0=bgW0*2/3; if(effW0<=0)effW0=bgW0;}
    if(bgTexture1_) { SDL_QueryTexture(bgTexture1_,0,0,&bgW1,&bgH1); effW1=bgW1*2/3; if(effW1<=0)effW1=bgW1;}
    if(bgTexture2_) { SDL_QueryTexture(bgTexture2_,0,0,&bgW2,&bgH2); effW2=bgW2*2/3; if(effW2<=0)effW2=bgW2;}

    // Draw background layers (back to front)
    drawTiledBg(bgTexture2_, bg_scroll_offset_2_, bgW2, bgH2, effW2, "Layer 2");
    drawTiledBg(bgTexture1_, bg_scroll_offset_1_, bgW1, bgH1, effW1, "Layer 1");    
    drawTiledBg(bgTexture0_, bg_scroll_offset_0_, bgW0, bgH0, effW0, "Layer 0");

    // Render the partner Digimon sprite
    SDL_Texture* currentTexture = partnerAnimator_.getCurrentTexture();
    SDL_Rect currentSourceRect = partnerAnimator_.getCurrentFrameRect();
    
    if (currentTexture && currentSourceRect.w > 0 && currentSourceRect.h > 0) {
        // Calculate the position for the scaled sprite
        // Note: We need to center the scaled sprite, not the original source size
        int scaledWidth = static_cast<int>(currentSourceRect.w * Constants::SPRITE_SCALE_FACTOR);
        int scaledHeight = static_cast<int>(currentSourceRect.h * Constants::SPRITE_SCALE_FACTOR);
        
        int drawX = (windowW / 2) - (scaledWidth / 2);
        int verticalOffset = 7; // This might need to be a constant or configurable
        int drawY = (windowH / 2) - (scaledHeight / 2) - verticalOffset;
        
        // Create scaled destination rectangle
        SDL_Rect dstRect = RenderUtils::ScaleDestRect(currentSourceRect, drawX, drawY);

        display.drawTexture(currentTexture, &currentSourceRect, &dstRect);
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

std::string AdventureState::getAnimationIdForCurrentState() const {
    // This function now needs the DigimonDefinition to get the spriteBaseId
    // However, its original purpose was just to return "Idle" or "Walk".
    // Let's keep it simple and assume the caller will combine it with spriteBaseId.
    switch (current_state_) {
        case STATE_WALKING: return "Walk";
        case STATE_IDLE:    
        default:            return "Idle";
    }
}

void AdventureState::loadBackgroundVariants(const std::string& environmentPath) {
    // This method implements the new variant-based background loading system
    // It attempts to load 1x scale assets with dynamic scaling
    
    AssetManager* assets = game_ptr->getAssetManager();
    
    // Map the current environment path to the new naming convention
    std::string environmentName = Digivice::BackgroundVariantManager::mapEnvironmentName(environmentPath);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, 
               "AdventureState: Loading variant backgrounds for environment: %s", 
               environmentName.c_str());
    
    // Create a temporary BackgroundLayerData to work with the variant system
    Digivice::BackgroundLayerData tempLayerData;
    Digivice::BackgroundVariantManager::initializeVariantsForNode(tempLayerData, environmentName);
    
    // Try to load foreground texture (1x scale asset)
    if (!tempLayerData.foregroundPaths.empty()) {
        std::string fgPath = Digivice::BackgroundVariantManager::getSelectedPath(
            tempLayerData.foregroundPaths, tempLayerData.selectedForegroundVariant);
        std::string fgTexId = environmentName + "_fg_v" + std::to_string(tempLayerData.selectedForegroundVariant + 1);
        
        if (assets->loadTexture(fgTexId, fgPath)) {
            bgTexture0_ = assets->getTexture(fgTexId);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Loaded foreground variant: %s", fgPath.c_str());
        }
    }
    
    // Try to load middleground texture (1x scale asset)
    if (!tempLayerData.middlegroundPaths.empty()) {
        std::string mgPath = Digivice::BackgroundVariantManager::getSelectedPath(
            tempLayerData.middlegroundPaths, tempLayerData.selectedMiddlegroundVariant);
        std::string mgTexId = environmentName + "_mg_v" + std::to_string(tempLayerData.selectedMiddlegroundVariant + 1);
        
        if (assets->loadTexture(mgTexId, mgPath)) {
            bgTexture1_ = assets->getTexture(mgTexId);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Loaded middleground variant: %s", mgPath.c_str());
        }
    }
    
    // Try to load background texture (1x scale asset)
    if (!tempLayerData.backgroundPaths.empty()) {
        std::string bgPath = Digivice::BackgroundVariantManager::getSelectedPath(
            tempLayerData.backgroundPaths, tempLayerData.selectedBackgroundVariant);
        std::string bgTexId = environmentName + "_bg_v" + std::to_string(tempLayerData.selectedBackgroundVariant + 1);
        
        if (assets->loadTexture(bgTexId, bgPath)) {
            bgTexture2_ = assets->getTexture(bgTexId);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Loaded background variant: %s", bgPath.c_str());
        }
    }
    
    // Note: The actual scaling will be handled in the render method
    // For now, we're just loading the 1x scale assets
}

void AdventureState::renderScaledBackgroundLayer(PCDisplay& display, SDL_Texture* texture, 
                                               int screenWidth, int screenHeight, 
                                               float globalScale, int layerIndex, 
                                               float scrollOffset) {
    if (!texture) return;
    
    // Get the 1x texture dimensions (new assets are 384×128)
    int originalWidth, originalHeight;
    SDL_QueryTexture(texture, nullptr, nullptr, &originalWidth, &originalHeight);
    
    // Calculate scaling to fill square display (384×128 → 466×466)
    float scaleX = static_cast<float>(screenWidth) / originalWidth;   // ~1.21x
    float scaleY = static_cast<float>(screenHeight) / originalHeight; // ~3.64x
    
    // Use larger scale to fill display completely (zoom to fill)
    float baseScale = std::max(scaleX, scaleY); // Will use ~3.64x for 1x assets
    
    // Apply user's global scale preference if provided
    float finalScale = baseScale * globalScale;
    
    // Calculate scaled dimensions
    int scaledWidth = static_cast<int>(originalWidth * finalScale);
    int scaledHeight = static_cast<int>(originalHeight * finalScale);
    
    // Calculate effective width for tiling (based on parallax factor)
    int effectiveWidth = scaledWidth;
    if (layerIndex == 0) {
        effectiveWidth = scaledWidth * 2/3; // Foreground scrolls fastest
    } else if (layerIndex == 1) {
        effectiveWidth = scaledWidth * 2/3; // Middleground moderate speed
    } else {
        effectiveWidth = scaledWidth * 2/3; // Background slowest
    }
    
    if (effectiveWidth <= 0) effectiveWidth = scaledWidth;
    
    // Apply scroll offset for parallax effect
    int scrollX = static_cast<int>(scrollOffset);
    
    // Calculate position to center vertically in the display
    int drawY = (screenHeight - scaledHeight) / 2;
    
    // Tile horizontally to fill the screen width
    int startX = scrollX % effectiveWidth;
    if (startX > 0) startX -= effectiveWidth;
    
    // Draw tiled instances
    for (int drawX = startX; drawX < screenWidth; drawX += effectiveWidth) {
        SDL_Rect destRect = { drawX, drawY, scaledWidth, scaledHeight };
        display.drawTexture(texture, nullptr, &destRect);
    }
    
    std::cout << "AdventureState: Rendered scaled layer " << layerIndex 
              << " - Original: " << originalWidth << "×" << originalHeight
              << ", Scaled: " << scaledWidth << "×" << scaledHeight 
              << ", Scale: " << finalScale << "x" << std::endl;
}