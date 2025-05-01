// File: src/core/Game.cpp

#include "core/Game.h"
#include "states/AdventureState.h" // Needed for initial state
// Note: GameState.h is included via Game.h now
#include <SDL_log.h>
#include <stdexcept>
#include <filesystem> // For C++17 filesystem path functions

Game::Game() : is_running(false), last_frame_time(0) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Game constructor called.");
}

Game::~Game() {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Game destructor called.");
    // Consider if close() needs to be explicitly called, depends on overall app structure
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

    // --- CWD LOGGING ---
    try {
        std::filesystem::path cwd = std::filesystem::current_path();
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Current Working Directory: %s", cwd.string().c_str());
    } catch (const std::filesystem::filesystem_error& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error getting CWD using std::filesystem: %s", e.what());
    }
    // --- END CWD LOGGING ---

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Attempting to load initial assets...");
     bool assets_ok = true;
     // --- Using Double Backslashes for Paths ---
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
     // --- END Paths ---

     if (!assets_ok) {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "One or more essential assets failed to load!");
         // Ensure cleanup happens correctly on init failure
         assetManager.shutdown();
         display.close();
         SDL_Quit();
         return false;
     }
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Finished loading initial assets attempt.");

    try {
       push_state(std::make_unique<AdventureState>(this));
       SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Initial AdventureState pushed.");
    } catch (const std::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create initial state: %s", e.what());
        // Ensure cleanup happens correctly on init failure
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

void Game::run() {
    if (!is_running || states_.empty()) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Game::run() called in invalid state."); return; }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Entering main game loop.");
    last_frame_time = SDL_GetTicks(); // Ensure timer starts correctly

    while (is_running) {
        Uint32 current_time = SDL_GetTicks();
        float delta_time = (current_time - last_frame_time) / 1000.0f;
        if (delta_time > 0.1f) delta_time = 0.1f;
        last_frame_time = current_time;

        if (states_.empty()) { SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,"State stack empty, quitting."); is_running = false; break; }

        SDL_Event event;
        while (SDL_PollEvent(&event)) { if (event.type == SDL_QUIT) { quit_game(); } }

        GameState* currentState = getCurrentState(); // Use the getter function
        if (currentState) {
            currentState->handle_input();
            currentState->update(delta_time);
            display.clear(0x0000);
            currentState->render();
            display.present();
        } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Top state is NULL! Exiting loop."); is_running = false; break; }
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Exited main game loop.");
}

void Game::push_state(std::unique_ptr<GameState> new_state) {
    if (new_state) { SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Pushing state..."); states_.push_back(std::move(new_state)); }
    else { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Attempted to push NULL state!"); }
}

void Game::pop_state() {
    if (!states_.empty()) { SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Popping state..."); states_.pop_back(); }
    else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Attempted pop from empty state stack!"); }
}

// <<< --- ADDED Missing Implementation --- >>>
GameState* Game::getCurrentState() {
    return states_.empty() ? nullptr : states_.back().get();
}
// <<< --- END Added Implementation --- >>>


void Game::quit_game() {
    if(is_running) { SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Quit requested."); is_running = false; }
}

PCDisplay* Game::get_display() {
    return &display;
}

AssetManager* Game::getAssetManager() {
     return &assetManager;
}

void Game::close() {
     if (display.isInitialized()) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Closing game systems...");
        states_.clear(); // Destroy states first
        assetManager.shutdown();
        display.close();
        SDL_Quit(); // Shutdown SDL last
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Game systems closed.");
    }
}