// File: src/core/Game.cpp

#include "core/Game.h"
#include "states/GameState.h"        // Include base class for enter/exit
#include "states/AdventureState.h"   // Include concrete states used
#include "states/TransitionState.h" // Include concrete states used
#include "core/PlayerData.h"
#include "core/InputManager.h"
#include "platform/pc/pc_display.h" // Needed for PCDisplay type
#include "core/AssetManager.h"
#include "ui/TextRenderer.h"
#include "core/AnimationManager.h"
#include "utils/ConfigManager.h" // Add ConfigManager include

#include <SDL_log.h>
#include <stdexcept>
#include <filesystem> // For CWD logging
#include <vector>
#include <memory>
#include <string>

Game::Game() : 
    display(),                 // Change from window_
    inputManager(),            // Change from input_manager_
    assetManager(),            // Change from asset_manager_
    states_(),
    is_running(true),          // Change from is_running_
    fade_step_(FadeSequenceStep::NONE),  
    targetStateAfterFade_(StateType::None),
    pop_until_target_type_(StateType::None)
{
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Game initializing...");
}

Game::~Game() {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Game destructor called.");
    // Cleanup happens in close() if game loop exited normally,
    // otherwise destructor handles unique_ptr cleanup.
}

bool Game::init(const std::string& title, int width, int height) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Initializing Game systems...");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL Init Error: %s", SDL_GetError()); return false; }
    if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0")) { SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "Hint Warning: %s", SDL_GetError()); }
    else { SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "SDL_HINT_RENDER_SCALE_QUALITY set to 0 (Nearest Neighbor)."); }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL Initialized.");

    // Get small screen size from config (if not found, use defaults)
    int smallScreenWidth = ConfigManager::getValue<int>("display.smallScreenWidth", DEFAULT_SMALL_SCREEN_WIDTH);
    int smallScreenHeight = ConfigManager::getValue<int>("display.smallScreenHeight", DEFAULT_SMALL_SCREEN_HEIGHT);
    bool vsync = ConfigManager::getValue<bool>("display.vsync", true);
    
    // Store the original received dimensions (from main)
    original_width_ = width;
    original_height_ = height;
    is_small_screen_ = false; // Ensure it starts with normal size
    
    // Pass vsync setting from config
    if (!display.init(title.c_str(), width, height, vsync)) { 
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay Init Error"); 
        SDL_Quit(); 
        return false; 
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay Initialized.");

    if (!assetManager.init(display.getRenderer())) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AssetManager Init Error"); display.close(); SDL_Quit(); return false; }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetManager Initialized.");

    // Log CWD
    try {
        std::filesystem::path cwd = std::filesystem::current_path();
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Current Working Directory: %s", cwd.string().c_str());
    } catch (const std::filesystem::filesystem_error& e) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error getting CWD using std::filesystem: %s", e.what()); }

    // Load Initial Assets
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Attempting to load initial assets...");
     bool assets_ok = true;
     assets_ok &= assetManager.loadTexture("ui_round_mask", "assets/ui/mask/round_mask.png"); // Use forward slashes for consistency
     ui_mask_texture_ = assetManager.getTexture("ui_round_mask"); // Load and store the mask texture
     if (!ui_mask_texture_) {
         SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Failed to load UI mask texture: assets/ui/mask/round_mask.png");
         // Decide if this is a critical failure or if the game can run without it
     }
     assets_ok &= assetManager.loadTexture("round_mask", "assets/ui/mask/round_mask.png");
     assets_ok &= assetManager.loadTexture("agumon_sheet", "assets/sprites/agumon_sheet.png");
     assets_ok &= assetManager.loadTexture("gabumon_sheet", "assets/sprites/gabumon_sheet.png");
     assets_ok &= assetManager.loadTexture("biyomon_sheet", "assets/sprites/biyomon_sheet.png");
     assets_ok &= assetManager.loadTexture("gatomon_sheet", "assets/sprites/gatomon_sheet.png");
     assets_ok &= assetManager.loadTexture("gomamon_sheet", "assets/sprites/gomamon_sheet.png");
     assets_ok &= assetManager.loadTexture("palmon_sheet", "assets/sprites/palmon_sheet.png");
     assets_ok &= assetManager.loadTexture("tentomon_sheet", "assets/sprites/tentomon_sheet.png");
     assets_ok &= assetManager.loadTexture("patamon_sheet", "assets/sprites/patamon_sheet.png");
     assets_ok &= assetManager.loadTexture("castle_bg_0", "assets/backgrounds/castlebackground0.png");
     assets_ok &= assetManager.loadTexture("castle_bg_1", "assets/backgrounds/castlebackground1.png");
     assets_ok &= assetManager.loadTexture("castle_bg_2", "assets/backgrounds/castlebackground2.png");
     assets_ok &= assetManager.loadTexture("menu_bg_blue", "assets/ui/backgrounds/menu_base_blue.png");
     assets_ok &= assetManager.loadTexture("transition_borders", "assets/ui/transition/transition_borders.png");
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Loading UI font...");
     assets_ok &= assetManager.loadTexture("ui_font_atlas", "assets/ui/fonts/bluewhitefont.png");
     if (!assets_ok) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "One or more essential assets failed to load! Check paths and file existence."); assetManager.shutdown(); display.close(); SDL_Quit(); return false; }
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Finished loading initial assets attempt.");

    // Initialize Text Renderer
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Initializing TextRenderer...");
    SDL_Texture* fontTexturePtr = assetManager.getTexture("ui_font_atlas");
    if (fontTexturePtr) {
        textRenderer_ = std::make_unique<TextRenderer>(fontTexturePtr);
        const std::string fontJsonPath = "assets/ui/fonts/bluewhitefont.json"; // Use forward slashes
        if (!textRenderer_->loadFontData(fontJsonPath)) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load font data for TextRenderer from '%s'. Text rendering may fail.", fontJsonPath.c_str());
            textRenderer_.reset();
        } else {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer initialized successfully.");
        }
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize TextRenderer: Font texture 'ui_font_atlas' not found in AssetManager.");
    }

    // Initialize Animation Manager & Load Animations
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Initializing AnimationManager...");
    try {
        animationManager_ = std::make_unique<AnimationManager>(&assetManager); // Pass AssetManager pointer

        bool anims_ok = true;
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager: Loading animation data files...");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/agumon_sheet.json", "agumon_sheet");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/gabumon_sheet.json", "gabumon_sheet");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/biyomon_sheet.json", "biyomon_sheet");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/gatomon_sheet.json", "gatomon_sheet");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/gomamon_sheet.json", "gomamon_sheet");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/palmon_sheet.json", "palmon_sheet");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/tentomon_sheet.json", "tentomon_sheet");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/patamon_sheet.json", "patamon_sheet");

        if (!anims_ok) {
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"One or more animation files failed to load properly. See errors above. Game may continue with missing/default animations.");
        } else {
             SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager finished loading animation data.");
        }

    } catch (const std::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create or initialize AnimationManager: %s", e.what());
        textRenderer_.reset();
        assetManager.shutdown();
        display.close();
        SDL_Quit();
        return false;
    }

    // Push Initial State and Call Enter
    try {
        states_.push_back(std::make_unique<AdventureState>(this));
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Initial AdventureState created and added.");
        if (!states_.empty()) {
             states_.back()->enter();
             SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Called enter() on initial state.");
        }
    } catch (const std::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create initial state: %s", e.what());
        animationManager_.reset();
        textRenderer_.reset();
        assetManager.shutdown();
        display.close();
        SDL_Quit();
        return false;
    }

    // Make sure fade step is reset
    fade_step_ = FadeSequenceStep::NONE;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Fade step reset to NONE during initialization.");

    is_running = true;
    last_frame_time = SDL_GetTicks();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Game Initialization Successful.");
    return true;
}

