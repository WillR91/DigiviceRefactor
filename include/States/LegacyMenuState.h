// File: include/states/MenuState.h
#pragma once

#include "states/GameState.h"
#include "ui/TextRenderer.h"
#include <vector>
#include <string>
#include <SDL.h>

// Forward declarations
class Game;
class InputManager;
class PlayerData;
class PCDisplay;

class MenuState : public GameState {
public:
    MenuState(Game* game, const std::vector<std::string>& options);
    ~MenuState() override;

    // Lifecycle methods
    void enter() override {};
    void exit() override {};

    // Core state functions
    void handle_input(InputManager& inputManager, PlayerData* playerData) override;
    void update(float delta_time, PlayerData* playerData) override;
    void render(PCDisplay& display) override;
    StateType getType() const override;

private:
    // --- Data Members ---
    std::vector<std::string> menuOptions_;
    size_t currentSelection_;
    SDL_Texture* backgroundTexture_;
    SDL_Texture* cursorTexture_;

    // --- Constants ---
    const int MENU_START_X = 50;
    const int MENU_START_Y = 100;
    const int MENU_ITEM_HEIGHT = 30;
};