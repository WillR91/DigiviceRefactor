// File: src/states/TransitionState.cpp

#include "states/TransitionState.h"
#include "core/Game.h"
#include "core/AssetManager.h"
#include "platform/pc/pc_display.h"
#include "states/MenuState.h"
#include <SDL.h> // <<< Ensure SDL.h is included for SDL_Renderer etc. >>>
#include <SDL_log.h>
#include <stdexcept>
#include <fstream>
#include "vendor/nlohmann/json.hpp" // <<< Need for JSON parsing
#include <map>                      // <<< Need for std::map
#include <string>                   // <<< Need for std::string
#include <vector>                   // <<< Need for std::vector
#include <algorithm> // For std::min

// Use the nlohmann::json namespace
using json = nlohmann::json;

// --- Constructor ---
// (Includes detailed logging check at the end)
TransitionState::TransitionState(Game* game, GameState* belowState, float duration, TransitionType type) :
    belowState_(belowState),
    duration_(duration),
    timer_(0.0f),
    type_(type),
    borderAtlasTexture_(nullptr),
    transition_complete_requested_(false) // Initialize member variable
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
            const std::string jsonPath = "assets/ui/transition/transition_borders.json"; // Or "border_segments.json"
            // Call the potentially modified loadBorderRectsFromJson
            if (!loadBorderRectsFromJson(jsonPath)) {
                 // Log already happens inside loadBorderRectsFromJson or here
                 SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"loadBorderRectsFromJson returned false for '%s'! Effect may not draw.", jsonPath.c_str());
                 borderAtlasTexture_ = nullptr; // Nullify on failure
            } else {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,"TransitionState: loadBorderRectsFromJson returned true for '%s'.", jsonPath.c_str());
                // Dimension check post-load (Still important)
                if (borderTopSrcRect_.h <= 0 || borderBottomSrcRect_.h <= 0 || borderLeftSrcRect_.w <= 0 || borderRightSrcRect_.w <= 0) {
                   SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Constructor Post-load check failed: Rect dimensions are invalid!");
                   borderAtlasTexture_ = nullptr; // Nullify if dimensions bad
                }
            }
        } else {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"TransitionState: Border atlas texture 'transition_borders' not found!");
            // borderAtlasTexture_ is already nullptr here
        }

        // Log the final state after attempting load
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TransitionState Constructor FINAL Check:");
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "  borderAtlasTexture_ Ptr: %p", (void*)borderAtlasTexture_);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "  borderTopSrcRect_    : {%d, %d, %d, %d}", borderTopSrcRect_.x, borderTopSrcRect_.y, borderTopSrcRect_.w, borderTopSrcRect_.h);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "  borderBottomSrcRect_ : {%d, %d, %d, %d}", borderBottomSrcRect_.x, borderBottomSrcRect_.y, borderBottomSrcRect_.w, borderBottomSrcRect_.h);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "  borderLeftSrcRect_   : {%d, %d, %d, %d}", borderLeftSrcRect_.x, borderLeftSrcRect_.y, borderLeftSrcRect_.w, borderLeftSrcRect_.h);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "  borderRightSrcRect_  : {%d, %d, %d, %d}", borderRightSrcRect_.x, borderRightSrcRect_.y, borderRightSrcRect_.w, borderRightSrcRect_.h);

    } // End if (type_ == BOX_IN_TO_MENU)
} // <<< Closing brace for Constructor >>>


// --- Destructor ---
// (Remains unchanged)
TransitionState::~TransitionState() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TransitionState Destroyed.");
}

