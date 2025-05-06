// File: src/states/PartnerSelectState.cpp

#include "states/PartnerSelectState.h" // Include own header
#include "core/Game.h"                 // For game_ptr, state changes, getTextRenderer
#include "ui/TextRenderer.h"           // For TextRenderer class
#include "core/AssetManager.h"         // For assets
#include "core/InputManager.h"         // For InputManager&
#include "core/GameAction.h"           // For GameAction enum
#include "core/PlayerData.h"           // For PlayerData*
#include "platform/pc/pc_display.h"    // For PCDisplay& and rendering
#include "graphics/Animation.h"        // Include if storing Animation objects directly
#include <SDL_log.h>
#include <stdexcept>                   // For runtime_error
// #include "vendor/nlohmann/json.hpp" // No longer needed for local font loading
// #include <fstream>                 // No longer needed for local font loading
// #include <map>                     // No longer needed for local fontCharMap_

// Use the nlohmann::json namespace
// using json = nlohmann::json; // No longer needed directly here

namespace { // Use anonymous namespace for file-local helpers if needed
    // TODO: Move animation creation helper to a shared utility or AnimationManager later
    // --- Animation Helper (Copied from AdventureState - NEEDS REFACTORING) ---
    Animation createAnimationFromIndices( SDL_Texture* texture, const std::vector<SDL_Rect>& allFrameRects,
                                          const std::vector<int>& indices, const std::vector<Uint32>& durations, bool loops)
    {
        Animation anim; anim.loops = loops;
        if (!texture) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "createAnimationFromIndices Error: Null texture."); return anim; }
        if (indices.size() != durations.size()) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "createAnimationFromIndices Error: Index/duration mismatch."); return anim; }
        if (allFrameRects.empty()) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "createAnimationFromIndices Error: Empty frame rects."); return anim; }
        for (size_t i = 0; i < indices.size(); ++i) {
            int frameIndex = indices[i]; Uint32 duration = durations[i];
            if (frameIndex >= 0 && static_cast<size_t>(frameIndex) < allFrameRects.size()) { anim.addFrame(SpriteFrame(texture, allFrameRects[frameIndex]), duration); }
            else { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "createAnimationFromIndices Error: Index %d out of bounds (%zu). Using frame 0.", frameIndex, allFrameRects.size()); anim.addFrame(SpriteFrame(texture, allFrameRects[0]), duration); }
        } return anim;
    }
    // --- Idle Animation Data (Copied from AdventureState - NEEDS REFACTORING) ---
    const std::vector<int> IDLE_INDICES = {0, 1};
    const std::vector<Uint32> IDLE_DURATIONS = {1000, 1000}; // Slow idle

} // end anonymous namespace


PartnerSelectState::PartnerSelectState(Game* game) :
    GameState(game),
    currentSelectionIndex_(0),
    backgroundTexture_(nullptr)
{
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PartnerSelectState Constructor Called.");
    if (!game_ptr) { throw std::runtime_error("PartnerSelectState requires a valid Game pointer!"); }

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

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PartnerSelectState Initialized.");
}

PartnerSelectState::~PartnerSelectState() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PartnerSelectState Destructor Called.");
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
        // TODO: Update animation based on new currentSelectionIndex_
    }

    if (inputManager.isActionJustPressed(GameAction::CONFIRM)) {
         if (!availablePartners_.empty() && playerData) {
             DigimonType selectedType = availablePartners_[currentSelectionIndex_];
             SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "PartnerSelect: Confirmed selection index %zu, type %d", currentSelectionIndex_, static_cast<int>(selectedType));
             playerData->currentPartner = selectedType;
             SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,"PartnerSelect: Updated PlayerData->currentPartner.");
             game_ptr->requestPopState();
         } else {
             if (!playerData) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"PartnerSelect: Cannot confirm, PlayerData is null!");
             if (availablePartners_.empty()) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"PartnerSelect: Cannot confirm, availablePartners_ is empty!");
         }
    }
    else if (inputManager.isActionJustPressed(GameAction::CANCEL)) {
        SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "PartnerSelect: Cancelled. Popping state.");
        game_ptr->requestPopState();
    }
}

void PartnerSelectState::update(float delta_time, PlayerData* playerData) {
    // TODO: Implement animation update logic
}

