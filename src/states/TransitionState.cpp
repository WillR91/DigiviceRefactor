// File: src/states/TransitionState.cpp

#include "vendor/nlohmann/json.hpp" // <<<< INCLUDE THIS FIRST (path relative to "include/" since CMake adds "include/" to paths)

#include "states/TransitionState.h" // Then your own header

#include "core/Game.h"
#include "core/AssetManager.h"
#include "platform/pc/pc_display.h"
#include "core/InputManager.h"
#include "core/PlayerData.h"
#include "core/GameConstants.h" 

#include <SDL.h>
#include <SDL_log.h>
#include <stdexcept>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <algorithm> 

using json = nlohmann::json; // This should be fine now if the above order is correct


// --- Constructor for BORDER_WIPE ---
// Signature uses nlohmann::json& from the forward declaration in the .h file
TransitionState::TransitionState(Game* game, float duration, SDL_Texture* borderTexture, const nlohmann::json& transitionData) :
    GameState(game),
    duration_(duration),
    timer_(0.0f),
    effectType_(TransitionEffectType::BORDER_WIPE),
    borderAtlasTexture_(borderTexture),
    borderTopSrcRect_{0,0,0,0},
    borderBottomSrcRect_{0,0,0,0},
    borderLeftSrcRect_{0,0,0,0},
    borderRightSrcRect_{0,0,0,0},
    fadeColor_{0,0,0,0}, 
    transitionComplete_(false)
{
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TransitionState Created (Type: BORDER_WIPE, Duration: %.2f)", duration);

    if (!game_ptr) { 
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"TransitionState Error: Null game pointer!"); 
        duration_ = 0.01f; 
        transitionComplete_ = true; 
        return; 
    }
    if (duration <= 0.0f) { 
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"TransitionState Warning: Duration zero/negative (%.2f). Setting to 0.01.", duration); 
        duration_ = 0.01f; 
    }

    if (!borderAtlasTexture_) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"TransitionState (BORDER_WIPE): borderTexture is null!");
    } else {
        // transitionData here is of type const nlohmann::json&
        if (!loadBorderRectsFromJson(transitionData)) { // This call is fine
             SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"loadBorderRectsFromJson returned false! Effect may not draw.");
             borderAtlasTexture_ = nullptr; 
        } else {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,"TransitionState: loadBorderRectsFromJson returned true.");
            if (borderTopSrcRect_.h <= 0 || borderBottomSrcRect_.h <= 0 || borderLeftSrcRect_.w <= 0 || borderRightSrcRect_.w <= 0) {
               SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Constructor Post-load check failed: Rect dimensions are invalid!");
               borderAtlasTexture_ = nullptr; 
            }
        }
    }
    if (!borderAtlasTexture_) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "TransitionState (BORDER_WIPE): Effect will not render due to missing texture or rects.");
        duration_ = 0.01f; 
        transitionComplete_ = true; 
    }
}

// --- Constructor for FADE effects ---
TransitionState::TransitionState(Game* game, float duration, TransitionEffectType effectType, SDL_Color fadeColor) :
    GameState(game),
    duration_(duration),
    timer_(0.0f),
    effectType_(effectType),
    borderAtlasTexture_(nullptr), 
    borderTopSrcRect_{0,0,0,0},   
    borderBottomSrcRect_{0,0,0,0},
    borderLeftSrcRect_{0,0,0,0},
    borderRightSrcRect_{0,0,0,0},
    fadeColor_(fadeColor),
    transitionComplete_(false)
{
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TransitionState Created (Type: %s, Duration: %.2f)",
        (effectType == TransitionEffectType::FADE_TO_COLOR ? "FADE_TO_COLOR" : "FADE_FROM_COLOR"),
        duration);

    if (!game_ptr) { 
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"TransitionState Error: Null game pointer!"); 
        duration_ = 0.01f; 
        transitionComplete_ = true;
        return; 
    }
    if (duration <= 0.0f) { 
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"TransitionState Warning: Duration zero/negative (%.2f). Setting to 0.01.", duration); 
        duration_ = 0.01f; 
    }
    if (effectType_ != TransitionEffectType::FADE_TO_COLOR && effectType_ != TransitionEffectType::FADE_FROM_COLOR) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TransitionState (FADE): Invalid effectType provided to fade constructor!");
        effectType_ = TransitionEffectType::FADE_TO_COLOR; 
        duration_ = 0.01f;
        transitionComplete_ = true;
    }
}

