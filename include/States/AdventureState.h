// File: include/states/AdventureState.h
#pragma once

#include "states/GameState.h"
#include "graphics/Animator.h"
#include "../entities/Digimon.h"
// Reduce includes if possible, move SDL includes to cpp if only needed there
#include <SDL_render.h> // Needed for SDL_Texture* member
#include <SDL_rect.h>   // Needed for SDL_Rect usage? (If helpers use it)
#include <vector>
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <string>   // Needed for helper return types

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

private:    // Structure to hold texture variant information for a layer
    struct LayerVariants {
        std::vector<SDL_Texture*> textures; // All texture variants for this layer
        std::vector<int> widths;            // Width of each texture variant
        std::vector<int> effectiveWidths;   // Effective width for scrolling (2/3 of actual width)
        size_t currentVariantIndex = 0;     // Index of the current variant being displayed
    };

    // Animator
    Animator partnerAnimator_;

    // Battle Trigger Members
    int current_area_step_goal_;        // How many steps to trigger a battle in the current area
    int total_steps_taken_in_area_;   // Accumulator for steps in the current area
    std::string current_area_enemy_id_; // Placeholder for future use to specify the enemy
    bool is_fading_to_battle_;          // True if currently fading out for a battle
    float battle_fade_alpha_;           // Alpha for the fade-to-black overlay (0.0 to 255.0)
    const float BATTLE_FADE_DURATION_SECONDS = 0.5f; // How long the fade to black should take
    const float BATTLE_TRANSITION_SCROLL_ADVANCE_SECONDS = 1.5f; // How much to advance scroll for battle transition

    // Other Data Members
    LayerVariants bgLayer0_; // Foreground layer variants
    LayerVariants bgLayer1_; // Middleground layer variants
    LayerVariants bgLayer2_; // Background layer variants
    
    // Legacy textures (kept for backward compatibility with BattleState)
    SDL_Texture* bgTexture0_ = nullptr;
    SDL_Texture* bgTexture1_ = nullptr;
    SDL_Texture* bgTexture2_ = nullptr;
    
    DigimonType current_digimon_;
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

    // Constants
    const int MAX_QUEUED_STEPS = 2;
    const float SCROLL_SPEED_0 = 3.0f * 60.0f;
    const float SCROLL_SPEED_1 = 1.0f * 60.0f;
    const float SCROLL_SPEED_2 = 0.5f * 60.0f;
    const int WINDOW_WIDTH = 466;
    const int WINDOW_HEIGHT = 466;

    // Private Helpers
    std::string getAnimationIdForCurrentState() const;
    std::string getDigimonTextureId(DigimonType type) const;
    
    // Helper for loading texture variants
    void loadTextureVariantsForLayer(LayerVariants& layer, 
                                    const std::vector<std::string>& texturePaths, 
                                    const std::string& baseTextureId);
                                    
    // Helper for rendering a background layer with variants
    void renderBackgroundLayerVariants(PCDisplay& display, 
                                      const LayerVariants& layer, 
                                      float scrollOffset);

}; // End of AdventureState class definition