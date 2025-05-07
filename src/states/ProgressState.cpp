// src/states/ProgressState.cpp
#include "states/ProgressState.h"
#include "core/Game.h"
#include "core/PlayerData.h"
#include "core/InputManager.h"
#include "core/GameAction.h"
#include "core/AssetManager.h"
#include "core/AnimationManager.h"
#include "graphics/AnimationData.h"
#include "ui/TextRenderer.h"
#include "platform/pc/pc_display.h"
#include "Utils/AnimationUtils.h"
#include "core/GameConstants.h" // Added include
#include <SDL_log.h>
#include <string> // For std::to_string

ProgressState::ProgressState(Game* game) :
    GameState(game),
    partnerAnimator_(),
    backgroundTexture_(nullptr)
{
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ProgressState Constructor Called.");
    if (!game_ptr || !game_ptr->getAssetManager() || !game_ptr->getTextRenderer() || !game_ptr->getAnimationManager() || !game_ptr->getPlayerData()) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "ProgressState: Missing required game systems!");
        // Consider throwing or setting an error state
        return;
    }

    AssetManager* assets = game_ptr->getAssetManager();
    backgroundTexture_ = assets->getTexture("menu_bg_blue"); // Or a custom background
    if (!backgroundTexture_) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ProgressState: Background texture not found!");
    }
}

ProgressState::~ProgressState() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ProgressState Destructor Called.");
}

void ProgressState::enter() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "--- ProgressState enter() ---");
    if (!game_ptr || !game_ptr->getPlayerData() || !game_ptr->getAnimationManager()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ProgressState::enter() - Missing required pointers!");
        return;
    }

    PlayerData* playerData = game_ptr->getPlayerData();
    DigimonType currentPartner = playerData->currentPartner;

    // Request "Walk" animation with explicit loop override
    std::string walkAnimId = AnimationUtils::GetAnimationId(currentPartner, "Walk");
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ProgressState Enter: Requesting Anim ID: '%s'", walkAnimId.c_str());

    AnimationManager* animManager = game_ptr->getAnimationManager();
    const AnimationData* walkAnimData = animManager->getAnimationData(walkAnimId);
    if (walkAnimData) {
        // Create a local copy of the animation data to modify loop behavior
        AnimationData* localWalkData = new AnimationData(*walkAnimData);
        localWalkData->loops = true; // Force looping for menu display
        partnerAnimator_.setAnimation(localWalkData, true); // true = owns data (will delete)
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ProgressState Enter: Set looping walk animation");
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ProgressState Enter: Failed to get walk animation data");
    }
}

void ProgressState::exit() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "--- ProgressState exit() ---");
    // No specific cleanup needed for this simple state yet
}

void ProgressState::handle_input(InputManager& inputManager, PlayerData* playerData) {
    // Pop state on CONFIRM or CANCEL
    if (inputManager.isActionJustPressed(GameAction::CONFIRM) || inputManager.isActionJustPressed(GameAction::CANCEL)) {
        SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "ProgressState: Confirm/Cancel pressed. Popping state.");
        if (game_ptr) {
            game_ptr->requestPopState();
        }
    }
}

void ProgressState::update(float delta_time, PlayerData* playerData) {
    partnerAnimator_.update(delta_time);
}

