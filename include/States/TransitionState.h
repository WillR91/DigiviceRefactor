// File: include/states/TransitionState.h
#pragma once

#include "states/GameState.h"
#include <SDL.h>
#include <string>

// Forward declarations
class Game;
class InputManager; // <<< ADDED
class PlayerData;   // <<< ADDED
class PCDisplay;    // <<< ADDED

// Enum to define different transition types
enum class TransitionType {
    BOX_IN_TO_MENU
};

class TransitionState : public GameState {
public:
    // Constructor takes the state that will be below this one during/after transition
    TransitionState(Game* game, GameState* belowState, float duration, TransitionType type); // Declaration OK
    ~TransitionState() override; // Declaration OK

    // Core state functions override (with NEW signatures)
    void handle_input(InputManager& inputManager, PlayerData* playerData) override; // <<< MODIFIED
    void update(float delta_time, PlayerData* playerData) override;                // <<< MODIFIED
    void render(PCDisplay& display) override;                                     // <<< MODIFIED

    // Function for the state below to signal exit
    void requestExit(); // Signature unchanged

private:
    // --- Data Members ---
    // (Remain unchanged)
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
    // (Signature remains unchanged)
    bool loadBorderRectsFromJson(const std::string& jsonPath);

}; // End TransitionState class