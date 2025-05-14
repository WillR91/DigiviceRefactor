// File: src/core/InputManager.cpp

#include "core/inputmanager.h"
#include "utils/configmanager.h" // Include ConfigManager
#include <SDL_log.h> // For logging
#include <string>
#include <algorithm> // For std::transform
#include <stdexcept> // For std::exception

InputManager::InputManager() {
    loadKeysFromConfig(); // Try to load from config first
    
    // Initialize all action states to false
    for (int i = 0; i < static_cast<int>(GameAction::_ACTION_COUNT); ++i) {
        GameAction action = static_cast<GameAction>(i);
        currentActionState_[action] = false;
        previousActionState_[action] = false;
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "InputManager Initialized with %zu key mappings.", keyToActionMap_.size());
}

InputManager::~InputManager() {
    SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "InputManager Shutdown.");
}

// Call this at the VERY START of each frame's input processing phase
void InputManager::prepareNewFrame() {
    // Copy the current state to the previous state map
    previousActionState_ = currentActionState_;
}

// Process a single SDL event
// <<< INTEGRATED LOGGING >>>
void InputManager::processEvent(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        SDL_Scancode scancode = event.key.keysym.scancode;
        bool isKeyDown = (event.type == SDL_KEYDOWN);

        // Log raw key event
        SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "InputManager::processEvent - Key Event: Scancode=%d (%s), State=%s",
                     (int)scancode, SDL_GetScancodeName(scancode), (isKeyDown ? "DOWN" : "UP"));

        // Find the corresponding GameAction in our map
        auto it = keyToActionMap_.find(scancode);
        if (it != keyToActionMap_.end()) {
            GameAction action = it->second;

            // Update the current state for this action
            currentActionState_[action] = isKeyDown;

            // Log mapped action state change
            SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "InputManager::processEvent - Mapped Scancode %d to Action %d. Set State: %d",
                         (int)scancode, static_cast<int>(action), isKeyDown);

        } else {
             // Log unmapped key presses
             if (isKeyDown) { // Only log on press to reduce spam
                SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "InputManager::processEvent - Scancode %d (%s) is not mapped to any GameAction.",
                             (int)scancode, SDL_GetScancodeName(scancode));
             }
        }
    }
    // Handle other event types if needed
}
// <<< END INTEGRATED LOGGING >>>


// --- Query Methods ---
// (Remain unchanged)
bool InputManager::isActionJustPressed(GameAction action) const {
    auto currentIt = currentActionState_.find(action);
    auto previousIt = previousActionState_.find(action);
    bool current = (currentIt != currentActionState_.end()) ? currentIt->second : false;
    bool previous = (previousIt != previousActionState_.end()) ? previousIt->second : false;
    return current && !previous;
}

bool InputManager::isActionHeld(GameAction action) const {
    auto it = currentActionState_.find(action);
    return (it != currentActionState_.end()) ? it->second : false;
}

bool InputManager::isActionJustReleased(GameAction action) const {
    auto currentIt = currentActionState_.find(action);
    auto previousIt = previousActionState_.find(action);
    bool current = (currentIt != currentActionState_.end()) ? currentIt->second : false;
    bool previous = (previousIt != previousActionState_.end()) ? previousIt->second : false;
    return !current && previous;
}


// --- Key Mapping Methods ---

// Convert a string key name to an SDL_Scancode
SDL_Scancode InputManager::stringToScancode(const std::string& keyName) {
    // First try direct SDL scancode name lookup
    std::string upperKeyName = keyName;
    std::transform(upperKeyName.begin(), upperKeyName.end(), upperKeyName.begin(), ::toupper);
    
    // Check for special key names
    if (upperKeyName == "UP") return SDL_SCANCODE_UP;
    if (upperKeyName == "DOWN") return SDL_SCANCODE_DOWN;
    if (upperKeyName == "LEFT") return SDL_SCANCODE_LEFT;
    if (upperKeyName == "RIGHT") return SDL_SCANCODE_RIGHT;
    if (upperKeyName == "ENTER" || upperKeyName == "RETURN") return SDL_SCANCODE_RETURN;
    if (upperKeyName == "SPACE") return SDL_SCANCODE_SPACE;
    if (upperKeyName == "ESC" || upperKeyName == "ESCAPE") return SDL_SCANCODE_ESCAPE;
    if (upperKeyName == "BACKSPACE") return SDL_SCANCODE_BACKSPACE;
    if (upperKeyName == "TAB") return SDL_SCANCODE_TAB;
    if (upperKeyName == "SHIFT") return SDL_SCANCODE_LSHIFT;
    if (upperKeyName == "CTRL" || upperKeyName == "CONTROL") return SDL_SCANCODE_LCTRL;
    if (upperKeyName == "ALT") return SDL_SCANCODE_LALT;
    
    // For single character keys
    if (upperKeyName.length() == 1) {
        char c = upperKeyName[0];
        if (c >= 'A' && c <= 'Z') {
            return static_cast<SDL_Scancode>(SDL_SCANCODE_A + (c - 'A'));
        }
        if (c >= '0' && c <= '9') {
            return static_cast<SDL_Scancode>(SDL_SCANCODE_1 + (c - '1'));
        }
    }
    
    // Handle function keys
    if (upperKeyName.length() >= 2 && upperKeyName[0] == 'F') {
        std::string numPart = upperKeyName.substr(1);
        try {
            int fNum = std::stoi(numPart);
            if (fNum >= 1 && fNum <= 12) {
                return static_cast<SDL_Scancode>(SDL_SCANCODE_F1 + (fNum - 1));
            }
        } catch (const std::exception&) {
            // Failed to parse F-key number, continue to fallback
        }
    }
    
    // If nothing matched, try SDL's built-in name lookup
    SDL_Scancode scancode = SDL_GetScancodeFromName(keyName.c_str());
    if (scancode != SDL_SCANCODE_UNKNOWN) {
        return scancode;
    }
    
    // Fallback if all else fails
    SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "Could not convert key name '%s' to scancode. Using default.", keyName.c_str());
    return SDL_SCANCODE_UNKNOWN;
}

