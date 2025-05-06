// File: src/states/PartnerSelectState.cpp

#include "states/PartnerSelectState.h" // Include own header
#include "core/Game.h"                 // For game_ptr, state changes, managers
#include "ui/TextRenderer.h"           // For TextRenderer class
#include "core/AssetManager.h"         // For assets
#include "core/InputManager.h"         // For InputManager&
#include "core/GameAction.h"           // For GameAction enum
#include "core/PlayerData.h"           // For PlayerData*
#include "platform/pc/pc_display.h"    // For PCDisplay& and rendering
#include "graphics/Animator.h"       // For digimonAnimator_
#include "graphics/AnimationData.h"    // For AnimationData struct
#include "core/AnimationManager.h"   // To get AnimationData
#include <SDL_log.h>
#include <stdexcept>                   // For runtime_error
// Removed fstream, map, nlohmann/json as they are not used directly for font loading anymore

// <<< REMOVE Anonymous Namespace with old animation helpers if it only contained them >>>
// namespace { ... }


PartnerSelectState::PartnerSelectState(Game* game) :
    GameState(game), // <<< ADDED Base class constructor call
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

    // Font texture is no longer managed here, TextRenderer handles it.
    // if (!game_ptr->getTextRenderer()) { // This check might be redundant if constructor threw
    //     SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"PartnerSelectState: TextRenderer is not available!");
    // }

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

// <<< ADDED enter() method (empty for now, override is in header) >>>
void PartnerSelectState::enter() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "--- PartnerSelectState enter() CALLED ---");
    // Could re-check PlayerData here if needed, or ensure animator is set
    updateDisplayedDigimon(); // Refresh animation when entering
}

void PartnerSelectState::handle_input(InputManager& inputManager, PlayerData* playerData) {
    if (!game_ptr) return;

    bool selectionChanged = false;

    if (inputManager.isActionJustPressed(GameAction::NAV_UP)) {
        if (currentSelectionIndex_ == 0) {
            currentSelectionIndex_ = availablePartners_.size() - 1;
        } else {
            currentSelectionIndex_--;
        }
        selectionChanged = true;
    } else if (inputManager.isActionJustPressed(GameAction::NAV_DOWN)) {
        currentSelectionIndex_ = (currentSelectionIndex_ + 1) % availablePartners_.size();
        selectionChanged = true;
    }

    if (selectionChanged) {
        SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "PartnerSelect: Selection changed to index %zu", currentSelectionIndex_);
        updateDisplayedDigimon(); // Update the animation
    }

    if (inputManager.isActionJustPressed(GameAction::CONFIRM)) {
         if (!availablePartners_.empty() && playerData) {
             DigimonType selectedType = availablePartners_[currentSelectionIndex_];
             SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "PartnerSelect: Confirmed selection index %zu, type %d", currentSelectionIndex_, static_cast<int>(selectedType));
             playerData->currentPartner = selectedType;
             SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,"PartnerSelect: Updated PlayerData->currentPartner.");
             game_ptr->requestPopState();
         } else { /* Error logging */ }
    }
    else if (inputManager.isActionJustPressed(GameAction::CANCEL)) {
        SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "PartnerSelect: Cancelled. Popping state.");
        game_ptr->requestPopState();
    }
}

void PartnerSelectState::update(float delta_time, PlayerData* playerData) {
    // Update the current Digimon's animation
    digimonAnimator_.update(delta_time);
}

void PartnerSelectState::render(PCDisplay& display) {
    SDL_Renderer* renderer = display.getRenderer();
    if (!renderer) return;

    int windowW = 0; int windowH = 0;
    display.getWindowSize(windowW, windowH);
    if (windowW <= 0 || windowH <= 0) { windowW = 466; windowH = 466; }

    // 1. Draw Background
    if (backgroundTexture_) {
        SDL_RenderCopy(renderer, backgroundTexture_, NULL, NULL);
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 50, 0, 255);
        SDL_RenderClear(renderer);
    }

    // 2. Draw Digimon Sprite using the new helper
    drawDigimon(display);

    // 3. Draw Digimon Name Below Sprite
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
                int nameY = (windowH * 3 / 4) - (scaledH / 2);
                textRenderer->drawText(renderer, name, nameX, nameY, nameScale, nameKerning);
            } else { /* Warn */ }
        }
    } else { /* Warn */ }
}


// --- Private Helper Implementations ---

