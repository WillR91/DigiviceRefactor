// File: src/states/TransitionState.cpp

#include "states/TransitionState.h"
#include "core/Game.h"
#include "core/AssetManager.h"
#include "platform/pc/pc_display.h"  // Needed for PCDisplay&
#include "core/InputManager.h"       // Needed for InputManager&
#include "core/PlayerData.h"         // Needed for PlayerData*
// #include "states/MenuState.h" // Only needed if you were doing dynamic_cast checks etc.
#include <SDL.h>
#include <SDL_log.h>
#include <stdexcept>
#include <fstream>
#include "vendor/nlohmann/json.hpp"
#include <map>
#include <string>
#include <vector>
#include <algorithm> // For std::min

// Use the nlohmann::json namespace
using json = nlohmann::json;

// --- Constructor ---
TransitionState::TransitionState(Game* game, GameState* belowState, float duration, TransitionType type) :
    GameState(game), // <<< --- ADDED Base class constructor call --- >>>
    belowState_(belowState),
    duration_(duration),
    timer_(0.0f),
    type_(type),
    borderAtlasTexture_(nullptr),
    borderTopSrcRect_{0,0,0,0},      // <<< Explicitly initialize rects
    borderBottomSrcRect_{0,0,0,0},
    borderLeftSrcRect_{0,0,0,0},
    borderRightSrcRect_{0,0,0,0},    // <<< ---
    transition_complete_requested_(false),
    transitionComplete_(false)
{
    // game_ptr is initialized by GameState(game) call above
    // this->game_ptr = game; // No longer needed

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TransitionState Created (Type: %d, Duration: %.2f)", (int)type, duration);

    // Validation logic... (remains the same)
    if (!game_ptr || !belowState_) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"TransitionState Error: Null game or belowState pointer!"); duration_ = 0.01f; return; }
    AssetManager* assets = game_ptr->getAssetManager();
    if (!assets) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"TransitionState Error: AssetManager is null!"); duration_ = 0.01f; return; }
    if (duration <= 0.0f) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"TransitionState Warning: Duration zero/negative (%.2f). Setting to 0.01.", duration); duration_ = 0.01f; }

    // Texture and JSON loading logic... (remains the same)
    if (type_ == TransitionType::BOX_IN_TO_MENU) {
        borderAtlasTexture_ = assets->getTexture("transition_borders");
        if (borderAtlasTexture_) {
            const std::string jsonPath = "assets/ui/transition/transition_borders.json";
            if (!loadBorderRectsFromJson(jsonPath)) {
                 SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"loadBorderRectsFromJson returned false for '%s'! Effect may not draw.", jsonPath.c_str());
                 borderAtlasTexture_ = nullptr;
            } else {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,"TransitionState: loadBorderRectsFromJson returned true for '%s'.", jsonPath.c_str());
                if (borderTopSrcRect_.h <= 0 || borderBottomSrcRect_.h <= 0 || borderLeftSrcRect_.w <= 0 || borderRightSrcRect_.w <= 0) {
                   SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Constructor Post-load check failed: Rect dimensions are invalid!");
                   borderAtlasTexture_ = nullptr;
                }
            }
        } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"TransitionState: Border atlas texture 'transition_borders' not found!"); }
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TransitionState Constructor FINAL Check:");
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "  borderAtlasTexture_ Ptr: %p", (void*)borderAtlasTexture_);
    }
}

// --- Destructor ---
TransitionState::~TransitionState() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TransitionState Destroyed.");
}