// Load key bindings from configuration
void InputManager::loadKeysFromConfig() {
    SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Loading key bindings from configuration...");
    keyToActionMap_.clear();
    
    // Check if ConfigManager is initialized
    if (!ConfigManager::isInitialized()) {
        SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "ConfigManager not initialized, falling back to default key map.");
        initializeDefaultKeyMap();
        return;
    }
    
    bool keysLoaded = false;
    
    // Load movement keys
    std::string upKey = ConfigManager::getValue<std::string>("input.moveKey.up", "Up");
    std::string downKey = ConfigManager::getValue<std::string>("input.moveKey.down", "Down");
    std::string leftKey = ConfigManager::getValue<std::string>("input.moveKey.left", "Left");
    std::string rightKey = ConfigManager::getValue<std::string>("input.moveKey.right", "Right");
    
    // Map movement keys
    SDL_Scancode upScancode = stringToScancode(upKey);
    SDL_Scancode downScancode = stringToScancode(downKey);
    SDL_Scancode leftScancode = stringToScancode(leftKey);
    SDL_Scancode rightScancode = stringToScancode(rightKey);
    
    if (upScancode != SDL_SCANCODE_UNKNOWN) {
        keyToActionMap_[upScancode] = GameAction::NAV_UP;
        keyToActionMap_[SDL_SCANCODE_W] = GameAction::NAV_UP; // Also keep W as alternative
        keysLoaded = true;
    }
    
    if (downScancode != SDL_SCANCODE_UNKNOWN) {
        keyToActionMap_[downScancode] = GameAction::NAV_DOWN;
        keyToActionMap_[SDL_SCANCODE_S] = GameAction::NAV_DOWN; // Also keep S as alternative
        keysLoaded = true;
    }
    
    if (leftScancode != SDL_SCANCODE_UNKNOWN) {
        keyToActionMap_[leftScancode] = GameAction::NAV_LEFT;
        keyToActionMap_[SDL_SCANCODE_A] = GameAction::NAV_LEFT; // Also keep A as alternative
        keysLoaded = true;
    }
    
    if (rightScancode != SDL_SCANCODE_UNKNOWN) {
        keyToActionMap_[rightScancode] = GameAction::NAV_RIGHT;
        keyToActionMap_[SDL_SCANCODE_D] = GameAction::NAV_RIGHT; // Also keep D as alternative
        keysLoaded = true;
    }
    
    // Load action keys
    std::string actionKey = ConfigManager::getValue<std::string>("input.actionKey", "Z");
    std::string backKey = ConfigManager::getValue<std::string>("input.backKey", "X");
    std::string menuKey = ConfigManager::getValue<std::string>("input.menuKey", "C");
    std::string toggleScreenKey = ConfigManager::getValue<std::string>("input.toggleScreenKey", "F");
    
    // Map action keys
    SDL_Scancode actionScancode = stringToScancode(actionKey);
    SDL_Scancode backScancode = stringToScancode(backKey);
    SDL_Scancode menuScancode = stringToScancode(menuKey);
    SDL_Scancode toggleScreenScancode = stringToScancode(toggleScreenKey);
    
    if (actionScancode != SDL_SCANCODE_UNKNOWN) {
        keyToActionMap_[actionScancode] = GameAction::CONFIRM;
        keysLoaded = true;
    }
    
    if (backScancode != SDL_SCANCODE_UNKNOWN) {
        keyToActionMap_[backScancode] = GameAction::CANCEL;
        keysLoaded = true;
    }
    
    if (menuScancode != SDL_SCANCODE_UNKNOWN) {
        keyToActionMap_[menuScancode] = GameAction::MENU_TOGGLE;
        keysLoaded = true;
    }
    
    if (toggleScreenScancode != SDL_SCANCODE_UNKNOWN) {
        keyToActionMap_[toggleScreenScancode] = GameAction::TOGGLE_SCREEN_SIZE;
        keysLoaded = true;
    }
    
    // Always add these defaults regardless of config
    keyToActionMap_[SDL_SCANCODE_RETURN] = GameAction::CONFIRM;
    keyToActionMap_[SDL_SCANCODE_KP_ENTER] = GameAction::CONFIRM;
    keyToActionMap_[SDL_SCANCODE_SPACE] = GameAction::STEP; // Also mapped to CONFIRM
    keyToActionMap_[SDL_SCANCODE_ESCAPE] = GameAction::CANCEL;
    
    // Map digimon selection keys (always use number keys for these)
    keyToActionMap_[SDL_SCANCODE_1] = GameAction::SELECT_DIGI_1;
    keyToActionMap_[SDL_SCANCODE_2] = GameAction::SELECT_DIGI_2;
    keyToActionMap_[SDL_SCANCODE_3] = GameAction::SELECT_DIGI_3;
    keyToActionMap_[SDL_SCANCODE_4] = GameAction::SELECT_DIGI_4;
    keyToActionMap_[SDL_SCANCODE_5] = GameAction::SELECT_DIGI_5;
    keyToActionMap_[SDL_SCANCODE_6] = GameAction::SELECT_DIGI_6;
    keyToActionMap_[SDL_SCANCODE_7] = GameAction::SELECT_DIGI_7;
    keyToActionMap_[SDL_SCANCODE_8] = GameAction::SELECT_DIGI_8;
    
    if (!keysLoaded) {
        SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "Failed to load keys from config, using defaults.");
        initializeDefaultKeyMap();
    } else {
        SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Successfully loaded key bindings from config.");
    }
}

