#pragma once // Use include guards
#include <memory> // For potential use with smart pointers if needed

class Game; // Forward declaration to avoid circular includes

class GameState {
public:
    virtual ~GameState() = default; // Virtual destructor is crucial!

    // Called to handle user input for this state
    virtual void handle_input() = 0;

    // Called to update game logic for this state
    // delta_time is the time elapsed since the last frame in seconds
    virtual void update(float delta_time) = 0;

    // Called to render the graphics for this state
    virtual void render() = 0;

protected:
    Game* game_ptr = nullptr; // Non-owning pointer to access Game resources
};