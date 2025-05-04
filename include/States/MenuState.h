#pragma once

#include "states/GameState.h"
#include <vector>           // Needed for std::vector
#include <string>           // Needed for std::string
#include <map>              // Needed for fontCharMap_
#include <SDL.h>            // For SDL_Texture*, SDL_Rect, SDL_Renderer*, SDL_Point

// Forward declarations
class Game;
struct SDL_Texture;
struct SDL_Rect;
struct SDL_Renderer;
// SDL_Point is usually defined in SDL_rect.h which is included by SDL.h

class MenuState : public GameState {
public:
    // Constructor takes the owning Game and the list of menu options
    MenuState(Game* game, const std::vector<std::string>& options);
    ~MenuState() override;

    void handle_input() override;
    void update(float delta_time) override;
    void render() override;

private:
    // --- Helper Functions ---
    // Helper for drawing text
    // <<< UPDATED signature to include scale >>>
    void drawText(SDL_Renderer* renderer, const std::string& text, int x, int y, float scale = 1.0f, int kerning = 1);
    // Helper for loading font data
    bool loadFontDataFromJson(const std::string& jsonPath);
    // <<< ADDED declaration for text dimension calculation >>>
    SDL_Point getTextDimensions(const std::string& text, int kerning = 1);

    // --- Data Members ---
    // Menu data
    std::vector<std::string> menuOptions_;
    size_t currentSelection_ = 0; // Index of the currently selected item

    // Assets (Non-owning pointers)
    SDL_Texture* backgroundTexture_ = nullptr;
    SDL_Texture* fontTexture_ = nullptr;
    SDL_Texture* cursorTexture_ = nullptr; // Kept for potential future use? Set to nullptr in constructor.
    std::map<char, SDL_Rect> fontCharMap_;

    // --- Constants ---
    // Menu drawing parameters (customize later)
    // These might become less relevant if centering everything
    const int MENU_START_X = 50; // Example X position for menu items
    const int MENU_START_Y = 100; // Example Y position for the first menu item
    const int MENU_ITEM_HEIGHT = 30; // Vertical spacing between items

}; // End of MenuState class definition