// Default key mapping as fallback
void InputManager::initializeDefaultKeyMap() {
    keyToActionMap_.clear();
    keyToActionMap_[SDL_SCANCODE_UP]    = GameAction::NAV_UP;
    keyToActionMap_[SDL_SCANCODE_W]     = GameAction::NAV_UP;
    keyToActionMap_[SDL_SCANCODE_DOWN]  = GameAction::NAV_DOWN;
    keyToActionMap_[SDL_SCANCODE_S]     = GameAction::NAV_DOWN;
    keyToActionMap_[SDL_SCANCODE_LEFT]  = GameAction::NAV_LEFT;
    keyToActionMap_[SDL_SCANCODE_A]     = GameAction::NAV_LEFT;
    keyToActionMap_[SDL_SCANCODE_RIGHT] = GameAction::NAV_RIGHT;
    keyToActionMap_[SDL_SCANCODE_D]     = GameAction::NAV_RIGHT;
    keyToActionMap_[SDL_SCANCODE_RETURN]    = GameAction::CONFIRM;
    keyToActionMap_[SDL_SCANCODE_KP_ENTER] = GameAction::CONFIRM;
    keyToActionMap_[SDL_SCANCODE_Z]         = GameAction::CONFIRM;
    keyToActionMap_[SDL_SCANCODE_SPACE]     = GameAction::CONFIRM; // Also mapped to STEP
    keyToActionMap_[SDL_SCANCODE_ESCAPE]    = GameAction::CANCEL;
    keyToActionMap_[SDL_SCANCODE_BACKSPACE] = GameAction::CANCEL;
    keyToActionMap_[SDL_SCANCODE_X]         = GameAction::CANCEL;
    keyToActionMap_[SDL_SCANCODE_SPACE]     = GameAction::STEP; // Also mapped to CONFIRM
    keyToActionMap_[SDL_SCANCODE_1] = GameAction::SELECT_DIGI_1;
    keyToActionMap_[SDL_SCANCODE_2] = GameAction::SELECT_DIGI_2;
    keyToActionMap_[SDL_SCANCODE_3] = GameAction::SELECT_DIGI_3;
    keyToActionMap_[SDL_SCANCODE_4] = GameAction::SELECT_DIGI_4;
    keyToActionMap_[SDL_SCANCODE_5] = GameAction::SELECT_DIGI_5;
    keyToActionMap_[SDL_SCANCODE_6] = GameAction::SELECT_DIGI_6;
    keyToActionMap_[SDL_SCANCODE_7] = GameAction::SELECT_DIGI_7;
    keyToActionMap_[SDL_SCANCODE_8] = GameAction::SELECT_DIGI_8;
    keyToActionMap_[SDL_SCANCODE_F] = GameAction::TOGGLE_SCREEN_SIZE; // F for Fullscreen toggle
    keyToActionMap_[SDL_SCANCODE_C] = GameAction::MENU_TOGGLE;        // C for menu
    SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Initialized InputManager default key map with %zu mappings.", keyToActionMap_.size());
}

// Placeholder for now, actual screen toggle logic will be in Game or PCDisplay
void InputManager::handleToggleScreenSizeAction() {
    if (isActionJustPressed(GameAction::TOGGLE_SCREEN_SIZE)) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Toggle screen size action detected by InputManager.");
        // The actual resizing will be handled by the Game class listening for this action.
    }
}