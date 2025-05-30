#include "states/EnhancedMenuState.h"
#include "core/Game.h"
#include "core/AssetManager.h"
#include "ui/TextRenderer.h"
#include "platform/pc/pc_display.h"
#include "core/InputManager.h"
#include "core/GameAction.h"
#include "core/PlayerData.h"
#include <SDL_log.h>
#include <memory>

// Include other states for transitions
#include "states/MapSystemState.h"
#include "states/PartnerSelectState.h"
#include "states/SettingsState.h"
#include "states/EnemyTestState.h"
#include "states/PlayerTestState.h"

EnhancedMenuState::EnhancedMenuState(Game* game, const std::vector<std::string>& options)
    : GameState(game) {
    
    if (!game_ptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "EnhancedMenuState: Game pointer is null!");
        return;
    }

    // Create the main screen with background
    screen_ = std::make_unique<DigiviceScreen>(game_ptr, "menu_bg_blue");
    
    if (!screen_) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "EnhancedMenuState: Failed to create DigiviceScreen!");
        return;
    }

    // Get screen dimensions for layout
    SDL_Point screenSize = screen_->getScreenSize();
    SDL_Point screenCenter = screen_->getScreenCenter();
    
    // Create menu list centered on screen
    int menuWidth = screenSize.x * 0.8f;  // 80% of screen width
    int menuHeight = screenSize.y * 0.6f; // 60% of screen height
    int menuX = (screenSize.x - menuWidth) / 2;
    int menuY = (screenSize.y - menuHeight) / 2;
    
    menuList_ = std::make_shared<MenuList>(menuX, menuY, menuWidth, menuHeight, game_ptr->getTextRenderer());
    
    if (!menuList_) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "EnhancedMenuState: Failed to create MenuList!");
        return;
    }

    // Configure menu appearance
    menuList_->setItems(options);
    menuList_->setAlignment(MenuList::Alignment::Center);
    menuList_->setTextScale(0.9f);
    menuList_->setTextKerning(-15);
    menuList_->setItemSpacing(20);
    
    // Set colors to match original menu style
    menuList_->setTextColor(255, 255, 255, 255);        // White text
    menuList_->setSelectedTextColor(255, 255, 0, 255);  // Yellow selected text
    
    // Setup cursor if available
    if (AssetManager* assets = game_ptr->getAssetManager()) {
        SDL_Texture* cursorTexture = assets->getTexture("menu_cursor");
        if (cursorTexture) {
            menuList_->setCursorTexture(cursorTexture, 16, 16);
            menuList_->setCursorOffset(-25, 0);
        }
    }
    
    // Set selection callback
    menuList_->setSelectionCallback([this](int index, const std::string& text) {
        onMenuItemSelected(index, text);
    });
    
    // Add menu to screen
    screen_->addChild(menuList_);
    
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "EnhancedMenuState created with %zu options", options.size());
}

void EnhancedMenuState::handle_input(InputManager& inputManager, PlayerData* playerData) {
    // Handle cancel action to pop state
    if (inputManager.isActionJustPressed(GameAction::CANCEL)) {
        game_ptr->requestPopState();
        return;
    }

    // Let the screen handle input (which will delegate to menu list)
    if (screen_) {
        screen_->handleInput(inputManager);
    }
}

void EnhancedMenuState::update(float deltaTime, PlayerData* playerData) {
    // Update the screen and all its children
    if (screen_) {
        screen_->update(deltaTime);
    }
}

void EnhancedMenuState::render(PCDisplay& display) {
    SDL_Renderer* renderer = display.getRenderer();
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "EnhancedMenuState: Renderer is null!");
        return;
    }

    // Render the screen (which will render background and all children)
    if (screen_) {
        screen_->render(renderer);
    }
}

StateType EnhancedMenuState::getType() const {
    return StateType::Menu;
}

void EnhancedMenuState::onMenuItemSelected(int selectedIndex, const std::string& selectedText) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "EnhancedMenuState: Selected '%s' at index %d", 
                selectedText.c_str(), selectedIndex);

    // Handle menu selections based on text content
    if (selectedText == "MAP") {
        handleMapSelection();
    } else if (selectedText == "DIGIMON") {
        handleDigimonSelection();
    } else if (selectedText == "DEBUG") {
        handleDebugSelection();
    } else if (selectedText == "TEST DIGIMON") {
        handleTestDigimonSelection();
    } else if (selectedText == "ENEMY DIGIMON") {
        handleEnemyDigimonSelection();
    } else if (selectedText == "PLAYER DIGIMON") {
        handlePlayerDigimonSelection();
    } else if (selectedText == "SETTINGS") {
        handleSettingsSelection();
    } else if (selectedText == "BACK") {
        handleBackSelection();
    } else {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "EnhancedMenuState: Unhandled menu option '%s'", 
                    selectedText.c_str());
    }
}

void EnhancedMenuState::handleMapSelection() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "EnhancedMenuState: Transitioning to Map System");
    auto mapState = std::make_unique<Digivice::MapSystemState>(game_ptr);
    game_ptr->requestFadeToState(std::move(mapState));
}

void EnhancedMenuState::handleDigimonSelection() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "EnhancedMenuState: Transitioning to Partner Select");
    auto partnerSelectState = std::make_unique<PartnerSelectState>(game_ptr);
    game_ptr->requestPushState(std::move(partnerSelectState));
}

void EnhancedMenuState::handleDebugSelection() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "EnhancedMenuState: Opening debug submenu");
    std::vector<std::string> debugOptions = {"TEST DIGIMON", "SETTINGS", "BACK"};
    auto debugMenuState = std::make_unique<EnhancedMenuState>(game_ptr, debugOptions);
    game_ptr->requestPushState(std::move(debugMenuState));
}

void EnhancedMenuState::handleTestDigimonSelection() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "EnhancedMenuState: Opening Digimon test submenu");
    std::vector<std::string> digimonTestOptions = {"ENEMY DIGIMON", "PLAYER DIGIMON", "BACK"};
    auto digimonTestMenuState = std::make_unique<EnhancedMenuState>(game_ptr, digimonTestOptions);
    game_ptr->requestPushState(std::move(digimonTestMenuState));
}

void EnhancedMenuState::handleEnemyDigimonSelection() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "EnhancedMenuState: Transitioning to Enemy Test State");
    auto enemyTestState = std::make_unique<EnemyTestState>(game_ptr);
    game_ptr->requestPushState(std::move(enemyTestState));
}

void EnhancedMenuState::handlePlayerDigimonSelection() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "EnhancedMenuState: Transitioning to Player Test State");
    auto playerTestState = std::make_unique<PlayerTestState>(game_ptr);
    game_ptr->requestPushState(std::move(playerTestState));
}

void EnhancedMenuState::handleSettingsSelection() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "EnhancedMenuState: Transitioning to Settings");
    auto settingsState = std::make_unique<SettingsState>(game_ptr);
    game_ptr->requestPushState(std::move(settingsState));
}

void EnhancedMenuState::handleBackSelection() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "EnhancedMenuState: Going back");
    game_ptr->requestPopState();
}
