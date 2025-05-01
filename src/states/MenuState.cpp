// File: src/states/MenuState.cpp

#include "states/MenuState.h"
#include "core/Game.h"
#include "core/AssetManager.h"
#include "platform/pc/pc_display.h"
#include <SDL_log.h>
#include <SDL.h>
#include <stdexcept>

// TEMP Constants - TODO: Get from display object or GameConstants later
const int WINDOW_WIDTH = 466;
const int WINDOW_HEIGHT = 466;


MenuState::MenuState(Game* game, const std::vector<std::string>& options) :
    menuOptions_(options),
    currentSelection_(0),
    backgroundTexture_(nullptr),
    fontTexture_(nullptr),
    cursorTexture_(nullptr)
{
    this->game_ptr = game;
    if (!game_ptr || !game_ptr->getAssetManager()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MenuState Error: Game or AssetManager pointer is null!");
        throw std::runtime_error("MenuState requires Game with AssetManager");
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState Created with %zu options.", options.size());

    AssetManager* assets = game_ptr->getAssetManager();
    backgroundTexture_ = assets->getTexture("menu_bg_blue");
    if (!backgroundTexture_) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "MenuState: Background texture 'menu_bg_blue' not found!");
    }
    // TODO: Load font/cursor textures
}

MenuState::~MenuState() {
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState Destroyed.");
}

void MenuState::handle_input() {
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    static bool esc_pressed_last_frame = false;
    static bool up_pressed_last_frame = false;
    static bool down_pressed_last_frame = false;

    // Exit Menu (Escape or Backspace)
    if (keys[SDL_SCANCODE_ESCAPE] || keys[SDL_SCANCODE_BACKSPACE]) {
        if (!esc_pressed_last_frame) {
            SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Exit key pressed in MenuState, requesting pop.");
            if(game_ptr) { game_ptr->requestPopState(); return; } // Request pop
        }
        esc_pressed_last_frame = true;
    } else {
        esc_pressed_last_frame = false;
    }

    // Navigate Up
    if (keys[SDL_SCANCODE_UP]) {
        if (!up_pressed_last_frame && !menuOptions_.empty()) {
             currentSelection_ = (currentSelection_ == 0) ? menuOptions_.size() - 1 : currentSelection_ - 1;
             SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "Menu: Selected option %zu - '%s'", currentSelection_, menuOptions_[currentSelection_].c_str());
        }
        up_pressed_last_frame = true;
    } else {
        up_pressed_last_frame = false;
    }

    // Navigate Down
    if (keys[SDL_SCANCODE_DOWN]) {
        if (!down_pressed_last_frame && !menuOptions_.empty()) {
            currentSelection_ = (currentSelection_ + 1) % menuOptions_.size();
            SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "Menu: Selected option %zu - '%s'", currentSelection_, menuOptions_[currentSelection_].c_str());
        }
        down_pressed_last_frame = true;
    } else {
        down_pressed_last_frame = false;
    }

    // TODO: Handle Selection with Enter/Space Keys
}

void MenuState::update(float delta_time) {
    // Nothing to update yet
}

// <<< --- RESTORED RENDER FUNCTION --- >>>
// Draws the menu background and logs placeholder text
void MenuState::render() {
    // Get necessary pointers (with checks)
    if (!game_ptr) return;
    PCDisplay* display = game_ptr->get_display();
    if (!display) return;
    // Renderer might be needed for fallback color
    SDL_Renderer* renderer = display->getRenderer();
    // if (!renderer) return; // Only needed for fallback

    // --- Draw the Menu Background ---
    if (backgroundTexture_) {
        int texW, texH;
        // Query the texture's dimensions to draw it correctly
        SDL_QueryTexture(backgroundTexture_, NULL, NULL, &texW, &texH);

        // Calculate destination rectangle (e.g., centered)
        // Using the temporary WINDOW_ constants defined at top of file
        int drawX = (WINDOW_WIDTH / 2) - (texW / 2);
        int drawY = (WINDOW_HEIGHT / 2) - (texH / 2);
        SDL_Rect dstRect = { drawX, drawY, texW, texH };

        // Use display wrapper to draw the texture
        // NULL for srcRect draws the entire texture
        display->drawTexture(backgroundTexture_, NULL, &dstRect);
    } else {
        // Fallback: Draw a solid dark blue rectangle if texture is missing
        if(renderer){ // Check renderer validity before using it
            SDL_SetRenderDrawColor(renderer, 0, 0, 50, 255); // Dark blue fallback
            SDL_RenderClear(renderer); // Clear whole screen with this color
        }
         SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "MenuState Render: Drawing fallback color - backgroundTexture_ is null.");
    }

    // --- Log placeholder options ---
    SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "--- Menu Render (Selection: %zu) ---", currentSelection_);
    for(size_t i = 0; i < menuOptions_.size(); ++i) {
        if (i == currentSelection_) { SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "> %s", menuOptions_[i].c_str()); }
        else { SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "  %s", menuOptions_[i].c_str()); }
    }
    // --- TODO later: Draw actual text using drawText ---
    // --- TODO later: Draw selection cursor ---

}
// <<< --- END RESTORED RENDER FUNCTION --- >>>


// Placeholder text drawing function
void MenuState::drawText(const std::string& text, int x, int y) {
     // TODO: Implement using bitmap font atlas later
     SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "drawText not implemented! Text: '%s' at (%d,%d)", text.c_str(), x, y);
}