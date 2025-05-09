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

#include <SDL_log.h>
#include <stdexcept>
#include <filesystem> // For CWD logging
#include <vector>
#include <memory>
#include <string>

Game::Game() :
    is_running(false),
    last_frame_time(0),
    request_pop_(false),
    request_push_(nullptr),
    pop_until_target_type_(StateType::None),
    fade_step_(FadeSequenceStep::NONE),
    pending_state_for_fade_(nullptr),
    active_fade_type_(TransitionEffectType::BORDER_WIPE), // Default
    fade_duration_(0.5f),
    pop_current_after_fade_out_(true), // Default behavior is to pop
    textRenderer_(nullptr),
    animationManager_(nullptr)
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

    if (!display.init(title.c_str(), width, height)) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay Init Error"); SDL_Quit(); return false; }
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
        }

        // State Logic
        applyStateChanges(); // Apply pops/pushes requested in previous frame

        if (!states_.empty()) {
            GameState* currentStatePtr = states_.back().get(); // Get current state *after* potential changes
            if (currentStatePtr) {
                PlayerData* pd = getPlayerData(); // Get player data pointer

                // Handle input for the current state
                currentStatePtr->handle_input(inputManager, pd);

                // Update the current state ONLY if it wasn't changed by input handling
                // (Check if the pointer is still the same as the top of the stack)
                if (is_running && !states_.empty() && states_.back().get() == currentStatePtr) {
                    currentStatePtr->update(delta_time, pd);
                }
            }
        }

        // Rendering
        if (is_running && !states_.empty()) {
            GameState* currentStateForRender = getCurrentState(); // Get current state again
            if (currentStateForRender) {
                // 1. Clear the screen
                display.clear(0x0000); // Black background

                // 2. Render the actual current game state
                currentStateForRender->render(display);

                // 3. Draw the Overlay Mask (Optional - keep if needed)
                SDL_Texture* maskTexture = assetManager.getTexture("ui_round_mask");
                if (maskTexture) {
                    SDL_Renderer* renderer = display.getRenderer();
                    if (renderer) {
                        SDL_SetTextureBlendMode(maskTexture, SDL_BLENDMODE_BLEND);
                        int w = 0, h = 0;
                        display.getWindowSize(w, h);
                        if (w > 0 && h > 0) {
                            SDL_Rect destRect = {0, 0, w, h};
                            SDL_RenderCopy(renderer, maskTexture, NULL, &destRect);
                        } else { /* Log Warn */ }
                    } else { /* Log Error */ }
                } else { /* Log Warn (once) */ }

                // 4. Present the final frame
                display.present();
            }
        } else if (!is_running) {
            // If quit was requested, break the loop immediately
            break;
        } else { // states_ is empty but is_running is true? Should not happen if init pushes a state.
             SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Game loop running but state stack is empty!");
             is_running = false; // Force quit
        }

        // --- Frame Limiter (Simplified) --- REMOVED for now, rely on VSync
        // ...
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Exited main game loop.");
    close(); // Ensure cleanup happens if loop exits
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
    if (!targetState) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "requestFadeToState called with null targetState."); return; }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Requesting Fade-To-State sequence (Target Type: %d, Duration: %.2fs, PopCurrent: %s).",
                (int)targetState->getType(), duration, popCurrent ? "true" : "false"); // Log target type
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

