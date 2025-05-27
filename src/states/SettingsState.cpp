#include "states/SettingsState.h"
#include "core/game.h"
#include "core/assetmanager.h"
#include "core/inputmanager.h"
#include "utils/configmanager.h"
#include "platform/pc/pc_display.h"
#include "ui/TextRenderer.h"
#include <SDL_log.h>

SettingsState::SettingsState(Game* game) : GameState(game) {
    // Initialize menu options
    menuOptions_ = {"CONTROLS", "AUDIO", "DISPLAY", "TEXT SCALE", "RETURN TO MENU"};
    
    // Get current text scale from config
    textScaleValue_ = ConfigManager::getValue<float>("ui.textScale", 1.0f);
      // Load background texture
    if (game_ptr && game_ptr->getAssetManager()) {
        backgroundTexture_ = game_ptr->getAssetManager()->requestTexture("menu_bg_blue");
        if (!backgroundTexture_) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "SettingsState: Background texture not found");
        }
    }
    
    // Initialize key binding options
    initializeKeyBindOptions();
    
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SettingsState: Initialized");
}

SettingsState::~SettingsState() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SettingsState: Destroyed");
}

void SettingsState::initializeKeyBindOptions() {
    keyBindOptions_.clear();
    
    // Add key binding options
    keyBindOptions_.push_back({GameAction::NAV_UP, "Move Up", "input.moveKey.up"});
    keyBindOptions_.push_back({GameAction::NAV_DOWN, "Move Down", "input.moveKey.down"});
    keyBindOptions_.push_back({GameAction::NAV_LEFT, "Move Left", "input.moveKey.left"});
    keyBindOptions_.push_back({GameAction::NAV_RIGHT, "Move Right", "input.moveKey.right"});
    keyBindOptions_.push_back({GameAction::CONFIRM, "Confirm/Action", "input.actionKey"});
    keyBindOptions_.push_back({GameAction::CANCEL, "Cancel/Back", "input.backKey"});
    keyBindOptions_.push_back({GameAction::MENU_TOGGLE, "Menu", "input.menuKey"});
    keyBindOptions_.push_back({GameAction::TOGGLE_SCREEN_SIZE, "Toggle Screen Size", "input.toggleScreenKey"});
}

void SettingsState::handle_input(InputManager& inputManager, PlayerData* playerData) {
    if (!game_ptr) return;    // If in rebind mode, wait for any key press
    if (isRebindingKey_) {
        // Check for key presses
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                // Get the scancode and update the binding
                SDL_Scancode scancode = event.key.keysym.scancode;
                
                // Skip these keys as they're system keys
                if (scancode == SDL_SCANCODE_ESCAPE || 
                    scancode == SDL_SCANCODE_F1 ||
                    scancode == SDL_SCANCODE_F12) {
                    continue;
                }
                
                // Update the binding
                if (inputManager.rebindAction(currentRebindAction_, scancode)) {
                    SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "SettingsState: Rebound key %s to %s", 
                        SDL_GetScancodeName(scancode),
                        keyBindOptions_[currentSelection_].displayName.c_str());
                    
                    // Update the config value
                    ConfigManager::setValue(keyBindOptions_[currentSelection_].configPath, 
                                          SDL_GetScancodeName(scancode));
                    
                    // Save the changes
                    if (!inputManager.saveKeyBindingsToConfig()) {
                        SDL_LogError(SDL_LOG_CATEGORY_INPUT, "Failed to save key bindings to config");
                    }
                }
                
                // Exit rebind mode
                isRebindingKey_ = false;
                break;
            }
        }
        return;
    }
    
    // If adjusting text scale
    if (isAdjustingTextScale_) {
        if (inputManager.isActionJustPressed(GameAction::NAV_LEFT)) {
            // Decrease text scale (minimum 0.5)
            textScaleValue_ = std::max(0.5f, textScaleValue_ - 0.1f);
            
            // Update the config
            ConfigManager::setValue("ui.textScale", textScaleValue_);
            ConfigManager::saveChanges();
            
            // Update the text renderer
            if (game_ptr) {
                game_ptr->updateFromConfig();
            }
            
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Text scale decreased to %.1f", textScaleValue_);
        }
        else if (inputManager.isActionJustPressed(GameAction::NAV_RIGHT)) {
            // Increase text scale (maximum 2.0)
            textScaleValue_ = std::min(2.0f, textScaleValue_ + 0.1f);
            
            // Update the config
            ConfigManager::setValue("ui.textScale", textScaleValue_);
            ConfigManager::saveChanges();
            
            // Update the text renderer
            if (game_ptr) {
                game_ptr->updateFromConfig();
            }
            
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Text scale increased to %.1f", textScaleValue_);
        }
        else if (inputManager.isActionJustPressed(GameAction::CONFIRM) || 
                 inputManager.isActionJustPressed(GameAction::CANCEL)) {
            // Exit text scale adjustment mode
            isAdjustingTextScale_ = false;
        }
        
        return;
    }
    
    // Normal menu navigation
    if (inputManager.isActionJustPressed(GameAction::NAV_UP)) {
        if (currentSelection_ == 0) {
            currentSelection_ = menuOptions_.size() - 1;
        } else {
            currentSelection_--;
        }
    } else if (inputManager.isActionJustPressed(GameAction::NAV_DOWN)) {
        currentSelection_ = (currentSelection_ + 1) % menuOptions_.size();
    }
    
    // Handle selection
    if (inputManager.isActionJustPressed(GameAction::CONFIRM)) {
        std::string selected = menuOptions_[currentSelection_];
        
        if (selected == "CONTROLS") {
            // Replace menu options with control bindings
            menuOptions_.clear();
            for (const auto& option : keyBindOptions_) {
                menuOptions_.push_back(option.displayName);
            }
            menuOptions_.push_back("BACK");
            currentSelection_ = 0;
        }        else if (selected == "AUDIO") {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Audio settings not implemented yet");
        } 
        else if (selected == "DISPLAY") {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Display settings not implemented yet");
        }
        else if (selected == "TEXT SCALE") {
            // Enter text scale adjustment mode
            isAdjustingTextScale_ = true;
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Adjusting text scale (current: %.1f)", textScaleValue_);
        }
        else if (selected == "RETURN TO MENU" || selected == "BACK") {
            // Return to previous state
            game_ptr->requestFadeToState(nullptr, 0.3f, true);
        }
        else {
            // Check if we're in the controls menu and selected a binding
            bool isKeyBindOption = false;
            for (size_t i = 0; i < keyBindOptions_.size(); i++) {
                if (selected == keyBindOptions_[i].displayName) {
                    isKeyBindOption = true;
                    isRebindingKey_ = true;
                    currentRebindAction_ = keyBindOptions_[i].action;
                    break;
                }
            }
            
            if (!isKeyBindOption) {
                SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "SettingsState: Unknown option %s", selected.c_str());
            }
        }
    } 
    else if (inputManager.isActionJustPressed(GameAction::CANCEL)) {
        // Go back to main menu
        game_ptr->requestFadeToState(nullptr, 0.3f, true);
    }
}