// --- Run Loop ---
void Game::run() {
    if (!is_running || states_.empty()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Game::run() called in invalid state (not initialized or no initial state).");
        return;
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Entering main game loop.");
    last_frame_time = SDL_GetTicks();

    while (is_running) {
        // Calculate Delta Time
        Uint32 current_time = SDL_GetTicks();
        float delta_time = (current_time - last_frame_time) / 1000.0f;
        if (delta_time > 0.1f) delta_time = 0.1f; // Clamp max delta time
        last_frame_time = current_time;

        // Input Processing
        inputManager.prepareNewFrame();
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit_game();
            }
            inputManager.processEvent(event);
        }        // Handle screen toggle action
        if (inputManager.isActionJustPressed(GameAction::TOGGLE_SCREEN_SIZE)) {
            is_small_screen_ = !is_small_screen_;
            if (is_small_screen_) {
                // Get small screen size from config
                int smallScreenWidth = ConfigManager::getValue<int>("display.smallScreenWidth", DEFAULT_SMALL_SCREEN_WIDTH);
                int smallScreenHeight = ConfigManager::getValue<int>("display.smallScreenHeight", DEFAULT_SMALL_SCREEN_HEIGHT);
                
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Toggling to small screen mode: %dx%d", smallScreenWidth, smallScreenHeight);
                display.setWindowSize(smallScreenWidth, smallScreenHeight);
                display.setLogicalSize(original_width_, original_height_); // Render at original size, scale down
            } else {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Toggling to normal screen mode: %dx%d", original_width_, original_height_);
                display.setWindowSize(original_width_, original_height_);
                display.setLogicalSize(original_width_, original_height_); // Reset logical size
            }
        }

        // State Logic
        if (!states_.empty()) {
            // Apply any pending state changes
            applyStateChanges();

            // Handle input for the current state
            if (!states_.empty()) { // Check again, applyStateChanges might alter the stack
                states_.back()->handle_input(inputManager, &playerData_);
            }

            // Update the current state
            if (!states_.empty()) { // Check again
                states_.back()->update(delta_time, &playerData_);
            }

            // Render Logic
            if (!states_.empty()) { // Check again in case update caused a state change
                GameState* currentStateForRender = states_.back().get();
                if (currentStateForRender) {
                    // Clear the screen (e.g., black or a debug color)
                    // display.clear(0, 0, 0, 255); // Example: Clear to black
                    currentStateForRender->render(display); // Pass by reference

                    // Apply the UI mask after the state has rendered its content
                    if (ui_mask_texture_) {
                        display.applyMask(ui_mask_texture_);
                    }
                }
            }
            // Present the frame
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Presenting frame.");
            display.present();
            
        } else {
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Game::run() - State stack is empty, nothing to update or render.");
        }
        
        // Add this debugging line somewhere in your update cycle
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Current fade step: %d", 
                    static_cast<int>(fade_step_));
        // ... rest of the run method ...
    }
}

