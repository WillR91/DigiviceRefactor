#include "states/BattleState.h"
#include "Core/Game.h"
#include "Core/InputManager.h"
#include "Core/PlayerData.h" 
#include "platform/pc/pc_display.h"
#include "utils/AnimationUtils.h" // Added
#include "Core/AnimationManager.h" // Added (ensure it's here)
#include "graphics/AnimationData.h"  // Added (ensure it's here)
#include "ui/TextRenderer.h"    // Corrected Path
#include <SDL_log.h>
#include <SDL_pixels.h> // For SDL_Color
#include <algorithm> // For std::min
#include <cmath>     // For other math functions if needed
#include "Core/AssetManager.h" // Corrected path

// Constants for battle visuals - adjust as needed
const int ENEMY_SPRITE_POS_X = 180; // Example X position
const int ENEMY_SPRITE_POS_Y = 100; // Example Y position
const int ENEMY_NAME_POS_X = 180;   // Example X position for name
const int ENEMY_NAME_POS_Y = 140;   // Example Y position for name
const float ENEMY_REVEAL_ANIM_DURATION_SECONDS = 2.0f; // How long the reveal animation/pause lasts

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
      bg_scroll_offset_2_(scrollOffset2),
      show_foreground_layer_(false), // Initialize the new flag to false (foreground off by default)
      enemy_digimon_type_(DIGI_COUNT), // Initialize enemy_digimon_type_
      enemy_name_texture_(nullptr),    // Initialize enemy_name_texture_
      enemy_sprite_position_({ENEMY_SPRITE_POS_X, ENEMY_SPRITE_POS_Y}),   // Initialize enemy_sprite_position_
      enemy_name_position_({ENEMY_NAME_POS_X, ENEMY_NAME_POS_Y})     // Initialize enemy_name_position_
{
    if (game_ptr) { // Ensure game_ptr is valid before using it
        asset_manager_ptr_ = game_ptr->getAssetManager();
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState created for player type %d and enemy ID %s", 
                static_cast<int>(player_digimon_type_), enemy_id_.c_str());
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState: Initial background scroll offsets: L0: %.2f, L1: %.2f, L2: %.2f", 
                bg_scroll_offset_0_, bg_scroll_offset_1_, bg_scroll_offset_2_);
}

BattleState::~BattleState() {
    if (enemy_name_texture_) {
        SDL_DestroyTexture(enemy_name_texture_);
        enemy_name_texture_ = nullptr;
    }
}

// Helper function to get Digimon name (similar to PartnerSelectState)
// TODO: Consider moving this to a common utility if used in more places.
std::string getDigimonNameForBattle(DigimonType type) {
    switch(type) {
        case DIGI_AGUMON: return "AGUMON";
        case DIGI_GABUMON: return "GABUMON";
        case DIGI_BIYOMON: return "BIYOMON";
        case DIGI_GATOMON: return "GATOMON";
        case DIGI_GOMAMON: return "GOMAMON";
        case DIGI_PALMON: return "PALMON";
        case DIGI_TENTOMON: return "TENTOMON";
        case DIGI_PATAMON: return "PATAMON";
        default: return "UNKNOWN";
    }
}

