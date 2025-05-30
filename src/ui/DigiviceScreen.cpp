#include "ui/DigiviceScreen.h"
#include "core/Game.h"
#include "core/AssetManager.h"
#include "ui/TextRenderer.h"
#include "platform/pc/pc_display.h"
#include "core/InputManager.h"
#include <SDL_log.h>

DigiviceScreen::DigiviceScreen(Game* game, const std::string& backgroundTextureName)
    : UIElement(0, 0, 0, 0), // Will be sized to screen
      game_(game),
      assetManager_(nullptr),
      textRenderer_(nullptr),
      backgroundTexture_(nullptr),
      backgroundColor_{0, 0, 80, 255}, // Default dark blue
      useBackgroundTexture_(false),
      cachedScreenWidth_(0),
      cachedScreenHeight_(0),
      screenSizeCached_(false) {
    
    if (!game_) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DigiviceScreen: Game pointer is null!");
        return;
    }

    assetManager_ = game_->getAssetManager();
    textRenderer_ = game_->getTextRenderer();

    if (!assetManager_) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "DigiviceScreen: AssetManager not available");
    }

    if (!textRenderer_) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "DigiviceScreen: TextRenderer not available");
    }

    // Set background if provided
    if (!backgroundTextureName.empty()) {
        setBackgroundTexture(backgroundTextureName);
    }

    // Initialize screen size
    updateScreenSize();
    setSize(cachedScreenWidth_, cachedScreenHeight_);
}

void DigiviceScreen::update(float deltaTime) {
    // Update screen size if needed
    updateScreenSize();
    
    // Update all child elements
    updateChildren(deltaTime);
}

void DigiviceScreen::render(SDL_Renderer* renderer) {
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "DigiviceScreen: Renderer is null!");
        return;
    }

    // Render background
    renderBackground(renderer);
    
    // Render all child elements
    renderChildren(renderer);
}

bool DigiviceScreen::handleInput(InputManager& inputManager) {
    // Let children handle input first
    if (handleChildrenInput(inputManager)) {
        return true;
    }
    
    // Base screen doesn't handle any input by default
    return false;
}

void DigiviceScreen::setBackgroundTexture(const std::string& textureName) {
    if (!assetManager_) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "DigiviceScreen: Cannot set background texture - AssetManager not available");
        return;
    }

    backgroundTexture_ = assetManager_->getTexture(textureName);
    if (backgroundTexture_) {
        useBackgroundTexture_ = true;
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "DigiviceScreen: Background texture '%s' loaded", textureName.c_str());
    } else {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "DigiviceScreen: Background texture '%s' not found", textureName.c_str());
        useBackgroundTexture_ = false;
    }
}

void DigiviceScreen::setBackgroundColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    backgroundColor_ = {r, g, b, a};
    if (!backgroundTexture_) {
        useBackgroundTexture_ = false;
    }
}

SDL_Point DigiviceScreen::getScreenSize() const {
    updateScreenSize();
    return {cachedScreenWidth_, cachedScreenHeight_};
}

SDL_Point DigiviceScreen::getScreenCenter() const {
    updateScreenSize();
    return {cachedScreenWidth_ / 2, cachedScreenHeight_ / 2};
}

void DigiviceScreen::updateScreenSize() const {
    if (screenSizeCached_) {
        return;
    }

    if (!game_) {
        // Use default size if game is not available
        cachedScreenWidth_ = 466;
        cachedScreenHeight_ = 466;
        screenSizeCached_ = true;
        return;
    }

    // Try to get screen size from the game's display
    // Note: This is a simplified approach - in practice you might need to access
    // the display through a different path depending on your architecture
    cachedScreenWidth_ = 466;  // Default Digivice screen width
    cachedScreenHeight_ = 466; // Default Digivice screen height
    screenSizeCached_ = true;

    SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "DigiviceScreen: Screen size updated to %dx%d", 
                 cachedScreenWidth_, cachedScreenHeight_);
}

void DigiviceScreen::renderBackground(SDL_Renderer* renderer) {
    if (useBackgroundTexture_ && backgroundTexture_) {
        // Render texture background
        SDL_RenderCopy(renderer, backgroundTexture_, nullptr, nullptr);
    } else {
        // Render solid color background
        SDL_SetRenderDrawColor(renderer, backgroundColor_.r, backgroundColor_.g, 
                              backgroundColor_.b, backgroundColor_.a);
        SDL_RenderClear(renderer);
    }
}

int DigiviceScreen::centerX(int elementWidth) const {
    updateScreenSize();
    return (cachedScreenWidth_ - elementWidth) / 2;
}

int DigiviceScreen::centerY(int elementHeight) const {
    updateScreenSize();
    return (cachedScreenHeight_ - elementHeight) / 2;
}

SDL_Point DigiviceScreen::centerPosition(int elementWidth, int elementHeight) const {
    return {centerX(elementWidth), centerY(elementHeight)};
}
