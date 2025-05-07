// File: include/core/Game.h
#pragma once

#include <string>
#include <memory>
#include <vector>
#include <SDL.h>
#include "platform/pc/pc_display.h"
#include "core/AssetManager.h"
#include "states/GameState.h"       // Includes StateType enum now
#include "core/InputManager.h"
#include "core/PlayerData.h"

// Forward declarations
class TextRenderer;
class AnimationManager;

class Game {
public:
    // Constructor and Destructor Declarations
    Game();
    ~Game();

    // Core Functions
    bool init(const std::string& title, int width, int height);
    void run();

    // --- State Management Requests ---
    void requestPushState(std::unique_ptr<GameState> state);
    void requestPopState();                      // Pops one state
    void requestPopUntil(StateType targetType); // <<< ADDED

    // --- Getters for Core Systems/Data ---
    void quit_game();
    PCDisplay* get_display();
    AssetManager* getAssetManager();
    GameState* getCurrentState();
    InputManager* getInputManager();
    PlayerData* getPlayerData();
    TextRenderer* getTextRenderer();
    AnimationManager* getAnimationManager();

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
    std::unique_ptr<TextRenderer> textRenderer_;
    std::unique_ptr<AnimationManager> animationManager_;
    bool is_running = false;
    std::vector<std::unique_ptr<GameState>> states_;
    Uint32 last_frame_time = 0;

    // State Change Request Variables
    bool request_pop_ = false;
    std::unique_ptr<GameState> request_push_ = nullptr;
    StateType pop_until_target_type_ = StateType::None; // <<< ADDED
};