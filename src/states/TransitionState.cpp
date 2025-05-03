// File: src/states/TransitionState.cpp

#include "states/TransitionState.h"
#include "core/Game.h"
#include "core/AssetManager.h"
#include "platform/pc/pc_display.h"
#include "states/MenuState.h" // Included for type checking/casting if needed, though not strictly required by this version
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
    belowState_(belowState),
    duration_(duration),
    timer_(0.0f),
    type_(type),
    borderAtlasTexture_(nullptr),
    transition_complete_requested_(false), // For exit request
    transitionComplete_(false) // Initialize new member
{
    this->game_ptr = game;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TransitionState Created (Type: %d, Duration: %.2f)", (int)type, duration);

    // Validation logic...
    if (!game_ptr || !belowState_) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"TransitionState Error: Null game or belowState pointer!"); duration_ = 0.01f; return; }
    AssetManager* assets = game_ptr->getAssetManager();
    if (!assets) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"TransitionState Error: AssetManager is null!"); duration_ = 0.01f; return; }
    if (duration <= 0.0f) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"TransitionState Warning: Duration zero/negative (%.2f). Setting to 0.01.", duration); duration_ = 0.01f; }

    // Texture and JSON loading logic...
    if (type_ == TransitionType::BOX_IN_TO_MENU) {
        borderAtlasTexture_ = assets->getTexture("transition_borders");
        if (borderAtlasTexture_) {
            const std::string jsonPath = "assets/ui/transition/transition_borders.json";
            // Use the OBJECT version of the loader
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
        // Logging check
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TransitionState Constructor FINAL Check:");
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "  borderAtlasTexture_ Ptr: %p", (void*)borderAtlasTexture_);
        // ... Log rects ...
    }
}

// --- Destructor ---
TransitionState::~TransitionState() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TransitionState Destroyed.");
}

// --- loadBorderRectsFromJson (OBJECT version) ---
bool TransitionState::loadBorderRectsFromJson(const std::string& jsonPath) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "--- loadBorderRectsFromJson (OBJECT VERSION) START ---");
    std::map<std::string, SDL_Rect> loadedRects;
    try {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Opening file stream...");
        std::ifstream jsonFile(jsonPath);
        if (!jsonFile.is_open()) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "loadBorderRectsFromJson Error: Could not OPEN file: %s", jsonPath.c_str());
            throw std::runtime_error("Could not open JSON file stream: " + jsonPath);
        }
         SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "File stream opened. Parsing JSON...");
        json data = json::parse(jsonFile);
        jsonFile.close();
         SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "JSON parsed successfully.");
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Checking for 'frames' object...");
        if (data.contains("frames") && data["frames"].is_object()) {
             SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "'frames' object found. Iterating through items...");
             const auto& framesNode = data["frames"];
             for (auto it = framesNode.begin(); it != framesNode.end(); ++it) {
                 std::string key = it.key();
                 const auto& frameData = it.value();
                 SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Processing frame object key '%s'", key.c_str());
                 if (frameData.contains("frame")) {
                     SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  Element '%s' has 'frame'.", key.c_str());
                     const auto& rectData = frameData["frame"];
                     if (rectData.contains("x") && rectData.contains("y") && rectData.contains("w") && rectData.contains("h")) {
                          SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  'frame' object contains x, y, w, h.");
                          int x = rectData["x"].get<int>(); int y = rectData["y"].get<int>();
                          int w = rectData["w"].get<int>(); int h = rectData["h"].get<int>();
                          SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  Read values: x=%d, y=%d, w=%d, h=%d", x,y,w,h);
                          loadedRects[key] = { x, y, w, h };
                          SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  Stored rect '%s': {%d,%d,%d,%d}", key.c_str(), loadedRects[key].x, loadedRects[key].y, loadedRects[key].w, loadedRects[key].h);
                     } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "  JSON rect missing x,y,w or h for key '%s' in %s", key.c_str(), jsonPath.c_str());}
                 } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "  JSON frame missing 'frame' object for key '%s' in %s", key.c_str(), jsonPath.c_str());}
             }
             SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Finished iterating through 'frames' object items.");
        } else {
            bool hasFrames = data.contains("frames"); bool isObject = hasFrames && data["frames"].is_object();
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "loadBorderRectsFromJson Error: JSON missing 'frames' object or 'frames' is not an object. Has 'frames': %d, Is Object: %d", hasFrames, isObject);
            throw std::runtime_error("JSON missing 'frames' object or 'frames' is not an object.");
        }
    } catch (json::parse_error& e) { /* ... */ return false; }
      catch (std::exception& e) { /* ... */ return false; }
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Assigning loaded rects to member variables...");
    bool success = true;
    if (loadedRects.count("border_top")) { borderTopSrcRect_ = loadedRects["border_top"]; /* Log */ } else { /* Log Warn */ success = false; }
    if (loadedRects.count("border_bottom")) { borderBottomSrcRect_ = loadedRects["border_bottom"]; /* Log */ } else { /* Log Warn */ success = false; }
    if (loadedRects.count("border_left")) { borderLeftSrcRect_ = loadedRects["border_left"]; /* Log */ } else { /* Log Warn */ success = false; }
    if (loadedRects.count("border_right")) { borderRightSrcRect_ = loadedRects["border_right"]; /* Log */ } else { /* Log Warn */ success = false; }
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Performing final dimension check...");
    if (success && !(borderTopSrcRect_.h > 0 && borderBottomSrcRect_.h > 0 && borderLeftSrcRect_.w > 0 && borderRightSrcRect_.w > 0)) {
        /* Log dimension errors */ success = false;
    } else if (success) { SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Final dimension check PASSED."); }
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "--- loadBorderRectsFromJson END (returning %d) ---", success);
    return success;
}

