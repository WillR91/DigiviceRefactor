// Test for ConfigManager
// To compile: Create a new main_config_test.cpp and run it

#include "utils/ConfigManager.h"
#include <SDL.h>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    // Initialize SDL for logging
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);
    SDL_Log("Starting ConfigManager test");
    
    // Initialize ConfigManager
    if (!ConfigManager::initialize()) {
        SDL_Log("Failed to initialize ConfigManager");
        SDL_Quit();
        return 1;
    }
    
    // Test reading values
    int width = ConfigManager::getValue<int>("display.width", 800);
    int height = ConfigManager::getValue<int>("display.height", 600);
    std::string title = ConfigManager::getValue<std::string>("display.title", "Default Title");
    bool fullscreen = ConfigManager::getValue<bool>("display.fullscreen", false);
    
    // Log values
    SDL_Log("Display settings from config:");
    SDL_Log("  - Width: %d", width);
    SDL_Log("  - Height: %d", height);
    SDL_Log("  - Title: %s", title.c_str());
    SDL_Log("  - Fullscreen: %s", fullscreen ? "Yes" : "No");
    
    // Test default values for non-existent keys
    std::string nonExistent = ConfigManager::getValue<std::string>("this.key.does.not.exist", "Default Value");
    SDL_Log("Non-existent key: %s", nonExistent.c_str());
    
    // Test modifying values
    SDL_Log("\nModifying configuration values:");
    bool success = ConfigManager::setValue("test.newValue", "This is a test string");
    SDL_Log("  - Added 'test.newValue': %s", success ? "Success" : "Failed");
    
    // Read back the modified value
    std::string testValue = ConfigManager::getValue<std::string>("test.newValue", "");
    SDL_Log("  - Read back 'test.newValue': %s", testValue.c_str());
    
    // Test saving changes
    success = ConfigManager::saveChanges();
    SDL_Log("  - Saved changes: %s", success ? "Success" : "Failed");
    
    // Test reloading
    success = ConfigManager::reload();
    SDL_Log("  - Reloaded config: %s", success ? "Success" : "Failed");
    
    // Verify value is still there after reload
    testValue = ConfigManager::getValue<std::string>("test.newValue", "");
    SDL_Log("  - After reload, 'test.newValue': %s", testValue.c_str());
    
    // Clean up
    ConfigManager::shutdown();
    SDL_Log("ConfigManager test complete");
    SDL_Quit();
    
    return 0;
}