// --- Destructor ---
TransitionState::~TransitionState() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TransitionState Destroyed.");
}

// --- enter ---
void TransitionState::enter() {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "TransitionState: Entering state (Type: %d)", static_cast<int>(effectType_));
    timer_ = 0.0f;
    transitionComplete_ = false;
    if (duration_ <= 0.001f) { 
        transitionComplete_ = true;
        if (game_ptr) {
            game_ptr->requestPopState();
        }
    }
}

// --- loadBorderRectsFromJson ---
// Signature uses nlohmann::json& from the forward declaration in the .h file
bool TransitionState::loadBorderRectsFromJson(const nlohmann::json& data) { // data is const nlohmann::json&
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "--- loadBorderRectsFromJson (JSON Object Version) START ---");
    std::map<std::string, SDL_Rect> loadedRects;
    try {
        // Here, 'data' is already a const nlohmann::json&. Operations on it should use the fully defined type's members.
        if (data.contains("frames") && data.at("frames").is_object()) { // Use .at() for const access or ensure .contains() implies validity for operator[]
             const auto& framesNode = data.at("frames"); // Use .at() for const access
             for (auto it = framesNode.begin(); it != framesNode.end(); ++it) {
                 std::string key = it.key(); 
                 const auto& frameData = it.value(); // frameData is also a nlohmann::json object (or sub-object)
                 if (frameData.contains("frame")) {
                     const auto& rectData = frameData.at("frame");
                     if (rectData.contains("x") && rectData.contains("y") && rectData.contains("w") && rectData.contains("h")) {
                          loadedRects[key] = { 
                              rectData.at("x").get<int>(), 
                              rectData.at("y").get<int>(), 
                              rectData.at("w").get<int>(), 
                              rectData.at("h").get<int>() 
                          };
                     } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "  JSON rect missing x,y,w or h for key '%s'", key.c_str());}
                 } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "  JSON frame missing 'frame' object for key '%s'", key.c_str());}
             }
        } else { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "loadBorderRectsFromJson Error: JSON missing 'frames' object or 'frames' is not an object."); return false;}
    } catch (const nlohmann::json::exception& e) { // MODIFIED to nlohmann::json::exception
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "loadBorderRectsFromJson Error: JSON parsing exception: %s", e.what()); return false; 
    } catch (const std::exception& e) { 
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "loadBorderRectsFromJson Error: Generic exception: %s", e.what()); return false; 
    }

    bool success = true;
    auto assignRect = [&](SDL_Rect& targetRect, const std::string& name) {
        if (loadedRects.count(name)) { 
            targetRect = loadedRects[name]; 
        } else { 
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "JSON data missing entry for '%s'", name.c_str()); 
            success = false; 
        }
    };

    assignRect(borderTopSrcRect_, "border_top");
    assignRect(borderBottomSrcRect_, "border_bottom");
    assignRect(borderLeftSrcRect_, "border_left");
    assignRect(borderRightSrcRect_, "border_right");

    if (success && !(borderTopSrcRect_.h > 0 && borderBottomSrcRect_.h > 0 && borderLeftSrcRect_.w > 0 && borderRightSrcRect_.w > 0)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "loadBorderRectsFromJson Error: One or more loaded rects have invalid dimensions."); success = false;
    }
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "--- loadBorderRectsFromJson END (returning %d) ---", success);
    return success;
}


// --- handle_input ---
void TransitionState::handle_input(InputManager& inputManager, PlayerData* playerData) {
    // ...
}

// --- update ---
void TransitionState::update(float delta_time, PlayerData* playerData) {
    // ... (no changes needed here regarding json)
    if (transitionComplete_) {
        return; 
    }
    timer_ += delta_time;
    if (timer_ >= duration_) {
        timer_ = duration_; 
        transitionComplete_ = true;
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TransitionState: Visual transition complete. Requesting PopState.");
        if (game_ptr) {
            game_ptr->requestPopState();
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TransitionState::update Error: game_ptr is null, cannot pop state!");
        }
    }
}

