// File: src/states/MenuState.cpp

#include "states/MenuState.h"
#include "core/Game.h"            // Needed for game_ptr access, requestPopState, requestPushState, getTextRenderer
#include "ui/TextRenderer.h"      // Needed for TextRenderer class
#include "core/AssetManager.h"    // Needed for asset loading (background, cursor)
#include "platform/pc/pc_display.h" // Needed for PCDisplay& parameter and methods
#include "core/InputManager.h"    // Needed for InputManager& parameter
#include "core/GameAction.h"      // Needed for GameAction enum
#include "core/PlayerData.h"      // Needed for PlayerData* parameter
#include <SDL_log.h>
#include <SDL.h>                  // Needed for SDL types (SDL_Renderer, SDL_Rect etc.)
#include <stdexcept>
#include <vector>
#include <string>
// #include <map> // No longer needed for fontCharMap_
// #include <fstream> // No longer needed for loadFontDataFromJson
#include <memory>                 // Needed for std::make_unique
// #include "vendor/nlohmann/json.hpp" // No longer needed for loadFontDataFromJson

#include "states/MapSystemState.h" // Added include for MapSystemState

// Need to potentially include PartnerSelectState.h if pushing it directly
#include "states/PartnerSelectState.h"

MenuState::MenuState(Game* game, const std::vector<std::string>& options) :
    GameState(game),
    menuOptions_(options),
    currentSelection_(0),
    backgroundTexture_(nullptr),
    // fontTexture_(nullptr), // Removed
    cursorTexture_(nullptr) // Explicitly initialize cursorTexture_ (assuming still used)
{
    if (!game_ptr || !game_ptr->getAssetManager()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MenuState requires valid Game pointer with initialized AssetManager!");
        // Consider throwing an exception or handling this error more robustly
    }
    else {
        AssetManager* assets = game_ptr->getAssetManager();
        backgroundTexture_ = assets->getTexture("menu_bg_blue");
        // fontTexture_ = assets->getTexture("ui_font_atlas"); // Removed
        // Cursor texture loading - kept assuming it's used independently
        cursorTexture_ = assets->getTexture("menu_cursor"); // Assuming a cursor texture ID
        if (!backgroundTexture_) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "MenuState Warning: Background texture 'menu_bg_blue' not found."); }
        if (!cursorTexture_) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "MenuState Warning: Cursor texture 'menu_cursor' not found."); }

        // Removed fontTexture_ check and loadFontDataFromJson call
        // if (!fontTexture_) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "MenuState Warning: Font texture 'ui_font_atlas' not found."); }
        // else {
        //     if (!loadFontDataFromJson("assets/ui/fonts/bluewhitefont.json")) {
        //         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MenuState Error: Failed to load font map data 'assets/ui/fonts/bluewhitefont.json'.");
        //     }
        // }
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState Created with %zu options.", options.size());
}

MenuState::~MenuState() {
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState Destroyed.");
}

