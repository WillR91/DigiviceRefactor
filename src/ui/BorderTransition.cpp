// File: src/ui/BorderTransition.cpp

#include "ui/BorderTransition.h"
#include "core/Game.h"
#include "core/AssetManager.h"
#include "platform/pc/pc_display.h"
#include "core/GameConstants.h"
#include <SDL_log.h>
#include <algorithm>

BorderTransition::BorderTransition(Game* game, float animationDuration, int inwardDistance) :
    game_(game),
    animationDuration_(animationDuration),
    animationTimer_(0.0f),
    currentState_(AnimationState::BORDERS_OUT),
    inwardDistance_(inwardDistance),
    borderTop_(nullptr),
    borderBottom_(nullptr),
    borderLeft_(nullptr),
    borderRight_(nullptr),
    topHeight_(0),
    bottomHeight_(0),
    leftWidth_(0),
    rightWidth_(0)
{
    if (!game_) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "BorderTransition: Game pointer is null!");
        return;
    }    loadBorderTextures();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BorderTransition: Created with duration %.2f seconds, inward distance %d pixels", animationDuration_, inwardDistance_);
}

BorderTransition::~BorderTransition() {
    // Textures are managed by AssetManager, no need to manually destroy them
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BorderTransition: Destroyed");
}

void BorderTransition::loadBorderTextures() {
    if (!game_) return;

    AssetManager* assetManager = game_->getAssetManager();
    if (!assetManager) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "BorderTransition: AssetManager is null!");
        return;
    }

    // Load border textures if not already loaded
    const char* borderAssets[] = {
        "border_top", "assets/ui/elements/bordertop.png",
        "border_bottom", "assets/ui/elements/borderbottom.png", 
        "border_left", "assets/ui/elements/borderleft.png",
        "border_right", "assets/ui/elements/borderright.png"
    };

    for (int i = 0; i < 8; i += 2) {
        const char* textureId = borderAssets[i];
        const char* filePath = borderAssets[i + 1];
        
        // Try to get existing texture first
        SDL_Texture* texture = assetManager->getTexture(textureId);
        if (!texture) {
            // Load if not already loaded
            if (assetManager->loadTexture(textureId, filePath)) {
                texture = assetManager->getTexture(textureId);
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BorderTransition: Loaded texture %s", textureId);
            } else {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "BorderTransition: Failed to load texture %s from %s", textureId, filePath);
            }
        }

        // Assign to appropriate member
        if (i == 0) borderTop_ = texture;
        else if (i == 2) borderBottom_ = texture;
        else if (i == 4) borderLeft_ = texture;
        else if (i == 6) borderRight_ = texture;
    }

    // Query texture dimensions
    if (borderTop_) {
        SDL_QueryTexture(borderTop_, nullptr, nullptr, nullptr, &topHeight_);
    }
    if (borderBottom_) {
        SDL_QueryTexture(borderBottom_, nullptr, nullptr, nullptr, &bottomHeight_);
    }
    if (borderLeft_) {
        SDL_QueryTexture(borderLeft_, nullptr, nullptr, &leftWidth_, nullptr);
    }
    if (borderRight_) {
        SDL_QueryTexture(borderRight_, nullptr, nullptr, &rightWidth_, nullptr);
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BorderTransition: Border dimensions - Top: %d, Bottom: %d, Left: %d, Right: %d", 
                topHeight_, bottomHeight_, leftWidth_, rightWidth_);
}

void BorderTransition::startAnimation() {
    if (currentState_ == AnimationState::BORDERS_OUT) {
        currentState_ = AnimationState::BORDERS_ANIMATING;
        animationTimer_ = 0.0f;
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BorderTransition: Animation started");
    }
}

void BorderTransition::update(float deltaTime) {
    if (currentState_ != AnimationState::BORDERS_ANIMATING) {
        return;
    }

    animationTimer_ += deltaTime;
    
    if (animationTimer_ >= animationDuration_) {
        animationTimer_ = animationDuration_;
        currentState_ = AnimationState::BORDERS_IN;
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BorderTransition: Animation complete");
    }
}

