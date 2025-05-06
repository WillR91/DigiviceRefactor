// File: include/states/GameState.h
#pragma once
#include <memory> // Standard Library - OK

// Forward Declarations
class Game;         // Forward declaration - OK (defined in core/Game.h)
class InputManager; // <<<--- ADDED Forward declaration (already present)
class PlayerData;   // <<<--- ADDED Forward declaration (already present)
class PCDisplay;    // <<<--- ADDED Forward declaration (for render) (already present)

class GameState {
public:
    // Constructor: Store the game pointer
    GameState(Game* game) : game_ptr(game) {} // <<<--- ADDED Constructor (Assumed needed) (already present)
    virtual ~GameState() = default; // Virtual destructor - OK

    // <<< --- ADDED LIFECYCLE METHODS --- >>>
    // Called when the state becomes the active top state
    virtual void enter() {}; // Default empty implementation
    // Called when the state is about to be removed or covered
    virtual void exit() {};  // Default empty implementation
    // <<< ----------------------------- >>>


    // <<<--- MODIFIED function signatures (already present) --->>>
    virtual void handle_input(InputManager& inputManager, PlayerData* playerData) = 0;
    virtual void update(float delta_time, PlayerData* playerData) = 0;
    virtual void render(PCDisplay& display) = 0; // Assuming render needs the display object

protected:
    Game* game_ptr = nullptr; // Non-owning pointer to access Game resources - OK
};