void SettingsState::update(float delta_time, PlayerData* playerData) {
    // No update logic needed
}

void SettingsState::render(PCDisplay& display) {
    if (!game_ptr) return;
    
    SDL_Renderer* renderer = display.getRenderer();
    if (!renderer) return;
    
    // Draw background
    if (backgroundTexture_) {
        SDL_RenderCopy(renderer, backgroundTexture_, NULL, NULL);
    } else {
        // Fallback background
        SDL_SetRenderDrawColor(renderer, 0, 0, 80, 255);
        SDL_RenderClear(renderer);
    }
    
    // Get text renderer
    TextRenderer* textRenderer = game_ptr->getTextRenderer();
    if (!textRenderer) return;
    
    // Draw title
    textRenderer->drawText(renderer, "SETTINGS", MENU_START_X, TITLE_Y, 1.0f);    // Special case for text scale adjustment mode
    if (isAdjustingTextScale_) {
        textRenderer->drawText(renderer, "TEXT SCALE ADJUSTMENT", MENU_START_X, MENU_START_Y, 1.2f);
        
        char valueText[64];
        sprintf(valueText, "Current Scale: %.1f", textScaleValue_);
        textRenderer->drawText(renderer, valueText, MENU_START_X, MENU_START_Y + MENU_ITEM_HEIGHT, 1.0f);
        
        textRenderer->drawText(renderer, "Use LEFT/RIGHT arrows to adjust", MENU_START_X, MENU_START_Y + MENU_ITEM_HEIGHT * 2, 0.8f);
        textRenderer->drawText(renderer, "Press CONFIRM/CANCEL when done", MENU_START_X, MENU_START_Y + MENU_ITEM_HEIGHT * 3, 0.8f);
        
        return;
    }

    // Draw menu options
    for (size_t i = 0; i < menuOptions_.size(); i++) {
        float scale = (i == currentSelection_) ? 1.2f : 0.9f;
        int y = MENU_START_Y + (static_cast<int>(i) * MENU_ITEM_HEIGHT);
        
        std::string displayText = menuOptions_[i];
        
        // If this is the text scale option, show the current value
        if (displayText == "TEXT SCALE") {
            char valueText[64];
            sprintf(valueText, "%.1f", textScaleValue_);
            displayText += ": " + std::string(valueText);
        }
        // If we're showing key bindings and this is a key binding option
        else if (i < keyBindOptions_.size() && displayText == keyBindOptions_[i].displayName) {
            std::string keyName;
            if (isRebindingKey_ && i == currentSelection_) {
                keyName = "Press a key...";
            } else if (game_ptr->getInputManager()) {
                keyName = getKeyNameForAction(keyBindOptions_[i].action, *game_ptr->getInputManager());
            }
            
            if (!keyName.empty()) {
                displayText += ": " + keyName;
            }
        }
        
        textRenderer->drawText(renderer, displayText, MENU_START_X, y, scale);
    }
}

std::string SettingsState::getKeyNameForAction(GameAction action, InputManager& inputManager) {
    // This is a bit of a hack since we don't have direct access to the key mappings
    // Ideally, InputManager would provide a method to get the key for an action
    std::string configPath;
    
    for (const auto& option : keyBindOptions_) {
        if (option.action == action) {
            configPath = option.configPath;
            break;
        }
    }
    
    if (configPath.empty()) return "";
    
    // Get the key name from config
    return ConfigManager::getValue<std::string>(configPath, "Unknown");
}

StateType SettingsState::getType() const {
    return StateType::Settings; // Assuming Settings type exists in the enum
}