// --- loadBorderRectsFromJson ---
// (Implementation remains the same)
bool TransitionState::loadBorderRectsFromJson(const std::string& jsonPath) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "--- loadBorderRectsFromJson (OBJECT VERSION) START ---");
    std::map<std::string, SDL_Rect> loadedRects;
    try {
        std::ifstream jsonFile(jsonPath);
        if (!jsonFile.is_open()) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "loadBorderRectsFromJson Error: Could not OPEN file: %s", jsonPath.c_str()); return false;}
        json data = json::parse(jsonFile); jsonFile.close();
        if (data.contains("frames") && data["frames"].is_object()) {
             const auto& framesNode = data["frames"];
             for (auto it = framesNode.begin(); it != framesNode.end(); ++it) {
                 std::string key = it.key(); const auto& frameData = it.value();
                 if (frameData.contains("frame")) {
                     const auto& rectData = frameData["frame"];
                     if (rectData.contains("x") && rectData.contains("y") && rectData.contains("w") && rectData.contains("h")) {
                          loadedRects[key] = { rectData["x"].get<int>(), rectData["y"].get<int>(), rectData["w"].get<int>(), rectData["h"].get<int>() };
                     } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "  JSON rect missing x,y,w or h for key '%s' in %s", key.c_str(), jsonPath.c_str());}
                 } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "  JSON frame missing 'frame' object for key '%s' in %s", key.c_str(), jsonPath.c_str());}
             }
        } else { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "loadBorderRectsFromJson Error: JSON missing 'frames' object or 'frames' is not an object."); return false;}
    } catch (std::exception& e) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "loadBorderRectsFromJson Error: Exception parsing %s: %s", jsonPath.c_str(), e.what()); return false; }

    bool success = true;
    if (loadedRects.count("border_top")) { borderTopSrcRect_ = loadedRects["border_top"]; } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "border_top missing"); success = false; }
    if (loadedRects.count("border_bottom")) { borderBottomSrcRect_ = loadedRects["border_bottom"]; } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "border_bottom missing"); success = false; }
    if (loadedRects.count("border_left")) { borderLeftSrcRect_ = loadedRects["border_left"]; } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "border_left missing"); success = false; }
    if (loadedRects.count("border_right")) { borderRightSrcRect_ = loadedRects["border_right"]; } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "border_right missing"); success = false; }
    if (success && !(borderTopSrcRect_.h > 0 && borderBottomSrcRect_.h > 0 && borderLeftSrcRect_.w > 0 && borderRightSrcRect_.w > 0)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "loadBorderRectsFromJson Error: One or more loaded rects have invalid dimensions."); success = false;
    }
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "--- loadBorderRectsFromJson END (returning %d) ---", success);
    return success;
}

// --- requestExit ---
// (Implementation remains the same)
void TransitionState::requestExit() {
    if (!transition_complete_requested_) {
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TransitionState: Exit requested. Requesting Pop(self).");
        transition_complete_requested_ = true;
        if (game_ptr) { // Check game_ptr before using
           game_ptr->requestPopState();
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"TransitionState::requestExit Error: game_ptr is null!");
        }
    }
}

// <<< --- MODIFIED handle_input Signature --- >>>
void TransitionState::handle_input(InputManager& inputManager, PlayerData* playerData) {
    // Only pass input to the state below if the transition visual is complete
    if (transitionComplete_ && belowState_) {
        // Call belowState's handle_input with the NEW signature
        belowState_->handle_input(inputManager, playerData);
    }
    // Transition state itself doesn't handle direct input
}
// <<< --- END MODIFIED handle_input --- >>>

// <<< --- MODIFIED update Signature --- >>>
void TransitionState::update(float delta_time, PlayerData* playerData) {
    // Update timer if visual transition isn't complete
    if (!transitionComplete_) {
        if (duration_ > 0.0f) timer_ += delta_time;
        if (timer_ >= duration_) {
            transitionComplete_ = true;
            timer_ = duration_; // Clamp timer
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TransitionState: Visual transition complete. State below is now fully active.");
        }
    }
    // If visual transition IS complete, update the state below
    else {
        if (belowState_) {
             // Call belowState's update with the NEW signature
             belowState_->update(delta_time, playerData);
        } else {
             // This case should ideally not happen if transitions are managed correctly
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "TransitionState: belowState is null after transition completed. Requesting self-pop.");
             requestExit(); // Pop self if state below disappeared
        }
    }
    // PlayerData* is passed but not directly used by the transition logic itself
}
// <<< --- END MODIFIED update --- >>>

