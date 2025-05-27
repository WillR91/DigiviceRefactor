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
#include <cmath>  // Added for std::fmod
#include "core/InputManager.h"
#include "core/GameAction.h"
#include "core/PlayerData.h"
#include "utils/AnimationUtils.h"
#include "core/GameConstants.h"     // Added for game constants
#include "states/BattleState.h"       // Forward declare or include for BattleState later
#include "Utils/RenderUtils.h"        // Added for sprite scaling utilities
#include "entities/DigimonRegistry.h" // <<< ADDED for DigimonRegistry access
#include "Core/BackgroundVariantManager.h" // Added for new variant background system
#include "graphics/SeamlessBackgroundRenderer.h" // Added for new background rendering
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
    previous_bg_scroll_offset_0_(0.0f),
    previous_bg_scroll_offset_1_(0.0f),
    previous_bg_scroll_offset_2_(0.0f),
    smooth_scroll_factor_(0.0f), // Initialize to 0 to prevent first-frame jump
    timeSinceLastStep_(0.0f),
    stepWindowTimer_(0.0f),
    stepsInWindow_(0),
    // Initialize battle trigger members
    current_area_step_goal_(0),
    total_steps_taken_in_area_(0),
    current_area_enemy_id_("DefaultEnemy"), // Placeholder
    is_fading_to_battle_(false),
    battle_fade_alpha_(0.0f),    bgTexture0_(nullptr),
    bgTexture1_(nullptr),
    bgTexture2_(nullptr),
    current_partner_definition_(nullptr),
    backgroundRenderer_(nullptr)
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
      // Legacy texture loading system removed - now handled by new variant system below
    
    // Load initial animation for the partner Digimon
    // Use partnerDef->spriteBaseId and "Idle"
    std::string initialAnimId = AnimationUtils::GetAnimationId(partnerDef->spriteBaseId, "Idle");
    AnimationManager* animManager = game_ptr->getAnimationManager();
    const AnimationData* initialAnimData = animManager->getAnimationData(initialAnimId);
    partnerAnimator_.setAnimation(initialAnimData);    // Try loading backgrounds using the new variant system
    // Check if this node uses the new variant system (has variant paths) or old system (has texture paths)
    if (!nodeData.adventureBackgroundLayers.empty()) {
        const auto& firstLayer = nodeData.adventureBackgroundLayers[0];
          // Check layer paths for proper background loading
          if (!firstLayer.foregroundPaths.empty() || !firstLayer.middlegroundPaths.empty() || !firstLayer.backgroundPaths.empty()) {            // New variant system - load variants directly
            loadBackgroundVariantsFromNodeData(nodeData.adventureBackgroundLayers[0], nodeData.id);
        } else if (!firstLayer.texturePaths.empty()) {            // Old system - map to new variant system
            loadBackgroundVariants(firstLayer.texturePaths[0]);
        } else {
            // No valid background paths found in layer data
        }
    } else {
        // No background layers found in node data
    }
      // Check final texture loading state for debugging if needed
      // Fallback to default backgrounds if loading fails
    if (!bgTexture0_) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load foreground from node data, using default");
        bgTexture0_ = assets->getTexture("castlebackground0");
        if (!bgTexture0_) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load fallback foreground: castlebackground0");
        } else {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Successfully loaded fallback foreground: castlebackground0");
        }
    }
    if (!bgTexture1_) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load midground from node data, using default");
        bgTexture1_ = assets->getTexture("castlebackground1");
        if (!bgTexture1_) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load fallback middleground: castlebackground1");
        } else {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Successfully loaded fallback middleground: castlebackground1");
        }
    }
    if (!bgTexture2_) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load background from node data, using default");
        bgTexture2_ = assets->getTexture("castlebackground2");
        if (!bgTexture2_) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load fallback background: castlebackground2");
        } else {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Successfully loaded fallback background: castlebackground2");
        }
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
    }    // Initialize SeamlessBackgroundRenderer
    PCDisplay* display = game_ptr->get_display();
    if (display) {
        backgroundRenderer_ = std::make_unique<SeamlessBackgroundRenderer>(display, display->getRenderer());
        
        // Debug: Check which textures are available
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Texture status - BG2: %s, BG1: %s, BG0: %s", 
                   bgTexture2_ ? "LOADED" : "NULL", 
                   bgTexture1_ ? "LOADED" : "NULL", 
                   bgTexture0_ ? "LOADED" : "NULL");
          // Add background layers from back to front (reverse order for proper layering)
        // NOTE: Negative scroll speeds make backgrounds move right when Digimon appears to move left
        if (bgTexture2_) {
            backgroundRenderer_->addLayer(bgTexture2_, -SCROLL_SPEED_2);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Added background layer (speed: %f)", -SCROLL_SPEED_2);
        }
        if (bgTexture1_) {
            backgroundRenderer_->addLayer(bgTexture1_, -SCROLL_SPEED_1);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Added middleground layer (speed: %f)", -SCROLL_SPEED_1);
        }        if (bgTexture0_) {
            backgroundRenderer_->addLayer(bgTexture0_, -SCROLL_SPEED_0);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Added foreground layer (speed: %f)", -SCROLL_SPEED_0);
        }          // Synchronize initial scroll positions to prevent first-frame jump
        // Instead of starting at 0, use a small initial offset for seamless appearance
        int screenWidth, screenHeight;
        game_ptr->get_display()->getWindowSize(screenWidth, screenHeight);
        float initialOffset = screenWidth * 0.1f; // Small offset to avoid edge artifacts
        
        // The layers are added in order: [0]=bgTexture2_, [1]=bgTexture1_, [2]=bgTexture0_
        if (backgroundRenderer_->getLayerCount() >= 1) {
            backgroundRenderer_->setLayerScrollPosition(0, initialOffset);
        }
        if (backgroundRenderer_->getLayerCount() >= 2) {
            backgroundRenderer_->setLayerScrollPosition(1, initialOffset);
        }
        if (backgroundRenderer_->getLayerCount() >= 3) {
            backgroundRenderer_->setLayerScrollPosition(2, initialOffset);
        }
        
        // Also update legacy scroll offsets to match
        bg_scroll_offset_0_ = initialOffset;
        bg_scroll_offset_1_ = initialOffset;
        bg_scroll_offset_2_ = initialOffset;
        
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: SeamlessBackgroundRenderer initialized with %zu layers", 
                   backgroundRenderer_->getLayerCount());
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AdventureState::enter() - Failed to get display for background renderer!");
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
    
    // Reset scroll offsets and smooth scrolling to prevent first-frame jump
    bg_scroll_offset_0_ = 0.0f;
    bg_scroll_offset_1_ = 0.0f;
    bg_scroll_offset_2_ = 0.0f;
    smooth_scroll_factor_ = 0.0f;
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
    }    // Scroll Background using SeamlessBackgroundRenderer (Only if walking and not fading to battle)
    if (current_state_ == STATE_WALKING && !is_fading_to_battle_ && backgroundRenderer_) {
        // Use consistent scroll speed from the first frame - no more smooth ramping
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "BackgroundUpdate: state=%d, deltaTime=%.6f, firstWalk=%s", 
                     current_state_, delta_time, firstWalkUpdate_ ? "true" : "false");
        backgroundRenderer_->updateScroll(delta_time);
        
        // Legacy scroll offsets are no longer updated since we use SeamlessBackgroundRenderer
        // The SeamlessBackgroundRenderer handles all scroll positioning internally
        
    } else {
        // Reset first walk flag when not walking
        if (current_state_ == STATE_IDLE) {
            firstWalkUpdate_ = true;
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
    display.getWindowSize(windowW, windowH);    auto drawTiledBg = [&](SDL_Texture* tex, float offset, int texW, int texH, int effectiveWidth, const char* layerName) { 
        if (!tex || texW <= 0 || effectiveWidth <= 0) { 
            return; 
        } 
          // Use proper float modulo for smooth scrolling (same fix as in scaled renderer)
        float normalizedOffset = std::fmod(offset, static_cast<float>(effectiveWidth));
        if (normalizedOffset < 0.0f) {
            normalizedOffset += static_cast<float>(effectiveWidth);
        }
        
        // Calculate starting position for seamless tiling with extra coverage
        // Ensure we have multiple tiles extending beyond both edges to prevent gaps
        int startX = -effectiveWidth * 2 - static_cast<int>(normalizedOffset);
        
        // Draw sufficient tiles to cover the entire screen width plus overlap
        for (int x = startX; x <= windowW + effectiveWidth * 2; x += effectiveWidth) {
            SDL_Rect dstRect = { x, 0, texW, texH }; 
            display.drawTexture(tex, NULL, &dstRect);        }
    };    // REFACTOR: Always use the new SeamlessBackgroundRenderer regardless of asset dimensions
    // This eliminates the problematic overlap-based tiling system entirely
    bool usingNewAssets = true;
    
    // For debugging: Log texture dimensions to verify what we're working with
    if (bgTexture0_ || bgTexture1_ || bgTexture2_) {
        std::vector<SDL_Texture*> textures = {bgTexture2_, bgTexture1_, bgTexture0_};
        for (SDL_Texture* tex : textures) {
            if (tex) {
                int testW = 0, testH = 0;
                SDL_QueryTexture(tex, nullptr, nullptr, &testW, &testH);
                
                // Debug: Always log the first texture dimensions we check
                static bool firstCheck = true;
                if (firstCheck) {
                    std::cout << "TEXTURE DIMENSIONS: " << testW << "x" << testH 
                              << " (using SeamlessBackgroundRenderer for all assets)" << std::endl;
                    firstCheck = false;
                }
            }
        }
    }
    
    // Always log which path we're taking
    std::cout << "RENDERING PATH: " << (usingNewAssets ? "NEW_SCALED" : "LEGACY_TILED") << std::endl;
    
    // Enhanced diagnostic logging for asset type detection
    static int renderFrameCount = 0;
    renderFrameCount++;
    if (renderFrameCount % 180 == 0) { // Log every 180 frames (every ~3 seconds at 60fps)
        // Get texture dimensions for logging
        int bg0W=0, bg0H=0, bg1W=0, bg1H=0, bg2W=0, bg2H=0;
        if (bgTexture0_) SDL_QueryTexture(bgTexture0_, nullptr, nullptr, &bg0W, &bg0H);
        if (bgTexture1_) SDL_QueryTexture(bgTexture1_, nullptr, nullptr, &bg1W, &bg1H);
        if (bgTexture2_) SDL_QueryTexture(bgTexture2_, nullptr, nullptr, &bg2W, &bg2H);
        
        std::cout << "Render Mode: " << (usingNewAssets ? "NEW_ASSETS" : "LEGACY") 
                  << " | Textures: FG=" << bg0W << "x" << bg0H 
                  << ", MG=" << bg1W << "x" << bg1H 
                  << ", BG=" << bg2W << "x" << bg2H << std::endl;
    }    if (usingNewAssets && backgroundRenderer_) {
        // Use SeamlessBackgroundRenderer for optimized rendering
        backgroundRenderer_->render();
        
        // Debug: Log renderer status occasionally
        static int debugFrameCount = 0;
        debugFrameCount++;
        if (debugFrameCount % 120 == 0) { // Log every 120 frames
            const auto& stats = backgroundRenderer_->getPerformanceStats();
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, 
                        "SeamlessBackgroundRenderer: %zu layers, %d render calls, %d texture updates",
                        backgroundRenderer_->getLayerCount(), stats.renderCalls, stats.textureUpdates);
        }
    } else {// Fallback to legacy tiled rendering for old upscaled assets        // Query texture dimensions for background rendering with consistent parallax factors
        int bgW0=0,bgH0=0,effW0=0, bgW1=0,bgH1=0,effW1=0, bgW2=0,bgH2=0,effW2=0;
        if(bgTexture0_) { 
            SDL_QueryTexture(bgTexture0_,0,0,&bgW0,&bgH0); 
            effW0 = static_cast<int>(bgW0 * 0.667f); // Foreground: 2/3 for standard tiling speed
            if(effW0<=0) effW0=bgW0;
        }
        if(bgTexture1_) { 
            SDL_QueryTexture(bgTexture1_,0,0,&bgW1,&bgH1); 
            effW1 = static_cast<int>(bgW1 * 0.75f); // Middleground: 3/4 for slightly slower parallax
            if(effW1<=0) effW1=bgW1;
        }
        if(bgTexture2_) { 
            SDL_QueryTexture(bgTexture2_,0,0,&bgW2,&bgH2); 
            effW2 = static_cast<int>(bgW2 * 0.833f); // Background: 5/6 for slowest parallax
            if(effW2<=0) effW2=bgW2;
        }        // Draw background layers (back to front)
        drawTiledBg(bgTexture2_, bg_scroll_offset_2_, bgW2, bgH2, effW2, "Layer 2");
        drawTiledBg(bgTexture1_, bg_scroll_offset_1_, bgW1, bgH1, effW1, "Layer 1");    
        drawTiledBg(bgTexture0_, bg_scroll_offset_0_, bgW0, bgH0, effW0, "Layer 0");
        
        // DEBUG: Log scroll offsets in legacy mode every 60 frames
        static int legacyScrollCount = 0;
        legacyScrollCount++;
        if (legacyScrollCount % 60 == 0) {
            std::cout << "LEGACY SCROLL: Layer0=" << bg_scroll_offset_0_ 
                      << ", Layer1=" << bg_scroll_offset_1_ 
                      << ", Layer2=" << bg_scroll_offset_2_ 
                      << " | effW=[" << effW0 << "," << effW1 << "," << effW2 << "]" << std::endl;
        }
    }

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
        
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Attempting to load FG path: %s with ID: %s", fgPath.c_str(), fgTexId.c_str());
        
        if (assets->loadTexture(fgTexId, fgPath)) {
            bgTexture0_ = assets->getTexture(fgTexId);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Successfully loaded foreground variant: %s", fgPath.c_str());
        } else {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Failed to load foreground variant: %s", fgPath.c_str());
        }
    }
      // Try to load middleground texture (1x scale asset)
    if (!tempLayerData.middlegroundPaths.empty()) {
        std::string mgPath = Digivice::BackgroundVariantManager::getSelectedPath(
            tempLayerData.middlegroundPaths, tempLayerData.selectedMiddlegroundVariant);
        std::string mgTexId = environmentName + "_mg_v" + std::to_string(tempLayerData.selectedMiddlegroundVariant + 1);
        
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Attempting to load MG path: %s with ID: %s", mgPath.c_str(), mgTexId.c_str());
        
        if (assets->loadTexture(mgTexId, mgPath)) {
            bgTexture1_ = assets->getTexture(mgTexId);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Successfully loaded middleground variant: %s", mgPath.c_str());
        } else {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Failed to load middleground variant: %s", mgPath.c_str());
        }
    }
      // Try to load background texture (1x scale asset)
    if (!tempLayerData.backgroundPaths.empty()) {
        std::string bgPath = Digivice::BackgroundVariantManager::getSelectedPath(
            tempLayerData.backgroundPaths, tempLayerData.selectedBackgroundVariant);
        std::string bgTexId = environmentName + "_bg_v" + std::to_string(tempLayerData.selectedBackgroundVariant + 1);
        
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Attempting to load BG path: %s with ID: %s", bgPath.c_str(), bgTexId.c_str());
        
        if (assets->loadTexture(bgTexId, bgPath)) {
            bgTexture2_ = assets->getTexture(bgTexId);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Successfully loaded background variant: %s", bgPath.c_str());
        } else {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Failed to load background variant: %s", bgPath.c_str());
        }
    }
      // Note: The actual scaling will be handled in the render method
    // For now, we're just loading the 1x scale assets
}

