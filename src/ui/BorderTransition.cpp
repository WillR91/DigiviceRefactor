// File: src/ui/BorderTransition.cpp

#include "ui/BorderTransition.h"
#include "core/Game.h"
#include "core/AssetManager.h"
#include "platform/pc/pc_display.h"
#include "core/GameConstants.h"
#include "utils/ScalingUtils.h"
#include <SDL_log.h>
#include <algorithm>

BorderTransition::BorderTransition(Game* game, float animationDuration, int inwardDistance) :
    game_(game),
    animationDuration_(animationDuration),
    animationTimer_(0.0f),
    currentState_(AnimationState::BORDERS_OUT),
    inwardDistance_(ScalingUtils::applyScaling(inwardDistance, ScalingUtils::ElementType::UI)),
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
        return;    }    loadBorderTextures();
    
    float uiScalingFactor = ScalingUtils::getScalingFactor(ScalingUtils::ElementType::UI);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BorderTransition: Created with duration %.2f seconds, original inward distance %d pixels, scaled inward distance %d pixels (UI scaling %.2f)", 
                animationDuration_, inwardDistance, inwardDistance_, uiScalingFactor);
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
    }    // Query texture dimensions for logging (original dimensions)
    if (borderTop_) {
        int originalHeight;
        SDL_QueryTexture(borderTop_, nullptr, nullptr, nullptr, &originalHeight);
        topHeight_ = originalHeight;
    }
    if (borderBottom_) {
        int originalHeight;
        SDL_QueryTexture(borderBottom_, nullptr, nullptr, nullptr, &originalHeight);
        bottomHeight_ = originalHeight;
    }
    if (borderLeft_) {
        int originalWidth;
        SDL_QueryTexture(borderLeft_, nullptr, nullptr, &originalWidth, nullptr);
        leftWidth_ = originalWidth;
    }
    if (borderRight_) {
        int originalWidth;
        SDL_QueryTexture(borderRight_, nullptr, nullptr, &originalWidth, nullptr);
        rightWidth_ = originalWidth;
    }

    float uiScalingFactor = ScalingUtils::getScalingFactor(ScalingUtils::ElementType::UI);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "BorderTransition: Border original dimensions - Top: %d, Bottom: %d, Left: %d, Right: %d (UI scaling: %.2f)", 
                topHeight_, bottomHeight_, leftWidth_, rightWidth_, uiScalingFactor);
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

    // Use logical screen dimensions (SDL logical scaling handles window-to-logical conversion automatically)
    const int LOGICAL_WIDTH = 466;
    const int LOGICAL_HEIGHT = 466;

    // Calculate animation progress with easing
    float progress = (animationDuration_ > 0.0f) ? (animationTimer_ / animationDuration_) : 1.0f;
    progress = std::min(progress, 1.0f);
    float easedProgress = easeInOutCubic(progress);

    // Calculate the central porthole dimensions (square viewport in center of logical screen)
    int portholeSize = std::min(LOGICAL_WIDTH, LOGICAL_HEIGHT) - (2 * inwardDistance_);
    int portholeX = (LOGICAL_WIDTH - portholeSize) / 2;
    int portholeY = (LOGICAL_HEIGHT - portholeSize) / 2;

    // Apply ScalingUtils scaling for UI elements (this scales textures within the logical coordinate system)
    float uiScale = ScalingUtils::getScalingFactor(ScalingUtils::ElementType::UI);

    // TOP BORDER: Slides down from off-screen to porthole top
    if (borderTop_) {
        // Get original texture dimensions and apply UI scaling
        int originalWidth, originalHeight;
        SDL_QueryTexture(borderTop_, nullptr, nullptr, &originalWidth, &originalHeight);
        
        int scaledWidth = static_cast<int>(originalWidth * uiScale);
        int scaledHeight = static_cast<int>(originalHeight * uiScale);
        
        // Calculate movement positions (in logical coordinates)
        int topStartY = -scaledHeight;  // Start completely off-screen
        int topEndY = portholeY - scaledHeight;  // End just above porthole
        int currentTopY = static_cast<int>(topStartY + (topEndY - topStartY) * easedProgress);
        
        // Center horizontally and span full logical width for complete coverage
        SDL_Rect destRect = { 0, currentTopY, LOGICAL_WIDTH, scaledHeight };
        display.drawTexture(borderTop_, nullptr, &destRect);
    }

    // BOTTOM BORDER: Slides up from off-screen to porthole bottom
    if (borderBottom_) {
        // Get original texture dimensions and apply UI scaling
        int originalWidth, originalHeight;
        SDL_QueryTexture(borderBottom_, nullptr, nullptr, &originalWidth, &originalHeight);
        
        int scaledWidth = static_cast<int>(originalWidth * uiScale);
        int scaledHeight = static_cast<int>(originalHeight * uiScale);
        
        // Calculate movement positions (in logical coordinates)
        int bottomStartY = LOGICAL_HEIGHT;  // Start completely off-screen
        int bottomEndY = portholeY + portholeSize;  // End just below porthole
        int currentBottomY = static_cast<int>(bottomStartY + (bottomEndY - bottomStartY) * easedProgress);
        
        // Span full logical width for complete coverage
        SDL_Rect destRect = { 0, currentBottomY, LOGICAL_WIDTH, scaledHeight };
        display.drawTexture(borderBottom_, nullptr, &destRect);
    }

    // LEFT BORDER: Slides right from off-screen to porthole left
    if (borderLeft_) {
        // Get original texture dimensions and apply UI scaling
        int originalWidth, originalHeight;
        SDL_QueryTexture(borderLeft_, nullptr, nullptr, &originalWidth, &originalHeight);
        
        int scaledWidth = static_cast<int>(originalWidth * uiScale);
        int scaledHeight = static_cast<int>(originalHeight * uiScale);
        
        // Calculate movement positions (in logical coordinates)
        int leftStartX = -scaledWidth;  // Start completely off-screen
        int leftEndX = portholeX - scaledWidth;  // End just left of porthole
        int currentLeftX = static_cast<int>(leftStartX + (leftEndX - leftStartX) * easedProgress);
        
        // Span full logical height for complete coverage
        SDL_Rect destRect = { currentLeftX, 0, scaledWidth, LOGICAL_HEIGHT };
        display.drawTexture(borderLeft_, nullptr, &destRect);
    }

    // RIGHT BORDER: Slides left from off-screen to porthole right
    if (borderRight_) {
        // Get original texture dimensions and apply UI scaling
        int originalWidth, originalHeight;
        SDL_QueryTexture(borderRight_, nullptr, nullptr, &originalWidth, &originalHeight);
        
        int scaledWidth = static_cast<int>(originalWidth * uiScale);
        int scaledHeight = static_cast<int>(originalHeight * uiScale);
        
        // Calculate movement positions (in logical coordinates)
        int rightStartX = LOGICAL_WIDTH;  // Start completely off-screen
        int rightEndX = portholeX + portholeSize;  // End just right of porthole
        int currentRightX = static_cast<int>(rightStartX + (rightEndX - rightStartX) * easedProgress);
        
        // Span full logical height for complete coverage
        SDL_Rect destRect = { currentRightX, 0, scaledWidth, LOGICAL_HEIGHT };
        display.drawTexture(borderRight_, nullptr, &destRect);
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
    // Use logical screen dimensions (SDL logical scaling handles window-to-logical conversion automatically)
    const int LOGICAL_WIDTH = 466;
    const int LOGICAL_HEIGHT = 466;

    // Calculate the central porthole dimensions (square viewport in center of logical screen)
    int portholeSize = std::min(LOGICAL_WIDTH, LOGICAL_HEIGHT) - (2 * inwardDistance_);
    int portholeX = (LOGICAL_WIDTH - portholeSize) / 2;
    int portholeY = (LOGICAL_HEIGHT - portholeSize) / 2;

    // Apply ScalingUtils scaling for UI elements
    float uiScale = ScalingUtils::getScalingFactor(ScalingUtils::ElementType::UI);

    // Final positions when animation is complete (borders framing the porthole):
    
    // Top border: positioned just above the porthole
    if (borderTop_) {
        int originalWidth, originalHeight;
        SDL_QueryTexture(borderTop_, nullptr, nullptr, &originalWidth, &originalHeight);
        int scaledHeight = static_cast<int>(originalHeight * uiScale);
        topRect = { 0, portholeY - scaledHeight, LOGICAL_WIDTH, scaledHeight };
    }

    // Bottom border: positioned just below the porthole
    if (borderBottom_) {
        int originalWidth, originalHeight;
        SDL_QueryTexture(borderBottom_, nullptr, nullptr, &originalWidth, &originalHeight);
        int scaledHeight = static_cast<int>(originalHeight * uiScale);
        bottomRect = { 0, portholeY + portholeSize, LOGICAL_WIDTH, scaledHeight };
    }

    // Left border: positioned just left of the porthole, covering full height
    if (borderLeft_) {
        int originalWidth, originalHeight;
        SDL_QueryTexture(borderLeft_, nullptr, nullptr, &originalWidth, &originalHeight);
        int scaledWidth = static_cast<int>(originalWidth * uiScale);
        leftRect = { portholeX - scaledWidth, 0, scaledWidth, LOGICAL_HEIGHT };
    }

    // Right border: positioned just right of the porthole, covering full height
    if (borderRight_) {
        int originalWidth, originalHeight;
        SDL_QueryTexture(borderRight_, nullptr, nullptr, &originalWidth, &originalHeight);
        int scaledWidth = static_cast<int>(originalWidth * uiScale);
        rightRect = { portholeX + portholeSize, 0, scaledWidth, LOGICAL_HEIGHT };
    }
}
