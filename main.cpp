#include "Game.h"
#include <SDL_log.h> // For logging

// --- Window Dimensions (Define globally or pass to Game::init) ---
const int WINDOW_WIDTH = 466;
const int WINDOW_HEIGHT = 466;

int main(int argc, char* argv[]) {
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG); // Enable SDL logging
    SDL_Log("--- Creating Game Instance ---");

    Game digivice_game;

    SDL_Log("--- Initializing Game ---");
    if (digivice_game.init("Digivice Sim - Refactored", WINDOW_WIDTH, WINDOW_HEIGHT)) {
        SDL_Log("--- Starting Game Loop ---");
        digivice_game.run();
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Game initialization failed!");
    }

    SDL_Log("--- Cleaning Up Game ---");
    // Cleanup is handled by the Game object's destructor which calls close()
    SDL_Log("--- Exiting ---");

    return 0;
}