#pragma once

#include <string>
#include <memory> // For std::unique_ptr
#include <SDL.h>
#include "platform/pc/pc_display.h" // Include your display wrapper

// Forward declaration
class GameState;

class Game {
public:
    Game();
    ~Game(); // Destructor will handle cleanup

    // Initialize SDL, window, renderer, etc.
    bool init(const std::string& title, int width, int height);

    // Contains the main game loop
    void run();

    // Change the active game state
    void change_state(std::unique_ptr<GameState> new_state);

    // Request the game loop to stop
    void quit_game();

    // Provide access to the display wrapper
    PCDisplay* get_display();

private:
    // Perform cleanup (called by destructor)
    void close();

    PCDisplay display; // Your display wrapper instance
    bool is_running = false;
    std::unique_ptr<GameState> current_state;
    Uint32 last_frame_time = 0;
};