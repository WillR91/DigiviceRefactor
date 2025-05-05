// File: include/states/MenuState.h
#pragma once

#include "states/GameState.h"
#include <vector>           // Needed for std::vector
#include <string>           // Needed for std::string
#include <map>              // Needed for fontCharMap_
#include <SDL.h>            // For SDL_Texture*, SDL_Rect, SDL_Renderer*, SDL_Point

// Forward declarations
class Game;
class InputManager; // <<< ADDED
class PlayerData;   // <<< ADDED
class PCDisplay;    // <<< ADDED
// SDL types are included via SDL.h

class MenuState : public GameState {
public:
    // Constructor takes the owning Game and the list of menu options
    MenuState(Game* game, const std::vector<std::string>& options); // Declaration OK
    ~MenuState() override; // Declaration OK

    // Core state functions override (with NEW signatures)
    void handle_input(InputManager& inputManager, PlayerData* playerData) override; // <<< MODIFIED
    void update(float delta_time, PlayerData* playerData) override;                // <<< MODIFIED
    void render(PCDisplay& display) override;                                     // <<< MODIFIED

private:
    // --- Helper Functions ---
    // (Signatures remain unchanged, implementation might need display access from game_ptr->get_display())
    void drawText(SDL_Renderer* renderer, const std::string& text, int x, int y, float scale = 1.0f, int kerning = 1);
    bool loadFontDataFromJson(const std::string& jsonPath);
    SDL_Point getTextDimensions(const std::string& text, int kerning = 1);

    // --- Data Members ---
    // (Remain unchanged)
    std::vector<std::string> menuOptions_;
    size_t currentSelection_;
    SDL_Texture* backgroundTexture_;
    SDL_Texture* fontTexture_;
    SDL_Texture* cursorTexture_;
    std::map<char, SDL_Rect> fontCharMap_;

    // --- Constants ---
    // (Remain unchanged)
    const int MENU_START_X = 50;
    const int MENU_START_Y = 100;
    const int MENU_ITEM_HEIGHT = 30;

}; // End of MenuState class definition