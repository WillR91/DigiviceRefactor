// File: include/core/Game.h
#pragma once

#include <string>
#include <memory>
#include <vector>
#include <SDL.h>
#include "platform/pc/pc_display.h"
#include "core/AssetManager.h"
#include "states/GameState.h"       // Includes StateType enum now
#include "states/TransitionState.h" // <<< ADDED to make TransitionEffectType visible
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
    void requestPopUntil(StateType targetType); 
    void requestFadeToState(std::unique_ptr<GameState> targetState, float duration = 0.5f, bool popCurrent = true); // <<< NEW

    // --- Getters for Core Systems/Data ---
    void quit_game();
    PCDisplay* get_display();
    AssetManager* getAssetManager();
    GameState* getCurrentState();
    GameState* getUnderlyingState(const GameState* currentState); // <<< --- ADDED DECLARATION --- >>>
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
    StateType pop_until_target_type_ = StateType::None;

    // --- For Fade Transitions ---
    enum class FadeSequenceStep {
        NONE,
        FADING_OUT,
        READY_FOR_STATE_SWAP,
        FADING_IN
    };
    FadeSequenceStep fade_step_ = FadeSequenceStep::NONE;
    std::unique_ptr<GameState> pending_state_for_fade_ = nullptr;
    float fade_duration_ = 0.5f;
    bool pop_current_after_fade_out_ = true; 
    TransitionEffectType active_fade_type_ = TransitionEffectType::BORDER_WIPE; // <<< NEW (default unimportant)
};