void MenuState::handle_input(InputManager& inputManager, PlayerData* playerData) {
    if (!game_ptr) return;

    bool navUp = inputManager.isActionJustPressed(GameAction::NAV_UP);
    bool navDown = inputManager.isActionJustPressed(GameAction::NAV_DOWN);
    bool confirm = inputManager.isActionJustPressed(GameAction::CONFIRM);
    bool cancel = inputManager.isActionJustPressed(GameAction::CANCEL);

    if (navUp || navDown || confirm || cancel || inputManager.isActionHeld(GameAction::NAV_UP) || inputManager.isActionHeld(GameAction::NAV_DOWN) || inputManager.isActionHeld(GameAction::CONFIRM) || inputManager.isActionHeld(GameAction::CANCEL) ) { SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "MenuState Input Check: JustPressed(Up=%d, Down=%d, Confirm=%d, Cancel=%d)", navUp, navDown, confirm, cancel); }

    if (navUp) { if (!menuOptions_.empty()) { currentSelection_ = (currentSelection_ == 0) ? menuOptions_.size() - 1 : currentSelection_ - 1; SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "Menu: Selected option %zu - '%s'", currentSelection_, menuOptions_[currentSelection_].c_str()); } }
    else if (navDown) { if (!menuOptions_.empty()) { currentSelection_ = (currentSelection_ + 1) % menuOptions_.size(); SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "Menu: Selected option %zu - '%s'", currentSelection_, menuOptions_[currentSelection_].c_str()); } }
    else if (confirm) {
        if (!menuOptions_.empty()) {
            const std::string& selectedOption = menuOptions_[currentSelection_];
            SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Menu: Confirmed '%s'", selectedOption.c_str());

            if (selectedOption == "EXIT") {
                // Fade out current MenuState, pop it, then fade in the state below (e.g., AdventureState)
                game_ptr->requestFadeToState(nullptr, 0.3f, true); 
            } else if (selectedOption == "DIGIMON") {
                std::vector<std::string> opts = {"PARTNER", "STATUS", "EVOLVE", "BACK"};
                auto subMenu = std::make_unique<MenuState>(game_ptr, opts);
                game_ptr->requestFadeToState(std::move(subMenu), 0.3f, false); // Fade, don't pop current
            } else if (selectedOption == "MAP") {
                std::vector<std::string> opts = {"VIEW MAP", "TRAVEL", "BACK"};
                auto subMenu = std::make_unique<MenuState>(game_ptr, opts);
                game_ptr->requestFadeToState(std::move(subMenu), 0.3f, false); // Fade, don't pop current
            } else if (selectedOption == "VIEW MAP") { // This is the option within the "MAP" submenu
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "VIEW MAP selected. Changing state to MapSystemState.");
                // Fade out current MenuState (the "VIEW MAP" submenu), pop it, then fade to MapSystemState
                // To achieve this, we first request the MapSystemState. The fade system should handle the current.
                // We want to pop the current submenu and then push MapSystemState.
                // The requestFadeToState with popCurrent=true will pop the current (sub-menu)
                // and then push the new state (MapSystemState).
                game_ptr->requestFadeToState(std::make_unique<Digivice::MapSystemState>(game_ptr), 0.3f, true);

            } else if (selectedOption == "TRAVEL") {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Action for TRAVEL not implemented.");
                // Potentially another state or direct action
            } else if (selectedOption == "ITEMS") {
                std::vector<std::string> opts = {"VIEW", "USE", "DROP", "BACK"};
                auto subMenu = std::make_unique<MenuState>(game_ptr, opts);
                game_ptr->requestFadeToState(std::move(subMenu), 0.3f, false); // Fade, don't pop current
            } else if (selectedOption == "SAVE") {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Action for SAVE not implemented.");
            } else if (selectedOption == "BACK") {
                // Fade out current sub-menu, pop it, then fade in the parent menu below
                game_ptr->requestFadeToState(nullptr, 0.3f, true); 
            } else if (selectedOption == "PARTNER") {
                SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Menu: Fading to PartnerSelectState.");
                auto partnerSelectState = std::make_unique<PartnerSelectState>(game_ptr);
                game_ptr->requestFadeToState(std::move(partnerSelectState), 0.3f, false); // Fade, don't pop current
            } else if (selectedOption == "STATUS") {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Action for STATUS not implemented.");
                // If STATUS leads to another state, use requestFadeToState:
                // auto statusState = std::make_unique<StatusState>(game_ptr); // Assuming StatusState exists
                // game_ptr->requestFadeToState(std::move(statusState), 0.3f, false);
            } else if (selectedOption == "EVOLVE") {
                 SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Action for EVOLVE not implemented.");
                // If EVOLVE leads to another state, use requestFadeToState:
                // auto evolveState = std::make_unique<EvolveState>(game_ptr); // Assuming EvolveState exists
                // game_ptr->requestFadeToState(std::move(evolveState), 0.3f, false);
            }
            else {
                SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "Selected option '%s' has no defined action yet.", selectedOption.c_str());
            }
        }
    }
    else if (cancel) { 
        SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Cancel action detected in MenuState, requesting fade and pop."); 
        // Fade out current MenuState, pop it, then fade in the state below
        game_ptr->requestFadeToState(nullptr, 0.5f, true); 
    }
}

void MenuState::update(float delta_time, PlayerData* playerData) {
    // Update logic remains empty for now
}

