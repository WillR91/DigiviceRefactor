// include/states/ProgressState.h
#ifndef PROGRESSSTATE_H
#define PROGRESSSTATE_H

#include "states/GameState.h"
#include "graphics/Animator.h"    // For the Digimon animator
#include "../entities/Digimon.h"  // For DigimonType (if needed for local storage, else PlayerData provides)
#include <SDL_render.h>           // For SDL_Texture*
#include <string>

// Forward declarations
class Game;
class InputManager;
class PlayerData;
class PCDisplay;
class TextRenderer;

class ProgressState : public GameState {
public:
    ProgressState(Game* game);
    ~ProgressState() override;

    void enter() override;
    void exit() override;
    void handle_input(InputManager& inputManager, PlayerData* playerData) override;
    void update(float delta_time, PlayerData* playerData) override;
    void render(PCDisplay& display) override;
    StateType getType() const override;

private:
    Animator partnerAnimator_;         // To animate the current partner
    SDL_Texture* backgroundTexture_ = nullptr; // Optional background

    // We don't need to store steps/goal here, we'll read from PlayerData
    // and GameConstants (or a future MapManager) during render.
};

#endif // PROGRESSSTATE_H