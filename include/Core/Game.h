// File: include/core/Game.h
#pragma once

#include <string>
#include <memory>
#include <vector>
#include <SDL.h>
#include "platform/pc/pc_display.h"
#include "core/AssetManager.h"
#include "states/GameState.h" // Include full definition
#include "core/InputManager.h" // <<< ADDED InputManager include >>>

class Game {
public:
    // Constructor and Destructor Declarations
    Game();
    ~Game();

    // Core Functions
    bool init(const std::string& title, int width, int height);
    void run();

    // --- State Management Requests (Called by States) ---
    void requestPushState(std::unique_ptr<GameState> state);
    void requestPopState();

    // Other Public Methods
    void quit_game();
    PCDisplay* get_display();
    AssetManager* getAssetManager();
    GameState* getCurrentState();
    InputManager* getInputManager(); // <<< ADDED Getter Declaration >>>

    // Debug helper (Keep or remove as needed)
    std::vector<std::unique_ptr<GameState>>& DEBUG_getStack();

private:
    // Private Helper Functions
    void close();
    // --- State Management (Internal - Called by run loop) ---
    void push_state(std::unique_ptr<GameState> new_state);
    void pop_state();
    void applyStateChanges(); // Apply queued requests

    // Member Variables
    PCDisplay display;
    AssetManager assetManager;
    InputManager inputManager; // <<< ADDED InputManager Member >>>
    bool is_running = false;
    std::vector<std::unique_ptr<GameState>> states_; // State stack
    Uint32 last_frame_time = 0;

    // --- State Change Request Flags/Data ---
    bool request_pop_ = false;
    std::unique_ptr<GameState> request_push_ = nullptr;
};