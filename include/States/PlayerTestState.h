#ifndef PLAYER_TEST_STATE_H
#define PLAYER_TEST_STATE_H

#include "states/GameState.h"
#include "graphics/Animator.h"

// Forward declarations
class Game;
class InputManager;
class PlayerData;
class PCDisplay;

// A test state for viewing player Digimon animations
class PlayerTestState : public GameState {
public:
    PlayerTestState(Game* game);
    ~PlayerTestState();

    void enter() override;
    void exit() override;
    void handle_input(InputManager& inputManager, PlayerData* playerData) override;
    void update(float delta_time, PlayerData* playerData) override;
    void render(PCDisplay& display) override;
    StateType getType() const override;

private:
    SDL_Texture* backgroundTexture_;
    Animator playerAnimator_;
    
    // List of player Digimon IDs to cycle through
    std::vector<std::string> playerDigimonIds_;
    size_t currentDigimonIndex_;
    
    // Current animation type (idle, walk, attack)
    std::string currentAnimationType_;
    
    // Update the displayed player Digimon
    void updateDisplayedDigimon();
    
    // Helper to get the display name of the current Digimon
    std::string getCurrentDigimonName() const;
};

#endif // PLAYER_TEST_STATE_H
