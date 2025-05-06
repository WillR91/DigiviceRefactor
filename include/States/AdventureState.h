// File: include/states/AdventureState.h
#pragma once

#include "states/GameState.h"
#include "graphics/Animator.h"
#include "../entities/Digimon.h"
// Reduce includes if possible, move SDL includes to cpp if only needed there
#include <SDL_render.h> // Needed for SDL_Texture* member
#include <SDL_rect.h>   // Needed for SDL_Rect usage? (If helpers use it)
#include <vector>
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <string>   // Needed for helper return types

// Forward declarations
class Game;
class InputManager;
class PlayerData;
class PCDisplay;
// SDL_Texture is included via SDL_render.h

// Enums specific to AdventureState logic
enum PlayerState { STATE_IDLE, STATE_WALKING };

class AdventureState : public GameState {
public:
    // Constructor & Destructor
    AdventureState(Game* game);
    ~AdventureState() override;

    void enter() override;
    void handle_input(InputManager& inputManager, PlayerData* playerData) override;
    void update(float delta_time, PlayerData* playerData) override;
    void render(PCDisplay& display) override;
    StateType getType() const override; // <<< ADDED getType override >>>

private:
    // Animator
    Animator partnerAnimator_;

    // Other Data Members
    SDL_Texture* bgTexture0_ = nullptr;
    SDL_Texture* bgTexture1_ = nullptr;
    SDL_Texture* bgTexture2_ = nullptr;
    DigimonType current_digimon_;
    PlayerState current_state_ = STATE_IDLE;
    int queued_steps_ = 0;
    float bg_scroll_offset_0_ = 0.0f;
    float bg_scroll_offset_1_ = 0.0f;
    float bg_scroll_offset_2_ = 0.0f;
    bool firstWalkUpdate_ = true;

    // Constants
    const int MAX_QUEUED_STEPS = 2;
    const float SCROLL_SPEED_0 = 3.0f * 60.0f;
    const float SCROLL_SPEED_1 = 1.0f * 60.0f;
    const float SCROLL_SPEED_2 = 0.5f * 60.0f;
    const int WINDOW_WIDTH = 466;  // Consider getting from display
    const int WINDOW_HEIGHT = 466; // Consider getting from display

    // Private Helpers
    std::string getAnimationIdForCurrentState() const;
    std::string getDigimonTextureId(DigimonType type) const;

}; // End of AdventureState class definition