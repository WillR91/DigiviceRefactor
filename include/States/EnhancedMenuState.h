#pragma once

#include "states/GameState.h"
#include "ui/DigiviceScreen.h"
#include "ui/MenuList.h"
#include <memory>
#include <vector>
#include <string>

// Forward declarations
class Game;
class PCDisplay;
class InputManager;
class PlayerData;

/**
 * @brief Enhanced MenuState using the new UI framework
 * 
 * This is a modernized version of the original MenuState that uses the
 * DigiviceScreen and MenuList components for a cleaner, more maintainable
 * implementation while preserving the same functionality.
 */
class EnhancedMenuState : public GameState {
public:
    /**
     * @brief Construct a new Enhanced Menu State
     * @param game Pointer to the main game instance
     * @param options Vector of menu option strings
     */
    EnhancedMenuState(Game* game, const std::vector<std::string>& options);
    
    virtual ~EnhancedMenuState() = default;

    // GameState interface implementation
    void handle_input(InputManager& inputManager, PlayerData* playerData) override;
    void update(float deltaTime, PlayerData* playerData) override;
    void render(PCDisplay& display) override;
    StateType getType() const override;

private:
    // UI Components
    std::unique_ptr<DigiviceScreen> screen_;
    std::shared_ptr<MenuList> menuList_;
    
    // Menu callback handler
    void onMenuItemSelected(int selectedIndex, const std::string& selectedText);
    
    // Helper methods for menu actions
    void handleMapSelection();
    void handleDigimonSelection();
    void handleDebugSelection();
    void handleTestDigimonSelection();
    void handleEnemyDigimonSelection();
    void handlePlayerDigimonSelection();
    void handleSettingsSelection();
    void handleBackSelection();
};