void BattleState::enter() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState enter: Performing initial enemy setup.");

    // Ensure the fade starts fully black
    general_fade_alpha_ = 255.0f;

    PCDisplay* display = game_ptr->get_display(); // Declare display once here

    // Ensure the screen is fully black before starting the fade-in
    if (display) {
        int screen_width = 0;
        int screen_height = 0;
        display->getWindowSize(screen_width, screen_height);
        display->setDrawBlendMode(SDL_BLENDMODE_BLEND);
        display->setDrawColor(0, 0, 0, 255); // Fully opaque black
        SDL_Rect fullScreenRect = {0, 0, screen_width, screen_height};
        display->fillRect(&fullScreenRect);
    }

    // --- BEGIN MOVED ENEMY_REVEAL_SETUP LOGIC ---
    if (!game_ptr) { // Ensure game_ptr is valid
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "BattleState::enter - game_ptr is null! Cannot setup enemy.");
        current_phase_ = VPetBattlePhase::OUTCOME_DISPLAY; 
        general_fade_alpha_ = 0.0f; // No fade if erroring out immediately
        return;
    }

    AnimationManager* animManager = game_ptr->getAnimationManager();
    TextRenderer* textRenderer = game_ptr->getTextRenderer();

    if (!display || !animManager || !textRenderer) { // Use the 'display' variable declared earlier
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "BattleState::enter - Missing required game systems (display, animManager, or textRenderer).");
        current_phase_ = VPetBattlePhase::OUTCOME_DISPLAY; 
        general_fade_alpha_ = 0.0f;
        return;
    }

    // 1. Determine Enemy Type (Hardcoded for now)
    if (enemy_id_ == "DefaultEnemy") {
        enemy_digimon_type_ = DIGI_GABUMON; // Example
    } else {
        enemy_digimon_type_ = DIGI_AGUMON; // Fallback
    }

    // 2. Get Enemy Name String
    std::string enemyNameStr = getDigimonNameForBattle(enemy_digimon_type_);

    // 3. Load Enemy Animation
    std::string enemyIdleAnimId = AnimationUtils::GetAnimationId(enemy_digimon_type_, "Idle");
    const AnimationData* enemyIdleAnimData = animManager->getAnimationData(enemyIdleAnimId);
    if (enemyIdleAnimData) {
        enemy_animator_.setAnimation(enemyIdleAnimData);
        enemy_animator_.update(0.0f); // Prime the animator for the first frame
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "BattleState::enter - Failed to get idle animation '%s' for enemy type %d.", enemyIdleAnimId.c_str(), static_cast<int>(enemy_digimon_type_));
    }

    // 4. Create Enemy Name Texture
    if (enemy_name_texture_) { 
        SDL_DestroyTexture(enemy_name_texture_);
        enemy_name_texture_ = nullptr;
    }
    SDL_Color textColor = {255, 255, 255, 255}; // White
    enemy_name_texture_ = textRenderer->renderTextToTexture(display->getRenderer(), enemyNameStr, textColor);
    if (!enemy_name_texture_) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "BattleState::enter - Failed to render enemy name texture for '%s'.", enemyNameStr.c_str());
    }

    // 5. Define Positions
    int screen_width = 0;
    int screen_height = 0;
    display->getWindowSize(screen_width, screen_height);

    enemy_sprite_position_ = {screen_width / 2, screen_height / 2};

    int sprite_frame_height = 0;
    const AnimationData* currentEnemyAnimData = enemy_animator_.getCurrentAnimationData(); // Use a different variable name
    if (currentEnemyAnimData && !currentEnemyAnimData->frameRects.empty()) {
        sprite_frame_height = currentEnemyAnimData->frameRects[0].h; 
    } else {
        sprite_frame_height = 50; 
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "BattleState::enter - Could not get enemy sprite height from animation, using fallback %dpx", sprite_frame_height);
    }
    
    const int name_gap_pixels = 10; 
    enemy_name_position_.x = enemy_sprite_position_.x; 
    enemy_name_position_.y = enemy_sprite_position_.y + (sprite_frame_height / 2) + name_gap_pixels;

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState::enter - Enemy setup complete. Enemy: %s, SpritePos: (%d,%d), NamePos: (%d,%d)", enemyNameStr.c_str(), enemy_sprite_position_.x, enemy_sprite_position_.y, enemy_name_position_.x, enemy_name_position_.y);
    // --- END MOVED ENEMY_REVEAL_SETUP LOGIC ---

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState enter: Initializing fade-in. Current phase: ENTERING_FADE_IN");
    current_phase_ = VPetBattlePhase::ENTERING_FADE_IN;
    phase_timer_ = 0.0f;
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
    if (current_phase_ == VPetBattlePhase::ENEMY_REVEAL_ANIM || current_phase_ == VPetBattlePhase::BATTLE_AWAITING_PLAYER_COMMAND) {
        if (inputManager.isActionJustPressed(GameAction::CONFIRM) || inputManager.isActionJustPressed(GameAction::CANCEL)) {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState: Player confirmed enemy reveal. Transitioning to PLAYER_ATTACK_SETUP.");
            current_phase_ = VPetBattlePhase::BATTLE_AWAITING_PLAYER_COMMAND; // Placeholder for next phase
            // Potentially reset phase_timer_ if PLAYER_ATTACK_SETUP uses it
        }
    }
    // ... other input handling ...
}

