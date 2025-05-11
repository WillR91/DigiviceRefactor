#pragma once

#include "States/GameState.h" // Corrected to use GameState.h
#include "entities/Digimon.h" // For DigimonType
#include <string>             // For std::string

// Forward declarations
class Game;
class InputManager;
class PCDisplay;
class PlayerData; // Added based on GameState.h methods

class BattleState : public GameState { // Corrected to inherit from GameState
public:
    // Updated constructor to accept player and enemy info
    BattleState(Game* game, DigimonType playerDigimonType, const std::string& enemyId); 
    ~BattleState() override = default;

    // GameState interface methods
    void enter() override;
    void exit() override;
    void handle_input(InputManager& inputManager, PlayerData* playerData) override;
    void update(float delta_time, PlayerData* playerData) override;
    void render(PCDisplay& display) override;
    StateType getType() const override;

    // Original IGameState methods (now covered or adapted)
    // void init() override; // Covered by enter() or constructor
    // void pause() override; // Could be part of a more complex state management or handled within update/handle_input
    // void resume() override; // Same as pause
    // void close() override; // Covered by exit() or destructor

private:
    Game* game_ptr_; // Retained from original, ensure it's initialized from GameState's game_ptr or passed
    DigimonType player_digimon_type_;
    std::string enemy_id_;
    // We will add VPetBattlePhase, timers, combatant data, etc., here later
};
