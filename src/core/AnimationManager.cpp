// src/core/AnimationManager.cpp
#include "core/AnimationManager.h"
#include "core/AssetManager.h"        // Include full definition
#include "graphics/AnimationData.h" // Include full definition
#include "vendor/nlohmann/json.hpp" // For JSON parsing
#include <SDL_log.h>
#include <fstream>
#include <vector>
#include <stdexcept> // For potential exceptions

// Use nlohmann::json namespace
using json = nlohmann::json;

AnimationManager::AnimationManager(AssetManager* assetMgr) :
    assetManager_(assetMgr)
{
    if (!assetManager_) {
        // This is critical, throw an exception or log fatal error
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "AnimationManager created with a null AssetManager!");
        throw std::runtime_error("AnimationManager requires a valid AssetManager pointer.");
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager instance created.");
}

AnimationManager::~AnimationManager() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager instance destroyed.");
    // No explicit cleanup needed here as AnimationData holds non-owning pointers
    // and the map will clean itself up.
}

// Helper function to parse frame rects from JSON data.
bool AnimationManager::parseFrameRects(const json& framesNode, std::vector<SDL_Rect>& outFrameRects) {
    outFrameRects.clear();
    try {
        if (framesNode.is_array()) {
            for (const auto& frameData : framesNode) {
                 if (frameData.contains("frame")) {
                    const auto& rectData = frameData["frame"];
                    if (rectData.contains("x") && rectData.contains("y") && rectData.contains("w") && rectData.contains("h")) {
                        int w = rectData["w"].get<int>();
                        int h = rectData["h"].get<int>();
                        if (w > 0 && h > 0) { // Basic validation
                             outFrameRects.push_back({ rectData["x"].get<int>(), rectData["y"].get<int>(), w, h });
                        } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "parseFrameRects: Invalid w/h (%d,%d) in frame data (array).", w, h); }
                    } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "parseFrameRects: Frame data missing x,y,w, or h (array)."); }
                 } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "parseFrameRects: Array item missing 'frame' object."); }
            }
        } else if (framesNode.is_object()) {
            // Assume keys are frame names/numbers, iterate through values
            for (auto& [key, frameData] : framesNode.items()) {
                 if (frameData.contains("frame")) {
                     const auto& rectData = frameData["frame"];
                    if (rectData.contains("x") && rectData.contains("y") && rectData.contains("w") && rectData.contains("h")) {
                         int w = rectData["w"].get<int>();
                         int h = rectData["h"].get<int>();
                         if (w > 0 && h > 0) { // Basic validation
                             outFrameRects.push_back({ rectData["x"].get<int>(), rectData["y"].get<int>(), w, h });
                         } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "parseFrameRects: Invalid w/h (%d,%d) in frame data (object key: %s).", w, h, key.c_str()); }
                     } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "parseFrameRects: Frame data missing x,y,w, or h (object key: %s).", key.c_str()); }
                 } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "parseFrameRects: Object item missing 'frame' object (key: %s).", key.c_str()); }
            }
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "parseFrameRects Error: 'frames' node is not an array or object.");
            return false;
        }
    } catch (const json::exception& e) {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "parseFrameRects Error: JSON exception while parsing frames: %s", e.what());
         return false;
    }
    return !outFrameRects.empty();
}


// Helper function to create and store a single AnimationData instance.
void AnimationManager::storeAnimation(
    const std::string& animId,
    SDL_Texture* texture,
    const std::vector<SDL_Rect>& frameRects,
    const std::vector<float>& durationsSec,
    bool loops)
{
    if (animId.empty() || !texture || frameRects.empty() || frameRects.size() != durationsSec.size()) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "storeAnimation: Invalid data provided for ID '%s'. Skipping.", animId.c_str());
        return;
    }

    AnimationData data;
    data.id = animId;
    data.textureAtlas = texture;
    data.frameRects = frameRects;
    data.frameDurationsSec = durationsSec;
    data.loops = loops;

    // Check for validity before storing
    if (!data.isValid()) {
         SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "storeAnimation: Constructed AnimationData for '%s' is invalid. Skipping.", animId.c_str());
         return;
    }

    auto result = loadedAnimations_.emplace(animId, std::move(data)); // Use emplace for efficiency
    if (result.second) {
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Stored animation definition: '%s'", animId.c_str());
    } else {
         SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "storeAnimation: Animation definition with ID '%s' already exists. Overwriting.", animId.c_str());
         loadedAnimations_[animId] = std::move(data); // Overwrite if emplace failed due to existing key
    }
}


