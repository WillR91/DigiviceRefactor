// File: src/states/MenuState.cpp

#include "states/MenuState.h"
#include "core/Game.h"
#include "core/AssetManager.h"      // Needed for getting assets
#include "platform/pc/pc_display.h" // Needed for drawing
#include <SDL_log.h>
#include <SDL.h>                    // <<< ADDED: Need SDL for input state / keys >>>
#include <stdexcept>

// Assume WINDOW_WIDTH/HEIGHT are accessible somehow or replace later
const int WINDOW_WIDTH = 466;
const int WINDOW_HEIGHT = 466;


MenuState::MenuState(Game* game, const std::vector<std::string>& options) :
    menuOptions_(options),
    currentSelection_(0),
    backgroundTexture_(nullptr) // <<< Initialize background pointer >>>
    // Initialize other placeholders
    // fontTexture_(nullptr),
    // cursorTexture_(nullptr)
{
    this->game_ptr = game;
    if (!game_ptr || !game_ptr->getAssetManager()) {
        // Log error or throw exception if core systems missing
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MenuState Error: Game or AssetManager pointer is null!");
        throw std::runtime_error("MenuState requires Game with AssetManager");
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState Created with %zu options.", options.size());

    // --- Get background texture from AssetManager ---
    AssetManager* assets = game_ptr->getAssetManager();
    backgroundTexture_ = assets->getTexture("menu_bg_blue"); // Use the ID from Game::init
    if (!backgroundTexture_) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "MenuState: Background texture 'menu_bg_blue' not found!");
    }
    // --- End Get Texture ---

    // TODO: Get font/cursor textures later
}

MenuState::~MenuState() {
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState Destroyed.");
}

void MenuState::handle_input() {
    // Get current keyboard state
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    static bool esc_pressed_last_frame = false; // Track only Escape for now

    // --- Handle Back/Exit (Escape key) ---
    if (keys[SDL_SCANCODE_ESCAPE]) {
        if (!esc_pressed_last_frame) {
            SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Escape pressed in MenuState, popping state.");
            if(game_ptr) {
                game_ptr->pop_state(); // Remove this MenuState from the stack
                // Return immediately to avoid processing other input in this state
                // during the same frame it's being removed.
                return;
            }
        }
        esc_pressed_last_frame = true; // Mark as held
    } else {
        esc_pressed_last_frame = false; // Mark as released
    }

    // --- TODO later: Handle Up/Down navigation ---
    // --- TODO later: Handle Select/Confirm action ---

}

void MenuState::update(float delta_time) {
    // No updates needed for a static menu yet
}

void MenuState::render() {
    // Get necessary pointers (with checks)
    if (!game_ptr) return;
    PCDisplay* display = game_ptr->get_display();
    if (!display) return;
    // Renderer needed only for fallback color
    // SDL_Renderer* renderer = display->getRenderer();

    // --- Draw the Menu Background ---
    if (backgroundTexture_) {
        int texW, texH;
        SDL_QueryTexture(backgroundTexture_, NULL, NULL, &texW, &texH);

        // Center the background image
        int drawX = (WINDOW_WIDTH / 2) - (texW / 2);
        int drawY = (WINDOW_HEIGHT / 2) - (texH / 2);
        SDL_Rect dstRect = { drawX, drawY, texW, texH };

        // Draw using the display wrapper
        display->drawTexture(backgroundTexture_, NULL, &dstRect);
    } else {
        // Fallback: Draw a solid dark blue rectangle if texture is missing
        SDL_Renderer* renderer = display->getRenderer(); // Get renderer for fallback
        if(renderer){
            SDL_SetRenderDrawColor(renderer, 0, 0, 50, 255); // Dark blue fallback
            SDL_RenderClear(renderer); // Clear whole screen with this color
             SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "MenuState Render: Drawing fallback color - backgroundTexture_ is null.");
        }
    }

    // --- TODO later: Draw actual menu options using drawText ---
    SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Menu Options Placeholder:");
     for(size_t i = 0; i < menuOptions_.size(); ++i) {
         if (i == currentSelection_) { SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "> %s", menuOptions_[i].c_str()); }
         else { SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "  %s", menuOptions_[i].c_str()); }
     }

    // --- TODO later: Draw selection cursor ---

}

// Placeholder text drawing function (Keep for now)
void MenuState::drawText(const std::string& text, int x, int y) {
     SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "drawText not implemented! Text: '%s' at (%d,%d)", text.c_str(), x, y);
}