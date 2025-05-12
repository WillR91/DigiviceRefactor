#pragma once

#include <string> // Include for std::string
#include "GameState.h" // Corrected: GameState.h is in the same directory
#include "../entities/Digimon.h" // For DigimonType enum

// Forward declarations
class Game;
class InputManager;
class PCDisplay;
class AssetManager; // For loading assets later

// Define a duration for the fade effect
const float BATTLE_STATE_FADE_DURATION_SECONDS = 0.75f; // Duration of fade in/out within BattleState

enum class VPetBattlePhase {
    ENTERING_FADE_IN,    // Fade in from black to battle scene
    ENEMY_REVEAL_SETUP,  // Prepare enemy sprite and name
    ENEMY_REVEAL_ANIM,   // Simple animation/timer for reveal
    PLAYER_ATTACK_SETUP,
    PLAYER_ATTACK_ANIM,
    ENEMY_ATTACK_SETUP,
    ENEMY_ATTACK_ANIM,
    ATTACKS_MEET_SETUP,
    ATTACKS_MEET_ANIM,
    MINIGAME_TAP_RACE_START,
    MINIGAME_TAP_RACE_ACTIVE,
    MINIGAME_RESOLVE,
    PLAYER_HIT_ANIM,
    ENEMY_HIT_ANIM,
    OUTCOME_DISPLAY,
    EXITING_FADE_OUT,    // Fade out from battle scene to black
    BATTLE_OVER_POP_STATE // Signal to Game to pop this state
};

class BattleState : public GameState {
public:
    // Constructor now takes player's Digimon type and an enemy ID string
    BattleState(Game* game, DigimonType playerDigimonType, const std::string& enemyId);
    ~BattleState() override = default;

    void enter() override; // Was init()
    void exit() override;  // Was close()
    void pause() override;
    void resume() override;
    void handle_input(InputManager& inputManager, PlayerData* playerData) override; // Match GameState
    void update(float delta_time, PlayerData* playerData) override;             // Match GameState
    void render(PCDisplay& display) override;
    StateType getType() const override;


private:
    // Game systems
    // Game* game_ptr_ already in base GameState
    AssetManager* asset_manager_ptr_;

    // Battle state
    VPetBattlePhase current_phase_;
    float general_fade_alpha_; // For fade in/out effects (0 = transparent, 255 = opaque)
    float phase_timer_;        // Generic timer for phases that need one

    // Combatant info (simplified for now)
    DigimonType player_digimon_type_;
    std::string enemy_id_;
    // SDL_Texture* player_sprite_texture_; // To be added
    // SDL_Texture* enemy_sprite_texture_;  // To be added
    // SDL_Texture* enemy_name_texture_;    // To be added
    // Basic data for now, will expand with Combatant struct later
};
