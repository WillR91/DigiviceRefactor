// include/states/PartnerSelectState.h
#pragma once

#include "states/GameState.h"      
#include "ui/TextRenderer.h"       
#include "../entities/Digimon.h"   // Keep for legacy enum compatibility
#include "../entities/DigimonDefinition.h" // Add for DigimonDefinition
#include "../entities/DigimonRegistry.h"   // Add for DigimonRegistry
#include "graphics/Animator.h"     
#include "utils/AnimationUtils.h"  
#include <SDL.h>                   
#include <vector>                  
#include <string>                  
#include <cstddef>                 

// Forward declarations
class Game;
class InputManager;
class PlayerData;
class PCDisplay;

class PartnerSelectState : public GameState {
public:
    PartnerSelectState(Game* game);
    ~PartnerSelectState() override;

    void enter() override;
    void exit() override {};
    void handle_input(InputManager& inputManager, PlayerData* playerData) override;
    void update(float delta_time, PlayerData* playerData) override;
    void render(PCDisplay& display) override;
    StateType getType() const override;

private:
    // --- Member Variables ---
    size_t currentSelectionIndex_;
    std::vector<std::string> availablePartnerIds_; // Changed from DigimonType to string ID
    SDL_Texture* backgroundTexture_;
    Animator digimonAnimator_;
    bool returnToAdventure_ = false;

    // --- Private Helper Methods ---
    void drawDigimon(PCDisplay& display);    
    void updateDisplayedDigimon();
    std::string getPartnerIdFromIndex(size_t index) const; // Changed to return ID instead of enum
    std::string getDigimonName(const std::string& partnerId) const; // Changed to take ID
    std::string getAnimationId(const std::string& partnerId, const std::string& animName) const; // Changed to take ID
};