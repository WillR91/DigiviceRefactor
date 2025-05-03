// File: include/states/TransitionState.h
#pragma once

#include "states/GameState.h"
#include <SDL.h>
#include <string>
// No longer need algorithm or map/vector includes here if they aren't used publicly

// Forward declarations
class Game;

// Enum to define different transition types
enum class TransitionType {
    BOX_IN_TO_MENU // Only type implemented currently
    // Add BOX_OUT_FROM_MENU later if needed
};

class TransitionState : public GameState {
public:
    // Constructor takes the state that will be below this one during/after transition
    TransitionState(Game* game, GameState* belowState, float duration, TransitionType type);
    ~TransitionState() override;

    void handle_input() override;
    void update(float delta_time) override;
    void render() override;

    // <<< ADDED: Function for the state below (MenuState) to signal exit >>>
    // This allows MenuState to tell TransitionState when it's done.
    void requestExit();

private:
    GameState* belowState_; // State underneath this transition (e.g., AdventureState or MenuState)
    float duration_;        // How long the transition takes
    float timer_;           // Current time elapsed in the transition
    TransitionType type_;   // Type of transition effect

    // --- Single Atlas for Border Segments ---
    SDL_Texture* borderAtlasTexture_ = nullptr; // Pointer to the loaded atlas texture

    // --- Source Rectangles for each segment ON the atlas ---
    SDL_Rect borderTopSrcRect_ = {0,0,0,0};
    SDL_Rect borderBottomSrcRect_ = {0,0,0,0};
    SDL_Rect borderLeftSrcRect_ = {0,0,0,0};
    SDL_Rect borderRightSrcRect_ = {0,0,0,0};
    // --- End Atlas Data ---

    // --- State Variables for Managing Flow ---
    // Used by update/requestExit to make pop request *once* when exiting
    bool transition_complete_requested_ = false;
    // Tracks if the visual IN-transition animation has finished
    bool transitionComplete_ = false; // <<< ADDED: Tracks if wipe animation finished

    // Helper function prototype
    bool loadBorderRectsFromJson(const std::string& jsonPath);

}; // End TransitionState class