// File: src/core/Game.cpp

#include "core/Game.h"
#include "states/AdventureState.h"
#include <SDL_log.h>
#include <stdexcept>
#include <filesystem>

Game::Game() : is_running(false), last_frame_time(0), request_pop_(false) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Game constructor called.");
}

Game::~Game() {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Game destructor called.");
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

    // --- CWD LOGGING (Keep this info) ---
    try {
        std::filesystem::path cwd = std::filesystem::current_path();
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Current Working Directory: %s", cwd.string().c_str());
    } catch (const std::filesystem::filesystem_error& e) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error getting CWD using std::filesystem: %s", e.what()); }
    // --- END CWD LOGGING ---

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Attempting to load initial assets...");
     bool assets_ok = true;
     // Keep logs for loading success/failure (using INFO/DEBUG/ERROR levels helps)
     assets_ok &= assetManager.loadTexture("agumon_sheet", "assets\\sprites\\agumon_sheet.png");
     assets_ok &= assetManager.loadTexture("gabumon_sheet", "assets\\sprites\\gabumon_sheet.png");
     assets_ok &= assetManager.loadTexture("biyomon_sheet", "assets\\sprites\\biyomon_sheet.png");
     assets_ok &= assetManager.loadTexture("gatomon_sheet", "assets\\sprites\\gatomon_sheet.png");
     assets_ok &= assetManager.loadTexture("gomamon_sheet", "assets\\sprites\\gomamon_sheet.png");
     assets_ok &= assetManager.loadTexture("palmon_sheet", "assets\\sprites\\palmon_sheet.png");
     assets_ok &= assetManager.loadTexture("tentomon_sheet", "assets\\sprites\\tentomon_sheet.png");
     assets_ok &= assetManager.loadTexture("patamon_sheet", "assets\\sprites\\patamon_sheet.png");
     assets_ok &= assetManager.loadTexture("castle_bg_0", "assets\\backgrounds\\castlebackground0.png");
     assets_ok &= assetManager.loadTexture("castle_bg_1", "assets\\backgrounds\\castlebackground1.png");
     assets_ok &= assetManager.loadTexture("castle_bg_2", "assets\\backgrounds\\castlebackground2.png");
     assets_ok &= assetManager.loadTexture("menu_bg_blue", "assets\\ui\\backgrounds\\menu_base_blue.png");
     assets_ok &= assetManager.loadTexture("transition_borders", "assets\\ui\\transition\\transition_borders.png");

     if (!assets_ok) {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "One or more essential assets failed to load!");
         assetManager.shutdown(); display.close(); SDL_Quit();
         return false;
     }
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Finished loading initial assets attempt.");

    // Push initial state directly
    try {
       states_.push_back(std::make_unique<AdventureState>(this));
       SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Initial AdventureState created and added.");
    } catch (const std::exception& e) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create initial state: %s", e.what()); assetManager.shutdown(); display.close(); SDL_Quit(); return false; }

    is_running = true;
    last_frame_time = SDL_GetTicks();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Game Initialization Successful.");
    return true;
}