// --- State Management - Actual Push/Pop ---
void Game::push_state(std::unique_ptr<GameState> new_state) {
    if (!new_state) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "push_state called with null state!"); return; }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Executing push_state for state %p (Type: %d)...",
                (void*)new_state.get(), (int)new_state->getType()); // Log type
    states_.push_back(std::move(new_state));
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "push_state complete. New stack size: %zu", states_.size());
}

void Game::pop_state() {
    if (!states_.empty()) {
         GameState* stateToPop = states_.back().get();
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Executing pop_state for state %p (Type: %d)...", (void*)stateToPop, (int)stateToPop->getType());
         // exit() called before this in applyStateChanges
         states_.pop_back(); // Removes the unique_ptr, deleting the state object
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "pop_state complete. New stack size: %zu", states_.size());
    } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "pop_state called on empty stack!"); }
}

// --- State Management - Requests ---
void Game::requestPushState(std::unique_ptr<GameState> state) {
    if (!state) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Attempted to request push of NULL state!"); return; }
    if (request_push_) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Overwriting previous push request for state %p with new request for %p.", (void*)request_push_.get(), (void*)state.get()); }
    request_push_ = std::move(state);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Push requested for state %p (Type: %d).",
                 (void*)request_push_.get(), (int)request_push_->getType()); // Log type
}

void Game::requestPopState() {
    if(request_pop_) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Multiple pop requests made before applyStateChanges.");
    request_pop_ = true;
    pop_until_target_type_ = StateType::None; // Cancel any pending pop_until
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Single Pop requested.");
}

void Game::requestPopUntil(StateType targetType) {
    if (targetType == StateType::None) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "requestPopUntil called with StateType::None. Performing single pop instead.");
        requestPopState();
        return;
    }
    pop_until_target_type_ = targetType;
    request_pop_ = false; // Cancel any pending single pop
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "PopUntil requested for type: %d", static_cast<int>(targetType));
}