// Loads animation definitions from a JSON file.
bool AnimationManager::loadAnimationDataFromFile(const std::string& jsonPath, const std::string& textureAtlasId) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager: Loading animations from '%s' using texture '%s'", jsonPath.c_str(), textureAtlasId.c_str());

    // 1. Get the texture pointer from AssetManager
    if (!assetManager_) return false; // Should have been checked in constructor
    SDL_Texture* texture = assetManager_->getTexture(textureAtlasId);
    if (!texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager Error: Texture '%s' not found in AssetManager.", textureAtlasId.c_str());
        return false;
    }

    // 2. Open and parse the JSON file
    std::ifstream jsonFile(jsonPath);
    if (!jsonFile.is_open()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager Error: Failed to open JSON file: %s", jsonPath.c_str());
        return false;
    }

    json data;
    try {
        data = json::parse(jsonFile);
        jsonFile.close();
    } catch (const json::parse_error& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager Error: Failed to parse JSON file '%s': %s (at byte %zu)", jsonPath.c_str(), e.what(), e.byte);
        return false;
    } catch (const std::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager Error: Exception reading JSON file '%s': %s", jsonPath.c_str(), e.what());
        return false;
    }

    // 3. Parse frame rectangles (assuming a top-level "frames" key like Aseprite export)
    std::vector<SDL_Rect> allFrameRects;
    if (data.contains("frames")) {
        if (!parseFrameRects(data["frames"], allFrameRects)) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager Error: Failed to parse frame rectangles from '%s'.", jsonPath.c_str());
            return false;
        }
    } else {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager Error: JSON file '%s' missing 'frames' data.", jsonPath.c_str());
         return false;
    }
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,"Parsed %zu total frames from %s", allFrameRects.size(), jsonPath.c_str());


    // 4. Parse animation sequences (assuming a top-level "meta" -> "frameTags" key like Aseprite)
    //    Or adapt to your specific JSON structure for defining sequences.
    //    This example assumes Aseprite structure.
    int animationsLoaded = 0;
    if (data.contains("meta") && data["meta"].contains("frameTags") && data["meta"]["frameTags"].is_array()) {
        const auto& frameTags = data["meta"]["frameTags"];
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Found %zu frameTags (animation sequences).", frameTags.size());

        for (const auto& tag : frameTags) {
            if (tag.contains("name") && tag.contains("from") && tag.contains("to") && tag.contains("direction")) {
                std::string animName = tag["name"].get<std::string>();
                int fromIndex = tag["from"].get<int>();
                int toIndex = tag["to"].get<int>();
                std::string direction = tag["direction"].get<std::string>(); // "forward", "reverse", "pingpong"

                // Construct the ID used for storage (e.g., "Agumon_Idle")
                std::string animationId = textureAtlasId + "_" + animName;

                // Extract frame indices and durations for this sequence
                std::vector<SDL_Rect> sequenceFrameRects;
                std::vector<float> sequenceDurationsSec;
                bool sequenceLoops = (direction != "forward"); // Crude guess: Only forward doesn't loop? Aseprite doesn't store loop bool directly. Often pingpong/reverse imply looping. Assume forward = no loop, others = loop. Needs refinement based on your needs.


                if (direction == "forward" || direction == "pingpong" || direction == "reverse") {
                    // --- Extract frame data for the sequence ---
                    // TODO: Need the frame duration from the main 'frames' data - Aseprite JSON stores duration per frame.
                    // We need to map indices 'from'/'to' to the 'allFrameRects' and find their durations in the original JSON 'frames' data.
                    // This requires a more complex parsing step than just grabbing rects.
                    // For now, let's assign a default duration and use the rects directly.
                    // THIS PART NEEDS SIGNIFICANT REFINEMENT BASED ON JSON STRUCTURE

                    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Animation sequence parsing from frameTags is NOT fully implemented. Using default durations.");
                     for (int i = fromIndex; i <= toIndex; ++i) {
                          if (i >= 0 && static_cast<size_t>(i) < allFrameRects.size()) {
                               sequenceFrameRects.push_back(allFrameRects[i]);
                               sequenceDurationsSec.push_back(0.3f); // !!! PLACEHOLDER DURATION !!! Need to get from JSON frame data
                          } else {
                                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"Frame index %d out of bounds for animation '%s'", i, animationId.c_str());
                          }
                     }
                     // TODO: Handle reverse and pingpong directions properly by adding frames in reverse/back-and-forth order


                    // Store the parsed animation
                    if (!sequenceFrameRects.empty()) {
                        storeAnimation(animationId, texture, sequenceFrameRects, sequenceDurationsSec, sequenceLoops);
                        animationsLoaded++;
                    }
                } else {
                    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unsupported frameTag direction '%s' for animation '%s'", direction.c_str(), animationId.c_str());
                }
            } else {
                 SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "FrameTag missing name, from, to, or direction in %s", jsonPath.c_str());
            }
        }
    } else {
         SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "JSON file '%s' missing 'meta/frameTags' array. Cannot load named sequences.", jsonPath.c_str());
         // --- Fallback: Create default animations if no tags? ---
         // Example: Create "TextureName_All" animation using all frames with default duration
         if (!allFrameRects.empty()) {
              std::vector<float> defaultDurations(allFrameRects.size(), 0.1f); // Default 100ms per frame
              storeAnimation(textureAtlasId + "_All", texture, allFrameRects, defaultDurations, true); // Assume loops
              animationsLoaded++;
              SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,"Created fallback '_All' animation for '%s'", textureAtlasId.c_str());
         }
    }


    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager: Finished loading from '%s'. Stored %d new animation definitions.", jsonPath.c_str(), animationsLoaded);
    return animationsLoaded > 0; // Return true if at least one animation was loaded
}


// Retrieves a pointer to a previously loaded AnimationData definition.
const AnimationData* AnimationManager::getAnimationData(const std::string& animationId) const {
    auto it = loadedAnimations_.find(animationId);
    if (it != loadedAnimations_.end()) {
        return &(it->second); // Return pointer to the found AnimationData
    }
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager::getAnimationData: Animation definition with ID '%s' not found.", animationId.c_str());
    return nullptr; // Not found
}