// --- requestExit ---
void TransitionState::requestExit() {
    if (!transition_complete_requested_) {
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TransitionState: Exit requested by belowState. Requesting Pop(self).");
        transition_complete_requested_ = true;
        game_ptr->requestPopState();
    }
}

// --- handle_input ---
void TransitionState::handle_input() {
    if (transitionComplete_ && belowState_) {
        belowState_->handle_input();
    }
}

// --- update ---
void TransitionState::update(float delta_time) {
    if (!game_ptr) return;
    if (!transitionComplete_) {
        if (duration_ > 0.0f) timer_ += delta_time;
        if (timer_ >= duration_) {
            transitionComplete_ = true;
            timer_ = duration_;
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TransitionState: Visual transition complete. Menu is now active.");
        }
    } else {
        if (belowState_) {
             belowState_->update(delta_time);
        } else {
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "TransitionState: belowState is null after transition completed. Requesting self-pop.");
             requestExit();
        }
    }
}

// --- Render Function ---
// <<< MODIFIED: Renders MenuState below, draws Wipe Effect (animated or static) on top >>>
void TransitionState::render() {
    // float current_t_progress = (duration_ > 0.0f) ? std::min(1.0f, timer_ / duration_) : 1.0f;
    // SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "--- TransitionState Render START (t=%.3f) ---", current_t_progress);

    if (!game_ptr) { SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Transition Render Error: Null game_ptr"); return; }
    PCDisplay* display = game_ptr->get_display();
    if (!display) { SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Transition Render Error: Null display"); return; }

    // <<< --- Render the state below (MenuState) FIRST --- >>>
    if (belowState_) {
        belowState_->render();
    }
    // <<< --------------------------------------------- >>>

    // Determine interpolation factor 't' based on completion status
    float t = transitionComplete_ ? 1.0f : ((duration_ > 0.0f) ? std::min(1.0f, timer_ / duration_) : 1.0f);

    if (type_ == TransitionType::BOX_IN_TO_MENU) {
        // Asset validity check
        if (!borderAtlasTexture_ || borderTopSrcRect_.h <= 0 || borderBottomSrcRect_.h <= 0 || borderLeftSrcRect_.w <= 0 || borderRightSrcRect_.w <= 0 ) {
             static bool logged_render_fail = false; if (!logged_render_fail) { /* Log details */ logged_render_fail = true; }
             SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "--- TransitionState Render FAIL CHECK (Asset/Rect Invalid) ---");
            return;
        }
        // SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Transition Render: Initial Asset/Rect check PASSED.");


        auto lerp = [](float start, float end, float factor) { return start + (end - start) * factor; };
        int windowW = 0, windowH = 0; display->getWindowSize(windowW, windowH);
        if (windowW <= 0 || windowH <= 0) { windowW = 800; windowH = 600; /* Log Warning */ }


        // Wipe Effect Logic (Using FULL Source Rects)
        int topY = static_cast<int>(lerp((float)-windowH, 0.0f, t));
        SDL_Rect topDst = {0, topY, windowW, windowH};
        int bottomY = static_cast<int>(lerp((float)windowH, 0.0f, t));
        SDL_Rect botDst = {0, bottomY, windowW, windowH};
        int leftX = static_cast<int>(lerp((float)-windowW, 0.0f, t));
        SDL_Rect lefDst = {leftX, 0, windowW, windowH};
        int rightX = static_cast<int>(lerp((float)windowW, 0.0f, t));
        SDL_Rect rigDst = {rightX, 0, windowW, windowH};


        // Draw the wiping borders using FULL Source Rects
        if (borderAtlasTexture_) {
             SDL_SetTextureColorMod(borderAtlasTexture_, 255, 255, 255);
             SDL_SetTextureAlphaMod(borderAtlasTexture_, 255);
             SDL_SetTextureBlendMode(borderAtlasTexture_, SDL_BLENDMODE_BLEND); // Use BLEND as source has transparency
        }

        // Draw calls using original border*SrcRect_
        if (topDst.h > 0 && borderAtlasTexture_) display->drawTexture(borderAtlasTexture_, &borderTopSrcRect_, &topDst);
        if (botDst.h > 0 && borderAtlasTexture_) display->drawTexture(borderAtlasTexture_, &borderBottomSrcRect_, &botDst);
        if (lefDst.w > 0 && borderAtlasTexture_) display->drawTexture(borderAtlasTexture_, &borderLeftSrcRect_, &lefDst);
        if (rigDst.w > 0 && borderAtlasTexture_) display->drawTexture(borderAtlasTexture_, &borderRightSrcRect_, &rigDst);
    }
    // SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "--- TransitionState Render END ---");
}