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
        return; // No rendering when borders are off-screen
    }

    int windowW = 0, windowH = 0;
    if (game_ && game_->get_display()) {
        game_->get_display()->getWindowSize(windowW, windowH);
    }
    if (windowW <= 0) windowW = GameConstants::WINDOW_WIDTH;
    if (windowH <= 0) windowH = GameConstants::WINDOW_HEIGHT;

    // Calculate animation progress with easing
    float progress = (animationDuration_ > 0.0f) ? (animationTimer_ / animationDuration_) : 1.0f;
    progress = std::min(progress, 1.0f);
    float easedProgress = easeInOutCubic(progress);

    // Calculate the central porthole dimensions (square viewport in center of screen)
    int portholeSize = std::min(windowW, windowH) - (2 * inwardDistance_);
    int portholeX = (windowW - portholeSize) / 2;
    int portholeY = (windowH - portholeSize) / 2;

    // Use actual texture dimensions for proper aspect ratio preservation
    int topBorderHeight = (topHeight_ > 0) ? topHeight_ : inwardDistance_;
    int bottomBorderHeight = (bottomHeight_ > 0) ? bottomHeight_ : inwardDistance_;
    int leftBorderWidth = (leftWidth_ > 0) ? leftWidth_ : inwardDistance_;
    int rightBorderWidth = (rightWidth_ > 0) ? rightWidth_ : inwardDistance_;
    
    // TOP BORDER: Slides down from off-screen to porthole top
    if (borderTop_) {
        int topStartY = -topBorderHeight;  // Start completely off-screen
        int topEndY = portholeY - topBorderHeight;  // End just above porthole
        int currentTopY = static_cast<int>(topStartY + (topEndY - topStartY) * easedProgress);
        
        SDL_Rect topRect = { 0, currentTopY, windowW, topBorderHeight };
        display.drawTexture(borderTop_, nullptr, &topRect);
    }

    // BOTTOM BORDER: Slides up from off-screen to porthole bottom
    if (borderBottom_) {
        int bottomStartY = windowH;  // Start completely off-screen
        int bottomEndY = portholeY + portholeSize;  // End just below porthole
        int currentBottomY = static_cast<int>(bottomStartY + (bottomEndY - bottomStartY) * easedProgress);
        
        SDL_Rect bottomRect = { 0, currentBottomY, windowW, bottomBorderHeight };
        display.drawTexture(borderBottom_, nullptr, &bottomRect);
    }

    // LEFT BORDER: Slides right from off-screen to porthole left
    if (borderLeft_) {
        int leftStartX = -leftBorderWidth;  // Start completely off-screen
        int leftEndX = portholeX - leftBorderWidth;  // End just left of porthole
        int currentLeftX = static_cast<int>(leftStartX + (leftEndX - leftStartX) * easedProgress);
        
        SDL_Rect leftRect = { currentLeftX, 0, leftBorderWidth, windowH };
        display.drawTexture(borderLeft_, nullptr, &leftRect);
    }

    // RIGHT BORDER: Slides left from off-screen to porthole right
    if (borderRight_) {
        int rightStartX = windowW;  // Start completely off-screen
        int rightEndX = portholeX + portholeSize;  // End just right of porthole
        int currentRightX = static_cast<int>(rightStartX + (rightEndX - rightStartX) * easedProgress);
        
        SDL_Rect rightRect = { currentRightX, 0, rightBorderWidth, windowH };
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
    if (windowH <= 0) windowH = GameConstants::WINDOW_HEIGHT;

    // Calculate the central porthole dimensions (square viewport in center of screen)
    int portholeSize = std::min(windowW, windowH) - (2 * inwardDistance_);
    int portholeX = (windowW - portholeSize) / 2;
    int portholeY = (windowH - portholeSize) / 2;

    // Use actual texture dimensions for proper aspect ratio preservation
    int topBorderHeight = (topHeight_ > 0) ? topHeight_ : inwardDistance_;
    int bottomBorderHeight = (bottomHeight_ > 0) ? bottomHeight_ : inwardDistance_;
    int leftBorderWidth = (leftWidth_ > 0) ? leftWidth_ : inwardDistance_;
    int rightBorderWidth = (rightWidth_ > 0) ? rightWidth_ : inwardDistance_;

    // Final positions when animation is complete (borders framing the porthole):
    // Top border: positioned just above the porthole
    topRect = { 0, portholeY - topBorderHeight, windowW, topBorderHeight };

    // Bottom border: positioned just below the porthole
    bottomRect = { 0, portholeY + portholeSize, windowW, bottomBorderHeight };

    // Left border: positioned just left of the porthole, covering full height
    leftRect = { portholeX - leftBorderWidth, 0, leftBorderWidth, windowH };

    // Right border: positioned just right of the porthole, covering full height
    rightRect = { portholeX + portholeSize, 0, rightBorderWidth, windowH };
}
