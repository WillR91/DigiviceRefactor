// File: include/states/TransitionState.h
#pragma once

#include "states/GameState.h"
#include <SDL.h>
#include <string>

// Forward declarations
class Game;
class InputManager;
class PlayerData;
class PCDisplay;

// Enum to define different transition types
enum class TransitionType {
    BOX_IN_TO_MENU
};

class TransitionState : public GameState {
public:
    // Constructor takes the state that will be below this one during/after transition
    TransitionState(Game* game, GameState* belowState, float duration, TransitionType type);
    ~TransitionState() override;

    // Lifecycle methods
    void enter() override {};  // Added
    void exit() override {};   // Added

    // Core state functions
    void handle_input(InputManager& inputManager, PlayerData* playerData) override;
    void update(float delta_time, PlayerData* playerData) override;
    void render(PCDisplay& display) override;
    StateType getType() const override;  // Added

    // Function for the state below to signal exit
    void requestExit();

private:
    // --- Data Members ---
    GameState* belowState_;
    float duration_;
    float timer_;
    TransitionType type_;
    SDL_Texture* borderAtlasTexture_;
    SDL_Rect borderTopSrcRect_;
    SDL_Rect borderBottomSrcRect_;
    SDL_Rect borderLeftSrcRect_;
    SDL_Rect borderRightSrcRect_;
    bool transition_complete_requested_;
    bool transitionComplete_;

    // --- Private Helper Methods ---
    bool loadBorderRectsFromJson(const std::string& jsonPath);

}; // End TransitionState class