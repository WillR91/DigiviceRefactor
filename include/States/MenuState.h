// File: include/states/MenuState.h
#pragma once

#include "states/GameState.h"
#include "ui/DigiviceScreen.h"
#include "ui/MenuList.h"
#include <memory>
#include <vector>
#include <string>
#include <SDL.h>

// Forward declarations
class Game;
class InputManager;
class PlayerData;
class PCDisplay;

/**
 * @brief Enhanced MenuState using the new UI framework
 * 
 * This is a modernized version that uses DigiviceScreen and MenuList components
 * for cleaner, more maintainable implementation while preserving exact compatibility
 * with the original MenuState interface.
 */
class MenuState : public GameState {
public:
    MenuState(Game* game, const std::vector<std::string>& options);
    ~MenuState() override;

    // Lifecycle methods - same interface as original
    void enter() override {};
    void exit() override {};

    // Core state functions - same interface as original
    void handle_input(InputManager& inputManager, PlayerData* playerData) override;
    void update(float delta_time, PlayerData* playerData) override;
    void render(PCDisplay& display) override;
    StateType getType() const override;

private:
    // Enhanced UI Components
    std::unique_ptr<DigiviceScreen> screen_;
    std::shared_ptr<MenuList> menuList_;
    
    // Original data members for compatibility
    std::vector<std::string> menuOptions_;
    size_t currentSelection_;
    SDL_Texture* backgroundTexture_;
    SDL_Texture* cursorTexture_;

    // Enhanced functionality
    void onMenuItemSelected(int selectedIndex, const std::string& selectedText);
    void handleMenuSelection(const std::string& selectedItem);
    void setupMenuLayout();
    
    // Helper methods for specific menu actions
    void handleMapSelection();
    void handleDigimonSelection();
    void handleDebugSelection();
    void handleTestDigimonSelection();
    void handleEnemyDigimonSelection();
    void handlePlayerDigimonSelection();
    void handleSettingsSelection();
    void handleSaveSelection();
    void handleExitSelection();
    void handleBackSelection();
};