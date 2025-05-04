// File: src/core/Game.cpp

#include "core/Game.h"
#include "states/AdventureState.h" // Needed for initial state push
#include <SDL_log.h>
#include <stdexcept>
#include <filesystem> // For CWD logging

// Include standard library headers needed by this file
#include <vector>
#include <memory>
#include <string>


Game::Game() : is_running(false), last_frame_time(0), request_pop_(false), request_push_(nullptr) { // <<< Initialized request_push_ >>>
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Game constructor called.");
}

Game::~Game() {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Game destructor called.");
    // Cleanup happens in close()
}

bool Game::init(const std::string& title, int width, int height) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Initializing Game systems...");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL Init Error: %s", SDL_GetError()); return false; }
    // Set texture filtering to nearest neighbor
    if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0")) { SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "Hint Warning: %s", SDL_GetError()); }
    else { SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "SDL_HINT_RENDER_SCALE_QUALITY set to 0 (Nearest Neighbor)."); }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL Initialized.");

    // Initialize display (window and renderer)
    if (!display.init(title.c_str(), width, height)) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay Init Error"); SDL_Quit(); return false; }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay Initialized.");

    // Initialize asset manager
    if (!assetManager.init(display.getRenderer())) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AssetManager Init Error"); display.close(); SDL_Quit(); return false; }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetManager Initialized.");

    // Log Current Working Directory
    try {
        std::filesystem::path cwd = std::filesystem::current_path();
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Current Working Directory: %s", cwd.string().c_str());
    } catch (const std::filesystem::filesystem_error& e) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error getting CWD using std::filesystem: %s", e.what()); }

    // Load initial assets
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Attempting to load initial assets...");
     bool assets_ok = true;

     // --- Digimon Sprites ---
     assets_ok &= assetManager.loadTexture("agumon_sheet", "assets\\sprites\\agumon_sheet.png");
     assets_ok &= assetManager.loadTexture("gabumon_sheet", "assets\\sprites\\gabumon_sheet.png");
     assets_ok &= assetManager.loadTexture("biyomon_sheet", "assets\\sprites\\biyomon_sheet.png");
     assets_ok &= assetManager.loadTexture("gatomon_sheet", "assets\\sprites\\gatomon_sheet.png");
     assets_ok &= assetManager.loadTexture("gomamon_sheet", "assets\\sprites\\gomamon_sheet.png");
     assets_ok &= assetManager.loadTexture("palmon_sheet", "assets\\sprites\\palmon_sheet.png");
     assets_ok &= assetManager.loadTexture("tentomon_sheet", "assets\\sprites\\tentomon_sheet.png");
     assets_ok &= assetManager.loadTexture("patamon_sheet", "assets\\sprites\\patamon_sheet.png");

     // --- Backgrounds ---
     assets_ok &= assetManager.loadTexture("castle_bg_0", "assets\\backgrounds\\castlebackground0.png");
     assets_ok &= assetManager.loadTexture("castle_bg_1", "assets\\backgrounds\\castlebackground1.png");
     assets_ok &= assetManager.loadTexture("castle_bg_2", "assets\\backgrounds\\castlebackground2.png");

     // --- UI Assets ---
     assets_ok &= assetManager.loadTexture("menu_bg_blue", "assets\\ui\\backgrounds\\menu_base_blue.png");
     assets_ok &= assetManager.loadTexture("transition_borders", "assets\\ui\\transition\\transition_borders.png"); // Keep if used elsewhere

     // <<< --- ADDED UI FONT LOADING --- >>>
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Loading UI font...");
     // Use forward slashes for better cross-platform compatibility
     assets_ok &= assetManager.loadTexture("ui_font_atlas", "assets/ui/fonts/bluewhitefont.png"); // Use the specified font path
     // Cursor loading removed as per clarification
     // <<< ----------------------------- >>>


     if (!assets_ok) {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "One or more essential assets failed to load! Check paths and file existence.");
         assetManager.shutdown(); display.close(); SDL_Quit();
         return false;
     }
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Finished loading initial assets attempt.");

    // Push initial state (AdventureState)
    try {
       states_.push_back(std::make_unique<AdventureState>(this));
       SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Initial AdventureState created and added.");
    } catch (const std::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create initial state: %s", e.what());
        assetManager.shutdown(); display.close(); SDL_Quit(); return false;
    }

    is_running = true;
    last_frame_time = SDL_GetTicks(); // Initialize frame timer
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
    last_frame_time = SDL_GetTicks(); // Ensure timer starts correctly

    while (is_running) {
        // Calculate delta time
        Uint32 current_time = SDL_GetTicks();
        float delta_time = (current_time - last_frame_time) / 1000.0f;
        // Clamp delta time to prevent large jumps if debugging/pausing
        if (delta_time > 0.1f) delta_time = 0.1f;
        last_frame_time = current_time;

        // --- Process OS Events ---
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit_game(); // Request quit
            }
            // TODO: Pass relevant events down to current state's handle_input if needed
        }

        // --- Update Top State (if any) ---
        if (!states_.empty()) {
            GameState* currentStatePtr = states_.back().get();
            if (currentStatePtr) {
                currentStatePtr->handle_input(); // State handles direct polling for now
                currentStatePtr->update(delta_time);
            } else {
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "RunLoop Update: Top state pointer is NULL despite non-empty stack!");
                is_running = false; // Treat as critical error
            }
        } else {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "RunLoop Update: State stack unexpectedly empty.");
            is_running = false; // No states left, stop running
        }

        // --- Apply Pending State Changes ---
        applyStateChanges();

        // --- Render Top State (if any) ---
        if (!states_.empty()) {
             GameState* currentStateForRender = getCurrentState();
             if (currentStateForRender) {
                 display.clear(0x0000); // Clear screen (to black)
                 currentStateForRender->render(); // Render the current state
                 display.present(); // Show the result on screen
             } else {
                 SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "RunLoop: Render phase - getCurrentState returned NULL despite non-empty stack?");
             }
        } else {
             // If stack becomes empty after state changes, maybe log info and stop
             SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,"RunLoop: Render phase - State stack empty, skipping render and stopping.");
             is_running = false;
        }

        // --- Check Running Flag ---
        if (!is_running) {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "RunLoop: is_running is false, breaking loop.");
            break;
        }

        // --- Frame Limiter (Optional but recommended) ---
        // Uint32 frame_duration = SDL_GetTicks() - current_time;
        // if (frame_duration < 16) { // Roughly 16ms per frame for 60 FPS
        //     SDL_Delay(16 - frame_duration);
        // }
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Exited main game loop.");
    close(); // Perform cleanup after loop ends
}

