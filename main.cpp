// File: main.cpp

#include "core/Game.h" // Path relative to include dir
#include "utils/ConfigManager.h" // Our new configuration system
#include <SDL_log.h>   // SDL logging

int main(int argc, char* argv[]) {
    // Set logging level to show INFO and above
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);
    SDL_Log("--- Creating Game Instance ---");

    // Initialize ConfigManager first
    if (!ConfigManager::initialize()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize ConfigManager!");
        return 1;
    }
    
    // Get configuration values
    int windowWidth = ConfigManager::getValue<int>("display.width", 466);
    int windowHeight = ConfigManager::getValue<int>("display.height", 466);
    std::string windowTitle = ConfigManager::getValue<std::string>("display.title", "Digivice Sim - Refactored");
    bool fullscreen = ConfigManager::getValue<bool>("display.fullscreen", false);
    
    SDL_Log("Loaded configuration: Window %dx%d, Title: %s, Fullscreen: %s", 
        windowWidth, windowHeight, windowTitle.c_str(), fullscreen ? "Yes" : "No");

    Game digivice_game; // Create Game object

    SDL_Log("--- Initializing Game ---");
    if (digivice_game.init(windowTitle, windowWidth, windowHeight)) {
        SDL_Log("--- Starting Game Loop ---");
        digivice_game.run(); // Run the main loop
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Game initialization failed!");
    }    // Game destructor handles cleanup via its 'close' method implicitly when it goes out of scope
    SDL_Log("--- Cleaning Up Game (via Game destructor) ---");
    
    // Shutdown ConfigManager
    ConfigManager::shutdown();
    
    SDL_Log("--- Exiting ---");

    return 0;
}