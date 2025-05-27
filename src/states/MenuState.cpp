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
#include "states/PartnerSelectState.h" // For PartnerSelectState
#include "states/SettingsState.h" // For SettingsState
#include "states/EnemyTestState.h" // For EnemyTestState
#include "states/PlayerTestState.h" // For PlayerTestState
#include "../../include/states/MapSystemState.h"  // Add this at the top with other includes

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
    }    else {
        AssetManager* assets = game_ptr->getAssetManager();
        backgroundTexture_ = assets->requestTexture("menu_bg_blue");
        // fontTexture_ = assets->getTexture("ui_font_atlas"); // Removed
        // Cursor texture loading - kept assuming it's used independently
        cursorTexture_ = assets->requestTexture("menu_cursor"); // Assuming a cursor texture ID
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
    if (inputManager.isActionJustPressed(GameAction::CANCEL)) {
        game_ptr->requestPopState();
        return;
    }

    if (inputManager.isActionJustPressed(GameAction::CONFIRM)) {
        if (currentSelection_ >= 0 && currentSelection_ < menuOptions_.size()) {
            const std::string& selectedItem = menuOptions_[currentSelection_];
              if (selectedItem == "MAP") {
                // Use full namespace if necessary (e.g., Digivice::MapSystemState)
                auto mapState = std::make_unique<Digivice::MapSystemState>(game_ptr);
                game_ptr->requestFadeToState(std::move(mapState));
                return;
            } 
            else if (selectedItem == "DIGIMON") {
                // Transition to PartnerSelectState
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState: DIGIMON selected. Pushing PartnerSelectState.");
                auto partnerSelectState = std::make_unique<PartnerSelectState>(game_ptr);
                game_ptr->requestPushState(std::move(partnerSelectState));
                return; // Added return to exit after handling
            }
            else if (selectedItem == "DEBUG") {
                // Create and show debug submenu
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState: DEBUG selected. Opening debug submenu.");
                std::vector<std::string> debugOptions = {"TEST DIGIMON", "SETTINGS", "BACK"};
                auto debugMenuState = std::make_unique<MenuState>(game_ptr, debugOptions);
                game_ptr->requestPushState(std::move(debugMenuState));
                return;
            }
            else if (selectedItem == "TEST DIGIMON") {
                // Create and show Digimon test submenu
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState: TEST DIGIMON selected. Opening Digimon test submenu.");
                std::vector<std::string> digimonTestOptions = {"ENEMY DIGIMON", "PLAYER DIGIMON", "BACK"};
                auto digimonTestMenuState = std::make_unique<MenuState>(game_ptr, digimonTestOptions);
                game_ptr->requestPushState(std::move(digimonTestMenuState));
                return;
            }
            else if (selectedItem == "ENEMY DIGIMON") {
                // Transition to EnemyTestState for debugging
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState: ENEMY DIGIMON selected. Pushing EnemyTestState.");
                auto enemyTestState = std::make_unique<EnemyTestState>(game_ptr);
                game_ptr->requestPushState(std::move(enemyTestState));
                return;
            }
            else if (selectedItem == "PLAYER DIGIMON") {
                // Transition to PlayerTestState for debugging player Digimon
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState: PLAYER DIGIMON selected. Pushing PlayerTestState.");
                auto playerTestState = std::make_unique<PlayerTestState>(game_ptr);
                game_ptr->requestPushState(std::move(playerTestState));
                return;
            }
            // Other menu options handling...
        }
    }

    // Navigate through menu options
    if (inputManager.isActionJustPressed(GameAction::NAV_UP)) {
        if (currentSelection_ > 0) {
            currentSelection_--;
        } else {
            currentSelection_ = menuOptions_.size() - 1;
        }
    } else if (inputManager.isActionJustPressed(GameAction::NAV_DOWN)) {
        if (currentSelection_ < menuOptions_.size() - 1) {
            currentSelection_++;
        } else {
            currentSelection_ = 0;
        }
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