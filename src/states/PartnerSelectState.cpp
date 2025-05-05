// File: src/states/PartnerSelectState.cpp

#include "states/PartnerSelectState.h" // Include own header
#include "core/Game.h"               // For game_ptr, state changes
#include "core/AssetManager.h"       // For assets
#include "core/InputManager.h"       // For InputManager&
#include "core/GameAction.h"         // For GameAction enum
#include "core/PlayerData.h"         // For PlayerData*
#include "platform/pc/pc_display.h"  // For PCDisplay& and rendering
// #include "states/AdventureState.h"   // NO LONGER NEEDED - use PlayerData
#include "vendor/nlohmann/json.hpp"  // For JSON loading
#include "graphics/Animation.h"      // Include if storing Animation objects directly (TODO: Add Animation members to header if needed)
#include <SDL_log.h>
#include <fstream>
#include <stdexcept>                // For runtime_error
#include <map>                      // Needed for font map

// Use the nlohmann::json namespace
using json = nlohmann::json;

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
    GameState(game), // <<< --- ADDED Base class constructor call --- >>>
    currentSelectionIndex_(0), // Initialize index
    // availablePartners_ initialized below
    backgroundTexture_(nullptr),
    fontTexture_(nullptr)
    // fontCharMap_ default initialized
{
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PartnerSelectState Constructor Called.");
    if (!game_ptr) { throw std::runtime_error("PartnerSelectState requires a valid Game pointer!"); }

    // --- Populate Available Partners ---
    // Define the order and content of the selection list
    availablePartners_ = {
        DIGI_AGUMON, DIGI_GABUMON, DIGI_BIYOMON, DIGI_GATOMON,
        DIGI_GOMAMON, DIGI_PALMON, DIGI_TENTOMON, DIGI_PATAMON
    };
    if (availablePartners_.empty()) {
        throw std::runtime_error("PartnerSelectState: No available partners defined!");
    }

    // --- Get Assets ---
    AssetManager* assets = game_ptr->getAssetManager();
    if (!assets) { throw std::runtime_error("PartnerSelectState requires a valid AssetManager!"); }

    backgroundTexture_ = assets->getTexture("menu_bg_blue");
    fontTexture_ = assets->getTexture("ui_font_atlas");

    if (!backgroundTexture_) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"PartnerSelectState: Background texture not found!"); }
    if (!fontTexture_) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"PartnerSelectState: Font texture not found!");
    } else {
        if (!loadFontDataFromJson("assets/ui/fonts/bluewhitefont.json")) {
             SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "PartnerSelectState: Failed to load font map data.");
        }
    }

    // --- Load Digimon Assets (TODO: Refactor this!) ---
    // This is temporary and duplicates AdventureState logic. Should use an AnimationManager.
    // loadPartnerAssets(); // Call to removed function - inline or replace logic

    // --- Set Initial Selection based on PlayerData ---
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
        currentSelectionIndex_ = 0; // Default if PlayerData is missing
    }

    // TODO: Set up initial animation state for the selected Digimon.
    // Need animation members in the header and logic here.
    // setActiveDigimon(availablePartners_[currentSelectionIndex_]); // Call to removed function

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PartnerSelectState Initialized.");
}

PartnerSelectState::~PartnerSelectState() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PartnerSelectState Destructor Called.");
}