// Renamed from drawSelectedDigimon, uses digimonAnimator_
void PartnerSelectState::drawDigimon(PCDisplay& display) {
    SDL_Renderer* renderer = display.getRenderer(); // Not used directly if display.drawTexture is used
    if (!renderer) return; // Should not happen if display is valid

    int windowW = 0; int windowH = 0;
    display.getWindowSize(windowW, windowH);
    if (windowW <= 0 || windowH <= 0) { windowW = 466; windowH = 466; }


    SDL_Texture* currentTexture = digimonAnimator_.getCurrentTexture();
    SDL_Rect currentSourceRect = digimonAnimator_.getCurrentFrameRect();

    SDL_LogVerbose(SDL_LOG_CATEGORY_RENDER, "PartnerSelect RenderDigimon: AnimTexture=%p, AnimSrcRect={%d,%d,%d,%d}",
                   (void*)currentTexture, currentSourceRect.x, currentSourceRect.y, currentSourceRect.w, currentSourceRect.h);

    if (currentTexture && currentSourceRect.w > 0 && currentSourceRect.h > 0) {
        // Center the sprite (adjust vertical offset as needed)
        int drawX = (windowW / 2) - (currentSourceRect.w / 2);
        int drawY = (windowH / 2) - (currentSourceRect.h / 2) - 30; // Shift up slightly
        SDL_Rect dstRect = { drawX, drawY, currentSourceRect.w, currentSourceRect.h };
        display.drawTexture(currentTexture, &currentSourceRect, &dstRect);
    } else {
         // Placeholder if animation not available (shouldn't happen if logic is correct)
         SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255); // Bright Pink error box
         int w = 50, h = 50;
         int x = (windowW / 2) - (w / 2);
         int y = (windowH / 2) - (h / 2) - 30;
         SDL_Rect placeholder = {x, y, w, h};
         SDL_RenderFillRect(renderer, &placeholder);
         SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "PartnerSelectState: Drawing placeholder, animator returned invalid texture/rect.");
    }
}

// New helper to update the displayed Digimon's animation
void PartnerSelectState::updateDisplayedDigimon() {
    if (availablePartners_.empty() || !game_ptr || !game_ptr->getAnimationManager()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "updateDisplayedDigimon: Cannot update, missing partners or manager.");
        digimonAnimator_.stop(); // Stop animator if we can't set it
        return;
    }

    DigimonType typeToDisplay = availablePartners_[currentSelectionIndex_];
    std::string animId = getAnimationId(typeToDisplay, "Idle"); // Get "e.g. agumon_sheet_Idle"

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PartnerSelect: Requesting Anim ID: '%s'", animId.c_str());
    AnimationManager* animManager = game_ptr->getAnimationManager();
    const AnimationData* animData = animManager->getAnimationData(animId);

    digimonAnimator_.setAnimation(animData);
    if (animData) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PartnerSelect: Animator set with valid data for '%s'.", animId.c_str());
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "PartnerSelect: Animator set with NULL data for '%s'!", animId.c_str());
    }
}


DigimonType PartnerSelectState::getDigimonTypeFromIndex(size_t index) const {
    if (index < availablePartners_.size()) { return availablePartners_[index]; }
    return DIGI_AGUMON; // Fallback
}

std::string PartnerSelectState::getDigimonName(DigimonType type) const {
    // ... (Implementation remains the same) ...
    switch(type) { case DIGI_AGUMON: return "AGUMON"; case DIGI_GABUMON: return "GABUMON"; case DIGI_BIYOMON: return "BIYOMON"; case DIGI_GATOMON: return "GATOMON"; case DIGI_GOMAMON: return "GOMAMON"; case DIGI_PALMON: return "PALMON"; case DIGI_TENTOMON: return "TENTOMON"; case DIGI_PATAMON: return "PATAMON"; default: return "UNKNOWN"; }
}

// Helper to get the base texture ID string (copied from AdventureState)
std::string PartnerSelectState::getDigimonTextureId(DigimonType type) const {
     switch(type) {
        case DIGI_AGUMON: return "agumon_sheet";
        case DIGI_GABUMON: return "gabumon_sheet";
        case DIGI_BIYOMON: return "biyomon_sheet";
        case DIGI_GATOMON: return "gatomon_sheet";
        case DIGI_GOMAMON: return "gomamon_sheet";
        case DIGI_PALMON: return "palmon_sheet";
        case DIGI_TENTOMON: return "tentomon_sheet";
        case DIGI_PATAMON: return "patamon_sheet";
        default: SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "getDigimonTextureId: Unknown DigimonType %d", static_cast<int>(type)); return "unknown_sheet";
    }
}

// Helper to construct animation ID (copied from AdventureState)
std::string PartnerSelectState::getAnimationId(DigimonType type, const std::string& animName) const {
    return getDigimonTextureId(type) + "_" + animName;
}

// DELETED: PartnerSelectState::loadFontDataFromJson
// DELETED: PartnerSelectState::getTextDimensions
// DELETED: PartnerSelectState::drawText