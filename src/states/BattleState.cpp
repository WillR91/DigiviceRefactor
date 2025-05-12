#include "States/BattleState.h"
#include "Core/Game.h"
#include "Core/InputManager.h"
#include "Core/PlayerData.h" 
#include "Platform/PC/pc_display.h"
#include <SDL_log.h>
#include <algorithm> // For std::min
#include <cmath>     // For other math functions if needed

// Updated constructor implementation
BattleState::BattleState(Game* game, DigimonType playerDigimonType, const std::string& enemyId,
                         SDL_Texture* bgLayer0, SDL_Texture* bgLayer1, SDL_Texture* bgLayer2,
                         float scrollOffset0, float scrollOffset1, float scrollOffset2)
    : GameState(game), 
      player_digimon_type_(playerDigimonType), 
      enemy_id_(enemyId),
      asset_manager_ptr_(nullptr), // Initialize asset_manager_ptr_
      current_phase_(VPetBattlePhase::ENTERING_FADE_IN), // Start with fade in
      general_fade_alpha_(255.0f), // Start fully opaque black
      phase_timer_(0.0f),
      bg_texture_layer0_(bgLayer0),
      bg_texture_layer1_(bgLayer1),
      bg_texture_layer2_(bgLayer2),
      bg_scroll_offset_0_(scrollOffset0),
      bg_scroll_offset_1_(scrollOffset1),
      bg_scroll_offset_2_(scrollOffset2)
{
    if (game_ptr) { // Ensure game_ptr is valid before using it
        asset_manager_ptr_ = game_ptr->getAssetManager();
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState created for player type %d and enemy ID %s", 
                static_cast<int>(player_digimon_type_), enemy_id_.c_str());
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState: Initial background scroll offsets: L0: %.2f, L1: %.2f, L2: %.2f", 
                bg_scroll_offset_0_, bg_scroll_offset_1_, bg_scroll_offset_2_);
}

void BattleState::enter() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState enter. Current phase: ENTERING_FADE_IN");
    current_phase_ = VPetBattlePhase::ENTERING_FADE_IN; // Explicitly set phase
    general_fade_alpha_ = 255.0f; // Start fade from black
    phase_timer_ = 0.0f;
    // Initialization logic (formerly init())
}

void BattleState::exit() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState exit");
    // Cleanup logic (formerly close())
}

void BattleState::pause() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState pause");
    // Pause logic
}

void BattleState::resume() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState resume");
    // Resume logic
}

void BattleState::handle_input(InputManager& inputManager, PlayerData* playerData) {
    // Input handling logic
}

void BattleState::update(float delta_time, PlayerData* playerData) {
    if (!game_ptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "BattleState::update - game_ptr is null!");
        return;
    }

    switch (current_phase_) {
        case VPetBattlePhase::ENTERING_FADE_IN:
            phase_timer_ += delta_time;
            // Calculate alpha: starts at 255 (opaque) and goes to 0 (transparent)
            general_fade_alpha_ = 255.0f * (1.0f - (phase_timer_ / BATTLE_STATE_FADE_DURATION_SECONDS));

            if (general_fade_alpha_ <= 0.0f) {
                general_fade_alpha_ = 0.0f; // Clamp to 0
                phase_timer_ = 0.0f; // Reset timer for the next phase
                current_phase_ = VPetBattlePhase::ENEMY_REVEAL_SETUP;
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState: Fade-in complete. Transitioning to ENEMY_REVEAL_SETUP.");
            }
            break;

        case VPetBattlePhase::ENEMY_REVEAL_SETUP:
            // Logic for this phase will be added in the next step
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState: In ENEMY_REVEAL_SETUP phase.");
            // For now, we'll stay in this phase until we implement its logic.
            // To prevent getting stuck in a loop of log messages if update is called rapidly,
            // consider adding a flag or moving to the next state once setup is truly done.
            // For this intermediate step, just logging once or moving to a placeholder is fine.
            // current_phase_ = VPetBattlePhase::ENEMY_REVEAL_ANIM; // Placeholder for now to see next step
            break;
        
        case VPetBattlePhase::ENEMY_REVEAL_ANIM:
            // Logic for this phase will be added later
            break;

        // ... other phases ...
        default:
            // Potentially log an unknown phase
            break;
    }

    // Update animators if any (e.g., enemy_animator_.update(delta_time);)
    // For now, no active animators in this step.
}

