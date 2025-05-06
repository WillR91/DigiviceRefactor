// File: main.cpp

#include "core/Game.h" // Path relative to include dir
#include <SDL_log.h>   // SDL logging

// --- Window Dimensions ---
const int WINDOW_WIDTH = 466;
const int WINDOW_HEIGHT = 466;

int main(int argc, char* argv[]) {
    // <<< --- Set Log Priority to Verbose --- >>>
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
    SDL_Log("--- Creating Game Instance ---");

    Game digivice_game; // Create Game object

    SDL_Log("--- Initializing Game ---");
    if (digivice_game.init("Digivice Sim - Refactored", WINDOW_WIDTH, WINDOW_HEIGHT)) {
        SDL_Log("--- Starting Game Loop ---");
        digivice_game.run(); // Run the main loop
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Game initialization failed!");
    }

    // Game destructor handles cleanup via its 'close' method implicitly when it goes out of scope
    SDL_Log("--- Cleaning Up Game (via Game destructor) ---");
    SDL_Log("--- Exiting ---");

    return 0;
}