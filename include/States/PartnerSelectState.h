// include/states/PartnerSelectState.h
#pragma once

#include "states/GameState.h"      // Include the base class definition
#include "ui/TextRenderer.h"       // <<< ADDED Include
#include "../entities/Digimon.h"   // Include DigimonType enum (path might need checking: entities/Digimon.h ?)
#include <SDL.h>                   // Includes SDL types
#include <vector>                  // For std::vector
#include <string>                  // For std::string
// #include <map>                  // No longer needed for fontCharMap_
#include <cstddef>                 // For size_t

// Forward declarations
class Game;
class InputManager; // Already Present / Correct
class PlayerData;   // Already Present / Correct
class PCDisplay;    // Already Present / Correct

class PartnerSelectState : public GameState {
public:
    // Constructor: Takes Game pointer, calls base constructor
    PartnerSelectState(Game* game); // Declaration OK

    // Destructor override
    ~PartnerSelectState() override; // Declaration OK

    // Core state functions override (Signatures are already correct)
    void handle_input(InputManager& inputManager, PlayerData* playerData) override; // OK
    void update(float delta_time, PlayerData* playerData) override;                // OK
    void render(PCDisplay& display) override;                                     // OK

private:
    // --- Member Variables ---
    size_t currentSelectionIndex_;
    std::vector<DigimonType> availablePartners_;
    SDL_Texture* backgroundTexture_;
    // SDL_Texture* fontTexture_; // <<< REMOVED
    // std::map<char, SDL_Rect> fontCharMap_; // <<< REMOVED

    // --- Private Helper Methods ---
    // (Removed font loading and basic text drawing helpers)
    // bool loadFontDataFromJson(const std::string& jsonPath); // <<< REMOVED
    // SDL_Point getTextDimensions(const std::string& text, int kerning); // <<< REMOVED
    // void drawText(SDL_Renderer* renderer, const std::string& text, int startX, int startY, float scale, int kerning); // <<< REMOVED

    // (Other specific helpers remain)
    void drawSelectedDigimon(PCDisplay& display);
    DigimonType getDigimonTypeFromIndex(size_t index) const;
    std::string getDigimonName(DigimonType type) const;

}; // End of PartnerSelectState class definition