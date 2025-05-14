// File: src/states/PartnerSelectState.cpp

#include "states/PartnerSelectState.h" // Include own header
#include "core/Game.h"                 // For game_ptr, state changes, managers
#include "ui/TextRenderer.h"           // For TextRenderer class
#include "core/AssetManager.h"         // For assets
#include "core/InputManager.h"         // For InputManager&
#include "core/GameAction.h"           // For GameAction enum
#include "core/PlayerData.h"           // For PlayerData*
#include "platform/pc/pc_display.h"    // For PCDisplay& and rendering
#include "graphics/Animator.h"         // For digimonAnimator_
#include "graphics/AnimationData.h"    // For AnimationData struct
#include "core/AnimationManager.h"     // To get AnimationData
#include "utils/AnimationUtils.h"      // <<< ADDED THIS INCLUDE >>>
#include <SDL_log.h>
#include <stdexcept>                   // For runtime_error
// Removed fstream, map, nlohmann/json as they are not used directly for font loading anymore

PartnerSelectState::PartnerSelectState(Game* game) :
    GameState(game),
    currentSelectionIndex_(0),
    backgroundTexture_(nullptr),
    digimonAnimator_() // Default construct the animator
{
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PartnerSelectState Constructor Called.");
    if (!game_ptr) { throw std::runtime_error("PartnerSelectState requires a valid Game pointer!"); }
    if (!game_ptr->getAnimationManager()) { throw std::runtime_error("PartnerSelectState requires AnimationManager!");}
    if (!game_ptr->getTextRenderer()) { throw std::runtime_error("PartnerSelectState requires TextRenderer!");}

    availablePartners_ = {
        DIGI_AGUMON, DIGI_GABUMON, DIGI_BIYOMON, DIGI_GATOMON,
        DIGI_GOMAMON, DIGI_PALMON, DIGI_TENTOMON, DIGI_PATAMON
    };
    if (availablePartners_.empty()) {
        throw std::runtime_error("PartnerSelectState: No available partners defined!");
    }

    AssetManager* assets = game_ptr->getAssetManager();
    if (!assets) { throw std::runtime_error("PartnerSelectState requires a valid AssetManager!"); }

    backgroundTexture_ = assets->getTexture("menu_bg_blue");
    if (!backgroundTexture_) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"PartnerSelectState: Background texture not found!"); }

    PlayerData* pd = game_ptr->getPlayerData();
    if (pd) {
        DigimonType currentPartner = pd->currentPartner;
        for (size_t i = 0; i < availablePartners_.size(); ++i) {
            if (availablePartners_[i] == currentPartner) {
                currentSelectionIndex_ = i;
                break;
            }
        }
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PartnerSelectState: Initial selection set to index %zu based on PlayerData.", currentSelectionIndex_);
    } else {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"PartnerSelectState: Could not get PlayerData! Defaulting selection to index 0.");
        currentSelectionIndex_ = 0;
    }

    // Set the initially displayed Digimon animation
    updateDisplayedDigimon();

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PartnerSelectState Initialized.");
}

PartnerSelectState::~PartnerSelectState() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PartnerSelectState Destructor Called.");
}

void PartnerSelectState::enter() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "--- PartnerSelectState enter() CALLED ---");
    updateDisplayedDigimon(); // Refresh animation when entering
}

void PartnerSelectState::handle_input(InputManager& inputManager, PlayerData* playerData) {
    if (!game_ptr) return;

    if (inputManager.isActionJustPressed(GameAction::NAV_UP)) {
        currentSelectionIndex_ = (currentSelectionIndex_ == 0) ? 
            availablePartners_.size() - 1 : currentSelectionIndex_ - 1;
        updateDisplayedDigimon();
    } else if (inputManager.isActionJustPressed(GameAction::NAV_DOWN)) {
        currentSelectionIndex_ = (currentSelectionIndex_ + 1) % availablePartners_.size();
        updateDisplayedDigimon();
    }
    
    if (inputManager.isActionJustPressed(GameAction::CONFIRM)) {
        if (!availablePartners_.empty() && playerData && game_ptr) { // Added game_ptr check for safety
            playerData->currentPartner = availablePartners_[currentSelectionIndex_];
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                        "PartnerSelectState: CONFIRM selected %s. Game ptr: %p",
                        getDigimonName(playerData->currentPartner).c_str(),
                        (void*)game_ptr);

            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PartnerSelectState: Calling requestFadeToState(nullptr, 0.5f, true)");
            game_ptr->requestFadeToState(nullptr, 0.5f, true); // Pop PartnerSelectState

            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PartnerSelectState: Calling setTargetStateAfterFade(StateType::Adventure)");
            game_ptr->setTargetStateAfterFade(StateType::Adventure); // Target AdventureState after fade & pop
            
            // Log to see if targetStateAfterFade_ was set, if getTargetStateAfterFade() exists and is accessible
            // This is illustrative; direct access or a getter would be needed.
            // SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PartnerSelectState: game_ptr->targetStateAfterFade_ is now %d (expected Adventure)", (int)game_ptr->getTargetStateAfterFade());

        }
    } else if (inputManager.isActionJustPressed(GameAction::CANCEL)) {
        if (game_ptr) {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PartnerSelectState: CANCEL pressed. Requesting pop via fade.");
            game_ptr->requestFadeToState(nullptr, 0.3f, true); // Pop current, fade to underlying
        }
    }
}

