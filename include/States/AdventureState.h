// File: include/states/AdventureState.h
#pragma once

#include "states/GameState.h"
#include "graphics/Animator.h"
#include "core/MapData.h"  // For BackgroundLayerData
#include "../entities/Digimon.h" // Keep for backward compatibility
#include "../entities/DigimonDefinition.h" // Add for DigimonDefinition
// Reduce includes if possible, move SDL includes to cpp if only needed there
#include <SDL_render.h> // Needed for SDL_Texture* member
#include <SDL_rect.h>   // Needed for SDL_Rect usage? (If helpers use it)
#include <vector>
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <string>   // Needed for helper return types
#include <memory>   // For std::unique_ptr

// Forward declarations
class Game;
class InputManager;
class PlayerData;
class PCDisplay;
class SeamlessBackgroundRenderer;
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

private:    // Animator
    Animator partnerAnimator_;

    // Background Rendering
    std::unique_ptr<SeamlessBackgroundRenderer> backgroundRenderer_;

    // Battle Trigger Members
    int current_area_step_goal_;        // How many steps to trigger a battle in the current area
    int total_steps_taken_in_area_;   // Accumulator for steps in the current area
    std::string current_area_enemy_id_; // Placeholder for future use to specify the enemy
    bool is_fading_to_battle_;          // True if currently fading out for a battle
    float battle_fade_alpha_;           // Alpha for the fade-to-black overlay (0.0 to 255.0)
    const float BATTLE_FADE_DURATION_SECONDS = 0.5f; // How long the fade to black should take
    const float BATTLE_TRANSITION_SCROLL_ADVANCE_SECONDS = 1.5f; // How much to advance scroll for battle transition    // Environment Layer Textures (clearer naming)
    SDL_Texture* foregroundTexture_ = nullptr;   // Closest to camera (was bgTexture0_)
    SDL_Texture* middlegroundTexture_ = nullptr; // Middle depth (was bgTexture1_)
    SDL_Texture* backgroundTexture_ = nullptr;   // Furthest back (was bgTexture2_)
    const Digimon::DigimonDefinition* current_partner_definition_ = nullptr;
    PlayerState current_state_ = STATE_IDLE;
    int queued_steps_ = 0;    float bg_scroll_offset_0_ = 0.0f;
    float bg_scroll_offset_1_ = 0.0f;
    float bg_scroll_offset_2_ = 0.0f;
    float previous_bg_scroll_offset_0_ = 0.0f;
    float previous_bg_scroll_offset_1_ = 0.0f;
    float previous_bg_scroll_offset_2_ = 0.0f;
    float smooth_scroll_factor_ = 1.0f;
    bool firstWalkUpdate_ = true;
    float timeSinceLastStep_ = 0.0f; // Added: Timer for returning to Idle    // Rate Limiting Members
    float stepWindowTimer_ = 0.0f;  // Timer tracking the current window
    int stepsInWindow_ = 0;         // Steps counted in the current window

    // Constants
    const int MAX_QUEUED_STEPS = 2;
    const float SCROLL_SPEED_0 = 1.2f * 60.0f;  // Foreground - fastest layer
    const float SCROLL_SPEED_1 = 0.8f * 60.0f;  // Middleground - medium speed
    const float SCROLL_SPEED_2 = 0.4f * 60.0f;  // Background - slowest layer    const int WINDOW_WIDTH = 466;
    const int WINDOW_HEIGHT = 466;
    
    // Private Helpers
    std::string getAnimationIdForCurrentState() const;
    void loadBackgroundVariants(const std::string& environmentPath); // New variant system (legacy path mapping)
    void loadBackgroundVariantsFromNodeData(const Digivice::BackgroundLayerData& layerData, const std::string& nodeId); // New variant system (direct loading)
    void renderScaledBackgroundLayer(PCDisplay& display, SDL_Texture* texture, 
                                   int screenWidth, int screenHeight, 
                                   float globalScale, int layerIndex, 
                                   float scrollOffset); // Enhanced scaling renderer

}; // End of AdventureState class definition