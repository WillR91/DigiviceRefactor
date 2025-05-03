// File: src/states/MenuState.cpp

#include "states/MenuState.h"
#include "core/Game.h"
#include "core/AssetManager.h"
#include "platform/pc/pc_display.h" // Needed for display->drawTexture
#include <SDL_log.h>
#include <SDL.h>                    // Included for SDL_SCANCODEs etc.
#include <stdexcept>

// TEMP Constants - Can likely be removed if window size is fetched correctly
// const int WINDOW_WIDTH = 466;
// const int WINDOW_HEIGHT = 466;


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
        // Consider throwing an exception or handling this error more gracefully
        // For now, we log and proceed, but background might be null.
    } else {
        AssetManager* assets = game_ptr->getAssetManager();
        backgroundTexture_ = assets->getTexture("menu_bg_blue"); // ID used to load the asset
        if (!backgroundTexture_) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "MenuState: Background texture 'menu_bg_blue' not found!");
        }
        // TODO: Load font/cursor textures using assets->getTexture(...)
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState Created with %zu options.", options.size());
}

MenuState::~MenuState() {
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState Destroyed.");
}

void MenuState::handle_input() {
    // Prevent input handling if not the top state
    if (game_ptr && game_ptr->getCurrentState() != this) {
        return;
    }

    const Uint8* keys = SDL_GetKeyboardState(NULL);
    static bool esc_pressed_last_frame = false;
    static bool up_pressed_last_frame = false;
    static bool down_pressed_last_frame = false;
    static bool select_pressed_last_frame = false; // Added for selection

    // Exit Menu (Escape or Backspace)
    if (keys[SDL_SCANCODE_ESCAPE] || keys[SDL_SCANCODE_BACKSPACE]) {
        if (!esc_pressed_last_frame) {
            SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Exit key pressed in MenuState, requesting pop.");
            if(game_ptr) {
                game_ptr->requestPopState();
                esc_pressed_last_frame = true; // Prevent re-triggering if pop is delayed
                return; // Exit input handling for this frame after requesting pop
            }
        }
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

    // --- Handle Selection (Enter key) ---
    if (keys[SDL_SCANCODE_RETURN]) { // Or SDL_SCANCODE_SPACE maybe?
        if (!select_pressed_last_frame && !menuOptions_.empty()) {
            const std::string& selectedOption = menuOptions_[currentSelection_];
            SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Menu: Selected '%s'", selectedOption.c_str());
            // TODO: Implement actions based on selectedOption
            // Example:
            // if (selectedOption == "QUIT") { game_ptr->quit(); }
            // else if (selectedOption == "DIGIMON") { /* request push PartnerSelectState */ }
            // etc.
            select_pressed_last_frame = true; // Mark as pressed
        }
    } else {
        select_pressed_last_frame = false;
    }
}

void MenuState::update(float delta_time) {
    // Usually empty unless menu has animations or timed elements
}


// --- Render Function ---
// <<< MODIFIED: UNCOMMENTED background drawing >>>
void MenuState::render() {
    if (!game_ptr) return;
    PCDisplay* display = game_ptr->get_display();
    if (!display) return;

    // --- 1. Draw the Background --- <<< UNCOMMENTED >>>

    if (backgroundTexture_) {
        // Use NULL for the destination SDL_Rect to draw the texture
        // stretched to the entire rendering target (the window).
        display->drawTexture(backgroundTexture_, NULL, NULL);
    } else {
        // Fallback: Clear the screen with a solid color if texture is missing
        SDL_Renderer* renderer = display->getRenderer();
        if (renderer) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 50, 255); // Dark blue fallback
            SDL_RenderClear(renderer);
        } else {
            SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "MenuState Render: Cannot draw fallback color - renderer is null.");
        }
         // Log only once if fallback is used
         static bool logged_fallback = false;
         if (!logged_fallback) {
             SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "MenuState Render: Drawing fallback color - backgroundTexture_ is null or renderer missing.");
             logged_fallback = true;
         }
    }

    // --- <<< END OF BLOCK TO UNCOMMENT >>> ---


    // --- 2. Draw Menu Options (Placeholder - Requires Font Rendering) ---
    // Logging for now, replace with drawText calls later
    // SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "--- Menu Render (Selection: %zu) ---", currentSelection_);
    for (size_t i = 0; i < menuOptions_.size(); ++i) {
         // TODO: Replace logging with actual text rendering + cursor logic
         if (i == currentSelection_) {
              SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "> %s at (%d, %d)", menuOptions_[i].c_str(), MENU_START_X, MENU_START_Y + (int)(i * MENU_ITEM_HEIGHT));
             // drawText(">" + menuOptions_[i], MENU_START_X - 20, MENU_START_Y + i * MENU_ITEM_HEIGHT); // Example with cursor ">"
             // Draw cursor texture here
         } else {
              SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "  %s at (%d, %d)", menuOptions_[i].c_str(), MENU_START_X, MENU_START_Y + (int)(i * MENU_ITEM_HEIGHT));
             // drawText(menuOptions_[i], MENU_START_X, MENU_START_Y + i * MENU_ITEM_HEIGHT);
         }
    }

    // --- 3. TODO: Draw Cursor Texture at selected position ---

}

// Placeholder text drawing function (Implementation still needed)
void MenuState::drawText(const std::string& text, int x, int y) {
     // TODO: Implement using bitmap font atlas later
     // This function will need access to the font texture, font metadata (char rects),
     // and the display/renderer to draw characters one by one.
     SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "drawText not implemented! Text: '%s' at (%d,%d)", text.c_str(), x, y);
}