void MenuState::render(PCDisplay& display) {
    SDL_Renderer* renderer = display.getRenderer();
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER,"MenuState Render Error: Renderer is null!");
        return;
    }

    TextRenderer* textRenderer = nullptr;
    if (game_ptr) {
        textRenderer = game_ptr->getTextRenderer();
    }
    if (!textRenderer) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "MenuState Render Error: TextRenderer is null!");
        // Optionally draw a fallback error message if textRenderer is crucial
    }


    int windowW = 0; int windowH = 0;
    display.getWindowSize(windowW, windowH);
    if (windowW <= 0 || windowH <= 0) {
        SDL_LogWarn(SDL_LOG_CATEGORY_RENDER,"MenuState Render Warning: Invalid window dimensions (%d x %d), defaulting.", windowW, windowH);
        windowW = 466; windowH = 466; // Default from original code
    }

    // 1. Draw Background
    if (backgroundTexture_) {
        SDL_RenderCopy(renderer, backgroundTexture_, NULL, NULL);
    }
    else {
        SDL_SetRenderDrawColor(renderer, 0, 0, 80, 255); // Default dark blue
        SDL_RenderClear(renderer);
    }

    // 2. Draw Currently Selected Menu Option
    SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "--- Menu Render Step 2 START ---");
    // Use textRenderer if available, otherwise skip text drawing or draw placeholder
    if (textRenderer && !menuOptions_.empty()) {
        SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Assets OK (TextRenderer: %p, Options size: %zu)", (void*)textRenderer, menuOptions_.size());

        const float textScale = 0.9f;
        const int kerning = -15; // This kerning might need to be a property of TextRenderer or passed differently
        const std::string& selectedText = menuOptions_[currentSelection_];
        SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Selected Text: '%s'", selectedText.c_str());

        // Assuming TextRenderer::getTextDimensions(text, scale, kerning)
        // and TextRenderer::drawText(renderer, text, x, y, scale, kerning)
        SDL_Point baseDimensions = textRenderer->getTextDimensions(selectedText, kerning);
                SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Base Dimensions calculated by TextRenderer: w=%d, h=%d", baseDimensions.x, baseDimensions.y);

        if (baseDimensions.x > 0 && baseDimensions.y > 0) {
            // Position calculation remains similar, scaling is handled by TextRenderer or here
            // If TextRenderer's getTextDimensions already accounts for scale, scaledTextWidth = baseDimensions.x
            // If not, scaledTextWidth = static_cast<int>(static_cast<float>(baseDimensions.x) * textScale);
            // For this example, assuming getTextDimensions gives unscaled, and drawText applies scale.
            // Or, if TextRenderer::getTextDimensions takes scale, it gives scaled dimensions.
            // Let's assume TextRenderer's methods handle scale internally or as a parameter.

            int scaledTextWidth = baseDimensions.x;  // Assuming getTextDimensions returns scaled if scale is passed
            int scaledTextHeight = baseDimensions.y; // Same assumption

            int startX = (windowW / 2) - (scaledTextWidth / 2);
            int startY = (windowH / 2) - (scaledTextHeight / 2);
            SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Calculated Position & Scale for TextRenderer: scale=%.2f, scaledW=%d, scaledH=%d, startX=%d, startY=%d", textScale, scaledTextWidth, scaledTextHeight, startX, startY);

            SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "--- Calling TextRenderer::drawText ---");
            textRenderer->drawText(renderer, selectedText, startX, startY, textScale, kerning);
            SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "--- Returned from TextRenderer::drawText ---");

        } else {
             SDL_LogError(SDL_LOG_CATEGORY_RENDER, "MenuState::render - SKIPPING DRAW due to invalid dimensions from TextRenderer for text: %s", selectedText.c_str());
        }
    } else {
        SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "MenuState::render - SKIPPING TEXT DRAW because TextRenderer missing or no options. TextRenderer: %p, Options Empty: %d", (void*)textRenderer, menuOptions_.empty());
    }
    SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "--- Menu Render Step 2 END ---");

    // Add cursor rendering logic here if cursorTexture_ is used
    // For example:
    // if (cursorTexture_ && !menuOptions_.empty()) {
    //     int cursorX = MENU_START_X - CURSOR_WIDTH - CURSOR_OFFSET_X; // Define these constants
    //     int cursorY = MENU_START_Y + (currentSelection_ * MENU_ITEM_HEIGHT) + (MENU_ITEM_HEIGHT / 2) - (CURSOR_HEIGHT / 2);
    //     SDL_Rect dst = { cursorX, cursorY, CURSOR_WIDTH, CURSOR_HEIGHT };
    //     SDL_RenderCopy(renderer, cursorTexture_, NULL, &dst);
    // }
}

StateType MenuState::getType() const {
    return StateType::Menu;
}

// --- Helper Functions ---
// bool MenuState::loadFontDataFromJson(const std::string& jsonPath) { /* ... DELETED ... */ }
// SDL_Point MenuState::getTextDimensions(const std::string& text, int kerning) { /* ... DELETED ... */ }
// void MenuState::drawText(SDL_Renderer* renderer, const std::string& text, int startX, int startY, float scale, int kerning) { /* ... DELETED ... */ }