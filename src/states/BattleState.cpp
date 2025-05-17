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
const float TOOTH_TRANSITION_DURATION_SECONDS = 0.3f; // Duration for closing/opening

// Updated constructor implementation
BattleState::BattleState(Game* game, DigimonType playerDigimonType, const std::string& enemyId,
                         SDL_Texture* bgLayer0, SDL_Texture* bgLayer1, SDL_Texture* bgLayer2,
                         float scrollOffset0, float scrollOffset1, float scrollOffset2)
    : GameState(game), 
      player_digimon_type_(playerDigimonType), 
      enemy_id_(enemyId),
      asset_manager_ptr_(nullptr), 
      current_phase_(VPetBattlePhase::ENTERING_FADE_IN), 
      general_fade_alpha_(255.0f), 
      phase_timer_(0.0f),
      tooth_top_texture_(nullptr),      // Added initialization
      tooth_bottom_texture_(nullptr),   // Added initialization
      tooth_transition_progress_(0.0f), // Added initialization
      instruction_text_texture_(nullptr),// Added initialization
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
    if (tooth_top_texture_) {
        SDL_DestroyTexture(tooth_top_texture_);
        tooth_top_texture_ = nullptr;
    }
    if (tooth_bottom_texture_) {
        SDL_DestroyTexture(tooth_bottom_texture_);
        tooth_bottom_texture_ = nullptr;
    }
    if (instruction_text_texture_) {
        SDL_DestroyTexture(instruction_text_texture_);
        instruction_text_texture_ = nullptr;
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
    current_phase_ = VPetBattlePhase::ENTERING_FADE_IN; // Ensure starting phase
    phase_timer_ = 0.0f;
    tooth_transition_progress_ = 0.0f; // Reset progress

    PCDisplay* display = game_ptr->get_display(); 

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
        current_phase_ = VPetBattlePhase::BATTLE_OVER_POP_STATE; // Corrected: Was OUTCOME_DISPLAY
        general_fade_alpha_ = 0.0f; // No fade if erroring out immediately
        return;
    }

    AnimationManager* animManager = game_ptr->getAnimationManager();
    TextRenderer* textRenderer = game_ptr->getTextRenderer();

    if (!display || !animManager || !textRenderer) { // Use the 'display' variable declared earlier
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "BattleState::enter - Missing required game systems (display, animManager, or textRenderer).");
        current_phase_ = VPetBattlePhase::BATTLE_OVER_POP_STATE; // Corrected: Was OUTCOME_DISPLAY
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
    // current_phase_ = VPetBattlePhase::ENTERING_FADE_IN; // Already set
    // phase_timer_ = 0.0f; // Already set
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
    // Existing input handling for ENEMY_REVEAL_ANIM is fine, let's adjust for ENEMY_REVEAL_DISPLAY
    if (current_phase_ == VPetBattlePhase::ENEMY_REVEAL_DISPLAY) {
        if (inputManager.isActionJustPressed(GameAction::CONFIRM)) { // Removed CANCEL for this specific transition
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState: Player confirmed enemy reveal. Transitioning to TOOTH_TRANSITION_START.");
            current_phase_ = VPetBattlePhase::TOOTH_TRANSITION_START;
            phase_timer_ = 0.0f;
            tooth_transition_progress_ = 0.0f; // Reset progress for closing
        }
    } else if (current_phase_ == VPetBattlePhase::INSTRUCTION_SCREEN_DISPLAY) {
        if (inputManager.isActionJustPressed(GameAction::CONFIRM)) {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState: Player confirmed instruction screen. Transitioning to TO_SELECTION_FADE_OUT (placeholder).");
            current_phase_ = VPetBattlePhase::TO_SELECTION_FADE_OUT; // Next logical step
            phase_timer_ = 0.0f;
            general_fade_alpha_ = 0.0f; // Start fade to black
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
    TextRenderer* textRenderer = game_ptr->getTextRenderer(); // Get text renderer for instruction text

    switch (current_phase_) {
        case VPetBattlePhase::ENTERING_FADE_IN:
            enemy_animator_.update(delta_time); 
            general_fade_alpha_ = 255.0f * (1.0f - (phase_timer_ / BATTLE_STATE_FADE_DURATION_SECONDS));
            if (general_fade_alpha_ <= 0.0f) {
                general_fade_alpha_ = 0.0f; 
                phase_timer_ = 0.0f; 
                current_phase_ = VPetBattlePhase::ENEMY_REVEAL_DISPLAY; // Changed from ENEMY_REVEAL_ANIM
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState: Fade-in complete. Transitioning to ENEMY_REVEAL_DISPLAY.");
            }
            break;
        
        // ENEMY_REVEAL_SETUP is done in enter()
        // ENEMY_REVEAL_ANIM is effectively ENEMY_REVEAL_DISPLAY now, waiting for input.
        case VPetBattlePhase::ENEMY_REVEAL_DISPLAY:
            enemy_animator_.update(delta_time); // Keep enemy animating
            // Waits for player input in handle_input
            break;

        case VPetBattlePhase::TOOTH_TRANSITION_START:
            // This phase can be used to load tooth assets if they were textures
            // For now, it just ensures progress is reset and moves to closing.
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState: Starting tooth transition.");
            tooth_transition_progress_ = 0.0f;
            phase_timer_ = 0.0f;
            current_phase_ = VPetBattlePhase::TOOTH_TRANSITION_CLOSING;
            break;

        case VPetBattlePhase::TOOTH_TRANSITION_CLOSING:
            tooth_transition_progress_ = std::min(1.0f, phase_timer_ / TOOTH_TRANSITION_DURATION_SECONDS);
            if (phase_timer_ >= TOOTH_TRANSITION_DURATION_SECONDS) {
                tooth_transition_progress_ = 1.0f;
                phase_timer_ = 0.0f;
                current_phase_ = VPetBattlePhase::TOOTH_TRANSITION_OPENING;
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState: Tooth transition closed. Opening...");
            }
            break;

        case VPetBattlePhase::TOOTH_TRANSITION_OPENING:
            tooth_transition_progress_ = std::max(0.0f, 1.0f - (phase_timer_ / TOOTH_TRANSITION_DURATION_SECONDS));
            if (phase_timer_ >= TOOTH_TRANSITION_DURATION_SECONDS) {
                tooth_transition_progress_ = 0.0f;
                phase_timer_ = 0.0f;
                current_phase_ = VPetBattlePhase::INSTRUCTION_SCREEN_DISPLAY;
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState: Tooth transition opened. Displaying instructions.");

                // Create instruction text texture
                if (textRenderer && game_ptr->get_display()) {
                    if (instruction_text_texture_) {
                        SDL_DestroyTexture(instruction_text_texture_);
                        instruction_text_texture_ = nullptr;
                    }
                    SDL_Color textColor = {255, 255, 255, 255}; // White
                    // Convert instruction text to uppercase
                    std::string instructionText = "PRESS ENTER";
                    for (char &c : instructionText) {
                        c = toupper(c);
                    }
                    instruction_text_texture_ = textRenderer->renderTextToTexture(game_ptr->get_display()->getRenderer(), instructionText, textColor);
                    if (!instruction_text_texture_) {
                        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "BattleState: Failed to render instruction text texture.");
                    }
                } else {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "BattleState: TextRenderer or Display not available for instruction text.");
                }
            }
            break;

        case VPetBattlePhase::INSTRUCTION_SCREEN_DISPLAY:
            // Waits for player input in handle_input
            break;
        
        case VPetBattlePhase::TO_SELECTION_FADE_OUT:
            general_fade_alpha_ = std::min(255.0f, (phase_timer_ / BATTLE_STATE_FADE_DURATION_SECONDS) * 255.0f);
            if (general_fade_alpha_ >= 255.0f) {
                general_fade_alpha_ = 255.0f;
                phase_timer_ = 0.0f;
                // For now, let's just loop back to enemy reveal for testing purposes
                // current_phase_ = VPetBattlePhase::TO_SELECTION_FADE_IN; // Next logical step
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState: Fade to selection screen complete (placeholder). Looping for test.");
                // Reset to beginning of battle flow for now to test loop
                current_phase_ = VPetBattlePhase::ENTERING_FADE_IN; 
                general_fade_alpha_ = 255.0f; // Reset for fade in
            }
            break;


        // ... other phases ...
        default:
            // Potentially log an unknown phase
            enemy_animator_.update(delta_time); // Keep enemy animating if in an unknown/unhandled state
            break;
    }
}

