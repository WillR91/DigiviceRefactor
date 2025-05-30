// Enhanced MenuState implementation using UI framework
// Maintains 100% compatibility with original MenuState interface

#include "states/MenuState.h"
#include "core/Game.h"
#include "ui/DigiviceScreen.h"
#include "ui/MenuList.h"
#include "ui/TextRenderer.h"
#include "core/AssetManager.h"
#include "platform/pc/pc_display.h"
#include "core/InputManager.h"
#include "core/GameAction.h"
#include "core/PlayerData.h"
#include <SDL_log.h>
#include <SDL.h>
#include <stdexcept>
#include <memory>

// Include states for transitions
#include "states/MapSystemState.h"
#include "states/PartnerSelectState.h"
#include "states/SettingsState.h"
#include "states/EnemyTestState.h"
#include "states/PlayerTestState.h"

MenuState::MenuState(Game* game, const std::vector<std::string>& options) :
    GameState(game),
    menuOptions_(options),
    currentSelection_(0),
    backgroundTexture_(nullptr),
    cursorTexture_(nullptr)
{
    if (!game_ptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MenuState: Game pointer is null!");
        return;
    }

    // Load original assets for compatibility
    if (AssetManager* assets = game_ptr->getAssetManager()) {
        backgroundTexture_ = assets->getTexture("menu_bg_blue");
        cursorTexture_ = assets->getTexture("menu_cursor");
        
        if (!backgroundTexture_) { 
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "MenuState: Background texture 'menu_bg_blue' not found."); 
        }
        if (!cursorTexture_) { 
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "MenuState: Cursor texture 'menu_cursor' not found."); 
        }
    }

    // Create enhanced UI components
    try {
        // Create the main screen with background
        screen_ = std::make_unique<DigiviceScreen>(game_ptr, "menu_bg_blue");
        if (!screen_) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MenuState: Failed to create DigiviceScreen!");
            return;
        }        // Get screen dimensions for layout
        SDL_Point screenSize = screen_->getScreenSize();
        SDL_Point screenCenter = screen_->getScreenCenter();
        
        // Create menu list covering the full screen area for precise centering (matching original behavior)
        // The carousel mode will center the text within this full area, just like the original
        menuList_ = std::make_shared<MenuList>(0, 0, screenSize.x, screenSize.y, game_ptr->getTextRenderer());
        if (!menuList_) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MenuState: Failed to create MenuList!");
            return;
        }        // Configure menu to match original appearance
        menuList_->setItems(menuOptions_);        menuList_->setDisplayMode(MenuList::DisplayMode::Carousel);  // Use carousel mode like original
        menuList_->setAlignment(MenuList::Alignment::Center);
        menuList_->setTextScale(0.9f);    // Same scale as original
        menuList_->setTextKerning(0);     // Fixed kerning (was -15 causing overlap)        menuList_->setItemSpacing(20);
        menuList_->setAnimationDuration(0.15f);  // Set animation duration (150ms - twice as fast)
          // Set colors to match original menu style
        menuList_->setTextColor(255, 255, 255, 255);        // White text
        menuList_->setSelectedTextColor(255, 255, 255, 255);  // White text for selected item too
        
        // Setup cursor if available
        if (cursorTexture_) {
            menuList_->setCursorTexture(cursorTexture_, 16, 16);
            menuList_->setCursorOffset(-25, 0);
        }
        
        // Set selection callback
        menuList_->setSelectionCallback([this](int index, const std::string& text) {
            this->onMenuItemSelected(index, text);
        });
        
        // Add menu to screen
        screen_->addChild(menuList_);
        
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState created with %zu options", options.size());

    } catch (const std::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MenuState: Exception during initialization: %s", e.what());
    }
}

MenuState::~MenuState() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState destroyed.");
}

void MenuState::setupMenuLayout() {
    // This method is kept for compatibility but functionality is now in constructor
    if (!menuList_) return;
    
    // Update current selection to sync with MenuList
    if (menuList_->getSelectedIndex() != currentSelection_) {
        menuList_->setSelectedIndex(currentSelection_);
    }
}

void MenuState::handle_input(InputManager& inputManager, PlayerData* playerData) {
    // Handle CANCEL - same as original
    if (inputManager.isActionJustPressed(GameAction::CANCEL)) {
        game_ptr->requestPopState();
        return;
    }

    // Let the screen handle input (which will delegate to menu list)
    if (screen_) {
        screen_->handleInput(inputManager);
    }
    
    // Sync currentSelection_ with MenuList for compatibility
    if (menuList_) {
        currentSelection_ = menuList_->getSelectedIndex();
    }
}

void MenuState::update(float delta_time, PlayerData* playerData) {
    // Update the screen and all its children
    if (screen_) {
        screen_->update(delta_time);
    }
}

