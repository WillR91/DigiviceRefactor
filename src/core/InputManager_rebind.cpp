// Implementation of key rebinding functionality

#include "core/inputmanager.h"
#include "utils/configmanager.h"
#include <SDL_log.h>
#include <string>
#include <algorithm>

// Rebind a specific action to a new key
bool InputManager::rebindAction(GameAction action, SDL_Scancode newKey) {
    // Check if the action is valid
    if (action >= GameAction::_ACTION_COUNT) {
        SDL_LogError(SDL_LOG_CATEGORY_INPUT, "Cannot rebind action: Invalid GameAction %d", static_cast<int>(action));
        return false;
    }
    
    // Check if the key is already mapped to another action
    for (const auto& pair : keyToActionMap_) {
        if (pair.first == newKey && pair.second != action) {
            SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, 
                "Key %s is already mapped to action %d. Will be overridden.", 
                SDL_GetScancodeName(newKey), 
                static_cast<int>(pair.second));
            // Continue anyway, we'll override it
        }
    }
    
    // Remove any existing mappings for this action
    // This is a bit inefficient but keeps the code simple
    for (auto it = keyToActionMap_.begin(); it != keyToActionMap_.end();) {
        if (it->second == action) {
            it = keyToActionMap_.erase(it);
        } else {
            ++it;
        }
    }
    
    // Add the new mapping
    keyToActionMap_[newKey] = action;
    
    SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, 
        "Rebound action %d to key %s", 
        static_cast<int>(action), 
        SDL_GetScancodeName(newKey));
    
    // Save the changes to config
    return saveKeyBindingsToConfig();
}