void BattleState::render(PCDisplay& display) {
    // Get window/logical screen dimensions
    int screen_width = 0;
    int screen_height = 0;
    display.getWindowSize(screen_width, screen_height);
    // If PCDisplay manages a separate logical size that differs from window size,
    // we might need a display.getLogicalWidth()/Height() or similar in the future.
    // For now, using window size.

    // 1. Render Background Layers (Parallax, similar to AdventureState)
    SDL_Rect srcRect = {0, 0, 0, 0}; 
    SDL_Rect destRect = {0, 0, screen_width, screen_height}; // Default dest to full screen

    // Layer 2 (Farthest)
    if (bg_texture_layer2_) {
        SDL_QueryTexture(bg_texture_layer2_, nullptr, nullptr, &srcRect.w, &srcRect.h);
        if (srcRect.w > 0) { // Ensure texture width is valid to prevent division by zero or weird behavior
            // Ensure offset is positive for modulo, or handle negative offsets if they can occur
            float positive_offset_2 = bg_scroll_offset_2_;
            while(positive_offset_2 < 0) positive_offset_2 += srcRect.w; // Ensure positive for modulo
            int bg2_x = static_cast<int>(positive_offset_2) % srcRect.w;
            
            destRect.w = srcRect.w; // Use actual texture width for drawing parts
            destRect.h = srcRect.h; // Use actual texture height
            // Adjust destRect y if backgrounds are not full height, for now assume they are or fill screen_height
            destRect.y = (screen_height - srcRect.h) / 2; // Example: center vertically if not full height
            if (srcRect.h >= screen_height) destRect.y = 0; // If texture taller or same, start at top
            destRect.h = std::min(srcRect.h, screen_height); // Clip height to screen
            
            SDL_Rect currentSrcRect = {0, 0, srcRect.w, srcRect.h};
            SDL_Rect currentDestRect = destRect;

            currentDestRect.x = -bg2_x;
            currentDestRect.w = srcRect.w; // Draw full width of texture initially
            display.drawTexture(bg_texture_layer2_, &currentSrcRect, &currentDestRect);

            // Draw the wrapped part if visible
            if (bg2_x != 0) { // If scrolled at all
                currentDestRect.x = srcRect.w - bg2_x;
                // Only draw if it's on screen
                if (currentDestRect.x < screen_width && (currentDestRect.x + srcRect.w) > 0) {
                     display.drawTexture(bg_texture_layer2_, &currentSrcRect, &currentDestRect);
                }
                // Also handle if the initial part scrolled completely off left
                currentDestRect.x = -bg2_x - srcRect.w;
                 if (currentDestRect.x < screen_width && (currentDestRect.x + srcRect.w) > 0) {
                     display.drawTexture(bg_texture_layer2_, &currentSrcRect, &currentDestRect);
                 }
            }
        }
    }
    // Layer 1 (Middle)
    if (bg_texture_layer1_) {
        SDL_QueryTexture(bg_texture_layer1_, nullptr, nullptr, &srcRect.w, &srcRect.h);
        if (srcRect.w > 0) {
            float positive_offset_1 = bg_scroll_offset_1_;
            while(positive_offset_1 < 0) positive_offset_1 += srcRect.w;
            int bg1_x = static_cast<int>(positive_offset_1) % srcRect.w;

            destRect.w = srcRect.w;
            destRect.h = srcRect.h;
            destRect.y = (screen_height - srcRect.h) / 2;
            if (srcRect.h >= screen_height) destRect.y = 0;
            destRect.h = std::min(srcRect.h, screen_height);
            SDL_Rect currentSrcRect = {0, 0, srcRect.w, srcRect.h};
            SDL_Rect currentDestRect = destRect;

            currentDestRect.x = -bg1_x;
            display.drawTexture(bg_texture_layer1_, &currentSrcRect, &currentDestRect);
            if (bg1_x != 0) {
                currentDestRect.x = srcRect.w - bg1_x;
                 if (currentDestRect.x < screen_width && (currentDestRect.x + srcRect.w) > 0) {
                    display.drawTexture(bg_texture_layer1_, &currentSrcRect, &currentDestRect);
                 }
                currentDestRect.x = -bg1_x - srcRect.w;
                 if (currentDestRect.x < screen_width && (currentDestRect.x + srcRect.w) > 0) {
                     display.drawTexture(bg_texture_layer1_, &currentSrcRect, &currentDestRect);
                 }
            }
        }
    }
    // Layer 0 (Nearest)
    if (bg_texture_layer0_) {
        SDL_QueryTexture(bg_texture_layer0_, nullptr, nullptr, &srcRect.w, &srcRect.h);
        if (srcRect.w > 0) {
            float positive_offset_0 = bg_scroll_offset_0_;
            while(positive_offset_0 < 0) positive_offset_0 += srcRect.w;
            int bg0_x = static_cast<int>(positive_offset_0) % srcRect.w;

            destRect.w = srcRect.w;
            destRect.h = srcRect.h;
            destRect.y = (screen_height - srcRect.h) / 2;
            if (srcRect.h >= screen_height) destRect.y = 0;
            destRect.h = std::min(srcRect.h, screen_height);
            SDL_Rect currentSrcRect = {0, 0, srcRect.w, srcRect.h};
            SDL_Rect currentDestRect = destRect;

            currentDestRect.x = -bg0_x;
            display.drawTexture(bg_texture_layer0_, &currentSrcRect, &currentDestRect);
            if (bg0_x != 0) {
                currentDestRect.x = srcRect.w - bg0_x;
                 if (currentDestRect.x < screen_width && (currentDestRect.x + srcRect.w) > 0) {
                    display.drawTexture(bg_texture_layer0_, &currentSrcRect, &currentDestRect);
                 }
                currentDestRect.x = -bg0_x - srcRect.w;
                 if (currentDestRect.x < screen_width && (currentDestRect.x + srcRect.w) > 0) {
                     display.drawTexture(bg_texture_layer0_, &currentSrcRect, &currentDestRect);
                 }
            }
        }
    }

    // 2. Render Fade-In Overlay (if active)
    if (current_phase_ == VPetBattlePhase::ENTERING_FADE_IN && general_fade_alpha_ > 0.0f) {
        display.setDrawBlendMode(SDL_BLENDMODE_BLEND); // Corrected method name
        display.setDrawColor(0, 0, 0, static_cast<Uint8>(general_fade_alpha_));
        SDL_Rect fullScreenRect = {0, 0, screen_width, screen_height};
        display.fillRect(&fullScreenRect); 
        // display.setDrawBlendMode(SDL_BLENDMODE_NONE); // Optional: Reset blend mode
    }

    // Future: Render Player Digimon, Enemy Digimon, UI elements etc.
}

StateType BattleState::getType() const {
    return StateType::Battle;
}
