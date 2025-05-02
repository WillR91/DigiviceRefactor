// File: include/states/TransitionState.h
#pragma once

#include "states/GameState.h"
#include <SDL.h>
#include <string>
#include <algorithm> // For std::min
#include <map>       // For storing rects loaded from json

class Game; // Forward declare

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

    // TEMP Constants - TODO: Get from game/display later
    const int WINDOW_WIDTH = 466;
    const int WINDOW_HEIGHT = 466;

    // Helper function to load rects from JSON
    bool loadBorderRectsFromJson(const std::string& jsonPath);
};