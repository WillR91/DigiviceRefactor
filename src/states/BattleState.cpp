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

// Static member definitions
const float BattleState::BATTLE_STATE_FADE_DURATION_SECONDS = 0.5f;
const float BattleState::TOOTH_TRANSITION_DURATION_SECONDS = 0.3f;
const float BattleState::PLAYER_ATTACK_BG_TRANSITION_DURATION_SECONDS = 0.5f;
const float BattleState::PLAYER_ATTACK_LARGE_SPRITE_DURATION_SECONDS = 1.5f; // Defined here

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
      // player_animator_ initialized by default constructor
      player_name_texture_(nullptr), // Added: Initialize player name texture
      player_sprite_position_({0,0}), // Added: Initialize player sprite position (will be set later)
      enemy_id_(enemyId),
      asset_manager_ptr_(nullptr), 
      current_phase_(VPetBattlePhase::ENTERING_FADE_IN), 
      general_fade_alpha_(255.0f), 
      phase_timer_(0.0f),
      tooth_top_texture_(nullptr),      // Added initialization
      tooth_bottom_texture_(nullptr),   // Added initialization
      tooth_transition_progress_(0.0f), // Added initialization
      instruction_text_texture_(nullptr),// Added initialization
      selection_screen_text_texture_(nullptr), // Added initialization
      player_hp_bar_texture_(nullptr), // Added: Initialize player HP bar texture
      enemy_hp_bar_texture_(nullptr),  // Added: Initialize enemy HP bar texture
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
    if (player_name_texture_) { // Added: Destroy player name texture
        SDL_DestroyTexture(player_name_texture_);
        player_name_texture_ = nullptr;
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
    if (selection_screen_text_texture_) { // Added
        SDL_DestroyTexture(selection_screen_text_texture_);
        selection_screen_text_texture_ = nullptr;
    }
    if (player_hp_bar_texture_) { // Added: Destroy player HP bar texture
        SDL_DestroyTexture(player_hp_bar_texture_);
        player_hp_bar_texture_ = nullptr;
    }
    if (enemy_hp_bar_texture_) { // Added: Destroy enemy HP bar texture
        SDL_DestroyTexture(enemy_hp_bar_texture_);
        enemy_hp_bar_texture_ = nullptr;
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

StateType BattleState::getType() const {
    return StateType::Battle; // Corrected BATTLE to Battle
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
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState: Player confirmed instruction screen. Transitioning to TO_SELECTION_FADE_OUT."); // Updated log
            current_phase_ = VPetBattlePhase::TO_SELECTION_FADE_OUT; 
            phase_timer_ = 0.0f;
            general_fade_alpha_ = 0.0f; // Start fade to black
        }
    } else if (current_phase_ == VPetBattlePhase::SELECTION_SCREEN_DISPLAY) {
        if (inputManager.isActionJustPressed(GameAction::CONFIRM)) {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState: Player confirmed selection. Transitioning to TO_PLAYER_REVEAL_FADE_OUT.");
            current_phase_ = VPetBattlePhase::TO_PLAYER_REVEAL_FADE_OUT;
            phase_timer_ = 0.0f;
            general_fade_alpha_ = 0.0f; // Start fade to black (alpha will increase from 0 to 255)
        }
    } else if (current_phase_ == VPetBattlePhase::PLAYER_REVEAL_DISPLAY) { 
        if (inputManager.isActionJustPressed(GameAction::CONFIRM)) {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState: Player confirmed player reveal. Transitioning to PLAYER_ATTACK_BG_TRANSITION.");
            current_phase_ = VPetBattlePhase::PLAYER_ATTACK_BG_TRANSITION; 
            phase_timer_ = 0.0f; // Reset timer for the new phase
            // general_fade_alpha_ should be managed by the new phase if needed, typically 0 for no fade initially
            general_fade_alpha_ = 0.0f; 
        }
    } else if (current_phase_ == VPetBattlePhase::PLAYER_ATTACK_BG_TRANSITION) {
        // This phase is timed and should transition automatically via update().
        // No input needed here for normal flow.
        // The previous test loop has been removed.
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
                general_fade_alpha_ = 255.0f; // Ensure it's fully black
                phase_timer_ = 0.0f;
                current_phase_ = VPetBattlePhase::TO_SELECTION_FADE_IN; // Next logical step
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState: Fade to black for selection screen complete. Transitioning to TO_SELECTION_FADE_IN.");
            }
            break;

        case VPetBattlePhase::TO_SELECTION_FADE_IN:
            general_fade_alpha_ = 255.0f * (1.0f - (phase_timer_ / BATTLE_STATE_FADE_DURATION_SECONDS));
            if (general_fade_alpha_ <= 0.0f) {
                general_fade_alpha_ = 0.0f;
                phase_timer_ = 0.0f;
                current_phase_ = VPetBattlePhase::SELECTION_SCREEN_DISPLAY;
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState: Fade in to selection screen complete. Transitioning to SELECTION_SCREEN_DISPLAY.");

                // Create selection screen text texture
                if (textRenderer && game_ptr->get_display()) {
                    if (selection_screen_text_texture_) {
                        SDL_DestroyTexture(selection_screen_text_texture_);
                        selection_screen_text_texture_ = nullptr;
                    }
                    SDL_Color textColor = {255, 255, 255, 255}; // White
                    std::string selectionText = "PLAYER SELECTION (PLACEHOLDER)";
                    // Convert to uppercase (assuming font only supports uppercase)
                    for (char &c : selectionText) {
                        c = toupper(c);
                    }
                    selection_screen_text_texture_ = textRenderer->renderTextToTexture(game_ptr->get_display()->getRenderer(), selectionText, textColor);
                    if (!selection_screen_text_texture_) {
                        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "BattleState: Failed to render selection screen text texture.");
                    }
                } else {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "BattleState: TextRenderer or Display not available for selection screen text.");
                }
            }
            break;

        case VPetBattlePhase::SELECTION_SCREEN_DISPLAY:
            // Placeholder: Waits for player input (to be implemented in handle_input)
            // For now, does nothing in update.
            break;

        case VPetBattlePhase::TO_PLAYER_REVEAL_FADE_OUT:
            general_fade_alpha_ = std::min(255.0f, (phase_timer_ / BATTLE_STATE_FADE_DURATION_SECONDS) * 255.0f);
            if (general_fade_alpha_ >= 255.0f) {
                general_fade_alpha_ = 255.0f; // Ensure it's fully black
                phase_timer_ = 0.0f;
                current_phase_ = VPetBattlePhase::TO_PLAYER_REVEAL_FADE_IN;
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState: Fade out from selection screen complete. Transitioning to TO_PLAYER_REVEAL_FADE_IN.");
            }
            break;

        case VPetBattlePhase::TO_PLAYER_REVEAL_FADE_IN:
            general_fade_alpha_ = 255.0f * (1.0f - (phase_timer_ / BATTLE_STATE_FADE_DURATION_SECONDS));
            if (general_fade_alpha_ <= 0.0f) {
                general_fade_alpha_ = 0.0f;
                phase_timer_ = 0.0f;
                current_phase_ = VPetBattlePhase::PLAYER_REVEAL_DISPLAY; // Next phase
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState: Fade in to player reveal screen complete. Transitioning to PLAYER_REVEAL_DISPLAY.");
                // Potentially load player Digimon assets here or ensure they are ready
            }
            break;

        case VPetBattlePhase::PLAYER_REVEAL_DISPLAY:
            // For now, this phase just waits for player input (handled in handle_input).
            // Player Digimon rendering and UI will be added here later.
            // player_animator_.update(delta_time); // Will be needed when player sprite is shown
            // enemy_animator_.update(delta_time); // Enemy might be visible in background or smaller
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState: PLAYER_REVEAL_DISPLAY active. Waiting for input.");
            break;

        case VPetBattlePhase::PLAYER_ATTACK_BG_TRANSITION:
            // player_animator_.update(delta_time); // Player likely static or has own anim during this
            if (phase_timer_ >= PLAYER_ATTACK_BG_TRANSITION_DURATION_SECONDS) {
                current_phase_ = VPetBattlePhase::PLAYER_ATTACK_LARGE_SPRITE; 
                phase_timer_ = 0.0f;
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState: PLAYER_ATTACK_BG_TRANSITION complete. Transitioning to PLAYER_ATTACK_LARGE_SPRITE.");
            }
            break;

        case VPetBattlePhase::PLAYER_ATTACK_LARGE_SPRITE:
            // player_animator_.update(delta_time); // Player likely static or has own anim during this
            if (phase_timer_ >= PLAYER_ATTACK_LARGE_SPRITE_DURATION_SECONDS) {
                current_phase_ = VPetBattlePhase::PLAYER_ATTACK_PIXEL_SETUP; // Transition to the next phase
                phase_timer_ = 0.0f;
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BattleState: PLAYER_ATTACK_LARGE_SPRITE complete. Transitioning to PLAYER_ATTACK_PIXEL_SETUP.");
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

    // --- Background Color Fill (Phase Dependant) ---
    // Default to black unless a specific phase needs a different solid background
    // or allows scene rendering.
    bool scene_is_rendered_this_frame = false;

    // --- Background and Scene Rendering ---
    bool allow_scene_rendering = false;
    bool allow_instruction_rendering = false;
    bool allow_selection_rendering = false;
    bool allow_player_digimon_rendering = false;
    bool allow_hp_bar_rendering = false;
    bool render_orange_background = false;

    switch (current_phase_) {
        case VPetBattlePhase::ENTERING_FADE_IN:
        case VPetBattlePhase::ENEMY_REVEAL_DISPLAY:
        case VPetBattlePhase::TOOTH_TRANSITION_START:     
        case VPetBattlePhase::TOOTH_TRANSITION_CLOSING:  
        // case VPetBattlePhase::TO_PLAYER_REVEAL_FADE_IN: // Scene (enemy) was visible here, but now PLAYER_REVEAL is different
            allow_scene_rendering = true;
            break;
        // For PLAYER_REVEAL_DISPLAY, we will handle background differently for now.
        // TO_PLAYER_REVEAL_FADE_IN will fade *to* this new PLAYER_REVEAL_DISPLAY background.
        case VPetBattlePhase::PLAYER_REVEAL_DISPLAY:
            allow_player_digimon_rendering = true; // Prepare for player rendering
            allow_hp_bar_rendering = true;         // and HP bars
            // Background remains black for this phase as per previous logic
            break;
        case VPetBattlePhase::PLAYER_ATTACK_BG_TRANSITION:
            render_orange_background = true;
            allow_player_digimon_rendering = true;
            allow_hp_bar_rendering = true;
            break;
        default: 
            allow_scene_rendering = false;
            break;
    }

    if (allow_scene_rendering) {
        scene_is_rendered_this_frame = true;
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
        
        // Render Enemy Sprite (if in ENEMY_REVEAL_DISPLAY or other relevant phases)
        // Adjusted condition to include PLAYER_REVEAL_DISPLAY and TO_PLAYER_REVEAL_FADE_IN
        if (current_phase_ == VPetBattlePhase::ENEMY_REVEAL_DISPLAY || 
            current_phase_ == VPetBattlePhase::ENTERING_FADE_IN ||
            current_phase_ == VPetBattlePhase::TO_PLAYER_REVEAL_FADE_IN || // Enemy visible during fade-in to player reveal
            current_phase_ == VPetBattlePhase::PLAYER_REVEAL_DISPLAY) {    // Enemy visible during player reveal
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
    } else {
        // If scene is not rendered, determine background color
        if (render_orange_background) {
            display.setDrawColor(255, 165, 0, 255); // Orange
        } else {
            // For other phases without scene rendering (e.g., INSTRUCTION_SCREEN_DISPLAY,
            // SELECTION_SCREEN_DISPLAY, or the initial black for PLAYER_REVEAL_DISPLAY before its specific draw call)
            // Default to black. PLAYER_REVEAL_DISPLAY will draw its own black screen later anyway.
            display.setDrawColor(0, 0, 0, 255); // Opaque Black
        }
        SDL_Rect bgRect = {0, 0, screen_width, screen_height};
        display.fillRect(&bgRect);
        scene_is_rendered_this_frame = false; 
    }

    // --- Phase-Specific Overlays / Content ---

    // --- Player Reveal Display Screen ---
    if (current_phase_ == VPetBattlePhase::TO_PLAYER_REVEAL_FADE_IN || 
        current_phase_ == VPetBattlePhase::PLAYER_REVEAL_DISPLAY) {
        // For now, just a black screen. Later this will have player digimon and UI.
        // This overrides the default black fill if allow_scene_rendering was false for these phases.
        display.setDrawColor(0, 0, 0, 255); // Opaque Black for player reveal background
        SDL_Rect player_reveal_bg_rect = {0, 0, screen_width, screen_height};
        display.fillRect(&player_reveal_bg_rect);
        scene_is_rendered_this_frame = false; // No parallax scene here for now
        // TODO: Render Player Digimon sprite
        // TODO: Render HP Bars and other UI
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

    // --- Instruction Screen Content Rendering ---
    if (current_phase_ == VPetBattlePhase::INSTRUCTION_SCREEN_DISPLAY ||
        current_phase_ == VPetBattlePhase::TO_SELECTION_FADE_OUT) { 
        // Black background for instruction screen (already handled by default fill if scene_is_rendered_this_frame is false)
        // display.setDrawColor(0, 0, 0, 255); 
        // SDL_Rect bgRect = {0, 0, screen_width, screen_height};
        // display.fillRect(&bgRect);

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
        }
    }

    // --- Selection Screen Content Rendering ---
    if (current_phase_ == VPetBattlePhase::TO_SELECTION_FADE_IN ||
        current_phase_ == VPetBattlePhase::SELECTION_SCREEN_DISPLAY ||
        current_phase_ == VPetBattlePhase::TO_PLAYER_REVEAL_FADE_OUT) {
        // Black background for selection screen (already handled by default fill if scene_is_rendered_this_frame is false)
        // display.setDrawColor(0, 0, 0, 255); 
        // SDL_Rect bgRect = {0, 0, screen_width, screen_height};
        // display.fillRect(&bgRect);

        // Render selection screen text
        // Text is created at the end of TO_SELECTION_FADE_IN update.
        // It should be visible during SELECTION_SCREEN_DISPLAY and TO_PLAYER_REVEAL_FADE_OUT.
        if ((current_phase_ == VPetBattlePhase::SELECTION_SCREEN_DISPLAY ||
             current_phase_ == VPetBattlePhase::TO_PLAYER_REVEAL_FADE_OUT) &&
            selection_screen_text_texture_) {
            int text_w, text_h;
            SDL_QueryTexture(selection_screen_text_texture_, nullptr, nullptr, &text_w, &text_h);
            SDL_Rect text_dest_rect = {
                (screen_width - text_w) / 2,
                (screen_height - text_h) / 2,
                text_w,
                text_h
            };
            display.drawTexture(selection_screen_text_texture_, nullptr, &text_dest_rect);
        }
    }
    
    // --- Fade Overlays ---
    if (general_fade_alpha_ > 0.0f) { 
        if (current_phase_ == VPetBattlePhase::ENTERING_FADE_IN ||
            current_phase_ == VPetBattlePhase::TO_SELECTION_FADE_OUT ||
            current_phase_ == VPetBattlePhase::TO_SELECTION_FADE_IN ||
            current_phase_ == VPetBattlePhase::TO_PLAYER_REVEAL_FADE_OUT || // Added
            current_phase_ == VPetBattlePhase::TO_PLAYER_REVEAL_FADE_IN    // Added
            ) {
            display.setDrawBlendMode(SDL_BLENDMODE_BLEND); 
            display.setDrawColor(0, 0, 0, static_cast<Uint8>(general_fade_alpha_));
            SDL_Rect fullScreenRect = {0, 0, screen_width, screen_height};
            display.fillRect(&fullScreenRect); 
        }
    }
}