void BorderTransition::render(PCDisplay& display) {
    if (currentState_ == AnimationState::BORDERS_OUT) {
        return; // Nothing to render
    }

    // Calculate animation progress (0.0 to 1.0)
    float progress = 0.0f;
    if (animationDuration_ > 0.001f) {
        progress = std::min(1.0f, animationTimer_ / animationDuration_);
    } else {
        progress = 1.0f;
    }

    // Apply easing for smooth animation
    float easedProgress = easeInOutCubic(progress);

    // Calculate border positions
    SDL_Rect topRect, bottomRect, leftRect, rightRect;
    calculateBorderPositions(topRect, bottomRect, leftRect, rightRect);

    // Apply animation offset based on progress
    int windowW = 0, windowH = 0;
    display.getWindowSize(windowW, windowH);
    if (windowW <= 0) windowW = GameConstants::WINDOW_WIDTH;
    if (windowH <= 0) windowH = GameConstants::WINDOW_HEIGHT;    // Top border slides down from off-screen to inward position
    int topStartY = -topHeight_;
    int topEndY = inwardDistance_;
    topRect.y = static_cast<int>(topStartY + (topEndY - topStartY) * easedProgress);

    // Bottom border slides up from off-screen to inward position
    int bottomStartY = windowH;
    int bottomEndY = windowH - bottomHeight_ - inwardDistance_;
    bottomRect.y = static_cast<int>(bottomStartY + (bottomEndY - bottomStartY) * easedProgress);

    // Left border slides right from off-screen to inward position
    int leftStartX = -leftWidth_;
    int leftEndX = inwardDistance_;
    leftRect.x = static_cast<int>(leftStartX + (leftEndX - leftStartX) * easedProgress);

    // Right border slides left from off-screen to inward position
    int rightStartX = windowW;
    int rightEndX = windowW - rightWidth_ - inwardDistance_;
    rightRect.x = static_cast<int>(rightStartX + (rightEndX - rightStartX) * easedProgress);

    // Render borders
    if (borderTop_ && topRect.y > -topHeight_) {
        display.drawTexture(borderTop_, nullptr, &topRect);
    }
    if (borderBottom_ && bottomRect.y < windowH) {
        display.drawTexture(borderBottom_, nullptr, &bottomRect);
    }
    if (borderLeft_ && leftRect.x > -leftWidth_) {
        display.drawTexture(borderLeft_, nullptr, &leftRect);
    }
    if (borderRight_ && rightRect.x < windowW) {
        display.drawTexture(borderRight_, nullptr, &rightRect);
    }
}

bool BorderTransition::isAnimationComplete() const {
    return currentState_ == AnimationState::BORDERS_IN;
}

BorderTransition::AnimationState BorderTransition::getAnimationState() const {
    return currentState_;
}

void BorderTransition::reset() {
    currentState_ = AnimationState::BORDERS_OUT;
    animationTimer_ = 0.0f;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BorderTransition: Reset to initial state");
}

float BorderTransition::easeInOutCubic(float t) const {
    if (t < 0.5f) {
        return 4.0f * t * t * t;
    } else {
        float p = 2.0f * t - 2.0f;
        return 1.0f + p * p * p / 2.0f;
    }
}

void BorderTransition::calculateBorderPositions(SDL_Rect& topRect, SDL_Rect& bottomRect,
                                                SDL_Rect& leftRect, SDL_Rect& rightRect) const {
    int windowW = 0, windowH = 0;
    if (game_ && game_->get_display()) {
        game_->get_display()->getWindowSize(windowW, windowH);
    }
    if (windowW <= 0) windowW = GameConstants::WINDOW_WIDTH;
    if (windowH <= 0) windowH = GameConstants::WINDOW_HEIGHT;    // Top border spans full width, positioned inward from top edge
    topRect = { 0, inwardDistance_, windowW, topHeight_ };

    // Bottom border spans full width, positioned inward from bottom edge  
    bottomRect = { 0, windowH - bottomHeight_ - inwardDistance_, windowW, bottomHeight_ };

    // Left border spans full height, positioned inward from left edge
    leftRect = { inwardDistance_, 0, leftWidth_, windowH };

    // Right border spans full height, positioned inward from right edge
    rightRect = { windowW - rightWidth_ - inwardDistance_, 0, rightWidth_, windowH };
}