void MenuState::render(PCDisplay& display) {
    SDL_Renderer* renderer = display.getRenderer();
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "MenuState: Renderer is null!");
        return;
    }

    // Render the screen (which will render background and all children)
    if (screen_) {
        screen_->render(renderer);
    } else {
        // Fallback rendering if screen is not available
        if (backgroundTexture_) {
            SDL_RenderCopy(renderer, backgroundTexture_, NULL, NULL);
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 0, 80, 255); // Default dark blue
            SDL_RenderClear(renderer);
        }
    }
}

StateType MenuState::getType() const {
    return StateType::Menu;
}

void MenuState::onMenuItemSelected(int selectedIndex, const std::string& selectedText) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState: Selected '%s' at index %d", 
                selectedText.c_str(), selectedIndex);

    // Update currentSelection_ for compatibility
    currentSelection_ = selectedIndex;

    // Handle menu selections based on text content
    if (selectedText == "MAP") {
        handleMapSelection();
    } else if (selectedText == "DIGIMON") {
        handleDigimonSelection();
    } else if (selectedText == "SAVE") {
        handleSaveSelection();
    } else if (selectedText == "SETTINGS") {
        handleSettingsSelection();
    } else if (selectedText == "DEBUG") {
        handleDebugSelection();
    } else if (selectedText == "EXIT") {
        handleExitSelection();
    } else if (selectedText == "TEST DIGIMON") {
        handleTestDigimonSelection();
    } else if (selectedText == "ENEMY DIGIMON") {
        handleEnemyDigimonSelection();
    } else if (selectedText == "PLAYER DIGIMON") {
        handlePlayerDigimonSelection();
    } else if (selectedText == "BACK") {
        handleBackSelection();
    } else {
        // Fallback to legacy handling
        handleMenuSelection(selectedText);
    }
}

void MenuState::handleMenuSelection(const std::string& selectedItem) {
    // Legacy menu selection handler for compatibility
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState: Legacy handler for '%s'", selectedItem.c_str());
    
    if (selectedItem == "MAP") {
        handleMapSelection();
    } else if (selectedItem == "DIGIMON") {
        handleDigimonSelection();
    } else if (selectedItem == "SAVE") {
        handleSaveSelection();
    } else if (selectedItem == "SETTINGS") {
        handleSettingsSelection();
    } else if (selectedItem == "DEBUG") {
        handleDebugSelection();
    } else if (selectedItem == "EXIT") {
        handleExitSelection();
    } else {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "MenuState: Unhandled menu option '%s'", selectedItem.c_str());
    }
}

void MenuState::handleMapSelection() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState: Transitioning to Map System");
    auto mapState = std::make_unique<Digivice::MapSystemState>(game_ptr);
    game_ptr->requestFadeToState(std::move(mapState));
}

void MenuState::handleDigimonSelection() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState: Transitioning to Partner Select");
    auto partnerSelectState = std::make_unique<PartnerSelectState>(game_ptr);
    game_ptr->requestPushState(std::move(partnerSelectState));
}

void MenuState::handleSaveSelection() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState: Save functionality not yet implemented");
    // TODO: Implement save functionality
}

void MenuState::handleSettingsSelection() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState: Transitioning to Settings");
    auto settingsState = std::make_unique<SettingsState>(game_ptr);
    game_ptr->requestPushState(std::move(settingsState));
}

void MenuState::handleDebugSelection() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState: Opening debug submenu");
    std::vector<std::string> debugOptions = {"TEST DIGIMON", "SETTINGS", "BACK"};
    auto debugMenuState = std::make_unique<MenuState>(game_ptr, debugOptions);
    game_ptr->requestPushState(std::move(debugMenuState));
}

void MenuState::handleExitSelection() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState: Exiting game");
    game_ptr->quit_game();
}

void MenuState::handleTestDigimonSelection() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState: Opening Digimon test submenu");
    std::vector<std::string> digimonTestOptions = {"ENEMY DIGIMON", "PLAYER DIGIMON", "BACK"};
    auto digimonTestMenuState = std::make_unique<MenuState>(game_ptr, digimonTestOptions);
    game_ptr->requestPushState(std::move(digimonTestMenuState));
}

void MenuState::handleEnemyDigimonSelection() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState: Transitioning to Enemy Test State");
    auto enemyTestState = std::make_unique<EnemyTestState>(game_ptr);
    game_ptr->requestPushState(std::move(enemyTestState));
}

void MenuState::handlePlayerDigimonSelection() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState: Transitioning to Player Test State");
    auto playerTestState = std::make_unique<PlayerTestState>(game_ptr);
    game_ptr->requestPushState(std::move(playerTestState));
}

void MenuState::handleBackSelection() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState: Going back");
    game_ptr->requestPopState();
}