void Game::requestFadeToState(std::unique_ptr<GameState> targetState, float duration, bool popCurrent) {
    if (fade_step_ != FadeSequenceStep::NONE) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "requestFadeToState called while a fade sequence is already in progress. Ignoring.");
        if (targetState) { SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "requestFadeToState: Discarding targetState due to ongoing fade."); }
        return;
    }
    // Allow nullptr targetState ONLY if we intend to pop the current state.
    // If popCurrent is false, a targetState is mandatory.
    if (!targetState && !popCurrent) { 
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "requestFadeToState called with null targetState and popCurrent is false. This is invalid."); 
        return; 
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Requesting Fade-To-State sequence (Target Type: %d, Duration: %.2fs, PopCurrent: %s).",
                (targetState ? (int)targetState->getType() : (int)StateType::None), // Log target type or None
                duration, popCurrent ? "true" : "false");
    pending_state_for_fade_ = std::move(targetState);
    fade_duration_ = duration > 0.001f ? duration : 0.001f;
    pop_current_after_fade_out_ = popCurrent;
    fade_step_ = FadeSequenceStep::FADING_OUT;
    active_fade_type_ = TransitionEffectType::FADE_TO_COLOR; // Set type for this fade

    auto fadeOutState = std::make_unique<TransitionState>(
        this,
        fade_duration_,
        active_fade_type_
    );
    requestPushState(std::move(fadeOutState)); // Use existing requestPushState
}

// Implementation moved to Game_fade.cpp

void Game::update(float delta_time) {
    if (states_.empty()) return;

    // Apply any state changes before update
    applyStateChanges();

    // Update the current state
    if (!states_.empty()) {
        states_.back()->update(delta_time, &playerData_);
    }
    
    /* Comment out the problematic fade detection code for now
    // Check if the current state is a TransitionState
    if (!states_.empty() && states_.back()->getType() == StateType::Transition) {
        TransitionState* transition = static_cast<TransitionState*>(states_.back().get());
        float progress = transition->getProgress();
        
        // Update fade_step_ based on transition progress
        if (transition->getEffectType() == TransitionEffectType::FADE_TO_COLOR) {
            if (progress >= 0.5f && fade_step_ == FadeSequenceStep::FADING_OUT) {
                fade_step_ = FadeSequenceStep::READY_FOR_STATE_SWAP;
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, 
                           "Fade transition halfway point reached");
            }
            else if (progress >= 0.95f && fade_step_ == FadeSequenceStep::READY_FOR_STATE_SWAP) {
                fade_step_ = FadeSequenceStep::FADING_IN;
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, 
                           "Fade transition almost complete, fading in");
            }
            else if (progress >= 1.0f && fade_step_ == FadeSequenceStep::FADING_IN) {
                fade_step_ = FadeSequenceStep::NONE;
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, 
                           "Fade transition complete");
            }
        }
    }
    */
}

