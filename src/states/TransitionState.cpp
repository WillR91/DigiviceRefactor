// File: src/states/TransitionState.cpp

#include "states/TransitionState.h"
#include "core/Game.h"
#include "core/AssetManager.h"
#include "platform/pc/pc_display.h"
#include "states/MenuState.h" // Included for type checking/casting if needed
#include <SDL.h>
#include <SDL_log.h>
#include <stdexcept>
#include <fstream>
#include "vendor/nlohmann/json.hpp"
#include <map>
#include <string>
#include <vector>
#include <algorithm> // For std::min, std::max

// Use the nlohmann::json namespace
using json = nlohmann::json;

// --- Constructor ---
TransitionState::TransitionState(Game* game, GameState* belowState, float duration, TransitionType type) :
    belowState_(belowState),
    duration_(duration),
    timer_(0.0f),
    type_(type),
    borderAtlasTexture_(nullptr),
    transition_complete_requested_(false),
    transitionComplete_(false)
{
    this->game_ptr = game;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TransitionState Created (Type: %d, Duration: %.2f)", (int)type, duration);
    // (Validation logic...)
    if (!game_ptr || !belowState_) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"TransitionState Error: Null game or belowState pointer!"); duration_ = 0.01f; return; }
    AssetManager* assets = game_ptr->getAssetManager();
    if (!assets) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"TransitionState Error: AssetManager is null!"); duration_ = 0.01f; return; }
    if (duration <= 0.0f) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"TransitionState Warning: Duration zero/negative (%.2f). Setting to 0.01.", duration); duration_ = 0.01f; }
    // (Texture and JSON loading logic...)
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
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "  borderTopSrcRect_    : {%d, %d, %d, %d}", borderTopSrcRect_.x, borderTopSrcRect_.y, borderTopSrcRect_.w, borderTopSrcRect_.h);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "  borderBottomSrcRect_ : {%d, %d, %d, %d}", borderBottomSrcRect_.x, borderBottomSrcRect_.y, borderBottomSrcRect_.w, borderBottomSrcRect_.h);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "  borderLeftSrcRect_   : {%d, %d, %d, %d}", borderLeftSrcRect_.x, borderLeftSrcRect_.y, borderLeftSrcRect_.w, borderLeftSrcRect_.h);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "  borderRightSrcRect_  : {%d, %d, %d, %d}", borderRightSrcRect_.x, borderRightSrcRect_.y, borderRightSrcRect_.w, borderRightSrcRect_.h);
    }
}

// --- Destructor ---
TransitionState::~TransitionState() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TransitionState Destroyed.");
}

