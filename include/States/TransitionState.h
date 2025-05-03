// File: include/states/TransitionState.h
#pragma once

#include "states/GameState.h"
#include <SDL.h>     // Needed for SDL_Rect, SDL_Texture*
#include <string>    // Needed for std::string used in helper function prototype

// Forward declarations
class Game;
// No need to forward declare nlohmann::json here

// Enum to define different transition types
enum class TransitionType {
    BOX_IN_TO_MENU // Only type implemented currently
    // BOX_OUT_FROM_MENU,
};

class TransitionState : public GameState {
public:
    TransitionState(Game* game, GameState* belowState, float duration, TransitionType type);
    ~TransitionState() override;

    void handle_input() override;
    void update(float delta_time) override;
    void render() override;

private:
    GameState* belowState_; // State underneath this transition
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

    // --- State Variable for Completion ---
    bool transition_complete_requested_ = false; // Tracks if pop/push has been requested for this instance

    // Helper function prototype
    bool loadBorderRectsFromJson(const std::string& jsonPath);

}; // End TransitionState class