void ProgressState::render(PCDisplay& display) {
    SDL_Renderer* renderer = display.getRenderer();
    if (!renderer) return;

    // 1. Draw Background
    if (backgroundTexture_) {
        SDL_RenderCopy(renderer, backgroundTexture_, NULL, NULL);
    } else {
        SDL_SetRenderDrawColor(renderer, 20, 20, 50, 255); // Dark blue fallback
        SDL_RenderClear(renderer);
    }

    // 2. Draw Animated Partner
    SDL_Texture* digiTexture = partnerAnimator_.getCurrentTexture();
    SDL_Rect digiSrcRect = partnerAnimator_.getCurrentFrameRect();
    int windowW, windowH;
    display.getWindowSize(windowW, windowH); // Get current window size

    if (digiTexture && digiSrcRect.w > 0 && digiSrcRect.h > 0) {
        // Center the sprite, maybe a bit higher
        int spriteCenterY = windowH / 3;
        int drawX = (windowW / 2) - (digiSrcRect.w / 2);
        int drawY = spriteCenterY - (digiSrcRect.h / 2);
        SDL_Rect dstRect = {drawX, drawY, digiSrcRect.w, digiSrcRect.h};
        display.drawTexture(digiTexture, &digiSrcRect, &dstRect);
    } else {
         // Placeholder if animation not available
         SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255); // Bright Pink error box
         int w = 50, h = 50;
         int spriteCenterY = windowH / 3;
         int x = (windowW / 2) - (w / 2);
         int y = spriteCenterY - (h / 2);
         SDL_Rect placeholder = {x, y, w, h};
         SDL_RenderFillRect(renderer, &placeholder);
         SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "ProgressState: Drawing placeholder, animator returned invalid texture/rect.");
    }

    // 3. Draw Progress Text
    TextRenderer* textRenderer = game_ptr ? game_ptr->getTextRenderer() : nullptr;
    PlayerData* playerData = game_ptr ? game_ptr->getPlayerData() : nullptr;

    if (textRenderer && playerData) {
        int steps_taken = playerData->stepsTakenThisChapter;
        int goal_steps = GameConstants::CURRENT_CHAPTER_STEP_GOAL;
        int steps_remaining = goal_steps - steps_taken;
        if (steps_remaining < 0) steps_remaining = 0;

        // --- Define Text Parts ---
        std::string line1Text = std::to_string(steps_remaining) + " STEPS";
        std::string line2Text = "REMAINING"; // Or "TO GOAL" if you prefer

        // --- Define Text Style ---
        const float textScale = 0.85f; // Adjust as needed
        const int textKerning = -10;   // Adjust as needed
        const int lineSpacing = 2;    // Pixels between the bottom of line 1 and top of line 2

        // --- Calculate Position for Line 1 ---
        SDL_Point dims1 = textRenderer->getTextDimensions(line1Text, textKerning);
        int scaledW1 = 0;
        int scaledH1 = 0;
        if (dims1.x > 0) {
            scaledW1 = static_cast<int>(dims1.x * textScale);
            scaledH1 = static_cast<int>(dims1.y * textScale);
        }

        // Center Line 1 horizontally, position vertically (e.g., below sprite)
        int windowW = 0; int windowH = 0;
        display.getWindowSize(windowW, windowH);
        if (windowW <= 0 || windowH <= 0) { windowW = 466; windowH = 466; }

        int line1X = (windowW / 2) - (scaledW1 / 2);
        // Example: Position line 1 below the digimon sprite area (adjust as needed)
        int line1Y = windowH * 12 / 20; // Roughly 2/3rds down

        // --- Calculate Position for Line 2 ---
        SDL_Point dims2 = textRenderer->getTextDimensions(line2Text, textKerning);
        int scaledW2 = 0;
        if (dims2.x > 0) {
             scaledW2 = static_cast<int>(dims2.x * textScale);
             // scaledH2 not strictly needed for positioning below line 1
        }

        // Center Line 2 horizontally, position it below Line 1
        int line2X = (windowW / 2) - (scaledW2 / 2);
        int line2Y = line1Y + scaledH1 + lineSpacing; // Position below line1's bottom edge + spacing

        // --- Draw Both Lines ---
        SDL_Renderer* renderer = display.getRenderer();
        if (renderer) {
             if (scaledW1 > 0) { // Only draw if dimensions are valid
                 textRenderer->drawText(renderer, line1Text, line1X, line1Y, textScale, textKerning);
             } else { 
                 SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "ProgressState: Zero dimensions for line 1: %s", line1Text.c_str());
             }

             if (scaledW2 > 0) { // Only draw if dimensions are valid
                 textRenderer->drawText(renderer, line2Text, line2X, line2Y, textScale, textKerning);
             } else { 
                 SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "ProgressState: Zero dimensions for line 2: %s", line2Text.c_str());
             }
        } else { 
            SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "ProgressState: Renderer is null when drawing text");
        }
    } else {
        if (!textRenderer) SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "ProgressState: TextRenderer is null.");
        if (!playerData) SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "ProgressState: PlayerData is null.");
    }
}

StateType ProgressState::getType() const {
    return StateType::Progress;
}