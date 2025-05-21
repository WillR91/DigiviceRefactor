#include "states/PlayerTestState.h"
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
#include <algorithm>

PlayerTestState::PlayerTestState(Game* game) :
    GameState(game),
    backgroundTexture_(nullptr),
    playerAnimator_(),
    currentDigimonIndex_(0),
    currentAnimationType_("Idle")
{
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PlayerTestState Constructor Called");
    
    if (!game_ptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "PlayerTestState requires a valid Game pointer!");
        return;
    }
    
    // Get all player Digimon from the registry
    if (game_ptr->getDigimonRegistry()) {
        auto rookies = game_ptr->getDigimonRegistry()->getDefinitionsByClass(Digimon::DigimonClass::PlayerRookie);
        auto champions = game_ptr->getDigimonRegistry()->getDefinitionsByClass(Digimon::DigimonClass::PlayerChampion);
        
        // Add IDs from both classes
        playerDigimonIds_.clear();
        for (const auto& def : rookies) {
            if (def) {
                playerDigimonIds_.push_back(def->id);
            }
        }
        for (const auto& def : champions) {
            if (def) {
                playerDigimonIds_.push_back(def->id);
            }
        }
        
        // If no player Digimon found, add hardcoded defaults
        if (playerDigimonIds_.empty()) {
            playerDigimonIds_ = {"agumon", "gabumon", "biyomon", "gatomon", "gomamon", "palmon", "tentomon", "patamon"};
        }
        
        // Sort alphabetically for easier navigation
        std::sort(playerDigimonIds_.begin(), playerDigimonIds_.end());
        
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PlayerTestState: Loaded %zu player Digimon for testing", playerDigimonIds_.size());
    } else {
        // Fallback to defaults if registry isn't available
        playerDigimonIds_ = {"agumon", "gabumon", "biyomon", "gatomon", "gomamon", "palmon", "tentomon", "patamon"};
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "PlayerTestState: DigimonRegistry not available, using default player Digimon");
    }
    
    AssetManager* assets = game_ptr->getAssetManager();
    if (assets) {
        backgroundTexture_ = assets->getTexture("menu_bg_blue");
        if (!backgroundTexture_) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "PlayerTestState: Background texture not found!");
        }
    }
    
    // Set up the initial player Digimon
    updateDisplayedDigimon();
}

PlayerTestState::~PlayerTestState() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PlayerTestState Destructor Called");
}

StateType PlayerTestState::getType() const {
    return StateType::Debug;
}

void PlayerTestState::enter() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "--- PlayerTestState enter() ---");
}

void PlayerTestState::exit() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "--- PlayerTestState exit() ---");
}

void PlayerTestState::handle_input(InputManager& inputManager, PlayerData* playerData) {
    if (!game_ptr) return;
    
    // Exit state on ESC press
    if (inputManager.isActionJustPressed(GameAction::CANCEL)) {
        game_ptr->requestPopState();
        return;
    }
    
    // Cycle through Digimon with LEFT/RIGHT
    if (inputManager.isActionJustPressed(GameAction::NAV_LEFT)) {
        if (currentDigimonIndex_ > 0) {
            currentDigimonIndex_--;
        } else {
            currentDigimonIndex_ = playerDigimonIds_.size() - 1;
        }
        updateDisplayedDigimon();
    } else if (inputManager.isActionJustPressed(GameAction::NAV_RIGHT)) {
        currentDigimonIndex_ = (currentDigimonIndex_ + 1) % playerDigimonIds_.size();
        updateDisplayedDigimon();
    }
    
    // Toggle animation type with UP/DOWN
    if (inputManager.isActionJustPressed(GameAction::NAV_UP) || 
        inputManager.isActionJustPressed(GameAction::NAV_DOWN)) {
        // Cycle between available animations: Idle, Walk, Attack
        if (currentAnimationType_ == "Idle") {
            currentAnimationType_ = "Walk";
        } else if (currentAnimationType_ == "Walk") {
            currentAnimationType_ = "Attack";
        } else {
            currentAnimationType_ = "Idle";
        }
        updateDisplayedDigimon();
    }
}

void PlayerTestState::update(float delta_time, PlayerData* playerData) {
    playerAnimator_.update(delta_time);
}