// --- Helper to apply queued changes ---
// <<< --- INTEGRATED NEW VERSION --- >>>
void Game::applyStateChanges() {

    bool state_popped_this_frame = false;
    TransitionEffectType popped_transition_type = TransitionEffectType::BORDER_WIPE; 
    StateType original_popped_state_type_if_transition = StateType::None; // Store type of state that _caused_ transition pop

    // --- 1. Handle Pops ---
    if (pop_until_target_type_ != StateType::None) {
        // --- Pop Until Logic ---
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Processing PopUntil target type %d.", (int)pop_until_target_type_);
        while (!states_.empty()) {
            GameState* topState = states_.back().get();
            if (topState->getType() == pop_until_target_type_) {
                SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges (PopUntil): Found target state %p (type %d). Calling enter().",
                             (void*)topState, (int)topState->getType());
                topState->enter(); 
                state_popped_this_frame = true; 
                break; 
            }
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges (PopUntil): Exiting and popping state %p (type %d).",
                         (void*)topState, (int)topState->getType());
            topState->exit();
            pop_state();
            state_popped_this_frame = true;
        }
        if (states_.empty() && pop_until_target_type_ != StateType::None) {
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: PopUntil target type %d not found in stack.", (int)pop_until_target_type_);
        }
        pop_until_target_type_ = StateType::None;
        request_pop_ = false; 
        request_push_.reset(); 

        if (fade_step_ != FadeSequenceStep::NONE) {
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: pop_until executed during fade sequence. Resetting fade.");
             fade_step_ = FadeSequenceStep::NONE;
             pending_state_for_fade_.reset();
             targetStateAfterFade_ = StateType::None;
             pop_current_after_fade_out_ = false;
        }
    }
    else if (request_pop_) {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Processing request_pop_ = true.");
        if (!states_.empty()) {
            GameState* topStateBeforePop = states_.back().get();
            StateType popped_state_type = topStateBeforePop->getType();
            original_popped_state_type_if_transition = popped_state_type; // Capture type

            if (popped_state_type == StateType::Transition) {
               TransitionState* ts = static_cast<TransitionState*>(topStateBeforePop);
               popped_transition_type = ts->getEffectType();
               SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Popped state was Transition, effect type: %d", (int)popped_transition_type);
            } else {
                popped_transition_type = TransitionEffectType::BORDER_WIPE; // Not a transition
            }

            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Calling exit() on state %p (type %d).", (void*)topStateBeforePop, (int)popped_state_type);
            topStateBeforePop->exit();
            pop_state(); 
            state_popped_this_frame = true;
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: State popped. Stack size AFTER pop: %zu", states_.size());

            // --- Handle Fade Sequence Logic After a TransitionState Pops ---
            if (fade_step_ == FadeSequenceStep::FADING_OUT &&
                original_popped_state_type_if_transition == StateType::Transition &&
                popped_transition_type == active_fade_type_ && // active_fade_type_ was FADE_TO_COLOR
                active_fade_type_ == TransitionEffectType::FADE_TO_COLOR)
            {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: FADE_TO_COLOR TransitionState sequence part finished.");

                if (pop_current_after_fade_out_ && !states_.empty()) {
                    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Popping original state %p (type %d) after its FADE_TO_COLOR sequence as requested by pop_current_after_fade_out_.",
                        (void*)states_.back().get(), (int)states_.back()->getType());
                    states_.back()->exit();
                    pop_state(); 
                }
                pop_current_after_fade_out_ = false; // Reset flag

                if (targetStateAfterFade_ != StateType::None) {
                    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Target state after fade is %d. Setting up target.", (int)targetStateAfterFade_);
                    fade_step_ = FadeSequenceStep::SETUP_TARGET_STATE;
                } else if (pending_state_for_fade_) {
                    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Pending new state %p (type %d) for fade. Swapping.",
                        (void*)pending_state_for_fade_.get(), (int)pending_state_for_fade_->getType());
                    fade_step_ = FadeSequenceStep::READY_FOR_STATE_SWAP;
                } else { // Fading back to an underlying state (e.g., "BACK" from a menu)
                    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: FADE_TO_COLOR finished. Fading in underlying state.");
                    if (!states_.empty()) {
                        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Calling enter() on underlying state %p (type %d) before fade-in.",
                            (void*)states_.back().get(), (int)states_.back()->getType());
                        states_.back()->enter();
                    } else {
                        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Stack empty, cannot enter underlying state for fade-in.");
                    }
                    auto fadeInState = std::make_unique<TransitionState>(this, fade_duration_, TransitionEffectType::FADE_FROM_COLOR, SDL_Color{0,0,0,255});
                    active_fade_type_ = TransitionEffectType::FADE_FROM_COLOR; // Update active fade type
                    requestPushState(std::move(fadeInState));
                    fade_step_ = FadeSequenceStep::FADING_IN;
                }
            }
            else if (fade_step_ == FadeSequenceStep::FADING_IN &&
                     original_popped_state_type_if_transition == StateType::Transition &&
                     popped_transition_type == active_fade_type_ && // active_fade_type_ was FADE_FROM_COLOR
                     active_fade_type_ == TransitionEffectType::FADE_FROM_COLOR)
            {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: FADE_FROM_COLOR TransitionState sequence part finished. Fade sequence complete.");
                fade_step_ = FadeSequenceStep::NONE;
                active_fade_type_ = TransitionEffectType::BORDER_WIPE; // Reset
                // The target state (e.g. AdventureState or new MenuState) should have already had enter() called
                // either in SETUP_TARGET_STATE or READY_FOR_STATE_SWAP.
                // If not, and it's the top state, call enter now.
                if (!states_.empty()) {
                     SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Re-confirming/calling enter() on final state %p (type %d) after FADE_FROM_COLOR.",
                                 (void*)states_.back().get(), (int)states_.back()->getType());
                    states_.back()->enter(); // Ensure the final state is properly entered/re-entered.
                }
            }
            else { // Not a fade-related transition pop, or an unexpected pop during a fade
                if (fade_step_ != FadeSequenceStep::NONE && original_popped_state_type_if_transition == StateType::Transition) {
                     SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Transition (type %d, effect %d) popped during fade step %d but was not the expected active_fade_type_ (%d). Resetting fade.",
                                 (int)popped_state_type, (int)popped_transition_type, (int)fade_step_, (int)active_fade_type_);
                     fade_step_ = FadeSequenceStep::NONE;
                     pending_state_for_fade_.reset();
                     targetStateAfterFade_ = StateType::None;
                     pop_current_after_fade_out_ = false;
                }
                SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Handling as normal pop (not part of expected fade sequence step).");
                if (!states_.empty()) {
                    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Calling enter() on new top state %p (type %d).",
                                 (void*)states_.back().get(), (int)states_.back()->getType());
                    states_.back()->enter();
                } else {
                    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: State stack is empty after pop!");
                }
            }
        } else {
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: request_pop_ was true, but state stack was already empty!");
        }
        request_pop_ = false; 
    }

    // --- 2. Handle State Setup/Swaps for Fades (after pops are processed) ---
    if (fade_step_ == FadeSequenceStep::SETUP_TARGET_STATE) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Processing SETUP_TARGET_STATE for type %d.", (int)targetStateAfterFade_);
        // Pop until targetStateAfterFade_ is on top.
        // Note: The original state that was faded from (e.g. PartnerSelect) should have already been popped if pop_current_after_fade_out_ was true.
        pop_until_target_type_ = targetStateAfterFade_; // Set the target for processPopUntil
        processPopUntil(); // This ALREADY calls enter() on the target state if found.

        if (!states_.empty() && states_.back()->getType() == targetStateAfterFade_) {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges (SETUP_TARGET_STATE): Target state %p (type %d) is now at top (enter() called by processPopUntil). Proceeding to fade-in.",
                (void*)states_.back().get(), (int)states_.back()->getType());
            // states_.back()->enter(); // REDUNDANT: processPopUntil already called enter().
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges (SETUP_TARGET_STATE): Failed to bring target %d to top or stack empty. Aborting fade.", (int)targetStateAfterFade_);
            fade_step_ = FadeSequenceStep::NONE; 
            targetStateAfterFade_ = StateType::None; // Ensure reset if aborted before fade-in setup
            // No pending_state_for_fade_ to worry about here, as SETUP_TARGET_STATE uses targetStateAfterFade_
        }

        if (fade_step_ == FadeSequenceStep::SETUP_TARGET_STATE) { // Check if not aborted
            auto fadeInState = std::make_unique<TransitionState>(this, fade_duration_, TransitionEffectType::FADE_FROM_COLOR, SDL_Color{0,0,0,255});
            active_fade_type_ = TransitionEffectType::FADE_FROM_COLOR;
            requestPushState(std::move(fadeInState)); // This will be handled in the push section below or next frame
            fade_step_ = FadeSequenceStep::FADING_IN;
        }
        targetStateAfterFade_ = StateType::None; // Reset
    }
    else if (fade_step_ == FadeSequenceStep::READY_FOR_STATE_SWAP) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Processing READY_FOR_STATE_SWAP.");
        // This step implies a FADE_TO_COLOR just finished, and pop_current_after_fade_out_ might have popped the original state.
        // Now, we push the pending_state_for_fade_ (if any) and then the FADE_FROM_COLOR transition.

        if (pending_state_for_fade_) {
             std::unique_ptr<GameState> targetStateToPush = std::move(pending_state_for_fade_);
             pending_state_for_fade_.reset(); 

             SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges (READY_FOR_STATE_SWAP): Pushing target state %p (type %d).",
                          (void*)targetStateToPush.get(), (int)targetStateToPush->getType());
             // Directly push, don't use requestPushState, as we need to call enter before fade-in.
             push_state(std::move(targetStateToPush)); 
             if (!states_.empty()) {
                SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges (READY_FOR_STATE_SWAP): Calling enter() on new state %p before fade-in.", (void*)states_.back().get());
                states_.back()->enter(); // Call enter on the new state
             }
        } else {
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges (READY_FOR_STATE_SWAP): No pending_state_for_fade_ to push. Underlying state should have been entered.");
             // If no pending state, the state that was under the popped one (if pop_current_after_fade_out_ was true)
             // should have already had enter() called in the FADING_OUT completion block.
        }
        
        auto fadeInState = std::make_unique<TransitionState>(this, fade_duration_, TransitionEffectType::FADE_FROM_COLOR, SDL_Color{0,0,0,255});
        active_fade_type_ = TransitionEffectType::FADE_FROM_COLOR;
        requestPushState(std::move(fadeInState)); // Request push for the fade-in
        fade_step_ = FadeSequenceStep::FADING_IN;
    }

    // --- 3. Handle Pushes ---
    if (request_push_) {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Processing request_push_ for state %p (type %d).",
                     (void*)request_push_.get(), (int)request_push_->getType());
        // If there's a current state, and it's not a transition state that's about to cover it, call its exit().
        // However, for fades, exit() of the underlying state is typically handled before the fade-out starts,
        // or not at all if it's meant to resume.
        // For now, let's assume exit() is handled by the states themselves or by pop logic.
        // if (!states_.empty() && states_.back()->getType() != StateType::Transition) {
        //     states_.back()->exit(); // Exit previous state if not a transition
        // }
        
        std::unique_ptr<GameState> newState = std::move(request_push_);
        request_push_.reset(); 

        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Calling enter() on pushed state %p.", (void*)newState.get());
        newState->enter(); // Call enter on the new state being pushed
        push_state(std::move(newState)); // Actual push
    }
}