void PartnerSelectState::render(PCDisplay& display) {
    SDL_Renderer* top_level_renderer = display.getRenderer(); // Renamed to avoid conflict if snippet re-declares
    if (!top_level_renderer) return;

    // Get window size once at the top for general use, can be fetched again if needed
    int initialWindowW = 0; int initialWindowH = 0;
    display.getWindowSize(initialWindowW, initialWindowH);
    if (initialWindowW <= 0 || initialWindowH <= 0) { initialWindowW = 466; initialWindowH = 466; }


    // 1. Draw Background
    if (backgroundTexture_) {
        SDL_RenderCopy(top_level_renderer, backgroundTexture_, NULL, NULL);
    } else {
        SDL_SetRenderDrawColor(top_level_renderer, 0, 50, 0, 255);
        SDL_RenderClear(top_level_renderer);
    }

    // 2. Draw Digimon Placeholder/Sprite
    drawSelectedDigimon(display); 

    // 3. Draw Digimon Name Below Sprite
    // Snippet integration starts here, replacing the previous logic for this section
    TextRenderer* textRenderer = nullptr; // Initialize to nullptr
    if (game_ptr) { // Ensure game_ptr is valid before using it
        textRenderer = game_ptr->getTextRenderer(); // Get the central renderer
    }

    if (textRenderer && !availablePartners_.empty()) { // Check if renderer and partners are valid
        DigimonType selectedType = availablePartners_[currentSelectionIndex_];
        std::string name = getDigimonName(selectedType); // Use helper

        if (!name.empty()) {
            // --- Define desired scale and kerning for the name ---
            const float nameScale = 0.9f; // Example: Make name larger
            const int nameKerning = -15;    // Example: No extra kerning

            // --- Use TextRenderer to get dimensions ---
            // Assuming TextRenderer::getTextDimensions(name, kerning) 
            // and scale is applied manually after
            SDL_Point baseDimensions = textRenderer->getTextDimensions(name, nameKerning);

            if (baseDimensions.x > 0 && baseDimensions.y > 0) {
                // --- Calculate Scaled Dimensions ---
                int scaledW = static_cast<int>(static_cast<float>(baseDimensions.x) * nameScale);
                int scaledH = static_cast<int>(static_cast<float>(baseDimensions.y) * nameScale);

                // --- Calculate Centered Position ---
                // Get window size from the display object passed into render()
                // (using initialWindowW, initialWindowH fetched earlier is also fine)
                int windowW = 0; int windowH = 0;
                display.getWindowSize(windowW, windowH);
                if (windowW <= 0 || windowH <= 0) { windowW = 466; windowH = 466; } // Fallback

                int nameX = (windowW / 2) - (scaledW / 2); // Center horizontally
                // Position below the sprite placeholder (adjust Y coordinate as needed)
                int nameY = (windowH * 3 / 4) - (scaledH / 2); // Example: Lower quarter

                // --- Draw using TextRenderer ---
                SDL_Renderer* renderer_for_text = display.getRenderer(); // Get renderer from display (can use top_level_renderer too)
                if (renderer_for_text) {
                     textRenderer->drawText(renderer_for_text, name, nameX, nameY, nameScale, nameKerning);
                } else {
                     SDL_LogError(SDL_LOG_CATEGORY_RENDER, "PartnerSelectState Render: Could not get renderer from display for text!");
                }
            } else {
                 SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "PartnerSelectState Render: getTextDimensions returned zero size for name '%s'", name.c_str());
            }
        }
    } else {
        if (!textRenderer && game_ptr) SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "PartnerSelectState Render: TextRenderer is null!");
        else if (!game_ptr) SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "PartnerSelectState Render: game_ptr is null, cannot get TextRenderer!");
        if (availablePartners_.empty()) SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "PartnerSelectState Render: availablePartners_ is empty!");
    }
    // End of integrated snippet
}


// --- Private Helper Implementations ---

void PartnerSelectState::drawSelectedDigimon(PCDisplay& display) {
    SDL_Renderer* renderer = display.getRenderer();
    if (!renderer || availablePartners_.empty()) return;

    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    int w = 50, h = 50;
    int x = (466 / 2) - (w / 2); // Assuming fixed window size for now, should use actual windowW
    int y = (466 / 2) - (h / 2) - 30; // Assuming fixed window size for now, should use actual windowH
    SDL_Rect placeholder = {x, y, w, h};
    SDL_RenderFillRect(renderer, &placeholder);

    // TODO: Real Digimon sprite rendering using animation system
}

DigimonType PartnerSelectState::getDigimonTypeFromIndex(size_t index) const {
    if (index < availablePartners_.size()) {
        return availablePartners_[index];
    }
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "getDigimonTypeFromIndex: Index %zu out of bounds!", index);
    return DIGI_AGUMON;
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

// DELETED: PartnerSelectState::loadFontDataFromJson
// DELETED: PartnerSelectState::getTextDimensions
// DELETED: PartnerSelectState::drawText