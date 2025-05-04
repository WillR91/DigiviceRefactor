// File: include/core/GameAction.h
#pragma once

enum class GameAction {
    // Navigation (Add more directions if needed)
    NAV_UP,
    NAV_DOWN,
    NAV_LEFT,
    NAV_RIGHT,

    // Confirmation / Interaction
    CONFIRM, // e.g., Select menu item, interact with object
    CANCEL,  // e.g., Back out of menu, cancel action

    // Adventure State Specific
    STEP, // Simulate taking a step

    // Digimon Selection (Can keep specific for now, or make generic ACTION_1..8)
    SELECT_DIGI_1,
    SELECT_DIGI_2,
    SELECT_DIGI_3,
    SELECT_DIGI_4,
    SELECT_DIGI_5,
    SELECT_DIGI_6,
    SELECT_DIGI_7,
    SELECT_DIGI_8,

    // System Actions (Maybe handled differently, but include for now)
    MENU_TOGGLE, // Might replace separate CONFIRM/CANCEL in some contexts
    QUIT_GAME,   // Usually handled by SDL_QUIT event

    // Add more actions as needed (ATTACK, USE_ITEM, etc.)

    _ACTION_COUNT // Helper to get the number of actions
};

// Optional: Define a mapping from Scancode to Action if useful globally,
// but usually this mapping happens inside InputManager.