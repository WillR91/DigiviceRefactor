// File: src/core/InputManager.cpp

#include "core/InputManager.h"
#include <SDL_log.h> // For logging

InputManager::InputManager() {
    initializeDefaultKeyMap();
    // Initialize all action states to false
    for (int i = 0; i < static_cast<int>(GameAction::_ACTION_COUNT); ++i) {
        GameAction action = static_cast<GameAction>(i);
        currentActionState_[action] = false;
        previousActionState_[action] = false;
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "InputManager Initialized.");
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


// --- Key Mapping Initialization ---
// (Remains unchanged)
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
    SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Initialized InputManager key map with %zu mappings.", keyToActionMap_.size());
}