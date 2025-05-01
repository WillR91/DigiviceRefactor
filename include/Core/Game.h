// File: include/core/Game.h
#pragma once

#include <string>
#include <memory> // For std::unique_ptr
#include <vector> // For state stack vector
#include <SDL.h>  // For SDL types like Uint32
#include "platform/pc/pc_display.h" // Your display abstraction
#include "core/AssetManager.h"      // Your asset manager
#include "states/GameState.h"       // <<< ADDED: Include full definition for unique_ptr >>>

// class GameState; // <<< REMOVED: Forward declaration no longer needed here >>>

class Game {
public:
    // Constructor and Destructor Declarations
    Game();
    ~Game();

    // Core Functions
    bool init(const std::string& title, int width, int height);
    void run();

    // --- State Management ---
    void push_state(std::unique_ptr<GameState> new_state); // Add state to top
    void pop_state();                                      // Remove top state
    GameState* getCurrentState();                         // Get pointer to top state (doesn't change ownership)
    // void change_state(std::unique_ptr<GameState> new_state); // Can add later if needed (pop + push)

    // Other Public Methods
    void quit_game(); // Signal the game loop to stop
    PCDisplay* get_display(); // Access display system (non-owning pointer)
    AssetManager* getAssetManager(); // Access asset manager (non-owning pointer)

private:
    // Private Helper Functions
    void close(); // Cleanup resources

    // Member Variables
    PCDisplay display;          // Owns the display object
    AssetManager assetManager;    // Owns the asset manager object
    bool is_running = false;    // Flag for main loop
    std::vector<std::unique_ptr<GameState>> states_; // State stack (owns the states)
    Uint32 last_frame_time = 0; // For delta time calculation
};