void PlayerTestState::render(PCDisplay& display) {
    SDL_Renderer* renderer = display.getRenderer();
    if (!renderer) return;
    
    // Draw background
    if (backgroundTexture_) {
        SDL_RenderCopy(renderer, backgroundTexture_, NULL, NULL);
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 0, 80, 255);
        SDL_RenderClear(renderer);
    }
      int windowW = 0, windowH = 0;
    display.getWindowSize(windowW, windowH);
    
    // Draw the Digimon at the center of the screen
    SDL_Texture* currentTexture = playerAnimator_.getCurrentTexture();
    SDL_Rect currentSourceRect = playerAnimator_.getCurrentFrameRect();
    
    if (currentTexture && currentSourceRect.w > 0 && currentSourceRect.h > 0) {
        // Center the Digimon on screen
        int drawX = (windowW / 2) - (currentSourceRect.w / 2);
        int drawY = (windowH / 2) - (currentSourceRect.h / 2) - 30;
        SDL_Rect dstRect = { drawX, drawY, currentSourceRect.w, currentSourceRect.h };
        display.drawTexture(currentTexture, &currentSourceRect, &dstRect);
    }
    
    // Draw information text
    TextRenderer* textRenderer = game_ptr ? game_ptr->getTextRenderer() : nullptr;
    if (textRenderer) {
        // Draw Digimon name and current animation at the top
        std::string digimonName = getCurrentDigimonName();
        std::string title = digimonName + " - " + currentAnimationType_ + " Animation";
        float titleScale = 0.8f;
        int titleKerning = -2;
        textRenderer->drawText(renderer, title, 20, 20, titleScale, titleKerning);
        
        // Draw controls at the bottom
        std::string controls = "LEFT/RIGHT: Change Digimon - UP/DOWN: Toggle Animation - ESC: Exit";
        float controlScale = 0.5f;
        int nameKerning = -2;
        textRenderer->drawText(renderer, controls, 20, windowH - 30, controlScale, nameKerning);
        
        // Display Digimon count
        std::string countText = std::to_string(currentDigimonIndex_ + 1) + " / " + 
                               std::to_string(playerDigimonIds_.size());
        textRenderer->drawText(renderer, countText, windowW - 70, 20, 0.7f, nameKerning);
    }
}

void PlayerTestState::updateDisplayedDigimon() {
    if (playerDigimonIds_.empty() || !game_ptr || !game_ptr->getAnimationManager()) {
        playerAnimator_.stop();
        return;
    }

    std::string digimonId = playerDigimonIds_[currentDigimonIndex_];
    
    // First, try to get the definition from registry
    const Digimon::DigimonDefinition* digimonDef = nullptr;
    if (game_ptr->getDigimonRegistry()) {
        digimonDef = game_ptr->getDigimonRegistry()->getDefinitionById(digimonId);
    }
    
    // Set the proper sprite base ID
    std::string spriteBaseId = digimonId;
    if (digimonDef && !digimonDef->spriteBaseId.empty()) {
        spriteBaseId = digimonDef->spriteBaseId;
    }
    
    // Get appropriate animation
    std::string animId = AnimationUtils::GetAnimationId(spriteBaseId, currentAnimationType_);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, 
                "PlayerTestState: Setting animation '%s' for Digimon '%s'", 
                animId.c_str(), digimonId.c_str());
    
    AnimationManager* animManager = game_ptr->getAnimationManager();
    const AnimationData* animData = animManager->getAnimationData(animId);
    
    if (animData) {
        playerAnimator_.setAnimation(animData);
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                    "PlayerTestState: Animation data not found for '%s'", 
                    animId.c_str());
        playerAnimator_.stop();
    }
}

std::string PlayerTestState::getCurrentDigimonName() const {
    if (currentDigimonIndex_ < playerDigimonIds_.size()) {
        std::string digimonId = playerDigimonIds_[currentDigimonIndex_];
        
        // Try to get display name from registry
        if (game_ptr && game_ptr->getDigimonRegistry()) {
            const Digimon::DigimonDefinition* def = 
                game_ptr->getDigimonRegistry()->getDefinitionById(digimonId);
            
            if (def && !def->displayName.empty()) {
                return def->displayName;
            }
        }
        
        // Fallback to capitalized ID
        std::string displayName = digimonId;
        if (!displayName.empty()) {
            displayName[0] = std::toupper(displayName[0]);
        }
        return displayName;
    }
    return "Unknown";
}
