#pragma once

#include "states/gamestate.h"
#include "core/gameaction.h"
#include <SDL.h>
#include <vector>
#include <string>
#include <map>

// Forward declarations
class Game;
class InputManager;
class PCDisplay;
class PlayerData;

// Represents a key binding that can be changed
struct KeyBindOption {
    GameAction action;
    std::string displayName;
    std::string configPath;
};

class SettingsState : public GameState {
public:
    SettingsState(Game* game);
    ~SettingsState() override;

    // Core state functions
    void handle_input(InputManager& inputManager, PlayerData* playerData) override;
    void update(float delta_time, PlayerData* playerData) override;
    void render(PCDisplay& display) override;
    StateType getType() const override;

private:
    // UI constants
    const int TITLE_Y = 30;
    const int MENU_START_X = 50;
    const int MENU_START_Y = 70;
    const int MENU_ITEM_HEIGHT = 30;
    
    // Textures
    SDL_Texture* backgroundTexture_ = nullptr;    // Menu settings
    std::vector<std::string> menuOptions_;
    size_t currentSelection_ = 0;
    
    // Key rebinding mode
    bool isRebindingKey_ = false;
    GameAction currentRebindAction_ = GameAction::_ACTION_COUNT;
    
    // Text scale adjustment
    bool isAdjustingTextScale_ = false;
    float textScaleValue_ = 1.0f;
    
    // Key binding options
    std::vector<KeyBindOption> keyBindOptions_;
    
    // Helper methods
    void initializeKeyBindOptions();
    std::string getKeyNameForAction(GameAction action, InputManager& inputManager);
    void drawText(PCDisplay& display, const std::string& text, int x, int y);
    void applySettings();
    void cancelSettings();
};