// --- quit_game ---
void Game::quit_game() {
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Quit requested.");
     is_running = false;
}

// --- Getters ---
PCDisplay* Game::get_display() { return &display; }
AssetManager* Game::getAssetManager() { return &assetManager; }
InputManager* Game::getInputManager() { return &inputManager; }
PlayerData* Game::getPlayerData() { return &playerData_; }
TextRenderer* Game::getTextRenderer() { return textRenderer_.get(); }
AnimationManager* Game::getAnimationManager() { return animationManager_.get(); }

GameState* Game::getCurrentState() {
    return states_.empty() ? nullptr : states_.back().get();
}

GameState* Game::getUnderlyingState(const GameState* currentState) {
    if (states_.size() < 2) { return nullptr; }
    // Find the index of currentState
    size_t currentIndex = states_.size(); // Invalid index initially
    for (size_t i = 0; i < states_.size(); ++i) {
        if (states_[i].get() == currentState) {
            currentIndex = i;
            break;
        }
    }
    // If found and not the bottom-most state, return the one below it
    if (currentIndex > 0 && currentIndex < states_.size()) {
        return states_[currentIndex - 1].get();
    }
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "getUnderlyingState: currentState (%p) not found on stack or is the bottom-most state.", (void*)currentState);
    return nullptr;
}

