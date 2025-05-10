// include/states/PartnerSelectState.h
#pragma once

#include "states/GameState.h"      
#include "ui/TextRenderer.h"       
#include "../entities/Digimon.h"   
#include "graphics/Animator.h"     
#include "Utils/AnimationUtils.h"  
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
    std::vector<DigimonType> availablePartners_;
    SDL_Texture* backgroundTexture_;
    Animator digimonAnimator_;
    bool returnToAdventure_ = false;

    // --- Private Helper Methods ---
    void drawDigimon(PCDisplay& display);    
    void updateDisplayedDigimon();
    DigimonType getDigimonTypeFromIndex(size_t index) const;
    std::string getDigimonName(DigimonType type) const;
    std::string getDigimonTextureId(DigimonType type) const;
    std::string getAnimationId(DigimonType type, const std::string& animName) const;
};