// <<< --- MODIFIED render Signature --- >>>
void TransitionState::render(PCDisplay& display) {
    // PCDisplay object passed by reference

    // <<< --- Render the state below FIRST --- >>>
    if (belowState_) {
        // Call belowState's render with the NEW signature
        belowState_->render(display);
    }
    // <<< ---------------------------------- >>>

    // Determine interpolation factor 't' based on completion status
    float t = transitionComplete_ ? 1.0f : ((duration_ > 0.0f) ? std::min(1.0f, timer_ / duration_) : 1.0f);

    if (type_ == TransitionType::BOX_IN_TO_MENU) {
        // Asset validity check
        if (!borderAtlasTexture_ || borderTopSrcRect_.h <= 0 || borderBottomSrcRect_.h <= 0 || borderLeftSrcRect_.w <= 0 || borderRightSrcRect_.w <= 0 ) {
             // Prevent spamming log if assets consistently fail
             static bool logged_render_fail = false; if (!logged_render_fail) { SDL_LogError(SDL_LOG_CATEGORY_RENDER, "TransitionState Render Error: Border atlas texture or source rects invalid!"); logged_render_fail = true; }
             return; // Cannot draw effect
        }

        // Get window size from display object
        int windowW = 0, windowH = 0;
        display.getWindowSize(windowW, windowH);
        if (windowW <= 0 || windowH <= 0) { windowW = 466; windowH = 466; /* Log Warning */ }

        // Linear interpolation function (lerp)
        auto lerp = [](float start, float end, float factor) { return start + (end - start) * factor; };

        // Calculate current positions based on interpolation factor 't'
        // Positions move from off-screen (-Width/-Height or +Width/+Height) towards 0
        int topY = static_cast<int>(lerp((float)-borderTopSrcRect_.h, 0.0f, t)); // Use source height
        SDL_Rect topDst = {0, topY, windowW, borderTopSrcRect_.h};

        int bottomY = static_cast<int>(lerp((float)windowH, (float)(windowH - borderBottomSrcRect_.h), t)); // Use source height
        SDL_Rect botDst = {0, bottomY, windowW, borderBottomSrcRect_.h};

        int leftX = static_cast<int>(lerp((float)-borderLeftSrcRect_.w, 0.0f, t)); // Use source width
        SDL_Rect lefDst = {leftX, 0, borderLeftSrcRect_.w, windowH};

        int rightX = static_cast<int>(lerp((float)windowW, (float)(windowW - borderRightSrcRect_.w), t)); // Use source width
        SDL_Rect rigDst = {rightX, 0, borderRightSrcRect_.w, windowH};


        // Draw the wiping borders using the display object
        if (borderAtlasTexture_) {
             // Set blend mode etc. if needed (drawTexture might handle this)
             // SDL_SetTextureBlendMode(borderAtlasTexture_, SDL_BLENDMODE_BLEND);
        }

        // Use the display object's drawing method
        if (topDst.h > 0 && borderAtlasTexture_) display.drawTexture(borderAtlasTexture_, &borderTopSrcRect_, &topDst);
        if (botDst.h > 0 && borderAtlasTexture_) display.drawTexture(borderAtlasTexture_, &borderBottomSrcRect_, &botDst);
        if (lefDst.w > 0 && borderAtlasTexture_) display.drawTexture(borderAtlasTexture_, &borderLeftSrcRect_, &lefDst);
        if (rigDst.w > 0 && borderAtlasTexture_) display.drawTexture(borderAtlasTexture_, &borderRightSrcRect_, &rigDst);
    }
}
// <<< --- END MODIFIED render --- >>>

// --- getType ---
StateType TransitionState::getType() const {
    return StateType::Transition;
}