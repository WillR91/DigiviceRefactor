#ifndef ENEMY_TEST_STATE_H
#define ENEMY_TEST_STATE_H

#include "states/GameState.h"
#include "graphics/Animator.h"

// Forward declarations
class Game;
class InputManager;
class PlayerData;
class PCDisplay;

// A test state for viewing enemy Digimon animations
class EnemyTestState : public GameState {
public:
    EnemyTestState(Game* game);
    ~EnemyTestState();

    void enter() override;
    void exit() override;
    void handle_input(InputManager& inputManager, PlayerData* playerData) override;
    void update(float delta_time, PlayerData* playerData) override;
    void render(PCDisplay& display) override;
    StateType getType() const override;

private:
    SDL_Texture* backgroundTexture_;
    Animator enemyAnimator_;
    
    // List of enemy Digimon IDs to cycle through
    std::vector<std::string> enemyDigimonIds_;
    size_t currentEnemyIndex_;
    
    // Current animation type (idle, attack)
    std::string currentAnimationType_;
    
    // Update the displayed enemy Digimon
    void updateDisplayedEnemy();
    
    // Helper to get the display name of the current enemy
    std::string getCurrentEnemyName() const;
};

#endif // ENEMY_TEST_STATE_H