void PartnerSelectState::update(float delta_time, PlayerData* playerData) {
    // Just update the animator without checking (it should handle null animations internally)
    digimonAnimator_.update(delta_time);
    
    // We've simplified the code by removing the fade transition logic
    // and directly popping to the adventure state from handle_input
}

void PartnerSelectState::render(PCDisplay& display) {
    SDL_Renderer* renderer = display.getRenderer();
    if (!renderer) return;

    int windowW = 0; int windowH = 0;
    display.getWindowSize(windowW, windowH);
    if (windowW <= 0 || windowH <= 0) { windowW = 466; windowH = 466; }

    if (backgroundTexture_) {
        SDL_RenderCopy(renderer, backgroundTexture_, NULL, NULL);
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 50, 0, 255);
        SDL_RenderClear(renderer);
    }

    drawDigimon(display);

    TextRenderer* textRenderer = game_ptr ? game_ptr->getTextRenderer() : nullptr;
    if (textRenderer && !availablePartners_.empty()) {
        DigimonType selectedType = availablePartners_[currentSelectionIndex_];
        std::string name = getDigimonName(selectedType);

        if (!name.empty()) {
            const float nameScale = 0.9f;
            const int nameKerning = -15;
            SDL_Point baseDimensions = textRenderer->getTextDimensions(name, nameKerning);
            if (baseDimensions.x > 0 && baseDimensions.y > 0) {
                int scaledW = static_cast<int>(static_cast<float>(baseDimensions.x) * nameScale);
                int scaledH = static_cast<int>(static_cast<float>(baseDimensions.y) * nameScale);
                int nameX = (windowW / 2) - (scaledW / 2);
                int nameY = (windowH * 2 / 3) - (scaledH / 2);
                textRenderer->drawText(renderer, name, nameX, nameY, nameScale, nameKerning);
            } else { /* Warn */ }
        }
    } else { /* Warn */ }
}

void PartnerSelectState::drawDigimon(PCDisplay& display) {
    SDL_Renderer* renderer = display.getRenderer();
    if (!renderer) return;

    int windowW = 0; int windowH = 0;
    display.getWindowSize(windowW, windowH);
    if (windowW <= 0 || windowH <= 0) { windowW = 466; windowH = 466; }

    SDL_Texture* currentTexture = digimonAnimator_.getCurrentTexture();
    SDL_Rect currentSourceRect = digimonAnimator_.getCurrentFrameRect();

    if (currentTexture && currentSourceRect.w > 0 && currentSourceRect.h > 0) {
        int drawX = (windowW / 2) - (currentSourceRect.w / 2);
        int drawY = (windowH / 2) - (currentSourceRect.h / 2) - 65;
        SDL_Rect dstRect = { drawX, drawY, currentSourceRect.w, currentSourceRect.h };
        display.drawTexture(currentTexture, &currentSourceRect, &dstRect);
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
        int w = 50, h = 50;
        int x = (windowW / 2) - (w / 2);
        int y = (windowH / 2) - (h / 2) - 30;
        SDL_Rect placeholder = {x, y, w, h};
        SDL_RenderFillRect(renderer, &placeholder);
    }
}

void PartnerSelectState::updateDisplayedDigimon() {
    if (availablePartners_.empty() || !game_ptr || !game_ptr->getAnimationManager()) {
        digimonAnimator_.stop();
        return;
    }

    DigimonType typeToDisplay = availablePartners_[currentSelectionIndex_];
    std::string animId = AnimationUtils::GetAnimationId(typeToDisplay, "Idle");
    AnimationManager* animManager = game_ptr->getAnimationManager();
    const AnimationData* animData = animManager->getAnimationData(animId);
    digimonAnimator_.setAnimation(animData);
}

DigimonType PartnerSelectState::getDigimonTypeFromIndex(size_t index) const {
    if (index < availablePartners_.size()) { return availablePartners_[index]; }
    return DIGI_AGUMON; // Fallback
}

std::string PartnerSelectState::getDigimonName(DigimonType type) const {
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

StateType PartnerSelectState::getType() const {
    return StateType::PartnerSelect;
}