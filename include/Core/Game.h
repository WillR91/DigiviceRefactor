// File: include/core/Game.h
#pragma once

#include <string>
#include <memory>
#include <vector>
#include <SDL.h>
#include "platform/pc/pc_display.h"
#include "core/AssetManager.h"
#include "states/GameState.h" // Include full definition

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
    // void requestChangeState(std::unique_ptr<GameState> state); // Add later if needed

    // Other Public Methods
    void quit_game();
    PCDisplay* get_display();
    AssetManager* getAssetManager();
    GameState* getCurrentState();

    // <<< --- ADDED HELPER to access stack (temporary/debug) --- >>>
    // Provides access to the state stack, primarily for MenuState to find TransitionState.
    // Note: Exposing the stack directly isn't ideal encapsulation long-term.
    std::vector<std::unique_ptr<GameState>>& DEBUG_getStack();
    // <<< ------------------------------------------------------- >>>


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
    bool is_running = false;
    std::vector<std::unique_ptr<GameState>> states_; // State stack
    Uint32 last_frame_time = 0;

    // --- State Change Request Flags/Data ---
    bool request_pop_ = false;
    std::unique_ptr<GameState> request_push_ = nullptr;
    // bool request_change_ = false;
    // std::unique_ptr<GameState> request_change_to_ = nullptr;
};