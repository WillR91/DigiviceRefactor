// File: include/core/Game.h
#pragma once

#include <string>
#include <memory>
#include <vector>
#include <SDL.h>
// Include necessary headers directly or use more forward declarations
#include "platform/pc/pc_display.h"
#include "core/AssetManager.h"
#include "states/GameState.h"
#include "core/InputManager.h"
#include "core/PlayerData.h"

// Forward declarations
class TextRenderer; // <<<--- ADDED Forward declaration

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

    // --- Getters for Core Systems/Data ---
    void quit_game();
    PCDisplay* get_display();
    AssetManager* getAssetManager();
    GameState* getCurrentState();
    InputManager* getInputManager();
    PlayerData* getPlayerData();
    TextRenderer* getTextRenderer(); // <<<--- ADDED Getter for TextRenderer

    // Debug helper
    std::vector<std::unique_ptr<GameState>>& DEBUG_getStack();

private:
    // Private Helper Functions
    void close();
    void push_state(std::unique_ptr<GameState> new_state);
    void pop_state();
    void applyStateChanges();

    // Member Variables
    PCDisplay display;
    AssetManager assetManager;
    InputManager inputManager;
    PlayerData playerData_;
    std::unique_ptr<TextRenderer> textRenderer_; // <<<--- ADDED unique_ptr member
    bool is_running = false;
    std::vector<std::unique_ptr<GameState>> states_;
    Uint32 last_frame_time = 0;
    bool request_pop_ = false;
    std::unique_ptr<GameState> request_push_ = nullptr;
};