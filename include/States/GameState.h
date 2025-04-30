// File: include/states/GameState.h
#pragma once
#include <memory> // Standard Library - OK

class Game; // Forward declaration - OK (defined in core/Game.h)

class GameState {
public:
    virtual ~GameState() = default;

    virtual void handle_input() = 0;
    virtual void update(float delta_time) = 0;
    virtual void render() = 0;

protected:
    Game* game_ptr = nullptr; // Non-owning pointer to access Game resources
};