// --- Helper Function to load Rects from JSON ---
// <<< MODIFIED: Reverted to OBJECT handling version from Response #92 >>>
bool TransitionState::loadBorderRectsFromJson(const std::string& jsonPath) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "--- loadBorderRectsFromJson (OBJECT VERSION) START ---");
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Attempting to load from: %s", jsonPath.c_str());
    std::map<std::string, SDL_Rect> loadedRects; // Use this to store found rects

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

        // <<< --- CHECKING FOR OBJECT --- >>>
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Checking for 'frames' object...");
        if (data.contains("frames") && data["frames"].is_object()) {
             SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "'frames' object found. Iterating through items...");
             const auto& framesNode = data["frames"];

             // <<< --- ITERATING THROUGH OBJECT ITEMS (key-value pairs) --- >>>
             for (auto it = framesNode.begin(); it != framesNode.end(); ++it) {
                 std::string key = it.key(); // Get the key ("border_top", etc.)
                 const auto& frameData = it.value(); // Get the value object
                 SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Processing frame object key '%s'", key.c_str());

                 if (frameData.contains("frame")) {
                     SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  Element '%s' has 'frame'.", key.c_str());
                     const auto& rectData = frameData["frame"];
                     if (rectData.contains("x") && rectData.contains("y") && rectData.contains("w") && rectData.contains("h")) {
                          SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  'frame' object contains x, y, w, h.");
                          int x = rectData["x"].get<int>(); int y = rectData["y"].get<int>();
                          int w = rectData["w"].get<int>(); int h = rectData["h"].get<int>();
                          SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  Read values: x=%d, y=%d, w=%d, h=%d", x,y,w,h);

                          loadedRects[key] = { x, y, w, h }; // Store using the object key
                          SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  Stored rect '%s': {%d,%d,%d,%d}", key.c_str(), loadedRects[key].x, loadedRects[key].y, loadedRects[key].w, loadedRects[key].h);
                     } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "  JSON rect missing x,y,w or h for key '%s' in %s", key.c_str(), jsonPath.c_str());}
                 } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "  JSON frame missing 'frame' object for key '%s' in %s", key.c_str(), jsonPath.c_str());}
             } // End for loop
             SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Finished iterating through 'frames' object items.");

        } else {
            bool hasFrames = data.contains("frames");
            bool isObject = hasFrames && data["frames"].is_object();
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "loadBorderRectsFromJson Error: JSON missing 'frames' object or 'frames' is not an object. Has 'frames': %d, Is Object: %d", hasFrames, isObject);
            throw std::runtime_error("JSON missing 'frames' object or 'frames' is not an object.");
        }

    } catch (json::parse_error& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "loadBorderRectsFromJson JSON Parse Error in '%s': %s (at byte %zu)", jsonPath.c_str(), e.what(), e.byte);
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "--- loadBorderRectsFromJson END (returning false due to parse error) ---");
        return false;
    } catch (std::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "loadBorderRectsFromJson Error processing JSON '%s': %s", jsonPath.c_str(), e.what());
         SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "--- loadBorderRectsFromJson END (returning false due to std::exception) ---");
        return false;
    }

    // (Assignment and final check logic remains the same)
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Assigning loaded rects to member variables...");
    bool success = true;
    if (loadedRects.count("border_top")) { borderTopSrcRect_ = loadedRects["border_top"]; SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Assigned 'border_top'"); } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Rect 'border_top' not found in loaded data"); success = false; }
    if (loadedRects.count("border_bottom")) { borderBottomSrcRect_ = loadedRects["border_bottom"]; SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Assigned 'border_bottom'"); } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Rect 'border_bottom' not found in loaded data"); success = false; }
    if (loadedRects.count("border_left")) { borderLeftSrcRect_ = loadedRects["border_left"]; SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Assigned 'border_left'"); } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Rect 'border_left' not found in loaded data"); success = false; }
    if (loadedRects.count("border_right")) { borderRightSrcRect_ = loadedRects["border_right"]; SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Assigned 'border_right'"); } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Rect 'border_right' not found in loaded data"); success = false; }
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Performing final dimension check...");
    if (success && !(borderTopSrcRect_.h > 0 && borderBottomSrcRect_.h > 0 && borderLeftSrcRect_.w > 0 && borderRightSrcRect_.w > 0)) {
        /* Log dimension errors */ success = false;
    } else if (success) { SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Final dimension check PASSED."); }
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "--- loadBorderRectsFromJson END (returning %d) ---", success);
    return success;
}


// --- Handle Input ---
// (Remains unchanged)
void TransitionState::handle_input() { /* Empty */ }

// --- Update Function ---
// (Remains unchanged)
void TransitionState::update(float delta_time) {
    if (!game_ptr) return;
    if (duration_ > 0.0f) timer_ += delta_time;
    if (timer_ >= duration_ && !transition_complete_requested_) {
        transition_complete_requested_ = true;
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Transition complete. Requesting Pop(self) and Push(MenuState).");
        std::vector<std::string> mainMenuOptions = {"DIGIMON", "MAP", "RECOVER", "SETTINGS", "QUIT"};
        game_ptr->requestPushState(std::make_unique<MenuState>(game_ptr, mainMenuOptions));
        game_ptr->requestPopState();
    }
}

