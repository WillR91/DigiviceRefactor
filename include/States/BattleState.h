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
    // Phase 1: Battle Start & Enemy Introduction
    ENTERING_FADE_IN,           // Fade in from black to reveal enemy
    ENEMY_REVEAL_SETUP,         // Prepare enemy sprite and name (occurs during/before fade-in)
    ENEMY_REVEAL_DISPLAY,       // Enemy is visible, name shown, awaiting player confirm

    // Phase 2: Battle Instructions Screen
    TOOTH_TRANSITION_START,     // Start the jagged tooth closing transition
    TOOTH_TRANSITION_CLOSING,   // Teeth are closing
    TOOTH_TRANSITION_OPENING,   // Teeth are opening to reveal instruction screen
    INSTRUCTION_SCREEN_DISPLAY, // Instruction screen is visible, awaiting player confirm

    // Phase 3: Player Digimon Selection Screen
    TO_SELECTION_FADE_OUT,      // Fade to black before selection screen
    TO_SELECTION_FADE_IN,       // Fade in to selection screen
    SELECTION_SCREEN_DISPLAY,   // Player Digimon selection screen visible, awaiting confirm (placeholder for now)

    // Phase 4: Player Digimon Enters Battle Arena
    TO_PLAYER_REVEAL_FADE_OUT,  // Fade to black before player Digimon reveal
    TO_PLAYER_REVEAL_FADE_IN,   // Fade in to show player Digimon
    PLAYER_REVEAL_DISPLAY,      // Player Digimon is visible, idle animation

    // Phase 5: Player's Attack Sequence
    PLAYER_ATTACK_BG_TRANSITION,// Transition to orange motion background
    PLAYER_ATTACK_LARGE_SPRITE, // Display large full-screen attack sprite
    PLAYER_ATTACK_PIXEL_SETUP,  // Setup for pixel sprite attack (HP bars, etc.)
    PLAYER_ATTACK_ANIM,         // Player pixel sprite attack animation (jump back, fire projectile)

    // Phase 6: Enemy's Attack Sequence (Mirrored)
    TO_ENEMY_ATTACK_FADE_WHITE, // Quick fade to white
    ENEMY_ATTACK_BG_TRANSITION, // Transition to orange motion background (mirrored)
    ENEMY_ATTACK_LARGE_SPRITE,  // Display large full-screen attack sprite (mirrored)
    ENEMY_ATTACK_PIXEL_SETUP,   // Setup for enemy pixel sprite attack (mirrored)
    ENEMY_ATTACK_ANIM,          // Enemy pixel sprite attack animation (mirrored)

    // Phase 7: Attack Clash & Duel Mechanic (Novel Implementation Area)
    CLASH_DUEL_SETUP,           // Setup for the novel duel mechanic screen/state
    CLASH_DUEL_ACTIVE,          // The duel mechanic is active
    CLASH_DUEL_RESOLVE,         // Duel outcome is being determined/shown

    // Phase 8: Health Points (HP) and Battle Conclusion (Hit Animations)
    PLAYER_HIT_ANIM,            // Player's Digimon shows hit animation
    ENEMY_HIT_ANIM,             // Enemy's Digimon shows hit animation

    // Phase 9: Player Victory Sequence
    VICTORY_DISPLAY,            // Player Digimon idle/celebrating
    VICTORY_DEDIGIVOLVE_FADE,   // White fade for de-digivolution (if applicable)
    VICTORY_DEDIGIVOLVE_REVEAL, // Show de-digivolved rookie
    VICTORY_CLEAR_UI,           // Show "CLEAR" UI bar

    // (Alternative) Player Defeat Sequence (To be detailed later)
    DEFEAT_DISPLAY,

    // Phase 10: Return to Overworld and Progression
    EXITING_FADE_OUT,           // Fade out from battle scene to black (to map)
    BATTLE_OVER_POP_STATE       // Signal to Game to pop this state
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
    void prepareInstructionScreenText(); // Helper to create text texture

    // Game systems
    // Game* game_ptr_ already in base GameState
    AssetManager* asset_manager_ptr_;

    // Battle state
    VPetBattlePhase current_phase_;
    float general_fade_alpha_; // For fade in/out effects (0 = transparent, 255 = opaque)
    float phase_timer_;        // Generic timer for phases that need one

    // --- New variables for Jagged Tooth Transition ---
    SDL_Texture* tooth_top_texture_;    // Texture for the top part of the teeth
    SDL_Texture* tooth_bottom_texture_; // Texture for the bottom part of the teeth
    float tooth_transition_progress_; // 0.0 to 1.0 for closing, then 1.0 to 0.0 for opening

    // --- New variables for Instruction Screen ---
    SDL_Texture* instruction_text_texture_; // Texture for the "Battle Instructions..." text

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