// --- Helper to apply queued changes ---
// <<< --- INTEGRATED NEW VERSION --- >>>
void Game::applyStateChanges() {

    bool state_popped_this_frame = false;
    TransitionEffectType popped_transition_type = TransitionEffectType::BORDER_WIPE; // Default unimportant

    // --- 1. Handle Pops ---
    if (pop_until_target_type_ != StateType::None) {
        // --- Pop Until Logic ---
        while (!states_.empty()) {
            GameState* topState = states_.back().get();
            if (topState->getType() == pop_until_target_type_) {
                SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "applyStateChanges (PopUntil): Found target state %p (type %d). Calling enter().",
                             (void*)topState, (int)topState->getType());
                topState->enter(); // Enter the target state
                state_popped_this_frame = true;
                break; // Stop popping
            }
            // Exit and pop states above the target
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "applyStateChanges (PopUntil): Exiting and popping state %p (type %d).",
                         (void*)topState, (int)topState->getType());
            topState->exit();
            pop_state();
            state_popped_this_frame = true;
        }
        if (states_.empty() && pop_until_target_type_ != StateType::None) {
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "PopUntil target type %d not found in stack.", (int)pop_until_target_type_);
        }
        // Reset pop_until regardless of whether target was found
        pop_until_target_type_ = StateType::None;
        request_pop_ = false; // Clear other pop flags
        request_push_.reset(); // Clear push requests

        // Abort any ongoing fade sequence if pop_until was executed
        if (fade_step_ != FadeSequenceStep::NONE) {
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "applyStateChanges: pop_until executed during fade sequence. Resetting fade.");
             fade_step_ = FadeSequenceStep::NONE;
             pending_state_for_fade_.reset();
             active_fade_type_ = TransitionEffectType::BORDER_WIPE;
        }
    }
    else if (request_pop_) {
        // --- Handle Single Pop ---
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "applyStateChanges: Processing request_pop_ = true.");
        if (!states_.empty()) {
            GameState* topState = states_.back().get();
            StateType popped_state_type = topState->getType();
            popped_transition_type = TransitionEffectType::BORDER_WIPE; // Reset
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "applyStateChanges: Popping state type %d (ptr: %p). Stack size BEFORE pop: %zu",
                         (int)popped_state_type, (void*)topState, states_.size());

            if (popped_state_type == StateType::Transition) {
               TransitionState* ts = static_cast<TransitionState*>(topState);
               popped_transition_type = ts->getEffectType();
               SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "applyStateChanges: Popped state was Transition, effect type: %d", (int)popped_transition_type);
            }

            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "applyStateChanges: Calling exit() on state %p.", (void*)topState);
            topState->exit();
            pop_state(); // Actual removal from stack
            state_popped_this_frame = true;
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "applyStateChanges: State popped. Stack size AFTER pop: %zu", states_.size());


            // Check if this pop completed a fade step
            if (fade_step_ == FadeSequenceStep::FADING_OUT &&
                popped_state_type == StateType::Transition &&
                popped_transition_type == TransitionEffectType::FADE_TO_COLOR &&
                pending_state_for_fade_)
            {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Fade Sequence: FADE_TO_COLOR transition finished.");
                fade_step_ = FadeSequenceStep::READY_FOR_STATE_SWAP;
            }
            else if (fade_step_ == FadeSequenceStep::FADING_IN &&
                     popped_state_type == StateType::Transition &&
                     popped_transition_type == TransitionEffectType::FADE_FROM_COLOR)
            {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Fade Sequence: FADE_FROM_COLOR transition finished. Sequence complete.");
                fade_step_ = FadeSequenceStep::NONE;
                active_fade_type_ = TransitionEffectType::BORDER_WIPE;
                if (!states_.empty()) {
                    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "applyStateChanges: Calling enter() on target state %p after FADE_FROM_COLOR.",
                                 (void*)states_.back().get());
                    states_.back()->enter();
                } else {
                    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "applyStateChanges: Stack empty after FADE_FROM_COLOR pop.");
                }
            }
            else {
                SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "applyStateChanges: Handling as normal pop.");
                if (!states_.empty()) {
                    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "applyStateChanges: Calling enter() on new top state %p (type %d).",
                                 (void*)states_.back().get(), (int)states_.back()->getType());
                    states_.back()->enter();
                } else {
                    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "applyStateChanges: State stack is empty after pop!");
                }
                if(fade_step_ != FadeSequenceStep::NONE ) {
                     SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "applyStateChanges: Pop (type %d, effect %d) occurred during fade step %d but was not expected. Resetting fade.",
                                 (int)popped_state_type, (int)popped_transition_type, (int)fade_step_);
                     fade_step_ = FadeSequenceStep::NONE;
                     pending_state_for_fade_.reset();
                     active_fade_type_ = TransitionEffectType::BORDER_WIPE;
                }
            }
        } else {
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "applyStateChanges: request_pop_ was true, but state stack was already empty!");
        }
        request_pop_ = false; // Reset the single pop request flag
    }

    // --- 2. Handle State Swap (if fade out just finished) ---
    if (fade_step_ == FadeSequenceStep::READY_FOR_STATE_SWAP) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Fade Sequence: Swapping states.");

        // Pop the original state if requested
        if (pop_current_after_fade_out_ && !states_.empty()) {
             if (pending_state_for_fade_ && states_.back().get() != pending_state_for_fade_.get()) {
                 SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Fade Swap: Popping state %p (type %d) because pop_current_after_fade_out_ is true.",
                              (void*)states_.back().get(), (int)states_.back()->getType());
                 states_.back()->exit();
                 pop_state();
             } else if (!pending_state_for_fade_ && !states_.empty()) {
                 SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Fade Swap: pop_current is true, but pending_state is null. Popping current top %p.", (void*)states_.back().get());
                 states_.back()->exit();
                 pop_state();
             }
        }

        // Push the target state (e.g., MenuState)
        if (pending_state_for_fade_) {
             std::unique_ptr<GameState> targetStateToPush = std::move(pending_state_for_fade_);
             pending_state_for_fade_.reset();

             // Push the target state
             SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Fade Swap: Pushing target state %p (type %d).",
                          (void*)targetStateToPush.get(), (int)targetStateToPush->getType());
             push_state(std::move(targetStateToPush));
             // DO NOT call enter() on targetState yet. It will be entered after FadeIn.

        } else {
             SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Fade Sequence Error: Ready for swap but pending_state_for_fade_ is null!");
             fade_step_ = FadeSequenceStep::NONE; // Abort
        }

        // Push the FADE_FROM_COLOR transition (only if swap didn't abort)
        if (fade_step_ == FadeSequenceStep::READY_FOR_STATE_SWAP) {
             active_fade_type_ = TransitionEffectType::FADE_FROM_COLOR;
             auto fadeInState = std::make_unique<TransitionState>( this, fade_duration_, active_fade_type_ );

             // No exit on the targetState here, the FadeIn state just goes on top.
             SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Fade Swap: Pushing FadeIn state %p over target state %p.",
                          (void*)fadeInState.get(), (states_.empty() ? nullptr : (void*)states_.back().get()) );
             push_state(std::move(fadeInState));

             if (!states_.empty()) {
                 SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Fade Swap: Entering FadeIn state %p.", (void*)states_.back().get());
                 states_.back()->enter(); // Enter ONLY the FadeIn State
             }
             fade_step_ = FadeSequenceStep::FADING_IN;
        }
        pending_state_for_fade_.reset(); // Ensure cleared if not already
    }

    // --- 3. Handle Regular Push ---
    if (request_push_) {
        if (fade_step_ == FadeSequenceStep::NONE || fade_step_ == FadeSequenceStep::FADING_OUT) {

            bool is_fade_out_push = (fade_step_ == FadeSequenceStep::FADING_OUT);

            if (!states_.empty()) {
                if (!is_fade_out_push) {
                    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "applyStateChanges: Exiting state %p before normal push.", (void*)states_.back().get());
                    states_.back()->exit();
                } else {
                    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "applyStateChanges: Pushing FadeOut transition OVER state %p (no exit called).", (void*)states_.back().get());
                }
            }

            std::unique_ptr<GameState> stateToPush = std::move(request_push_);
            // request_push_ is now nullptr

            push_state(std::move(stateToPush));

            if (!states_.empty()) {
                SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "applyStateChanges: Entering newly pushed state %p.", (void*)states_.back().get());
                states_.back()->enter();
            }
        } else {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "applyStateChanges: Push requested during fade step %d. Push ignored.", (int)fade_step_);
            request_push_.reset(); // Discard the push if ignored
        }
    }
    // If request_push_ was moved from, it's already null. If it was reset, it's null.
    // No need for a final reset here.

} // End applyStateChanges


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