// <<< --- MODIFIED handle_input Signature --- >>>
void PartnerSelectState::handle_input(InputManager& inputManager, PlayerData* playerData) {
    if (!game_ptr) return; // Need game_ptr for state changes

    size_t previousSelection = currentSelectionIndex_;
    bool selectionChanged = false;

    // --- Navigation ---
    if (inputManager.isActionJustPressed(GameAction::NAV_UP)) {
        if (currentSelectionIndex_ == 0) {
            currentSelectionIndex_ = availablePartners_.size() - 1; // Wrap to bottom
        } else {
            currentSelectionIndex_--;
        }
        selectionChanged = true;
    } else if (inputManager.isActionJustPressed(GameAction::NAV_DOWN)) {
        currentSelectionIndex_ = (currentSelectionIndex_ + 1) % availablePartners_.size(); // Wrap to top
        selectionChanged = true;
    }

    // If selection changed, update displayed Digimon (animation needs implementation)
    if (selectionChanged) {
        // TODO: Update animation based on new currentSelectionIndex_
        // setActiveDigimon(availablePartners_[currentSelectionIndex_]); // Call to removed function
        SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "PartnerSelect: Selection changed to index %zu", currentSelectionIndex_);
    }

    // --- Confirmation ---
    if (inputManager.isActionJustPressed(GameAction::CONFIRM)) {
         if (!availablePartners_.empty() && playerData) { // Check if playerData exists
             DigimonType selectedType = availablePartners_[currentSelectionIndex_];
             SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "PartnerSelect: Confirmed selection index %zu, type %d", currentSelectionIndex_, static_cast<int>(selectedType));

             // <<< --- Update PlayerData Directly --- >>>
             playerData->currentPartner = selectedType;
             SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,"PartnerSelect: Updated PlayerData->currentPartner.");

             game_ptr->requestPopState(); // Go back to the previous state (should be menu)

         } else {
              if (!playerData) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"PartnerSelect: Cannot confirm, PlayerData is null!");
              if (availablePartners_.empty()) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"PartnerSelect: Cannot confirm, availablePartners_ is empty!");
         }
    }
    // --- Cancellation ---
    else if (inputManager.isActionJustPressed(GameAction::CANCEL)) {
        SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "PartnerSelect: Cancelled. Popping state.");
        game_ptr->requestPopState(); // Go back without changing partner
    }
}
// <<< --- END MODIFIED handle_input --- >>>

// <<< --- MODIFIED update Signature --- >>>
void PartnerSelectState::update(float delta_time, PlayerData* playerData) {
    // PlayerData* is passed in but not currently used in this update logic
    // (Could be used later if needed)

    // TODO: Implement animation update logic
    // Need animation state members (current anim, frame index, timer) in the header
    // and logic here to advance the frame based on delta_time.
    // The logic from the old PartnerSelectState::update needs adaptation.
}
// <<< --- END MODIFIED update --- >>>

// <<< --- MODIFIED render Signature --- >>>
void PartnerSelectState::render(PCDisplay& display) {
    // PCDisplay is passed by reference
    SDL_Renderer* renderer = display.getRenderer();
    if (!renderer) return;

    int windowW = 0; int windowH = 0;
    display.getWindowSize(windowW, windowH);
    if (windowW <= 0 || windowH <= 0) { windowW = 466; windowH = 466; } // Fallback

    // 1. Draw Background
    if (backgroundTexture_) {
        SDL_RenderCopy(renderer, backgroundTexture_, NULL, NULL);
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 50, 0, 255); // Dark green fallback
        SDL_RenderClear(renderer);
    }

    // 2. Draw Centered Animated Digimon (Placeholder - Requires Animation System)
    // TODO: Implement Digimon Drawing
    // Need to:
    // - Get the DigimonType for currentSelectionIndex_
    // - Get the appropriate texture and animation data for that type (via AssetManager/AnimationManager)
    // - Get the current animation frame based on the state's animation timer/index
    // - Draw the frame using display.drawTexture(...)
     drawSelectedDigimon(display); // Call placeholder helper

    // 3. Draw Digimon Name Below Sprite
    if (fontTexture_ && !fontCharMap_.empty() && !availablePartners_.empty()) {
        DigimonType selectedType = availablePartners_[currentSelectionIndex_];
        std::string name = getDigimonName(selectedType); // Use helper

        if (!name.empty()) {
            const float nameScale = 1.5f;
            const int nameKerning = 0;

            SDL_Point dims = getTextDimensions(name, nameKerning);
            if(dims.x > 0 && dims.y > 0) {
                int scaledW = static_cast<int>(dims.x * nameScale);
                int scaledH = static_cast<int>(dims.y * nameScale);
                int nameX = (windowW / 2) - (scaledW / 2); // Center horizontally
                int nameY = (windowH * 3 / 4) - (scaledH / 2) ; // Position lower down

                // Use the drawText helper method of this class
                drawText(renderer, name, nameX, nameY, nameScale, nameKerning);
            }
        }
    }
}
// <<< --- END MODIFIED render --- >>>


