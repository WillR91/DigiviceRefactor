// File: include/Core/Game.h (Assuming it moved to Core/)
#pragma once

#include <string>
#include <memory> // For std::unique_ptr
#include <SDL.h>

// --- CORRECTED INCLUDE PATH ---
#include <Platform/PC/pc_display.h>
// -----------------------------

// Forward declaration
class GameState; // Needs definition from <States/GameState.h> if used by value/reference, pointer/unique_ptr is okay

class Game {
public:
    Game();
    ~Game(); // Destructor will handle cleanup

    // Initialize SDL, window, renderer, etc.
    bool init(const std::string& title, int width, int height);

    // Contains the main game loop
    void run();

    // Change the active game state
    // Note: Requires full definition of GameState if unique_ptr is constructed here.
    // Might need #include <States/GameState.h> in the corresponding Game.cpp file.
    void change_state(std::unique_ptr<GameState> new_state);

    // Request the game loop to stop
    void quit_game();

    // Provide access to the display wrapper
    // Needs full definition of PCDisplay from <Platform/PC/pc_display.h> (included above)
    PCDisplay* get_display();

private:
    // Perform cleanup (called by destructor)
    void close();

    // Needs full definition of PCDisplay from <Platform/PC/pc_display.h> (included above)
    PCDisplay display; // Your display wrapper instance

    bool is_running = false;

    // Needs full definition of GameState if used directly, forward declaration okay for unique_ptr member
    std::unique_ptr<GameState> current_state;

    Uint32 last_frame_time = 0;
};