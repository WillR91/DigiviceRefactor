// File: include/states/MenuState.h
#pragma once

#include "states/GameState.h"
#include "ui/TextRenderer.h" // <<< ADDED Include
#include <vector>            // Needed for std::vector
#include <string>            // Needed for std::string
// #include <map>            // No longer needed for fontCharMap_
#include <SDL.h>             // For SDL_Texture*, SDL_Rect, SDL_Renderer*, SDL_Point (SDL_Renderer* might be removable if drawText is fully replaced)

// Forward declarations
class Game;
class InputManager; // <<< ADDED (already present)
class PlayerData;   // <<< ADDED (already present)
class PCDisplay;    // <<< ADDED (already present)
// SDL types are included via SDL.h

class MenuState : public GameState {
public:
    // Constructor takes the owning Game and the list of menu options
    MenuState(Game* game, const std::vector<std::string>& options); // Declaration OK
    ~MenuState() override; // Declaration OK

    // Core state functions override (with NEW signatures)
    void handle_input(InputManager& inputManager, PlayerData* playerData) override; // <<< MODIFIED (already present)
    void update(float delta_time, PlayerData* playerData) override;                // <<< MODIFIED (already present)
    void render(PCDisplay& display) override;                                     // <<< MODIFIED (already present)

private:
    // --- Helper Functions ---
    // (Removed private text rendering helper declarations)

    // --- Data Members ---
    std::vector<std::string> menuOptions_;
    size_t currentSelection_;
    SDL_Texture* backgroundTexture_;
    // SDL_Texture* fontTexture_; // <<< REMOVED
    SDL_Texture* cursorTexture_;
    // std::map<char, SDL_Rect> fontCharMap_; // <<< REMOVED

    // --- Constants ---
    // (Remain unchanged)
    const int MENU_START_X = 50;
    const int MENU_START_Y = 100;
    const int MENU_ITEM_HEIGHT = 30;

}; // End of MenuState class definition