// --- Run Loop (Reduced Logging) ---
void Game::run() {
    if (!is_running || states_.empty()) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Game::run() called in invalid state."); return; }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Entering main game loop.");
    last_frame_time = SDL_GetTicks();

    while (is_running) {
        Uint32 current_time = SDL_GetTicks();
        float delta_time = (current_time - last_frame_time) / 1000.0f;
        if (delta_time > 0.1f) delta_time = 0.1f;
        last_frame_time = current_time;

        // --- Process OS Events ---
        SDL_Event event;
        while (SDL_PollEvent(&event)) { if (event.type == SDL_QUIT) { quit_game(); } }

        // --- Update Top State (if any) ---
        if (!states_.empty()) {
            GameState* currentStatePtr = states_.back().get();
            if (currentStatePtr) {
                // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "RunLoop: Updating state %p", currentStatePtr); // <<< COMMENTED OUT >>>
                currentStatePtr->handle_input();
                currentStatePtr->update(delta_time);
            } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "RunLoop Update: Top state pointer is NULL despite non-empty stack!"); is_running = false; }
        } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "RunLoop Update: State stack unexpectedly empty."); is_running = false; }

        // --- Apply Pending State Changes ---
        // Keep logs for state changes as they are important events
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "RunLoop: Before applyStateChanges. Stack size = %zu", states_.size());
        applyStateChanges();
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "RunLoop: After applyStateChanges. Stack size = %zu", states_.size());

        // --- Render Top State (if any) ---
        // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "RunLoop: Preparing to render."); // <<< COMMENTED OUT >>>
        if (!states_.empty()) {
             GameState* currentStateForRender = getCurrentState(); // Calls function which no longer logs frequently
             // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "RunLoop: Got currentStateForRender = %p.", currentStateForRender); // <<< COMMENTED OUT >>>
             if (currentStateForRender) {
                 // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "RunLoop: Calling render() for state %p...", currentStateForRender); // <<< COMMENTED OUT >>>
                 display.clear(0x0000);
                 currentStateForRender->render(); // This might still have its own logs
                 display.present();
                 // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "RunLoop: Finished render() for state %p.", currentStateForRender); // <<< COMMENTED OUT >>>
             } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "RunLoop: Render phase - getCurrentState returned NULL despite non-empty stack?"); }
        } else {
             SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,"RunLoop: Render phase - State stack empty, skipping render.");
             is_running = false;
        }

        // --- Check Running Flag ---
        if (!is_running) { /* SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "RunLoop: is_running is false, breaking loop."); // <<< COMMENTED OUT >>> */ break; }

        // --- Frame Limiter (Optional) ---
        // SDL_Delay(...);
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Exited main game loop.");
}

// --- State Management - Actual Push/Pop ---
// Keep INFO logs for these
void Game::push_state(std::unique_ptr<GameState> new_state) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Executing push_state for state %p...", new_state.get());
    states_.push_back(std::move(new_state));
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "push_state complete. New stack size: %zu", states_.size());
}

void Game::pop_state() {
    if (!states_.empty()) {
         GameState* stateToPop = states_.back().get();
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Executing pop_state for state %p...", stateToPop);
         states_.pop_back();
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "pop_state complete. New stack size: %zu", states_.size());
    } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "pop_state called on empty stack!"); }
}

// --- State Management - Requests ---
// Keep DEBUG logs for requests
void Game::requestPushState(std::unique_ptr<GameState> state) {
    if (!state) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Attempted to request push of NULL state!"); return; }
    if (request_push_) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Overwriting previous push request."); }
    request_push_ = std::move(state);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Push requested.");
}

void Game::requestPopState() {
    if(request_pop_) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Multiple pop requests made in one frame.");
    request_pop_ = true;
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Pop requested.");
}

// --- Helper to apply queued changes ---
// Keep DEBUG logs here
void Game::applyStateChanges() {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyStateChanges: Start. RequestPop=%d, RequestPush=%p", request_pop_, request_push_.get());
    if (request_pop_) { SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyStateChanges: Applying Pop request."); pop_state(); request_pop_ = false; }
    if (request_push_) { SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyStateChanges: Applying Push request for state %p.", request_push_.get()); push_state(std::move(request_push_)); request_push_.reset(); }
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyStateChanges: End. Stack size = %zu", states_.size());
}

// --- getCurrentState ---
// Removed frequent logging from here
GameState* Game::getCurrentState() {
    GameState* topState = states_.empty() ? nullptr : states_.back().get();
    return topState;
}

// --- Other functions ---
// Keep INFO logs
void Game::quit_game() { /* ... */ }
PCDisplay* Game::get_display() { return &display; }
AssetManager* Game::getAssetManager() { return &assetManager; }
void Game::close() { /* ... */ }