// --- Render Function ---
// <<< Uses Wipe Effect, Full Source Rects, Blend Mode NONE >>>
void TransitionState::render() {
    float current_t_progress = (duration_ > 0.0f) ? std::min(1.0f, timer_ / duration_) : 1.0f;
    SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "--- TransitionState Render START (t=%.3f) ---", current_t_progress);

    if (!game_ptr || !belowState_) { /* Error handling */ return; }
    PCDisplay* display = game_ptr->get_display();
    if (!display) { /* Error handling */ return; }

    // Render the state below FIRST - it will get covered
    belowState_->render();

    float t = current_t_progress;

    if (type_ == TransitionType::BOX_IN_TO_MENU) {
        // (Asset validity check remains the same - uses original SrcRect dimensions)
        if (!borderAtlasTexture_ || borderTopSrcRect_.h <= 0 || borderBottomSrcRect_.h <= 0 || borderLeftSrcRect_.w <= 0 || borderRightSrcRect_.w <= 0 ) {
             /* Log failure and return */
             static bool logged_render_fail = false; if (!logged_render_fail) { /* Log details */ logged_render_fail = true; }
             SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "--- TransitionState Render FAIL CHECK (Asset/Rect Invalid) ---");
            return;
        }
        SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Transition Render: Initial Asset/Rect check PASSED.");


        auto lerp = [](float start, float end, float factor) { return start + (end - start) * factor; };

        int windowW = 0, windowH = 0;
        display->getWindowSize(windowW, windowH);
        if (windowW <= 0 || windowH <= 0) { windowW = 800; windowH = 600; /* Log Warning */ }
        SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Transition Render: Window Size %dx%d", windowW, windowH);

        // --- START OF WIPE EFFECT LOGIC ---

        // <<< --- REMOVED BORDER_THICKNESS and SrcSlice calculations --- >>>

        // --- Calculate Wipe Destination Rectangles ---
        // (These calculations remain the same as the Wipe Effect)
        int topY = static_cast<int>(lerp((float)-windowH, 0.0f, t));
        SDL_Rect topDst = {0, topY, windowW, windowH};

        int bottomY = static_cast<int>(lerp((float)windowH, 0.0f, t));
        SDL_Rect botDst = {0, bottomY, windowW, windowH};

        int leftX = static_cast<int>(lerp((float)-windowW, 0.0f, t));
        SDL_Rect lefDst = {leftX, 0, windowW, windowH};

        int rightX = static_cast<int>(lerp((float)windowW, 0.0f, t));
        SDL_Rect rigDst = {rightX, 0, windowW, windowH};


        // --- Draw the wiping borders using FULL Source Rects ---
        if (borderAtlasTexture_) {
             SDL_SetTextureColorMod(borderAtlasTexture_, 255, 255, 255);
             SDL_SetTextureAlphaMod(borderAtlasTexture_, 255);
             // <<< USING BLEND MODE NONE >>>
             SDL_SetTextureBlendMode(borderAtlasTexture_, SDL_BLENDMODE_BLEND);
             SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Transition Wipe: Reset Texture Modulation (Blend Mode: BLEND)");
        }

        // Draw calls now use the original border*SrcRect_ member variables
        SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Transition Wipe TOP: Src={%d,%d,%d,%d} Dst={%d,%d,%d,%d}", borderTopSrcRect_.x, borderTopSrcRect_.y, borderTopSrcRect_.w, borderTopSrcRect_.h, topDst.x, topDst.y, topDst.w, topDst.h);
        if (topDst.h > 0 && borderAtlasTexture_) display->drawTexture(borderAtlasTexture_, &borderTopSrcRect_, &topDst); // Use Original SrcRect

        SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Transition Wipe BOT: Src={%d,%d,%d,%d} Dst={%d,%d,%d,%d}", borderBottomSrcRect_.x, borderBottomSrcRect_.y, borderBottomSrcRect_.w, borderBottomSrcRect_.h, botDst.x, botDst.y, botDst.w, botDst.h);
        if (botDst.h > 0 && borderAtlasTexture_) display->drawTexture(borderAtlasTexture_, &borderBottomSrcRect_, &botDst); // Use Original SrcRect

        SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Transition Wipe LFT: Src={%d,%d,%d,%d} Dst={%d,%d,%d,%d}", borderLeftSrcRect_.x, borderLeftSrcRect_.y, borderLeftSrcRect_.w, borderLeftSrcRect_.h, lefDst.x, lefDst.y, lefDst.w, lefDst.h);
        if (lefDst.w > 0 && borderAtlasTexture_) display->drawTexture(borderAtlasTexture_, &borderLeftSrcRect_, &lefDst); // Use Original SrcRect

        SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Transition Wipe RGT: Src={%d,%d,%d,%d} Dst={%d,%d,%d,%d}", borderRightSrcRect_.x, borderRightSrcRect_.y, borderRightSrcRect_.w, borderRightSrcRect_.h, rigDst.x, rigDst.y, rigDst.w, rigDst.h);
        if (rigDst.w > 0 && borderAtlasTexture_) display->drawTexture(borderAtlasTexture_, &borderRightSrcRect_, &rigDst); // Use Original SrcRect
        // --- <<< END OF WIPE EFFECT LOGIC --- >>>

    }
    SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "--- TransitionState Render END ---");
} // <<< Closing brace for render function >>>