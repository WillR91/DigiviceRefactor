#include "Game.h"
#include "GameState.h"
#include "AdventureState.h" // Include the initial state
#include <SDL_log.h>
#include <stdexcept> // For runtime_error


Game::Game() : is_running(false), last_frame_time(0) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Game constructor called.");
}

// Destructor automatically calls close() through RAII principles implicitly
Game::~Game() {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Game destructor called.");
    close();
}

bool Game::init(const std::string& title, int width, int height) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Initializing Game systems...");

    // Initialize SDL subsystems (video required for window)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return false;
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL Initialized.");

    // Initialize the display wrapper (creates window & renderer)
    if (!display.init(title.c_str(), width, height)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay initialization failed!");
        SDL_Quit(); // Clean up SDL if display fails
        return false;
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay Initialized.");

    // Set initial state (Adventure State)
    try {
       current_state = std::make_unique<AdventureState>(this);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Initial AdventureState created.");
    } catch (const std::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create initial state: %s", e.what());
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
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Entering main game loop.");
    while (is_running) {
        // --- Timing ---
        Uint32 current_time = SDL_GetTicks();
        // Delta time in seconds (float for smoother calculations)
        float delta_time = (current_time - last_frame_time) / 1000.0f;
        last_frame_time = current_time;

        // --- Basic Event Handling (Only Quit for now) ---
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                is_running = false;
            }
             // Pass other events to the state? (More advanced)
             // if (current_state) current_state->handle_event(event);
        }


        // --- State Logic ---
        if (current_state) {
            // Handle state-specific input
            current_state->handle_input();

            // Update state logic
            current_state->update(delta_time);

            // Render state graphics
            // Clear is usually done before state rendering
            display.clear(0x0000); // Use your clear color

            current_state->render();

            // Present is usually done after all rendering
            display.present();
        } else {
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "No current state to run!");
             // Optionally handle this case, maybe switch to a default state or quit
             is_running = false;
        }


        // --- Frame Limiter (Approx 60 FPS) ---
        // Calculate time taken and delay if needed
        Uint32 frame_duration = SDL_GetTicks() - current_time;
        const Uint32 MIN_FRAME_DURATION_MS = 16; // ~60 FPS
        if (frame_duration < MIN_FRAME_DURATION_MS) {
            SDL_Delay(MIN_FRAME_DURATION_MS - frame_duration);
        }
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Exited main game loop.");
}

void Game::change_state(std::unique_ptr<GameState> new_state) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Changing game state.");
    // unique_ptr automatically handles deletion of the old state
    current_state = std::move(new_state);
    if (!current_state) {
         SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Changed to a NULL state!");
    }
}

void Game::quit_game() {
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Quit requested.");
     is_running = false;
}


PCDisplay* Game::get_display() {
    return &display;
}


void Game::close() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Closing game systems...");
    // Clear the current state ptr *before* closing display/SDL
    current_state.reset(); // Explicitly destroy the state object

    display.close(); // Close your display wrapper first
    SDL_Quit(); // Then shutdown SDL subsystems
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Game systems closed.");
}