// --- render ---
void TransitionState::render(PCDisplay& display) {
    // ... (no changes needed here regarding json)
    if (!game_ptr) return;

    float t = 0.0f;
    if (duration_ > 0.001f) { 
        t = std::min(1.0f, timer_ / duration_);
    } else {
        t = 1.0f; 
    }
    
    // For both FADE_TO_COLOR and FADE_FROM_COLOR, render the state underneath.
    // For FADE_TO_COLOR, we are fading out the state below.
    // For FADE_FROM_COLOR, we are fading in to reveal the state below.
    if (effectType_ == TransitionEffectType::FADE_TO_COLOR || effectType_ == TransitionEffectType::FADE_FROM_COLOR) {
        GameState* underlyingState = game_ptr->getUnderlyingState(this); 
        if (underlyingState) {
            underlyingState->render(display); // Render the state that is being faded out from or faded into.
        }
    }

    switch (effectType_) {
        case TransitionEffectType::BORDER_WIPE: {
            if (!borderAtlasTexture_ || borderTopSrcRect_.h <= 0 || borderBottomSrcRect_.h <= 0 || borderLeftSrcRect_.w <= 0 || borderRightSrcRect_.w <= 0 ) {
                 return; 
            }
            int windowW = 0, windowH = 0;
            display.getWindowSize(windowW, windowH); 
            if (windowW <= 0) windowW = GameConstants::WINDOW_WIDTH; 
            if (windowH <= 0) windowH = GameConstants::WINDOW_HEIGHT; 

            auto lerp = [](float start, float end, float factor) { return start + (end - start) * factor; };

            int topY = static_cast<int>(lerp((float)-borderTopSrcRect_.h, 0.0f, t));
            SDL_Rect topDst = {0, topY, windowW, borderTopSrcRect_.h};
            int bottomY = static_cast<int>(lerp((float)windowH, (float)(windowH - borderBottomSrcRect_.h), t));
            SDL_Rect botDst = {0, bottomY, windowW, borderBottomSrcRect_.h};
            int leftX = static_cast<int>(lerp((float)-borderLeftSrcRect_.w, 0.0f, t));
            SDL_Rect lefDst = {leftX, 0, borderLeftSrcRect_.w, windowH};
            int rightX = static_cast<int>(lerp((float)windowW, (float)(windowW - borderRightSrcRect_.w), t));
            SDL_Rect rigDst = {rightX, 0, borderRightSrcRect_.w, windowH};

            if (topDst.h > 0) display.drawTexture(borderAtlasTexture_, &borderTopSrcRect_, &topDst);
            if (botDst.h > 0) display.drawTexture(borderAtlasTexture_, &borderBottomSrcRect_, &botDst);
            if (lefDst.w > 0) display.drawTexture(borderAtlasTexture_, &borderLeftSrcRect_, &lefDst);
            if (rigDst.w > 0) display.drawTexture(borderAtlasTexture_, &borderRightSrcRect_, &rigDst);
            break;
        }
        case TransitionEffectType::FADE_TO_COLOR: {
            Uint8 alpha = static_cast<Uint8>(std::min(255.0f, std::max(0.0f, t * 255.0f)));
            display.setDrawBlendMode(SDL_BLENDMODE_BLEND);
            display.setDrawColor(fadeColor_.r, fadeColor_.g, fadeColor_.b, alpha);
            SDL_Rect screenRect = {0, 0, GameConstants::WINDOW_WIDTH, GameConstants::WINDOW_HEIGHT}; 
            display.fillRect(&screenRect); 
            break;
        }
        case TransitionEffectType::FADE_FROM_COLOR: {
            Uint8 alpha = static_cast<Uint8>(std::min(255.0f, std::max(0.0f, (1.0f - t) * 255.0f)));
            display.setDrawBlendMode(SDL_BLENDMODE_BLEND);
            display.setDrawColor(fadeColor_.r, fadeColor_.g, fadeColor_.b, alpha);
            SDL_Rect screenRect = {0, 0, GameConstants::WINDOW_WIDTH, GameConstants::WINDOW_HEIGHT};
            display.fillRect(&screenRect);
            break;
        }
    }
}

// <<< --- ADDED IMPLEMENTATION --- >>>
TransitionEffectType TransitionState::getEffectType() const {
    return effectType_;
}
// <<< --- END ADDED IMPLEMENTATION --- >>>

// --- getType ---
StateType TransitionState::getType() const {
    return StateType::Transition;
}