// --- loadBorderRectsFromJson (OBJECT version) ---
bool TransitionState::loadBorderRectsFromJson(const std::string& jsonPath) {
    // (Implementation from response #94 - unchanged)
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "--- loadBorderRectsFromJson (OBJECT VERSION) START ---");
    std::map<std::string, SDL_Rect> loadedRects;
    try {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Opening file stream...");
        std::ifstream jsonFile(jsonPath);
        if (!jsonFile.is_open()) { /* ... error handling ... */ throw std::runtime_error("Could not open JSON file stream: " + jsonPath); }
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
                 // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Processing frame object key '%s'", key.c_str());
                 if (frameData.contains("frame")) {
                     const auto& rectData = frameData["frame"];
                     if (rectData.contains("x") && rectData.contains("y") && rectData.contains("w") && rectData.contains("h")) {
                          int x = rectData["x"].get<int>(); int y = rectData["y"].get<int>();
                          int w = rectData["w"].get<int>(); int h = rectData["h"].get<int>();
                          loadedRects[key] = { x, y, w, h };
                          // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  Stored rect '%s': {%d,%d,%d,%d}", key.c_str(), loadedRects[key].x, loadedRects[key].y, loadedRects[key].w, loadedRects[key].h);
                     } else { /* Log Warn */ }
                 } else { /* Log Warn */ }
             }
             SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Finished iterating through 'frames' object items.");
        } else { /* Log Error */ throw std::runtime_error("JSON missing 'frames' object or 'frames' is not an object."); }
    } catch (json::parse_error& e) { /* Log Error */ (void)e; return false; }
      catch (std::exception& e) { /* Log Error */ (void)e; return false; }
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Assigning loaded rects to member variables...");
    bool success = true;
    if (loadedRects.count("border_top")) { borderTopSrcRect_ = loadedRects["border_top"]; } else { /* Log Warn */ success = false; }
    if (loadedRects.count("border_bottom")) { borderBottomSrcRect_ = loadedRects["border_bottom"]; } else { /* Log Warn */ success = false; }
    if (loadedRects.count("border_left")) { borderLeftSrcRect_ = loadedRects["border_left"]; } else { /* Log Warn */ success = false; }
    if (loadedRects.count("border_right")) { borderRightSrcRect_ = loadedRects["border_right"]; } else { /* Log Warn */ success = false; }
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
// <<< MODIFIED: Frame Effect with Porthole and Corrected Dst Rect calculations >>>
void TransitionState::render() {
    // Determine interpolation factor 't' based on completion status
    float t = transitionComplete_ ? 1.0f : ((duration_ > 0.0f) ? std::min(1.0f, timer_ / duration_) : 1.0f);
    // SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "--- TransitionState Render START (t=%.3f) ---", t);

    if (!game_ptr) { SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Transition Render Error: Null game_ptr"); return; }
    PCDisplay* display = game_ptr->get_display();
    if (!display) { SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Transition Render Error: Null display"); return; }

    // Render the state below (MenuState or AdventureState) FIRST
    if (belowState_) {
        belowState_->render();
    }

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
        if (windowW <= 0 || windowH <= 0) { windowW = 466; windowH = 466; /* Use fallback */ }
        // SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Transition Render: Window Size %dx%d", windowW, windowH);

        // --- START OF CORRECTED FRAME LOGIC ---

        // <<< --- DEFINE PORTHOLE SIZE --- >>>
        const int portholeWidth = 1;  // <<< YOU MUST ADJUST THIS VALUE >>>
        const int portholeHeight = 1; // <<< YOU MUST ADJUST THIS VALUE >>>
        // Basic validation (Allow 0 size now, handled by thickness calc)
        if (portholeWidth > windowW || portholeHeight > windowH || portholeWidth < 1 || portholeHeight < 1) {
             SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Invalid porthole size requested!");
             return; // Cannot proceed
        }
        // <<< ---------------------------- >>>

        // Calculate porthole coordinates
        const int portholeX = (windowW - portholeWidth) / 2;
        const int portholeY = (windowH - portholeHeight) / 2;

        // Calculate the thickness of the borders needed
        // Ensure thickness isn't negative if porthole is larger than window somehow (should be caught above)
        const int horizontalBorderThickness = std::max(0, portholeX);
        const int verticalBorderThickness = std::max(0, portholeY);

        // Calculate END positions for the LEADING edges of the borders
        const float topEndY = 0.0f;
        const float bottomEndY = static_cast<float>(portholeY + portholeHeight);
        const float leftEndX = 0.0f;
        const float rightEndX = static_cast<float>(portholeX + portholeWidth);

        // Calculate Interpolated Positions for the LEADING edges
        // Start slightly off-screen using the calculated border thickness
        int topY = static_cast<int>(lerp((float)-verticalBorderThickness, topEndY, t));
        int bottomY = static_cast<int>(lerp((float)windowH, bottomEndY, t));
        int leftX = static_cast<int>(lerp((float)-horizontalBorderThickness, leftEndX, t));
        int rightX = static_cast<int>(lerp((float)windowW, rightEndX, t));

        // Use FULL Source Rects (as decided before)
        SDL_Rect topSrc = borderTopSrcRect_;
        SDL_Rect botSrc = borderBottomSrcRect_;
        SDL_Rect lefSrc = borderLeftSrcRect_;
        SDL_Rect rigSrc = borderRightSrcRect_;

        // --- Calculate Corrected Destination Rectangles ---
        // Top border: Covers area from its current top (topY) down to the porthole top (portholeY)
        SDL_Rect topDst = {0, topY, windowW, portholeY - topY };
        if (topDst.h < 0) topDst.h = 0; // Prevent negative height early in transition

        // Bottom border: Covers area from its current top (bottomY) down to the window bottom
        SDL_Rect botDst = {0, bottomY, windowW, windowH - bottomY };
        if (botDst.h < 0) botDst.h = 0;

        // Left border: Covers area from its current left (leftX) across to the porthole left (portholeX)
        SDL_Rect lefDst = {leftX, 0, portholeX - leftX, windowH};
        if (lefDst.w < 0) lefDst.w = 0; // Prevent negative width early in transition

        // Right border: Covers area from its current left (rightX) across to the window right
        SDL_Rect rigDst = {rightX, 0, windowW - rightX, windowH};
        if (rigDst.w < 0) rigDst.w = 0;

        // --- Draw the borders ---
        if (borderAtlasTexture_) {
             SDL_SetTextureColorMod(borderAtlasTexture_, 255, 255, 255);
             SDL_SetTextureAlphaMod(borderAtlasTexture_, 255);
             SDL_SetTextureBlendMode(borderAtlasTexture_, SDL_BLENDMODE_BLEND); // Use BLEND
        }

        // Draw calls using original source rects and calculated destinations
        // Optional: Add logging for Dst rects here if needed for debugging position/size
        if (topDst.h > 0 && borderAtlasTexture_) display->drawTexture(borderAtlasTexture_, &topSrc, &topDst);
        if (botDst.h > 0 && borderAtlasTexture_) display->drawTexture(borderAtlasTexture_, &botSrc, &botDst);
        if (lefDst.w > 0 && borderAtlasTexture_) display->drawTexture(borderAtlasTexture_, &lefSrc, &lefDst);
        if (rigDst.w > 0 && borderAtlasTexture_) display->drawTexture(borderAtlasTexture_, &rigSrc, &rigDst);
        // --- <<< END OF CORRECTED FRAME LOGIC --- >>>

    }
    // SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "--- TransitionState Render END ---");
} // <<< Closing brace for render function >>>
