// File: include/states/AdventureState.h
#pragma once

#include "states/GameState.h"
#include "graphics/Animation.h"
#include <SDL.h>
#include <vector>
#include <cmath>
#include <cstdint>
#include <map>
#include <cstddef> // <<< ADDED for size_t >>>

class Game;

enum PlayerState { STATE_IDLE, STATE_WALKING };
enum DigimonType { DIGI_AGUMON, DIGI_GABUMON, DIGI_BIYOMON, DIGI_GATOMON, DIGI_GOMAMON, DIGI_PALMON, DIGI_TENTOMON, DIGI_PATAMON, DIGI_COUNT };

class AdventureState : public GameState {
public:
    AdventureState(Game* game);
    ~AdventureState() override;

    void handle_input() override;
    void update(float delta_time) override;
    void render() override;

private:
    std::map<DigimonType, Animation> idleAnimations_;
    std::map<DigimonType, Animation> walkAnimations_;
    SDL_Texture* bgTexture0_ = nullptr;
    SDL_Texture* bgTexture1_ = nullptr;
    SDL_Texture* bgTexture2_ = nullptr;

    DigimonType current_digimon_ = DIGI_AGUMON;
    PlayerState current_state_ = STATE_IDLE;
    Animation* active_anim_ = nullptr;
    // <<< CHANGED type from int to size_t >>>
    size_t current_anim_frame_idx_ = 0;
    float current_frame_elapsed_time_ = 0.0f;
    int queued_steps_ = 0; // Keep int for step count likely

    float bg_scroll_offset_0_ = 0.0f;
    float bg_scroll_offset_1_ = 0.0f;
    float bg_scroll_offset_2_ = 0.0f;

    const int MAX_QUEUED_STEPS = 2;
    const float SCROLL_SPEED_0 = 3.0f * 60.0f;
    const float SCROLL_SPEED_1 = 1.0f * 60.0f;
    const float SCROLL_SPEED_2 = 0.5f * 60.0f;
    const int WINDOW_WIDTH = 466;
    const int WINDOW_HEIGHT = 466;

     void setActiveAnimation();
     void initializeAnimations();
};