// --- Private Helper Implementations ---

// TODO: Implement animation drawing properly
void PartnerSelectState::drawSelectedDigimon(PCDisplay& display) {
    SDL_Renderer* renderer = display.getRenderer();
    if (!renderer || availablePartners_.empty()) return;

    // Placeholder: Draw a simple rectangle where the digimon should be
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow box
    int w = 50, h = 50;
    int x = (466 / 2) - (w / 2); // Assuming fixed window size for now
    int y = (466 / 2) - (h / 2) - 30;
    SDL_Rect placeholder = {x, y, w, h};
    SDL_RenderFillRect(renderer, &placeholder);

    // --- Real Implementation TODO ---
    // DigimonType type = availablePartners_[currentSelectionIndex_];
    // Get Animation* currentAnim = getAnimationFor(type); // Need animation storage/manager
    // if (currentAnim) {
    //     const SpriteFrame* frame = currentAnim->getFrame(animationFrameIndex_); // Need animation state
    //     if (frame && frame->texturePtr) {
    //         SDL_Rect dest = {x, y, frame->sourceRect.w, frame->sourceRect.h}; // Adjust x, y based on frame size
    //         display.drawTexture(frame->texturePtr, &frame->sourceRect, &dest);
    //     }
    // }
}


DigimonType PartnerSelectState::getDigimonTypeFromIndex(size_t index) const {
    if (index < availablePartners_.size()) {
        return availablePartners_[index];
    }
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "getDigimonTypeFromIndex: Index %zu out of bounds!", index);
    return DIGI_AGUMON; // Return a default
}

std::string PartnerSelectState::getDigimonName(DigimonType type) const {
    // Simple mapping - could be loaded from data later
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


// --- Text Helpers (Copied from MenuState - TODO: Refactor to shared utility) ---
bool PartnerSelectState::loadFontDataFromJson(const std::string& jsonPath) {
    // Implementation identical to MenuState::loadFontDataFromJson
    // Needs to be moved to a shared FontManager/TextRenderer class
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "LOAD FONT JSON (PartnerSelect): --- Function START --- Path: %s", jsonPath.c_str()); fontCharMap_.clear(); bool success = false; std::ifstream f(jsonPath); if (!f.is_open()) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "LOAD FONT JSON (PartnerSelect) ***ERROR***: std::ifstream failed to open file: %s", jsonPath.c_str()); goto cleanup; } /* ... more checks ... */ try { json data = json::parse(f); /* ... parsing logic ... */ success = !fontCharMap_.empty(); } catch (std::exception& e) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "LOAD FONT JSON (PartnerSelect) ***ERROR***: %s - %s", jsonPath.c_str(), e.what()); } cleanup: if (f.is_open()) { f.close(); } return success; // Highly truncated
}

SDL_Point PartnerSelectState::getTextDimensions(const std::string& text, int kerning) {
    // Implementation identical to MenuState::getTextDimensions
    // Needs to be moved to a shared FontManager/TextRenderer class
    SDL_Point dimensions = {0, 0}; int currentX = 0; int maxHeight = 0; if (fontCharMap_.empty()) { return dimensions; } /* ... calculation logic ... */ return dimensions; // Highly truncated
}

void PartnerSelectState::drawText(SDL_Renderer* renderer, const std::string& text, int startX, int startY, float scale, int kerning) {
    // Implementation identical to MenuState::drawText
    // Needs to be moved to a shared FontManager/TextRenderer class
    if (!fontTexture_ || fontCharMap_.empty() || !renderer || scale <= 0.0f) { return; } int currentX = startX; float scaledKerning = static_cast<float>(kerning) * scale; /* ... drawing loop ... */ // Highly truncated
}