// Update game settings from configuration
void Game::updateFromConfig() {
    if (textRenderer_) {
        textRenderer_->updateGlobalTextScaleFromConfig();
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Text renderer updated from config.");
    }
    
    // Other configuration updates can be added here in the future
}

// Reload configuration and update settings
bool Game::reloadConfig() {
    bool success = ConfigManager::reload();
    if (success) {
        updateFromConfig();
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Configuration reloaded and settings updated.");
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to reload configuration.");
    }
    return success;
}


// --- close ---
void Game::close() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Shutting down Game systems...");
    if (!states_.empty()) {
         states_.back()->exit();
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Called exit() on final state during shutdown.");
    }
    states_.clear();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "State stack cleared.");

    animationManager_.reset();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager reset.");
    textRenderer_.reset();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer reset.");

    assetManager.shutdown();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetManager shutdown.");
    display.close();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay closed.");
    SDL_Quit();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL quit.");
}

// --- DEBUG_getStack Implementation ---
std::vector<std::unique_ptr<GameState>>& Game::DEBUG_getStack() { return states_; }

void Game::processPopUntil() {
    if (pop_until_target_type_ != StateType::None) {
        StateType targetType = pop_until_target_type_;
        pop_until_target_type_ = StateType::None;
        
        // Find the target state in the stack
        bool found = false;
        for (auto it = states_.rbegin(); it != states_.rend(); ++it) {
            if ((*it)->getType() == targetType) {
                found = true;
                break;
            }
        }
        
        if (!found) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "processPopUntil: Target state %d not found in stack!", (int)targetType);
            return;
        }
        
        // Pop states until we reach the target
        while (!states_.empty() && states_.back()->getType() != targetType) {
            states_.back()->exit();
            pop_state();
        }
        
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "processPopUntil: Popped states until reaching %d", (int)targetType);
        
        if (!states_.empty()) {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "processPopUntil: Calling enter() on target state %p (type %d).",
                        (void*)states_.back().get(), (int)states_.back()->getType());
            states_.back()->enter();
        }
    }
}