// --- State Management - Actual Push/Pop ---
void Game::push_state(std::unique_ptr<GameState> new_state) {
    if (!new_state) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "push_state called with null state!");
        return;
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Executing push_state for state %p...", (void*)new_state.get());
    states_.push_back(std::move(new_state));
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "push_state complete. New stack size: %zu", states_.size());
}

void Game::pop_state() {
    if (!states_.empty()) {
         GameState* stateToPop = states_.back().get();
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Executing pop_state for state %p...", (void*)stateToPop);
         // unique_ptr handles destruction when popped
         states_.pop_back();
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "pop_state complete. New stack size: %zu", states_.size());
    } else {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "pop_state called on empty stack!");
    }
}

// --- State Management - Requests ---
void Game::requestPushState(std::unique_ptr<GameState> state) {
    if (!state) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Attempted to request push of NULL state!"); return; }
    // If a push is already pending, the new one replaces it
    if (request_push_) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Overwriting previous push request."); }
    request_push_ = std::move(state);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Push requested for state %p.", (void*)request_push_.get());
}

void Game::requestPopState() {
    // Only log warning if multiple pops requested *before* applyStateChanges runs
    if(request_pop_) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Multiple pop requests made before applyStateChanges.");
    request_pop_ = true;
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Pop requested.");
}

// --- Helper to apply queued changes ---
void Game::applyStateChanges() {
    // Process pop first, then push for this cycle
    if (request_pop_) {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyStateChanges: Applying Pop request.");
        pop_state();
        request_pop_ = false; // Reset flag after processing
    }
    if (request_push_) {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyStateChanges: Applying Push request for state %p.", (void*)request_push_.get());
        push_state(std::move(request_push_)); // push_state moves ownership
        request_push_.reset(); // Clear the pending request unique_ptr
    }
}

// --- getCurrentState ---
GameState* Game::getCurrentState() {
    // Returns nullptr if stack is empty
    GameState* topState = states_.empty() ? nullptr : states_.back().get();
    return topState;
}

// --- quit_game ---
void Game::quit_game() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Quit requested.");
    is_running = false; // Set flag to end the run loop
}

// --- Getters ---
PCDisplay* Game::get_display() {
    return &display;
}

AssetManager* Game::getAssetManager() {
    return &assetManager;
}

// --- close ---
void Game::close() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Shutting down Game systems...");
    // Clear state stack (unique_ptrs handle deletion)
    states_.clear();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "State stack cleared.");
    // Shutdown subsystems
    assetManager.shutdown();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetManager shutdown.");
    display.close();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay closed.");
    SDL_Quit();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL quit.");
}

// --- DEBUG_getStack Implementation ---
std::vector<std::unique_ptr<GameState>>& Game::DEBUG_getStack() {
    return states_; // Return reference to the stack
}