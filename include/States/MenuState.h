#pragma once

#include "states/GameState.h"
#include <vector>
#include <string>
#include <SDL.h> // For rendering types

class Game; // Forward declare

class MenuState : public GameState {
public:
    // Constructor takes the owning Game and the list of menu options
    MenuState(Game* game, const std::vector<std::string>& options);
    ~MenuState() override;

    void handle_input() override;
    void update(float delta_time) override;
    void render() override;

private:
    // Menu drawing parameters (customize later)
    const int MENU_START_X = 50;
    const int MENU_START_Y = 100;
    const int MENU_ITEM_HEIGHT = 30; // Spacing between items
    SDL_Texture* backgroundTexture_ = nullptr; // Already declared correctly

    // Menu data
    std::vector<std::string> menuOptions_;
    size_t currentSelection_ = 0; // Index of the currently selected item

    // Assets (need font later)
    SDL_Texture* fontTexture_ = nullptr; // Placeholder for bitmap font
    SDL_Texture* cursorTexture_ = nullptr; // Placeholder for selection cursor
    // Need to load these via AssetManager...

    // Helper for drawing text (to be implemented later)
    void drawText(const std::string& text, int x, int y);
};