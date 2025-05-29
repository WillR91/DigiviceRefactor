#include "states/EnemyTestState.h"
#include "core/Game.h"
#include "core/InputManager.h"
#include "core/GameAction.h"
#include "core/AssetManager.h"
#include "core/AnimationManager.h"
#include "graphics/AnimationData.h"
#include "ui/TextRenderer.h"
#include "platform/pc/pc_display.h"
#include "utils/AnimationUtils.h"
#include "entities/DigimonRegistry.h"
#include <SDL_log.h>

EnemyTestState::EnemyTestState(Game* game) :
    GameState(game),
    backgroundTexture_(nullptr),
    enemyAnimator_(),
    currentEnemyIndex_(0),
    currentAnimationType_("Idle")
{
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "EnemyTestState Constructor Called");
    
    if (!game_ptr) { 
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "EnemyTestState requires a valid Game pointer!");
        return;
    }
    
    // Get all enemy Digimon from the registry instead of hardcoding
    if (game_ptr->getDigimonRegistry()) {
        auto enemyDefs = game_ptr->getDigimonRegistry()->getDefinitionsByClass(Digimon::DigimonClass::StandardEnemy);
        auto bossDefs = game_ptr->getDigimonRegistry()->getDefinitionsByClass(Digimon::DigimonClass::Boss);
        
        // Add IDs from both enemy and boss classes
        enemyDigimonIds_.clear();
        for (const auto& def : enemyDefs) {
            if (def) {
                enemyDigimonIds_.push_back(def->id);
            }
        }
        for (const auto& def : bossDefs) {
            if (def) {
                enemyDigimonIds_.push_back(def->id);
            }
        }
        
        // Sort alphabetically for easier navigation
        std::sort(enemyDigimonIds_.begin(), enemyDigimonIds_.end());
        
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "EnemyTestState: Loaded %zu enemy Digimon for testing", enemyDigimonIds_.size());
    } else {
        // Fallback to just kuwagamon if registry isn't available
        enemyDigimonIds_ = {"kuwagamon"};
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "EnemyTestState: DigimonRegistry not available, using default enemy");
    }
      AssetManager* assets = game_ptr->getAssetManager();
    if (assets) {
        backgroundTexture_ = assets->requestTexture("menu_bg_blue");
        if (!backgroundTexture_) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "EnemyTestState: Background texture not found!");
        }
    }
    
    // Set up the initial enemy Digimon
    updateDisplayedEnemy();
}

EnemyTestState::~EnemyTestState() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "EnemyTestState Destructor Called");
}

StateType EnemyTestState::getType() const {
    // You can define a new StateType for this or reuse an existing one
    return StateType::Debug; // Assuming you have a Debug state type
}

void EnemyTestState::enter() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "--- EnemyTestState enter() ---");
    updateDisplayedEnemy();
}

void EnemyTestState::exit() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "--- EnemyTestState exit() ---");
}

void EnemyTestState::handle_input(InputManager& inputManager, PlayerData* playerData) {
    if (!game_ptr) return;

    // Previous enemy
    if (inputManager.isActionJustPressed(GameAction::NAV_LEFT)) {
        if (currentEnemyIndex_ == 0) {
            currentEnemyIndex_ = enemyDigimonIds_.size() - 1;
        } else {
            currentEnemyIndex_--;
        }
        updateDisplayedEnemy();
    }
    
    // Next enemy
    if (inputManager.isActionJustPressed(GameAction::NAV_RIGHT)) {
        currentEnemyIndex_ = (currentEnemyIndex_ + 1) % enemyDigimonIds_.size();
        updateDisplayedEnemy();
    }
    
    // Toggle animation type (IDLE or ATTACK)
    if (inputManager.isActionJustPressed(GameAction::NAV_UP) || 
        inputManager.isActionJustPressed(GameAction::NAV_DOWN)) {
        currentAnimationType_ = (currentAnimationType_ == "Idle") ? "Attack" : "Idle";
        updateDisplayedEnemy();
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, 
                    "EnemyTestState: Changed animation to '%s' for enemy '%s'", 
                    currentAnimationType_.c_str(), getCurrentEnemyName().c_str());
    }
    
    // Exit test screen
    if (inputManager.isActionJustPressed(GameAction::CANCEL) || 
        inputManager.isActionJustPressed(GameAction::CONFIRM)) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "EnemyTestState: Exiting test screen");
        game_ptr->requestPopState();
    }
}

void EnemyTestState::update(float delta_time, PlayerData* playerData) {
    // Update the animator
    enemyAnimator_.update(delta_time);
}

