// File: include/states/AdventureState.h
#pragma once

#include "states/GameState.h"
#include "graphics/Animator.h"
#include "../entities/Digimon.h" // Keep for backward compatibility
#include "../entities/DigimonDefinition.h" // Add for DigimonDefinition
#include "Core/MapData.h" // Include for NodeData
// Reduce includes if possible, move SDL includes to cpp if only needed there
#include <SDL_render.h> // Needed for SDL_Texture* member
#include <SDL_rect.h>   // Needed for SDL_Rect usage? (If helpers use it)
#include <vector>
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <string>   // Needed for helper return types
#include <map>      // Needed for environment configuration mapping

// Forward declarations
class Game;
class InputManager;
class PlayerData;
class PCDisplay;
// SDL_Texture is included via SDL_render.h

// Enums specific to AdventureState logic
enum PlayerState { STATE_IDLE, STATE_WALKING };

class AdventureState : public GameState {
public:
    // Constructor & Destructor
    AdventureState(Game* game);
    ~AdventureState() override;

    void enter() override;
    void handle_input(InputManager& inputManager, PlayerData* playerData) override;
    void update(float delta_time, PlayerData* playerData) override;
    void render(PCDisplay& display) override;
    StateType getType() const override;

private:
    // Animator
    Animator partnerAnimator_;

    // Battle Trigger Members
    int current_area_step_goal_;        // How many steps to trigger a battle in the current area
    int total_steps_taken_in_area_;   // Accumulator for steps in the current area
    std::string current_area_enemy_id_; // Placeholder for future use to specify the enemy
    bool is_fading_to_battle_;          // True if currently fading out for a battle
    float battle_fade_alpha_;           // Alpha for the fade-to-black overlay (0.0 to 255.0)
    const float BATTLE_FADE_DURATION_SECONDS = 0.5f; // How long the fade to black should take
    const float BATTLE_TRANSITION_SCROLL_ADVANCE_SECONDS = 1.5f; // How much to advance scroll for battle transition    // Other Data Members    
    SDL_Texture* bgTexture0_ = nullptr;
    SDL_Texture* bgTexture1_ = nullptr;
    SDL_Texture* bgTexture2_ = nullptr;
    const Digimon::DigimonDefinition* current_partner_definition_ = nullptr;
    PlayerState current_state_ = STATE_IDLE;
    int queued_steps_ = 0;
    float bg_scroll_offset_0_ = 0.0f;
    float bg_scroll_offset_1_ = 0.0f;
    float bg_scroll_offset_2_ = 0.0f;
    bool firstWalkUpdate_ = true;
    float timeSinceLastStep_ = 0.0f; // Added: Timer for returning to Idle

    // Rate Limiting Members
    float stepWindowTimer_ = 0.0f;  // Timer tracking the current window
    int stepsInWindow_ = 0;         // Steps counted in the current window

    // Environment constants
    struct EnvironmentConfig {
        int groundOffset;  // Distance from bottom of screen to ground
        float parallaxFactor; // How much parallax to apply
    };
    
    std::map<std::string, EnvironmentConfig> environmentConfigs_;
    void initializeEnvironmentConfigs() {
        // Set up ground offsets for each environment
        environmentConfigs_["tropicaljungle"] = {50, 1.0f};
        environmentConfigs_["lake"] = {60, 0.8f};
        // Add other environments...
    }
      // Function to get current ground offset
    int getCurrentGroundOffset() const {
        // Look up based on current node ID
        if (currentNode_ && environmentConfigs_.count(currentNode_->id) > 0) {
            return environmentConfigs_.at(currentNode_->id).groundOffset;
        }
        return 40; // Default fallback value
    }

    // Rendering methods
    void render_background_layers(PCDisplay& display);
    void render_character(PCDisplay& display);

    // Constants
    const int MAX_QUEUED_STEPS = 2;
    const float SCROLL_SPEED_0 = 3.0f * 60.0f;
    const float SCROLL_SPEED_1 = 1.0f * 60.0f;
    const float SCROLL_SPEED_2 = 0.5f * 60.0f;
    const int WINDOW_WIDTH = 466;
    const int WINDOW_HEIGHT = 466;    // Private Helpers
    std::string getAnimationIdForCurrentState() const;    
    
    // Current map node
    const Digivice::NodeData* currentNode_ = nullptr;

    // Default values
    int groundOffset_ = 40;        // Default ground offset
    int characterHeight = 32;      // Default character height
}; // End of AdventureState class definition