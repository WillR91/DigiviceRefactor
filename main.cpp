// File: main.cpp

#include "core/Game.h" // <<< CORRECTED path relative to include dir >>>
#include <SDL_log.h>   // <<< CORRECTED SDL Include >>>

// --- Window Dimensions ---
const int WINDOW_WIDTH = 466;
const int WINDOW_HEIGHT = 466;

int main(int argc, char* argv[]) {
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
    SDL_Log("--- Creating Game Instance ---");

    Game digivice_game; // Needs full definition from core/Game.h

    SDL_Log("--- Initializing Game ---");
    if (digivice_game.init("Digivice Sim - Refactored", WINDOW_WIDTH, WINDOW_HEIGHT)) {
        SDL_Log("--- Starting Game Loop ---");
        digivice_game.run();
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Game initialization failed!");
    }

    SDL_Log("--- Cleaning Up Game ---");
    SDL_Log("--- Exiting ---");

    return 0;
}