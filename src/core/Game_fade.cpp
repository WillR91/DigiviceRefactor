#include "core/Game.h"
#include "states/TransitionState.h"
#include "utils/ConfigManager.h" // Add ConfigManager
#include <SDL_log.h>

// Implementation of the fade to black functionality
void Game::requestFadeToBlack(float duration) {
    // If duration is not specified (0 or negative), use the value from config
    if (duration <= 0.0f) {
        duration = ConfigManager::getValue<float>("game.transitionSpeed", 0.3f);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Game::requestFadeToBlack using duration %.2f from config", duration);
    } else {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Game::requestFadeToBlack called with specified duration %.2f", duration);
    }
    
    // Create a transition state for fade to black
    std::unique_ptr<TransitionState> transition = std::make_unique<TransitionState>(
        this,                               // Game pointer
        duration,                           // Duration
        TransitionEffectType::FADE_TO_COLOR, // Effect type
        SDL_Color{0, 0, 0, 255}             // Black color
    );
    
    // Set the fade step to indicate we're starting to fade out
    fade_step_ = FadeSequenceStep::FADING_OUT;
    
    // Push the transition state
    requestPushState(std::move(transition));
}
