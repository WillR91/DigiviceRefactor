// File: src/states/MenuState.cpp

#include "states/MenuState.h"
#include "core/Game.h"              // Needed for game_ptr access
#include "core/AssetManager.h"      // Needed for asset loading
#include "platform/pc/pc_display.h" // Needed for display pointer
#include "states/TransitionState.h" // <<< NEEDED to call parent->requestExit() >>>
#include <SDL_log.h>
#include <SDL.h>
#include <stdexcept>
#include <vector>                   // <<< Need for stack access >>>
#include <memory>                   // <<< Need for unique_ptr access >>>


MenuState::MenuState(Game* game, const std::vector<std::string>& options) :
    menuOptions_(options),
    currentSelection_(0),
    backgroundTexture_(nullptr), // Still load it, just don't draw it here
    fontTexture_(nullptr),
    cursorTexture_(nullptr)
{
    this->game_ptr = game;
    if (!game_ptr || !game_ptr->getAssetManager()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MenuState Error: Game or AssetManager pointer is null!");
    } else {
        AssetManager* assets = game_ptr->getAssetManager();
        // Load the background texture even if we don't draw it directly in this state
        backgroundTexture_ = assets->getTexture("menu_bg_blue");
        if (!backgroundTexture_) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "MenuState: Background texture 'menu_bg_blue' not found!");
        }
        // TODO: Load font/cursor textures
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState Created with %zu options.", options.size());
}

MenuState::~MenuState() {
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState Destroyed.");
}

void MenuState::handle_input() {
    // Input is now passed down from TransitionState when appropriate.
    // No need for a top-state check here assuming TransitionState handles that.

    const Uint8* keys = SDL_GetKeyboardState(NULL);
    static bool esc_pressed_last_frame = false;
    static bool up_pressed_last_frame = false;
    static bool down_pressed_last_frame = false;
    static bool select_pressed_last_frame = false;

    // Exit Menu (Escape or Backspace)
    if (keys[SDL_SCANCODE_ESCAPE] || keys[SDL_SCANCODE_BACKSPACE]) {
        if (!esc_pressed_last_frame) {
            SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Exit key pressed in MenuState, signalling TransitionState parent.");
            if(game_ptr) {
                // <<< --- MODIFIED: Signal the TransitionState above to exit --- >>>
                auto& stack = game_ptr->DEBUG_getStack(); // Get stack via helper
                if (stack.size() >= 2) {
                    // Get the state below us (index size-2) and try casting
                    TransitionState* parent = dynamic_cast<TransitionState*>(stack[stack.size() - 2].get());
                    if (parent) {
                         parent->requestExit(); // Tell parent TransitionState to handle popping
                    } else {
                         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MenuState Error: State below is not TransitionState! Cannot signal exit.");
                         // Perhaps do nothing, as popping self would leave parent stuck
                    }
                } else {
                     SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MenuState Error: Stack size < 2, cannot get parent TransitionState!");
                     // Perhaps do nothing
                }
                // <<< --- END MODIFICATION --- >>>
                esc_pressed_last_frame = true; // Prevent re-triggering
                return; // Exit input handling for this frame
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

    // Handle Selection (Enter key)
    if (keys[SDL_SCANCODE_RETURN]) {
        if (!select_pressed_last_frame && !menuOptions_.empty()) {
            const std::string& selectedOption = menuOptions_[currentSelection_];
            SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Menu: Selected '%s'", selectedOption.c_str());
            // TODO: Implement actions (e.g., push another state, call game quit)
            select_pressed_last_frame = true;
        }
    } else {
        select_pressed_last_frame = false;
    }
}

void MenuState::update(float delta_time) {
    // Nothing state-specific to update here yet
}


// --- Render Function ---
// <<< MODIFIED: ONLY draws menu items, NO background/border >>>
void MenuState::render() {
    if (!game_ptr) return;
    PCDisplay* display = game_ptr->get_display();
    if (!display) return;

    // <<< --- NO BACKGROUND OR BORDER DRAWING --- >>>
    // The TransitionState below this one is responsible for drawing the background (AdventureState)
    // and the border frame on top of it. This state only needs to draw its contents.

    // --- Draw Menu Options (Placeholder - Requires Font Rendering) ---
    // Ensure MENU_START_X/Y constants are appropriate for drawing *inside* the border
    for (size_t i = 0; i < menuOptions_.size(); ++i) {
         if (i == currentSelection_) {
              SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "> %s at (%d, %d)", menuOptions_[i].c_str(), MENU_START_X, MENU_START_Y + (int)(i * MENU_ITEM_HEIGHT));
             // TODO: drawText(...) or draw cursor texture
         } else {
              SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "  %s at (%d, %d)", menuOptions_[i].c_str(), MENU_START_X, MENU_START_Y + (int)(i * MENU_ITEM_HEIGHT));
             // TODO: drawText(...)
         }
    }
    // --- TODO: Draw Cursor Texture ---
}

// Placeholder text drawing function (Implementation still needed)
void MenuState::drawText(const std::string& text, int x, int y) {
     SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "drawText not implemented! Text: '%s' at (%d,%d)", text.c_str(), x, y);
}