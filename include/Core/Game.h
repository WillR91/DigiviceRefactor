#pragma once

#include <string>
#include <memory> // For std::unique_ptr
#include <SDL.h> // <<< CORRECTED SDL Include >>>
#include "platform/pc/pc_display.h" // Assumes pc_display.h is in include/platform/pc/
#include "core/AssetManager.h"      // Assumes AssetManager.h is in include/core/

// Forward declaration
class GameState; // Defined in states/GameState.h

class Game {
public:
    Game();
    ~Game(); // Destructor will handle cleanup

    // Initialize SDL, window, renderer, AssetManager, etc.
    bool init(const std::string& title, int width, int height);

    // Contains the main game loop
    void run();

    // Change the active game state
    void change_state(std::unique_ptr<GameState> new_state);

    // Request the game loop to stop
    void quit_game();

    // Provide access to core systems
    PCDisplay* get_display();
    AssetManager* getAssetManager();

private:
    // Perform cleanup (called by destructor)
    void close();

    PCDisplay display;          // Needs definition from platform/pc/pc_display.h
    AssetManager assetManager;    // Needs definition from core/AssetManager.h
    bool is_running = false;
    std::unique_ptr<GameState> current_state; // Needs definition from states/GameState.h eventually
    Uint32 last_frame_time = 0; // Needs definition from SDL.h
};