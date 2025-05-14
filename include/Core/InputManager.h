// File: include/core/InputManager.h
#pragma once

#include "core/GameAction.h" // Include the action definitions
#include <SDL.h>            // For SDL_Event, SDL_Scancode
#include <vector>
#include <map>

class InputManager {
public:
    InputManager();
    ~InputManager();

    // Called once per frame BEFORE any state's handle_input is called
    void prepareNewFrame();

    // Called by the main game loop to process SDL events
    void processEvent(const SDL_Event& event);

    // Called once per frame AFTER processing events, usually before state updates
    // (Optional but good for handling held keys if not using SDL_GetKeyboardState)
    // void update();

    // --- Query Methods (Called by game states) ---
    
    // Save current key bindings to config file
    bool saveKeyBindingsToConfig();
    
    // Rebind a specific action to a new key
    bool rebindAction(GameAction action, SDL_Scancode newKey);

    // Check if an action was just triggered in the current frame
    bool isActionJustPressed(GameAction action) const;

    // Check if an action is currently being held down
    bool isActionHeld(GameAction action) const;

    // Check if an action was just released in the current frame
    bool isActionJustReleased(GameAction action) const;

private:
    // --- Internal State Tracking ---

    // Maps an abstract GameAction to its current state
    std::map<GameAction, bool> currentActionState_;
    // Maps an abstract GameAction to its state in the *previous* frame
    std::map<GameAction, bool> previousActionState_;    // Maps SDL Scancodes to our GameActions
    // Customize this mapping!
    std::map<SDL_Scancode, GameAction> keyToActionMap_;    // Helper methods to initialize key mappings
    void initializeDefaultKeyMap();
    void loadKeysFromConfig();
    SDL_Scancode stringToScancode(const std::string& keyName);
    void handleToggleScreenSizeAction(); // Added to handle screen size toggle
};