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
#include "States/MapSystemState.h" // Added for MapSystemState
#include "core/InputManager.h"
#include "core/PlayerData.h"
#include "core/GraphicsTypes.h" // For Color struct

// Forward declarations
class TextRenderer;
class AnimationManager;

// Define screen size constants
const int SMALL_SCREEN_WIDTH = 172;
const int SMALL_SCREEN_HEIGHT = 172;

// Add this near the top of the file, after includes but before any class definitions
enum class FadeSequenceStep {
    NONE,
    FADING_OUT,               // A fade-out TransitionState is active
    READY_FOR_STATE_SWAP,     // For fading TO a new state (e.g., Adventure -> Menu)
    SETUP_TARGET_STATE,       // For ensuring target state (e.g., Adventure from PartnerSelect) is active before fade-in
    FADING_IN                 // A fade-in TransitionState is active
};

class Game {
public:
    // Constructor and Destructor Declarations
    Game();
    ~Game();

    // Core Functions
    bool init(const std::string& title, int width, int height);
    void run();

    // Main update method
    void update(float delta_time);

    // --- State Management Requests ---
    void requestPushState(std::unique_ptr<GameState> state);
    void requestPopState();                      // Pops one state
    void requestPopUntil(StateType targetType); 
    void requestFadeToState(std::unique_ptr<GameState> targetState, float duration = 0.5f, bool popCurrent = true); // <<< NEW

    // Request a fade to black without immediately changing state
    void requestFadeToBlack(float duration = 0.3f);

    // Get current fade step to check progress
    FadeSequenceStep getFadeStep() const {
        return fade_step_;
    }

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

    // Add these new methods:
    void setTargetStateAfterFade(StateType target) { 
        targetStateAfterFade_ = target; 
    }
    
    StateType getTargetStateAfterFade() const {
        return targetStateAfterFade_;
    }

private:
    // Private Helper Functions
    void close();
    void push_state(std::unique_ptr<GameState> new_state);
    void pop_state();
    void applyStateChanges();
    void processPopUntil(); // Add this method declaration inside the Game class:

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

    // Screen toggle state
    bool is_small_screen_ = false;
    int original_width_ = 0;
    int original_height_ = 0;
    SDL_Texture* ui_mask_texture_ = nullptr; // For the round UI mask

    // State Change Request Variables
    bool request_pop_ = false;
    std::unique_ptr<GameState> request_push_ = nullptr;
    StateType pop_until_target_type_ = StateType::None;

    // --- For Fade Transitions ---
    FadeSequenceStep fade_step_ = FadeSequenceStep::NONE;
    std::unique_ptr<GameState> pending_state_for_fade_ = nullptr;
    float fade_duration_ = 0.5f;
    bool pop_current_after_fade_out_ = true; 
    TransitionEffectType active_fade_type_ = TransitionEffectType::BORDER_WIPE; // <<< NEW (default unimportant)

    // Add this new member variable inside the Game class:
    StateType targetStateAfterFade_ = StateType::None;
};