void BattleState::update(float delta_time, PlayerData* playerData) {
    if (!game_ptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "BattleState::update - game_ptr is null!");
        return;
    }

    phase_timer_ += delta_time;

    switch (current_phase_) {
        case VPetBattlePhase::ENTERING_FADE_IN:
            enemy_animator_.update(delta_time); // Update enemy animation during fade-in
            // phase_timer_ is incremented outside the switch.
            // Calculate alpha: starts at 255 (opaque) and goes to 0 (transparent)
            general_fade_alpha_ = 255.0f * (1.0f - (phase_timer_ / BATTLE_STATE_FADE_DURATION_SECONDS));

            if (general_fade_alpha_ <= 0.0f) {
                general_fade_alpha_ = 0.0f; // Clamp to 0
                phase_timer_ = 0.0f; // Reset timer for the ENEMY_REVEAL_ANIM phase
                current_phase_ = VPetBattlePhase::ENEMY_REVEAL_ANIM;
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState: Fade-in complete. Transitioning to ENEMY_REVEAL_ANIM.");
            }
            break;

        // Note: ENEMY_REVEAL_SETUP case is now removed from update()
        
        case VPetBattlePhase::ENEMY_REVEAL_ANIM:
            // Update enemy animation
            enemy_animator_.update(delta_time);
            // This phase currently waits for player input or timer, handled in handle_input or below
            if (phase_timer_ >= ENEMY_REVEAL_ANIM_DURATION_SECONDS) { // Wait for a couple of seconds
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Enemy reveal animation/pause complete. Awaiting player input."); // Changed SDL_Log to SDL_LogInfo
                current_phase_ = VPetBattlePhase::BATTLE_AWAITING_PLAYER_COMMAND; // Transition to awaiting input
            }
            break;

        case VPetBattlePhase::BATTLE_AWAITING_PLAYER_COMMAND:
            // Idle, wait for player input in handle_input
            enemy_animator_.update(delta_time); // Keep enemy animating
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
    // Get window/logical screen dimensions first
    int screen_width = 0;
    int screen_height = 0;
    display.getWindowSize(screen_width, screen_height);

    // Ensure the fade overlay is rendered during the first frame of ENTERING_FADE_IN
    if (current_phase_ == VPetBattlePhase::ENTERING_FADE_IN && general_fade_alpha_ > 0.0f) {
        display.setDrawBlendMode(SDL_BLENDMODE_BLEND);
        display.setDrawColor(0, 0, 0, static_cast<Uint8>(general_fade_alpha_));
        SDL_Rect fullScreenRect = {0, 0, screen_width, screen_height}; // Now screen_width and screen_height are declared
        display.fillRect(&fullScreenRect);
    }

    // If PCDisplay manages a separate logical size that differs from window size,
    // we might need a display.getLogicalWidth()/Height() or similar in the future.
    // For now, using window size.

    // Determine if scene elements (background, enemy) should be rendered
    bool allow_scene_rendering = false;
    if (current_phase_ == VPetBattlePhase::ENTERING_FADE_IN) {
        // Always render scene elements; the overlay will manage actual visibility.
        allow_scene_rendering = true; 
    } else if (current_phase_ > VPetBattlePhase::ENTERING_FADE_IN && 
               current_phase_ < VPetBattlePhase::EXITING_FADE_OUT) {
        // Always render scene if past fade-in and not yet fading out
        allow_scene_rendering = true;
    }

    if (allow_scene_rendering) {
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

        // Render Enemy Sprite and Name (AFTER Layer 1, BEFORE Layer 0)
        const AnimationData* currentEnemyAnim = enemy_animator_.getCurrentAnimationData();
        if (currentEnemyAnim && currentEnemyAnim->textureAtlas) { 
            SDL_Rect srcR = enemy_animator_.getCurrentFrameRect();
            // Center the sprite at enemy_sprite_position_
            SDL_Rect destR = {
                enemy_sprite_position_.x - srcR.w / 2,
                enemy_sprite_position_.y - srcR.h / 2,
                srcR.w,
                srcR.h
            };
            display.drawTexture(currentEnemyAnim->textureAtlas, &srcR, &destR, SDL_FLIP_HORIZONTAL); 
        }

        // Render Enemy Name -- MOVED AFTER LAYER 0
        /* 
        if (enemy_name_texture_) {
            int name_w, name_h;
            SDL_QueryTexture(enemy_name_texture_, nullptr, nullptr, &name_w, &name_h);
            // Center the name texture horizontally at enemy_name_position_.x, use enemy_name_position_.y as top
            SDL_Rect nameDestR = {
                enemy_name_position_.x - name_w / 2, 
                enemy_name_position_.y, 
                name_w, 
                name_h
            };
            display.drawTexture(enemy_name_texture_, nullptr, &nameDestR);
        }
        */

        // Layer 0 (Nearest) - Now controlled by show_foreground_layer_
        if (show_foreground_layer_ && bg_texture_layer0_) { // <--- MODIFIED HERE: Use the flag
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
        } // End of Layer 0 rendering

        // Render Enemy Name (AFTER Layer 0)
        if (enemy_name_texture_) {
            int name_w, name_h;
            SDL_QueryTexture(enemy_name_texture_, nullptr, nullptr, &name_w, &name_h);
            // Center the name texture horizontally at enemy_name_position_.x, use enemy_name_position_.y as top
            SDL_Rect nameDestR = {
                enemy_name_position_.x - name_w / 2, 
                enemy_name_position_.y, 
                name_w, 
                name_h
            };
            display.drawTexture(enemy_name_texture_, nullptr, &nameDestR);
        }

        // Render Enemy Sprite and Name (now inside the same conditional as background)
        // The previous specific 'if' condition for enemy rendering is removed as it's covered by allow_scene_rendering.
        // MOVED EARLIER, BETWEEN LAYER 1 AND LAYER 0
        /*
        const AnimationData* currentEnemyAnim = enemy_animator_.getCurrentAnimationData();
        if (currentEnemyAnim && currentEnemyAnim->textureAtlas) { // Changed textureSheet to textureAtlas
            SDL_Rect srcR = enemy_animator_.getCurrentFrameRect();
            // Center the sprite at enemy_sprite_position_
            SDL_Rect destR = {
                enemy_sprite_position_.x - srcR.w / 2,
                enemy_sprite_position_.y - srcR.h / 2,
                srcR.w,
                srcR.h
            };
            display.drawTexture(currentEnemyAnim->textureAtlas, &srcR, &destR, SDL_FLIP_HORIZONTAL); // Added SDL_FLIP_HORIZONTAL
        }

        // Render Enemy Name
        if (enemy_name_texture_) {
            int name_w, name_h;
            SDL_QueryTexture(enemy_name_texture_, nullptr, nullptr, &name_w, &name_h);
            // Center the name texture horizontally at enemy_name_position_.x, use enemy_name_position_.y as top
            SDL_Rect nameDestR = {
                enemy_name_position_.x - name_w / 2, 
                enemy_name_position_.y, 
                name_w, 
                name_h
            };
            display.drawTexture(enemy_name_texture_, nullptr, &nameDestR);
        }
        */
    } // End of if (allow_scene_rendering)

    // 2. Render Fade-In Overlay (if active)
    // This overlay is drawn on top of everything else during the fade-in.
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
