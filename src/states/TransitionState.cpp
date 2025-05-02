// File: src/states/TransitionState.cpp

#include "states/TransitionState.h"
#include "core/Game.h"
#include "core/AssetManager.h"
#include "platform/pc/pc_display.h" // Needed for drawing
#include <SDL_log.h>
#include <stdexcept>                // For std::runtime_error
#include <fstream>                  // For std::ifstream
#include "vendor/nlohmann/json.hpp" // For JSON parsing (Ensure path is correct)
#include <map>                      // For std::map
#include <string>                   // For std::string
#include <algorithm>                // For std::min

// Use the nlohmann::json namespace
using json = nlohmann::json;

TransitionState::TransitionState(Game* game, GameState* belowState, float duration, TransitionType type) :
    belowState_(belowState),
    duration_(duration),
    timer_(0.0f),
    type_(type),
    borderAtlasTexture_(nullptr) // Initialize atlas ptr
    // Rects are default initialized to {0,0,0,0}
{
    this->game_ptr = game;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TransitionState Created (Type: %d, Duration: %.2f)", (int)type, duration);

    // Basic validation of pointers and duration
    if (!game_ptr || !belowState_) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"TransitionState Error: Null game or belowState pointer!");
        duration_ = 0.01f; return; // Fail fast
    }
    AssetManager* assets = game_ptr->getAssetManager();
    if (!assets) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"TransitionState Error: AssetManager is null!");
        duration_ = 0.01f; return; // Fail fast
    }
     if (duration <= 0.0f) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"TransitionState Warning: Duration zero/negative (%.2f). Setting to 0.01.", duration);
         duration_ = 0.01f;
    }

    // --- Get single atlas texture and load rects from JSON ---
    if (type_ == TransitionType::BOX_IN_TO_MENU) {
        borderAtlasTexture_ = assets->getTexture("transition_borders"); // Get atlas texture
        if (borderAtlasTexture_) {
            // Attempt to load rectangles from corresponding JSON
            if (!loadBorderRectsFromJson("assets/ui/transition/border_segments.json")) { // Use correct JSON path
                 SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Failed to load border rects from JSON! Effect may not draw.");
                 // Mark as failed by nulling texture? Or allow fallback?
                 borderAtlasTexture_ = nullptr;
            } else {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,"TransitionState: Border segment rects loaded successfully.");
            }
        } else {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"TransitionState: Border atlas texture 'transition_borders' not found!");
        }
    }
    // Load assets for other transition types here later...
}

TransitionState::~TransitionState() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TransitionState Destroyed.");
}

// --- Helper Function to load Rects from JSON ---
bool TransitionState::loadBorderRectsFromJson(const std::string& jsonPath) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Loading border rects from: %s", jsonPath.c_str());
    std::map<std::string, SDL_Rect> loadedRects; // Temporary map to store rects by name

    try {
        std::ifstream jsonFile(jsonPath);
        if (!jsonFile.is_open()) { throw std::runtime_error("Could not open JSON file: " + jsonPath); }
        json data = json::parse(jsonFile);
        jsonFile.close();

        // Expecting structure like: { "frames": { "border_top": {"frame": {x,y,w,h}}, ... } }
        if (data.contains("frames") && data["frames"].is_object()) {
             const auto& framesNode = data["frames"];
             for (auto& [key, frameData] : framesNode.items()) { // Key is "border_top", "border_bottom" etc.
                 if (frameData.contains("frame")) {
                     const auto& rectData = frameData["frame"];
                     if (rectData.contains("x") && rectData.contains("y") && rectData.contains("w") && rectData.contains("h")) {
                          loadedRects[key] = { // Store using the key from JSON
                              rectData["x"].get<int>(), rectData["y"].get<int>(),
                              rectData["w"].get<int>(), rectData["h"].get<int>() };
                          SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Loaded rect '%s': {%d,%d,%d,%d}", key.c_str(), loadedRects[key].x, loadedRects[key].y, loadedRects[key].w, loadedRects[key].h);
                     } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "JSON rect missing x,y,w or h for key '%s' in %s", key.c_str(), jsonPath.c_str());}
                 } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "JSON frame missing 'frame' object for key '%s' in %s", key.c_str(), jsonPath.c_str());}
             }
        } else { throw std::runtime_error("JSON missing 'frames' object or 'frames' is not an object."); }

    } catch (json::parse_error& e) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "JSON Parse Error in '%s': %s (at byte %zu)", jsonPath.c_str(), e.what(), e.byte); return false; }
      catch (std::exception& e) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error processing JSON '%s': %s", jsonPath.c_str(), e.what()); return false; }

    // Assign from map to member variables, checking if keys exist
    bool success = true;
    if (loadedRects.count("border_top"))    borderTopSrcRect_ = loadedRects["border_top"];
    else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Rect 'border_top' not found in JSON"); success = false; }

    if (loadedRects.count("border_bottom")) borderBottomSrcRect_ = loadedRects["border_bottom"];
    else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Rect 'border_bottom' not found in JSON"); success = false; }

    if (loadedRects.count("border_left"))   borderLeftSrcRect_ = loadedRects["border_left"];
    else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Rect 'border_left' not found in JSON"); success = false; }

    if (loadedRects.count("border_right"))  borderRightSrcRect_ = loadedRects["border_right"];
    else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Rect 'border_right' not found in JSON"); success = false; }

    // Check if all essential rects have valid dimensions
    if (!(borderTopSrcRect_.h > 0 && borderBottomSrcRect_.h > 0 && borderLeftSrcRect_.w > 0 && borderRightSrcRect_.w > 0)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "One or more loaded border rects have zero width/height.");
        success = false;
    }

    return success;
}


