#pragma once

#include "ui/UIElement.h"
#include <SDL.h>
#include <string>
#include <memory>

// Forward declarations
class Game;
class PCDisplay;
class InputManager;
class PlayerData;
class AssetManager;
class TextRenderer;

/**
 * @brief Base class for all Digivice screen interfaces
 * 
 * DigiviceScreen serves as the foundation for creating structured UI screens
 * in the Digivice interface. It provides common functionality like background
 * management, text rendering, and layout coordination.
 */
class DigiviceScreen : public UIElement {
public:
    /**
     * @brief Construct a new DigiviceScreen
     * @param game Pointer to the main game instance
     * @param backgroundTextureName Optional background texture name
     */
    DigiviceScreen(Game* game, const std::string& backgroundTextureName = "");
    
    virtual ~DigiviceScreen() = default;

    // Core screen lifecycle
    void update(float deltaTime) override;
    void render(SDL_Renderer* renderer) override;
    bool handleInput(InputManager& inputManager) override;

    // Screen-specific methods that derived classes can override
    virtual void onShow() {}
    virtual void onHide() {}
    virtual void onScreenSizeChanged(int width, int height) {}

    // Background management
    void setBackgroundTexture(const std::string& textureName);
    void setBackgroundColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);

    // Screen size utilities
    SDL_Point getScreenSize() const;
    SDL_Point getScreenCenter() const;

protected:
    // Game system access
    Game* game_;
    AssetManager* assetManager_;
    TextRenderer* textRenderer_;

    // Background rendering
    SDL_Texture* backgroundTexture_;
    SDL_Color backgroundColor_;
    bool useBackgroundTexture_;

    // Screen dimensions cache
    mutable int cachedScreenWidth_;
    mutable int cachedScreenHeight_;
    mutable bool screenSizeCached_;

    // Helper methods for derived classes
    void updateScreenSize() const;
    void renderBackground(SDL_Renderer* renderer);
    
    // Commonly used positioning helpers
    int centerX(int elementWidth) const;
    int centerY(int elementHeight) const;
    SDL_Point centerPosition(int elementWidth, int elementHeight) const;
};
