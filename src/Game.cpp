// File: src/core/Game.cpp

#include "core/Game.h"           // Include own header
#include "states/GameState.h"    // Include base state class
#include "states/AdventureState.h" // Include the initial state class
#include <SDL_log.h>             // <<< CORRECTED SDL Include >>>
#include <stdexcept>             // Standard

// ... (rest of Game.cpp implementation remains the same as provided before) ...

Game::Game() : is_running(false), last_frame_time(0) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Game constructor called.");
}

Game::~Game() {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Game destructor called.");
    close(); // Ensure cleanup happens via destructor
}

bool Game::init(const std::string& title, int width, int height) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Initializing Game systems...");

    // Initialize SDL subsystems (video required for window)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return false;
    }
     // <<< ADDED: Set Render Hint for Pixel Art BEFORE creating renderer >>>
     if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0")) { // "0" is nearest pixel sampling
         SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "Warning: Nearest pixel sampling hint not enabled!");
     } else {
          SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "SDL_HINT_RENDER_SCALE_QUALITY set to 0 (Nearest Neighbor).");
     }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL Initialized.");

    // Initialize the display wrapper (creates window & renderer)
    if (!display.init(title.c_str(), width, height)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay initialization failed!");
        SDL_Quit(); // Clean up SDL if display fails
        return false;
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay Initialized.");

    // <<< INITIALIZE ASSET MANAGER (needs renderer from display) >>>
    if (!assetManager.init(display.getRenderer())) {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AssetManager initialization failed!");
         display.close();
         SDL_Quit();
         return false;
    }
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetManager Initialized.");

     // <<< LOAD INITIAL ASSETS TEST >>>
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Attempting to load initial assets...");
     // Use relative paths from the executable's expected working directory
     bool assets_ok = true;
     assets_ok &= assetManager.loadTexture("agumon_sheet", "assets/sprites/agumon_sheet.png");
     assets_ok &= assetManager.loadTexture("castle_bg_0", "assets/backgrounds/castlebackground0.png");
     assets_ok &= assetManager.loadTexture("castle_bg_1", "assets/backgrounds/castlebackground1.png");
     assets_ok &= assetManager.loadTexture("castle_bg_2", "assets/backgrounds/castlebackground2.png");
     // Add loads for other digimon sheets if you have them ready
     // assets_ok &= assetManager.loadTexture("gabumon_sheet", "assets/sprites/gabumon_sheet.png");

     if (!assets_ok) {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "One or more essential assets failed to load!");
         // Consider cleanup and returning false if loading fails
         assetManager.shutdown();
         display.close();
         SDL_Quit();
         return false;
     }
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Finished loading initial assets attempt.");


    // Set initial state (Adventure State)
    try {
        // Pass 'this' (pointer to the Game object) to the state constructor
       current_state = std::make_unique<AdventureState>(this);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Initial AdventureState created.");
    } catch (const std::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create initial state: %s", e.what());
        assetManager.shutdown(); // Cleanup assets if state creation fails
        display.close();
        SDL_Quit();
        return false;
    }

    is_running = true;
    last_frame_time = SDL_GetTicks(); // Initialize frame timer
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Game Initialization Successful.");
    return true;
}

void Game::run() {
    if (!is_running) {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Game::run() called but game is not initialized/running.");
         return;
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Entering main game loop.");
    while (is_running) {
        // --- Timing ---
        Uint32 current_time = SDL_GetTicks();
        float delta_time = (current_time - last_frame_time) / 1000.0f;
        last_frame_time = current_time;

        // --- Event Handling (Basic Quit) ---
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit_game(); // Use the quit function
            }
             // Later: Pass events to InputHandler or current_state->handle_event(event);
        }

        // --- State Logic ---
        if (current_state) {
            current_state->handle_input(); // State handles its specific input logic
            current_state->update(delta_time); // State updates its logic

            // --- Rendering ---
            display.clear(0x0000); // Clear screen (using display wrapper)
            current_state->render();   // State draws itself
            display.present(); // Show the rendered frame (using display wrapper)

        } else {
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "No current state to run! Exiting loop.");
             is_running = false; // Exit if state becomes null unexpectedly
        }

        // --- Frame Limiter (Optional) ---
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Exited main game loop.");
}

void Game::change_state(std::unique_ptr<GameState> new_state) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Changing game state.");
    current_state = std::move(new_state);
    if (!current_state) {
         SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Changed to a NULL state!");
    }
}

void Game::quit_game() {
     if (is_running) { // Prevent multiple quit messages
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Quit requested.");
        is_running = false;
     }
}

// Getter implementations
AssetManager* Game::getAssetManager() {
     return &assetManager;
}

PCDisplay* Game::get_display() {
    return &display;
}


void Game::close() {
    // Ensure close is only effective once
    // Check if display was ever initialized as a proxy for needing cleanup
    if (display.isInitialized()) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Closing game systems...");
        current_state.reset(); // Destroy the current state object first

        assetManager.shutdown(); // <<< SHUTDOWN AssetManager >>>
        display.close();        // <<< CLOSE Display >>>

        SDL_Quit();             // <<< SHUTDOWN SDL >>>
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Game systems closed.");
    }
}