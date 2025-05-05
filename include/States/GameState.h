// File: include/states/GameState.h
#pragma once
#include <memory> // Standard Library - OK

// Forward Declarations
class Game;         // Forward declaration - OK (defined in core/Game.h)
class InputManager; // <<<--- ADDED Forward declaration
class PlayerData;   // <<<--- ADDED Forward declaration
class PCDisplay;    // <<<--- ADDED Forward declaration (for render)

class GameState {
public:
    // Constructor: Store the game pointer
    GameState(Game* game) : game_ptr(game) {} // <<<--- ADDED Constructor (Assumed needed)
    virtual ~GameState() = default; // Virtual destructor - OK

    // <<<--- MODIFIED function signatures --->>>
    virtual void handle_input(InputManager& inputManager, PlayerData* playerData) = 0;
    virtual void update(float delta_time, PlayerData* playerData) = 0;
    virtual void render(PCDisplay& display) = 0; // Assuming render needs the display object

protected:
    Game* game_ptr = nullptr; // Non-owning pointer to access Game resources - OK
};