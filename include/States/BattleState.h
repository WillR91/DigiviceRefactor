#pragma once

#include <string> // Include for std::string
#include <SDL_render.h> // For SDL_Texture*
#include "GameState.h" // Corrected: GameState.h is in the same directory
#include "../entities/Digimon.h" // For DigimonType enum
#include "graphics/Animator.h" // Added for enemy_animator_

// Forward declarations
class Game;
class InputManager;
class PCDisplay;
class AssetManager; // For loading assets later
struct SDL_Texture; // Forward declare SDL_Texture

// Define a duration for the fade effect
const float BATTLE_STATE_FADE_DURATION_SECONDS = 0.75f; // Duration of fade in/out within BattleState

enum class VPetBattlePhase {
    ENTERING_FADE_IN,    // Fade in from black to battle scene
    ENEMY_REVEAL_SETUP,  // Prepare enemy sprite and name
    ENEMY_REVEAL_ANIM,   // Simple animation/timer for reveal
    BATTLE_AWAITING_PLAYER_COMMAND, // Added new phase
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
    // Constructor now takes player's Digimon type, an enemy ID string,
    // and background texture information.
    BattleState(Game* game, DigimonType playerDigimonType, const std::string& enemyId,
                SDL_Texture* bgLayer0, SDL_Texture* bgLayer1, SDL_Texture* bgLayer2,
                float scrollOffset0, float scrollOffset1, float scrollOffset2);
    ~BattleState() override; // Removed = default

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
    DigimonType enemy_digimon_type_; // Added: Actual type of the enemy
    Animator enemy_animator_;      // Added: Animator for the enemy
    SDL_Texture* enemy_name_texture_ = nullptr; // Added: Texture for enemy's name
    SDL_Point enemy_sprite_position_;
    SDL_Point enemy_name_position_;
    // SDL_Texture* player_sprite_texture_; // To be added
    // SDL_Texture* enemy_sprite_texture_;  // To be added
    // SDL_Texture* enemy_name_texture_;    // To be added
    // Basic data for now, will expand with Combatant struct later

    // Background Textures & Offsets
    SDL_Texture* bg_texture_layer0_;
    SDL_Texture* bg_texture_layer1_;
    SDL_Texture* bg_texture_layer2_;
    float bg_scroll_offset_0_;
    float bg_scroll_offset_1_;
    float bg_scroll_offset_2_;
    bool show_foreground_layer_; // Added: Flag to control foreground rendering
    // int enemy_id_; // Removed redefinition
};
