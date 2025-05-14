#include "core/inputmanager.h"
#include "utils/configmanager.h"
#include <SDL_log.h>
#include <string>
#include <algorithm>

// Methods for runtime key rebinding and saving

bool InputManager::saveKeyBindingsToConfig() {
    if (!ConfigManager::isInitialized()) {
        SDL_LogError(SDL_LOG_CATEGORY_INPUT, "Cannot save key bindings: ConfigManager not initialized");
        return false;
    }
    
    // Create a reverse map for finding keys by action
    std::map<GameAction, SDL_Scancode> actionToKeyMap;
    for (const auto& pair : keyToActionMap_) {
        // Skip duplicate mappings (like W and UP for NAV_UP)
        if (actionToKeyMap.find(pair.second) == actionToKeyMap.end()) {
            actionToKeyMap[pair.second] = pair.first;
        }
    }
    
    // Save navigation keys
    if (actionToKeyMap.find(GameAction::NAV_UP) != actionToKeyMap.end()) {
        const char* keyName = SDL_GetScancodeName(actionToKeyMap[GameAction::NAV_UP]);
        ConfigManager::setValue("input.moveKey.up", keyName);
    }
    
    if (actionToKeyMap.find(GameAction::NAV_DOWN) != actionToKeyMap.end()) {
        const char* keyName = SDL_GetScancodeName(actionToKeyMap[GameAction::NAV_DOWN]);
        ConfigManager::setValue("input.moveKey.down", keyName);
    }
    
    if (actionToKeyMap.find(GameAction::NAV_LEFT) != actionToKeyMap.end()) {
        const char* keyName = SDL_GetScancodeName(actionToKeyMap[GameAction::NAV_LEFT]);
        ConfigManager::setValue("input.moveKey.left", keyName);
    }
    
    if (actionToKeyMap.find(GameAction::NAV_RIGHT) != actionToKeyMap.end()) {
        const char* keyName = SDL_GetScancodeName(actionToKeyMap[GameAction::NAV_RIGHT]);
        ConfigManager::setValue("input.moveKey.right", keyName);
    }
    
    // Save action keys
    if (actionToKeyMap.find(GameAction::CONFIRM) != actionToKeyMap.end()) {
        const char* keyName = SDL_GetScancodeName(actionToKeyMap[GameAction::CONFIRM]);
        ConfigManager::setValue("input.actionKey", keyName);
    }
    
    if (actionToKeyMap.find(GameAction::CANCEL) != actionToKeyMap.end()) {
        const char* keyName = SDL_GetScancodeName(actionToKeyMap[GameAction::CANCEL]);
        ConfigManager::setValue("input.backKey", keyName);
    }
    
    if (actionToKeyMap.find(GameAction::MENU_TOGGLE) != actionToKeyMap.end()) {
        const char* keyName = SDL_GetScancodeName(actionToKeyMap[GameAction::MENU_TOGGLE]);
        ConfigManager::setValue("input.menuKey", keyName);
    }
    
    if (actionToKeyMap.find(GameAction::TOGGLE_SCREEN_SIZE) != actionToKeyMap.end()) {
        const char* keyName = SDL_GetScancodeName(actionToKeyMap[GameAction::TOGGLE_SCREEN_SIZE]);
        ConfigManager::setValue("input.toggleScreenKey", keyName);
    }
    
    // Save changes to the config file
    if (ConfigManager::saveChanges()) {
        SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Key bindings saved to configuration file");
        return true;
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_INPUT, "Failed to save key bindings to configuration file");
        return false;
    }
}

bool InputManager::rebindAction(GameAction action, SDL_Scancode newKey) {
    if (action == GameAction::_ACTION_COUNT) {
        SDL_LogError(SDL_LOG_CATEGORY_INPUT, "Cannot rebind invalid action enum value");
        return false;
    }
    
    // First, remove any existing bindings for this key
    auto existingBinding = keyToActionMap_.find(newKey);
    if (existingBinding != keyToActionMap_.end()) {
        SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Removing existing binding: Key %s (%d) was bound to action %d",
                   SDL_GetScancodeName(newKey), newKey, static_cast<int>(existingBinding->second));
        keyToActionMap_.erase(existingBinding);
    }
    
    // Add the new binding
    keyToActionMap_[newKey] = action;
    SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Added binding: Key %s (%d) now bound to action %d",
               SDL_GetScancodeName(newKey), newKey, static_cast<int>(action));
    
    return true;
}