void AdventureState::loadBackgroundVariantsFromNodeData(const Digivice::BackgroundLayerData& layerData, const std::string& nodeId) {
    // This method loads backgrounds directly from pre-populated variant paths
    AssetManager* assets = game_ptr->getAssetManager();
    
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, 
               "AdventureState: Loading pre-populated variant backgrounds for node '%s'. FG:%zu MG:%zu BG:%zu", 
               nodeId.c_str(), layerData.foregroundPaths.size(), layerData.middlegroundPaths.size(), layerData.backgroundPaths.size());
    
    // Debug: Print all paths that were generated
    if (!layerData.foregroundPaths.empty()) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: FG Paths Available:");
        for (size_t i = 0; i < layerData.foregroundPaths.size(); ++i) {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "  FG[%zu]: %s", i, layerData.foregroundPaths[i].c_str());
        }
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Selected FG variant: %d", layerData.selectedForegroundVariant);
    }
    
    if (!layerData.middlegroundPaths.empty()) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: MG Paths Available:");
        for (size_t i = 0; i < layerData.middlegroundPaths.size(); ++i) {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "  MG[%zu]: %s", i, layerData.middlegroundPaths[i].c_str());
        }
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Selected MG variant: %d", layerData.selectedMiddlegroundVariant);
    }
    
    if (!layerData.backgroundPaths.empty()) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: BG Paths Available:");
        for (size_t i = 0; i < std::min(layerData.backgroundPaths.size(), size_t(5)); ++i) { // Only show first 5 to avoid spam
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "  BG[%zu]: %s", i, layerData.backgroundPaths[i].c_str());
        }
        if (layerData.backgroundPaths.size() > 5) {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "  ... and %zu more BG paths", layerData.backgroundPaths.size() - 5);
        }
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Selected BG variant: %d", layerData.selectedBackgroundVariant);
    }
      // Load foreground texture
    if (!layerData.foregroundPaths.empty()) {
        std::string fgPath = Digivice::BackgroundVariantManager::getSelectedPath(
            layerData.foregroundPaths, layerData.selectedForegroundVariant);
        std::string fgTexId = nodeId + "_adventure_fg_" + std::to_string(layerData.selectedForegroundVariant);
        
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Attempting to load FG path: %s with ID: %s", fgPath.c_str(), fgTexId.c_str());
        
        if (assets->loadTexture(fgTexId, fgPath)) {
            bgTexture0_ = assets->getTexture(fgTexId);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Successfully loaded foreground variant: %s", fgPath.c_str());
        } else {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Failed to load foreground variant: %s", fgPath.c_str());
        }
    }
      // Load middleground texture
    if (!layerData.middlegroundPaths.empty()) {
        std::string mgPath = Digivice::BackgroundVariantManager::getSelectedPath(
            layerData.middlegroundPaths, layerData.selectedMiddlegroundVariant);
        std::string mgTexId = nodeId + "_adventure_mg_" + std::to_string(layerData.selectedMiddlegroundVariant);
        
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Attempting to load MG path: %s with ID: %s", mgPath.c_str(), mgTexId.c_str());
        
        if (assets->loadTexture(mgTexId, mgPath)) {
            bgTexture1_ = assets->getTexture(mgTexId);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Successfully loaded middleground variant: %s", mgPath.c_str());
        } else {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Failed to load middleground variant: %s", mgPath.c_str());
        }
    }
      // Load background texture
    if (!layerData.backgroundPaths.empty()) {
        std::string bgPath = Digivice::BackgroundVariantManager::getSelectedPath(
            layerData.backgroundPaths, layerData.selectedBackgroundVariant);
        std::string bgTexId = nodeId + "_adventure_bg_" + std::to_string(layerData.selectedBackgroundVariant);
        
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Attempting to load BG path: %s with ID: %s", bgPath.c_str(), bgTexId.c_str());
        
        if (assets->loadTexture(bgTexId, bgPath)) {
            bgTexture2_ = assets->getTexture(bgTexId);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Successfully loaded background variant: %s", bgPath.c_str());
        } else {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Failed to load background variant: %s", bgPath.c_str());
        }
    }
}