void TransitionState::handle_input() {
    // Transitions ignore input by default
}

void TransitionState::update(float delta_time) {
    if (!game_ptr) return; // Safety check
    timer_ += delta_time; // Increment timer

    // If timer exceeds duration, request to pop this state
    if (timer_ >= duration_) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Transition complete, requesting pop.");
        game_ptr->requestPopState();
    }
}

// --- render Function using Single Atlas ---
void TransitionState::render() {
    if (!game_ptr || !belowState_) return;
    PCDisplay* display = game_ptr->get_display();
     if (!display) return;

    // 1. Render the state underneath
    belowState_->render();

    // 2. Render the transition effect on top
    float t = std::min(1.0f, timer_ / duration_); // Calculate progress (0.0 to 1.0)

    if (type_ == TransitionType::BOX_IN_TO_MENU) {
        // Check if atlas texture and ALL rects are valid before drawing
        if (!borderAtlasTexture_ || borderTopSrcRect_.h <= 0 || borderBottomSrcRect_.h <= 0 || borderLeftSrcRect_.w <= 0 || borderRightSrcRect_.w <= 0 ) {
             SDL_LogWarn(SDL_LOG_CATEGORY_RENDER,"Transition Render: Missing border atlas texture or invalid rects, skipping effect.");
             // Optional: Add fallback fade effect here if desired
             // ... (Fallback fade code from previous example) ...
            return;
        }

        // Linear interpolation helper
        auto lerp = [](float start, float end, float factor) { return start + (end - start) * factor; };

        // Define target center square (adjust dimensions as needed)
        const int targetW = WINDOW_WIDTH / 2;
        const int targetH = WINDOW_HEIGHT / 2;
        const int targetX = (WINDOW_WIDTH - targetW) / 2;
        const int targetY = (WINDOW_HEIGHT - targetH) / 2;

        // Calculate positions and draw each segment using its specific SrcRect

        // --- Top Segment ---
        int topY = static_cast<int>(lerp((float)-borderTopSrcRect_.h, (float)targetY, t));
        int topX = targetX + (targetW / 2) - (borderTopSrcRect_.w / 2);
        SDL_Rect topDst = {topX, topY, borderTopSrcRect_.w, borderTopSrcRect_.h};
        display->drawTexture(borderAtlasTexture_, &borderTopSrcRect_, &topDst); // Use Top SrcRect

        // --- Bottom Segment ---
        int bottomY = static_cast<int>(lerp((float)WINDOW_HEIGHT, (float)(targetY + targetH - borderBottomSrcRect_.h), t));
        int bottomX = targetX + (targetW / 2) - (borderBottomSrcRect_.w / 2);
        SDL_Rect botDst = {bottomX, bottomY, borderBottomSrcRect_.w, borderBottomSrcRect_.h};
        display->drawTexture(borderAtlasTexture_, &borderBottomSrcRect_, &botDst); // Use Bottom SrcRect

        // --- Left Segment ---
        int leftX = static_cast<int>(lerp((float)-borderLeftSrcRect_.w, (float)targetX, t));
        int leftY = targetY + (targetH / 2) - (borderLeftSrcRect_.h / 2);
        SDL_Rect lefDst = {leftX, leftY, borderLeftSrcRect_.w, borderLeftSrcRect_.h};
        display->drawTexture(borderAtlasTexture_, &borderLeftSrcRect_, &lefDst); // Use Left SrcRect

         // --- Right Segment ---
         int rightX = static_cast<int>(lerp((float)WINDOW_WIDTH, (float)(targetX + targetW - borderRightSrcRect_.w), t));
         int rightY = targetY + (targetH / 2) - (borderRightSrcRect_.h / 2);
         SDL_Rect rigDst = {rightX, rightY, borderRightSrcRect_.w, borderRightSrcRect_.h};
         display->drawTexture(borderAtlasTexture_, &borderRightSrcRect_, &rigDst); // Use Right SrcRect

         SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Transition Render: t=%.2f", t);

    }
    // Add other transition types later...
}