void EnemyTestState::render(PCDisplay& display) {
    SDL_Renderer* renderer = display.getRenderer();
    if (!renderer) return;

    int windowW = 0, windowH = 0;
    display.getWindowSize(windowW, windowH);
    if (windowW <= 0 || windowH <= 0) {
        windowW = 466;
        windowH = 466;
    }

    // Render background
    if (backgroundTexture_) {
        SDL_RenderCopy(renderer, backgroundTexture_, NULL, NULL);
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 0, 60, 255); // Dark blue background
        SDL_RenderClear(renderer);
    }
    
    // Render enemy Digimon
    SDL_Texture* currentTexture = enemyAnimator_.getCurrentTexture();
    SDL_Rect currentSourceRect = enemyAnimator_.getCurrentFrameRect();
      if (currentTexture && currentSourceRect.w > 0 && currentSourceRect.h > 0) {
        // Scale sprites 2x bigger
        int scaledWidth = currentSourceRect.w * 2;
        int scaledHeight = currentSourceRect.h * 2;
        // Center the Digimon on screen
        int drawX = (windowW / 2) - (scaledWidth / 2);
        int drawY = (windowH / 2) - (scaledHeight / 2) - 30;
        SDL_Rect dstRect = { drawX, drawY, scaledWidth, scaledHeight };
        display.drawTexture(currentTexture, &currentSourceRect, &dstRect);
    } else {
        // Draw placeholder if texture not available
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
        int w = 64, h = 64;
        int x = (windowW / 2) - (w / 2);
        int y = (windowH / 2) - (h / 2);
        SDL_Rect placeholder = {x, y, w, h};
        SDL_RenderFillRect(renderer, &placeholder);
    }
    
    // Render info text
    TextRenderer* textRenderer = game_ptr ? game_ptr->getTextRenderer() : nullptr;
    if (textRenderer) {
        // Display enemy name and ID
        std::string enemyName = getCurrentEnemyName();
        std::string enemyId = enemyDigimonIds_[currentEnemyIndex_];
        float nameScale = 1.0f;
        int nameKerning = -2;
        textRenderer->drawText(renderer, enemyName, 20, 20, nameScale, nameKerning);
        textRenderer->drawText(renderer, "ID: " + enemyId, 20, 45, 0.7f, nameKerning);
        
        // Display current animation type
        std::string animText = "ANIM: " + currentAnimationType_;
        textRenderer->drawText(renderer, animText, 20, 70, nameScale, nameKerning);
          // Display controls - FIXED: using dashes instead of pipe characters
        std::string controls = "LEFT/RIGHT: Change Enemy - UP/DOWN: Toggle Animation - ESC: Exit";
        float controlScale = 0.5f;
        textRenderer->drawText(renderer, controls, 20, windowH - 30, controlScale, nameKerning);
        
        // Display enemy count
        std::string countText = std::to_string(currentEnemyIndex_ + 1) + " / " + 
                               std::to_string(enemyDigimonIds_.size());
        textRenderer->drawText(renderer, countText, windowW - 70, 20, 0.7f, nameKerning);
    }
}

void EnemyTestState::updateDisplayedEnemy() {
    if (enemyDigimonIds_.empty() || !game_ptr || !game_ptr->getAnimationManager() || 
        !game_ptr->getDigimonRegistry()) {
        enemyAnimator_.stop();
        return;
    }

    std::string enemyId = enemyDigimonIds_[currentEnemyIndex_];
    Digimon::DigimonRegistry* registry = game_ptr->getDigimonRegistry();
    const Digimon::DigimonDefinition* enemyDef = registry->getDefinitionById(enemyId);
    
    if (!enemyDef) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                     "EnemyTestState: No definition found for enemy ID: %s", 
                     enemyId.c_str());
        enemyAnimator_.stop();
        return;
    }
    
    // Get appropriate animation
    std::string animId = AnimationUtils::GetAnimationId(enemyDef->spriteBaseId, currentAnimationType_);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, 
                "EnemyTestState: Setting animation '%s' for enemy '%s'", 
                animId.c_str(), enemyId.c_str());
    
    AnimationManager* animManager = game_ptr->getAnimationManager();
    const AnimationData* animData = animManager->getAnimationData(animId);
    
    if (animData) {
        enemyAnimator_.setAnimation(animData);
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                    "EnemyTestState: Animation data not found for '%s'", 
                    animId.c_str());
        enemyAnimator_.stop();
    }
}

std::string EnemyTestState::getCurrentEnemyName() const {
    if (enemyDigimonIds_.empty() || !game_ptr || !game_ptr->getDigimonRegistry()) {
        return "Unknown";
    }
    
    std::string enemyId = enemyDigimonIds_[currentEnemyIndex_];
    const Digimon::DigimonDefinition* def = game_ptr->getDigimonRegistry()->getDefinitionById(enemyId);
    
    if (def) {
        return def->displayName;
    }
    
    return enemyId; // Fallback to ID if no definition found
}