void AdventureState::renderScaledBackgroundLayer(PCDisplay& display, SDL_Texture* texture, 
                                               int screenWidth, int screenHeight, 
                                               float globalScale, int layerIndex, 
                                               float scrollOffset) {
    if (!texture) {
        // std::cerr << "renderScaledBackgroundLayer: Texture is null for layer " << layerIndex << std::endl;
        return;
    }

    int originalTextureWidth, originalTextureHeight;
    SDL_QueryTexture(texture, nullptr, nullptr, &originalTextureWidth, &originalTextureHeight);

    if (originalTextureWidth == 0 || originalTextureHeight == 0) {
        // std::cerr << "renderScaledBackgroundLayer: Texture dimensions are zero for layer " << layerIndex << std::endl;
        return;
    }    // For overlap-based tiling system where lefthand third overlays righthand third
    // We need to account for the effective tile width being 2/3 of the actual texture width
    float effectiveTileWidth = static_cast<float>(originalTextureWidth) * (2.0f / 3.0f);
    
    // Calculate proper scaling to fill the square display (384×128 → 466×466)
    float scaleX = static_cast<float>(screenWidth) / static_cast<float>(originalTextureWidth);   // ~1.21x for 466/384
    float scaleY = static_cast<float>(screenHeight) / static_cast<float>(originalTextureHeight); // ~3.64x for 466/128
    
    // Use larger scale to fill display completely (zoom to fill) - prioritize filling height
    float baseScale = std::max(scaleX, scaleY); // Will use ~3.64x to fill height
    
    // Apply user's global scale preference
    float finalScale = baseScale * globalScale;
    
    // Use std::round for scaled dimensions
    int scaledWidth = static_cast<int>(std::round(static_cast<float>(originalTextureWidth) * finalScale));
    int scaledHeight = static_cast<int>(std::round(static_cast<float>(originalTextureHeight) * finalScale));
    int scaledEffectiveWidth = static_cast<int>(std::round(effectiveTileWidth * finalScale));

    // Safety check for scaled dimensions
    if (scaledWidth <= 0 || scaledHeight <= 0 || scaledEffectiveWidth <= 0) {
        // std::cerr << "renderScaledBackgroundLayer: Scaled dimensions are non-positive for layer " << layerIndex << std::endl;
        return;
    }

    // Center the scaled background vertically (since we're scaling to fill height, this should be ~0)
    int drawY = (screenHeight - scaledHeight) / 2;    // For overlap-based tiling, use the effective width for scroll calculations
    float actualPeriodWidth = effectiveTileWidth * finalScale; 
    if (actualPeriodWidth <= 0.0001f) { // Avoid division by zero
        // std::cerr << "renderScaledBackgroundLayer: actualPeriodWidth is too small for layer " << layerIndex << std::endl;
        return;
    }

    float normalizedScrollOffset = std::fmod(scrollOffset, actualPeriodWidth);
    if (normalizedScrollOffset < 0.0f) {
        normalizedScrollOffset += actualPeriodWidth;
    }

    // Calculate starting position - need to start early enough to cover screen with overlaps
    int startX = static_cast<int>(std::round(-normalizedScrollOffset)) - scaledWidth;

    int tilesDrawn = 0;
    // Draw tiles with overlap - each tile advances by effective width but draws full width
    for (int currentDrawX = startX; currentDrawX < screenWidth + scaledWidth; currentDrawX += scaledEffectiveWidth) {
        SDL_Rect destRect = { currentDrawX, drawY, scaledWidth, scaledHeight };
        
        display.drawTexture(texture, nullptr, &destRect);
        tilesDrawn++;
    }
    
    // Debug logging (can be removed later)
    // if (layerIndex == 0) { // Example: Log only for foreground
    //     std::cout << "L" << layerIndex << " scroll: " << scrollOffset
    //               << " normOff: " << normalizedScrollOffset
    //               << " startX: " << startX
    //               << " scaledW: " << scaledWidth
    //               << " effW: " << scaledEffectiveWidth
    //               << " periodW: " << actualPeriodWidth
    //               << " tiles: " << tilesDrawn << std::endl;
    // }
}