void BattleState::render(PCDisplay& display) {
    int screen_width = 0;
    int screen_height = 0;
    display.getWindowSize(screen_width, screen_height);

    // --- Background and Scene Rendering (similar to before) ---
    bool allow_scene_rendering = false;
    if (current_phase_ == VPetBattlePhase::ENTERING_FADE_IN || 
        current_phase_ == VPetBattlePhase::ENEMY_REVEAL_DISPLAY) {
        allow_scene_rendering = true; 
    } else if (current_phase_ > VPetBattlePhase::ENTERING_FADE_IN && 
               current_phase_ < VPetBattlePhase::EXITING_FADE_OUT &&
               current_phase_ != VPetBattlePhase::TOOTH_TRANSITION_CLOSING && // Don't render scene when teeth are closing
               current_phase_ != VPetBattlePhase::TOOTH_TRANSITION_OPENING && // Or opening (unless we want background visible through teeth)
               current_phase_ != VPetBattlePhase::INSTRUCTION_SCREEN_DISPLAY && // Instruction screen is standalone
               current_phase_ != VPetBattlePhase::TO_SELECTION_FADE_OUT // Not during fade out
               ) {
        allow_scene_rendering = true;
    }


    if (allow_scene_rendering) {
        // Render Background Layers (Parallax)
        SDL_Rect srcRect = {0, 0, 0, 0}; 
        SDL_Rect destRect = {0, 0, screen_width, screen_height}; 

        // Layer 2 (Farthest)
        if (bg_texture_layer2_) {
            SDL_QueryTexture(bg_texture_layer2_, nullptr, nullptr, &srcRect.w, &srcRect.h);
            if (srcRect.w > 0) { 
                float positive_offset_2 = bg_scroll_offset_2_;
                while(positive_offset_2 < 0) positive_offset_2 += srcRect.w; 
                int bg2_x = static_cast<int>(positive_offset_2) % srcRect.w;
                
                destRect.w = srcRect.w; 
                destRect.h = srcRect.h; 
                destRect.y = (screen_height - srcRect.h) / 2; 
                if (srcRect.h >= screen_height) destRect.y = 0; 
                destRect.h = std::min(srcRect.h, screen_height); 
                
                SDL_Rect currentSrcRect = {0, 0, srcRect.w, srcRect.h};
                SDL_Rect currentDestRect = destRect;

                currentDestRect.x = -bg2_x;
                currentDestRect.w = srcRect.w; 
                display.drawTexture(bg_texture_layer2_, &currentSrcRect, &currentDestRect);

                if (bg2_x != 0) { 
                    currentDestRect.x = srcRect.w - bg2_x;
                    if (currentDestRect.x < screen_width && (currentDestRect.x + srcRect.w) > 0) {
                         display.drawTexture(bg_texture_layer2_, &currentSrcRect, &currentDestRect);
                    }
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
        
        // Render Enemy Sprite (if in ENEMY_REVEAL_DISPLAY)
        if (current_phase_ == VPetBattlePhase::ENEMY_REVEAL_DISPLAY || current_phase_ == VPetBattlePhase::ENTERING_FADE_IN) {
            const AnimationData* currentEnemyAnim = enemy_animator_.getCurrentAnimationData();
            if (currentEnemyAnim && currentEnemyAnim->textureAtlas) { 
                SDL_Rect srcR = enemy_animator_.getCurrentFrameRect();
                SDL_Rect destR = {
                    enemy_sprite_position_.x - srcR.w / 2,
                    enemy_sprite_position_.y - srcR.h / 2,
                    srcR.w,
                    srcR.h
                };
                display.drawTexture(currentEnemyAnim->textureAtlas, &srcR, &destR, SDL_FLIP_HORIZONTAL); 
            }
            // Render Enemy Name
            if (enemy_name_texture_) {
                int name_w, name_h;
                SDL_QueryTexture(enemy_name_texture_, nullptr, nullptr, &name_w, &name_h);
                SDL_Rect nameDestR = {
                    enemy_name_position_.x - name_w / 2, 
                    enemy_name_position_.y, 
                    name_w, 
                    name_h
                };
                display.drawTexture(enemy_name_texture_, nullptr, &nameDestR);
            }
        }
        // Layer 0 (Nearest) - Now controlled by show_foreground_layer_
        if (show_foreground_layer_ && bg_texture_layer0_) { 
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
    }


    // --- Jagged Tooth Transition Rendering ---
    if (current_phase_ == VPetBattlePhase::TOOTH_TRANSITION_CLOSING || current_phase_ == VPetBattlePhase::TOOTH_TRANSITION_OPENING) {
        // Ensure a black background during tooth transition if no scene is rendered
        if (!allow_scene_rendering) {
            display.setDrawColor(0, 0, 0, 255); // Opaque Black
            SDL_Rect bgRect = {0, 0, screen_width, screen_height};
            display.fillRect(&bgRect);
        }

        float tooth_height = (static_cast<float>(screen_height) / 2.0f) * tooth_transition_progress_;
        
        display.setDrawColor(255, 0, 0, 255); // Red for placeholder teeth

        SDL_Rect top_tooth_rect = {0, 0, screen_width, static_cast<int>(tooth_height)};
        display.fillRect(&top_tooth_rect);

        SDL_Rect bottom_tooth_rect = {0, screen_height - static_cast<int>(tooth_height), screen_width, static_cast<int>(tooth_height)};
        display.fillRect(&bottom_tooth_rect);
    }

    // --- Instruction Screen Rendering ---
    if (current_phase_ == VPetBattlePhase::INSTRUCTION_SCREEN_DISPLAY) {
        // Black background for instruction screen
        display.setDrawColor(0, 0, 0, 255); // Opaque Black
        SDL_Rect bgRect = {0, 0, screen_width, screen_height};
        display.fillRect(&bgRect);

        if (instruction_text_texture_) {
            int text_w, text_h;
            SDL_QueryTexture(instruction_text_texture_, nullptr, nullptr, &text_w, &text_h);
            SDL_Rect text_dest_rect = {
                (screen_width - text_w) / 2,
                (screen_height - text_h) / 2,
                text_w,
                text_h
            };
            display.drawTexture(instruction_text_texture_, nullptr, &text_dest_rect);
        } else {
            // Fallback if texture somehow failed (though error is logged in update)
            // You could use TextRenderer::drawText directly here for a fallback if it supports immediate mode.
        }
    }
    
    // --- Fade Overlays (ENTERING_FADE_IN or TO_SELECTION_FADE_OUT) ---
    if ((current_phase_ == VPetBattlePhase::ENTERING_FADE_IN && general_fade_alpha_ > 0.0f) ||
        (current_phase_ == VPetBattlePhase::TO_SELECTION_FADE_OUT && general_fade_alpha_ > 0.0f)) {
        display.setDrawBlendMode(SDL_BLENDMODE_BLEND); 
        display.setDrawColor(0, 0, 0, static_cast<Uint8>(general_fade_alpha_));
        SDL_Rect fullScreenRect = {0, 0, screen_width, screen_height};
        display.fillRect(&fullScreenRect); 
    }
}

StateType BattleState::getType() const {
    return StateType::Battle;
}
