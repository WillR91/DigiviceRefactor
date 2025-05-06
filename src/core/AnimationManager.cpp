// src/core/AnimationManager.cpp
#include "core/AnimationManager.h"
#include "core/AssetManager.h"      // Include full definition
#include "graphics/AnimationData.h" // Include full definition
#include "graphics/AnimationDefinitions.h"
#include "vendor/nlohmann/json.hpp" // For JSON parsing
#include <SDL_log.h>
#include <fstream>
#include <vector>
#include <stdexcept> // For potential exceptions
#include <map>       // <<<--- Needed for map lookups ---<<<
#include <string>    // Needed for stoi, substr, etc. in parseFrameRects

// Use nlohmann::json namespace
using json = nlohmann::json;

// --- Keep hardcoded definitions (Consider moving to a constants file later) ---
const std::vector<int> IDLE_INDICES = {0, 1};
const std::vector<Uint32> IDLE_DURATIONS_MS = {1000, 1000}; // In Milliseconds
const std::vector<int> WALK_INDICES = {2, 3, 2, 3};
const std::vector<Uint32> WALK_DURATIONS_MS = {300, 300, 300, 300}; // In Milliseconds
// Add other animation definitions (Attack, Hurt, etc.) here if needed
// const std::vector<int> ATTACK_INDICES = {1, 0, 3, 8};
// const std::vector<Uint32> ATTACK_DURATIONS_MS = {200, 150, 150, 400};


AnimationManager::AnimationManager(AssetManager* assetMgr) :
    assetManager_(assetMgr)
{
    if (!assetManager_) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "AnimationManager created with a null AssetManager!");
        throw std::runtime_error("AnimationManager requires a valid AssetManager pointer.");
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager instance created.");
}

AnimationManager::~AnimationManager() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager instance destroyed.");
}

// Helper: Parses frame rects from JSON. Assumes keys "FrameName_0", "FrameName_1", etc.
// OR an array of frames. It returns a map from index to SDL_Rect.
bool AnimationManager::parseFrameRects(const json& framesNode, std::map<int, SDL_Rect>& outFrameRectsMap) {
    outFrameRectsMap.clear();
    int maxIndex = -1;
    try {
        if (framesNode.is_object()) {
            for (auto& [key, frameData] : framesNode.items()) {
                if (frameData.contains("frame")) {
                    const auto& rectData = frameData["frame"];
                    if (rectData.contains("x") && rectData.contains("y") && rectData.contains("w") && rectData.contains("h")) {
                        int w = rectData["w"].get<int>();
                        int h = rectData["h"].get<int>();
                        // Try to parse index from key like "Agumon_3" or just "3"
                        size_t underscorePos = key.rfind('_');
                        std::string indexStr = (underscorePos != std::string::npos) ? key.substr(underscorePos + 1) : key;

                        try {
                            int index = std::stoi(indexStr);
                            if (w > 0 && h > 0) {
                                outFrameRectsMap[index] = {rectData["x"].get<int>(), rectData["y"].get<int>(), w, h};
                                if (index > maxIndex) maxIndex = index;
                                SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "parseFrameRects: Parsed frame key '%s' -> index %d", key.c_str(), index);
                            } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "parseFrameRects: Invalid w/h (%d,%d) for frame key '%s'.", w, h, key.c_str()); }
                        } catch (const std::invalid_argument&) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "parseFrameRects: Could not parse index from frame key '%s'.", key.c_str()); }
                          catch (const std::out_of_range&) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "parseFrameRects: Parsed index out of range for frame key '%s'.", key.c_str()); }
                    } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "parseFrameRects: Frame data missing x,y,w, or h for frame key '%s'.", key.c_str()); }
                } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "parseFrameRects: Object item missing 'frame' object for key '%s'.", key.c_str()); }
            }
        } else if (framesNode.is_array()) { // Handle simple array format too
             for (size_t i = 0; i < framesNode.size(); ++i) {
                const auto& frameData = framesNode[i];
                if (frameData.contains("frame")) {
                    const auto& rectData = frameData["frame"];
                     if (rectData.contains("x") && rectData.contains("y") && rectData.contains("w") && rectData.contains("h")) {
                        int w = rectData["w"].get<int>();
                        int h = rectData["h"].get<int>();
                         if (w > 0 && h > 0) {
                            outFrameRectsMap[static_cast<int>(i)] = {rectData["x"].get<int>(), rectData["y"].get<int>(), w, h};
                            if (static_cast<int>(i) > maxIndex) maxIndex = static_cast<int>(i);
                         } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "parseFrameRects: Invalid w/h (%d,%d) for frame index %zu.", w, h, i); }
                     } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "parseFrameRects: Frame data missing x,y,w, or h for frame index %zu.", i); }
                } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "parseFrameRects: Array item missing 'frame' object at index %zu.", i); }
             }
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "parseFrameRects Error: 'frames' node is not an array or object.");
            return false;
        }

    } catch (const json::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "parseFrameRects Error: JSON exception while parsing frames: %s", e.what());
        return false;
    }

    // Basic check: did we parse frames up to the max index found?
    // This check might be overly strict if frames aren't contiguous, but okay for now.
    bool complete = !outFrameRectsMap.empty();
    // for(int i=0; i <= maxIndex; ++i) { if(outFrameRectsMap.find(i) == outFrameRectsMap.end()) complete = false; }
    // Simpler check: just ensure the map isn't empty. Rely on later lookups to fail if specific frames missing.
    return !outFrameRectsMap.empty();
}


// Helper function to create and store a single AnimationData instance.
void AnimationManager::storeAnimation(
    const std::string& animId,
    SDL_Texture* texture,
    const std::vector<SDL_Rect>& frameRects,
    const std::vector<float>& durationsSec, // Expects seconds now
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

    if (!data.isValid()) {
         SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "storeAnimation: Constructed AnimationData for '%s' is invalid. Skipping.", animId.c_str());
         return;
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Storing Anim ID='%s', Texture=%p, Frames=%zu, Loops=%d",
                 animId.c_str(), (void*)texture, frameRects.size(), loops);
    for(size_t i = 0; i < frameRects.size(); ++i) {
         if (i < durationsSec.size()) {
             SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "  Frame %zu: Rect={%d,%d,%d,%d}, Duration=%.3fs",
                           i, frameRects[i].x, frameRects[i].y, frameRects[i].w, frameRects[i].h, durationsSec[i]);
         } else {
             SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "  Frame %zu: Rect={%d,%d,%d,%d}, Duration=MISSING!",
                           i, frameRects[i].x, frameRects[i].y, frameRects[i].w, frameRects[i].h);
         }
    }

    auto result = loadedAnimations_.emplace(animId, std::move(data));
    if (!result.second) { // If emplace failed (key already existed)
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "storeAnimation: Animation definition with ID '%s' already exists. Overwriting.", animId.c_str());
        // Need to use assignment to overwrite the existing element's value
        loadedAnimations_[animId] = std::move(result.first->second); // Move the data intended for insertion
    }
}


// Loads animation definitions from a JSON file - REVISED LOGIC
bool AnimationManager::loadAnimationDataFromFile(const std::string& jsonPath, const std::string& textureAtlasId) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager: Loading animations from '%s' using texture '%s'", 
                jsonPath.c_str(), textureAtlasId.c_str());

    if (!assetManager_) {
         SDL_LogError(SDL_LOG_CATEGORY_ERROR, "AnimationManager::loadAnimationDataFromFile - AssetManager is null!");
         return false;
    }
    SDL_Texture* texture = assetManager_->getTexture(textureAtlasId);
    if (!texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager Error: Texture '%s' not found in AssetManager for JSON '%s'.", 
                    textureAtlasId.c_str(), jsonPath.c_str());
        return false;
    }

    std::ifstream jsonFile(jsonPath);
    if (!jsonFile.is_open()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager Error: Failed to open JSON file: %s", 
                    jsonPath.c_str());
        return false;
    }

    json data;
    try {
        data = json::parse(jsonFile);
        jsonFile.close();
    } catch (const json::parse_error& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager Error: Failed to parse JSON file '%s': %s (at byte %zu)", 
                    jsonPath.c_str(), e.what(), e.byte);
        if(jsonFile.is_open()) jsonFile.close();
        return false;
    } catch (const std::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager Error: Exception reading JSON file '%s': %s", 
                    jsonPath.c_str(), e.what());
        if(jsonFile.is_open()) jsonFile.close();
        return false;
    }

    std::map<int, SDL_Rect> frameRectsMap;
    if (!data.contains("frames") || !parseFrameRects(data["frames"], frameRectsMap)) {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager Error: Failed to parse frame rectangles from '%s'.", 
                     jsonPath.c_str());
         return false;
    }
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Parsed %zu total frame rects from %s", 
                 frameRectsMap.size(), jsonPath.c_str());

    int animationsStored = 0;

    // --- Create Idle Animation ---
    std::vector<SDL_Rect> idleRects;
    std::vector<float> idleDurationsSec;
    bool idleOk = true;
    if (AnimDefs::IDLE_INDICES.size() != AnimDefs::IDLE_DURATIONS_MS.size()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "IDLE definition mismatch: %zu indices vs %zu durations for %s", 
                    AnimDefs::IDLE_INDICES.size(), AnimDefs::IDLE_DURATIONS_MS.size(), textureAtlasId.c_str());
        idleOk = false;
    } else {
        for (size_t i = 0; i < AnimDefs::IDLE_INDICES.size(); ++i) {
            int frameIndex = AnimDefs::IDLE_INDICES[i];
            Uint32 durationMs = AnimDefs::IDLE_DURATIONS_MS[i];
            if (frameRectsMap.count(frameIndex)) {
                 idleRects.push_back(frameRectsMap[frameIndex]);
                 idleDurationsSec.push_back(static_cast<float>(durationMs) / 1000.0f);
            } else {
                 SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Frame rect missing for idle index %d in %s", 
                            frameIndex, jsonPath.c_str());
                 idleOk = false;
                 break;
            }
        }
    }
    if (idleOk && !idleRects.empty()) {
         storeAnimation(textureAtlasId + "_Idle", texture, idleRects, idleDurationsSec, true);
         animationsStored++;
    } else {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create complete Idle animation for %s", 
                     textureAtlasId.c_str());
    }

    // --- Create Walk Animation ---
    std::vector<SDL_Rect> walkRects;
    std::vector<float> walkDurationsSec;
    bool walkOk = true;
    if (AnimDefs::WALK_INDICES.size() != AnimDefs::WALK_DURATIONS_MS.size()) {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "WALK definition mismatch: %zu indices vs %zu durations for %s", 
                     AnimDefs::WALK_INDICES.size(), AnimDefs::WALK_DURATIONS_MS.size(), textureAtlasId.c_str());
         walkOk = false;
    } else {
         for (size_t i = 0; i < AnimDefs::WALK_INDICES.size(); ++i) {
             int frameIndex = AnimDefs::WALK_INDICES[i];
             Uint32 durationMs = AnimDefs::WALK_DURATIONS_MS[i];
             if (frameRectsMap.count(frameIndex)) {
                 walkRects.push_back(frameRectsMap[frameIndex]);
                 walkDurationsSec.push_back(static_cast<float>(durationMs) / 1000.0f);
             } else {
                 SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Frame rect missing for walk index %d in %s", 
                            frameIndex, jsonPath.c_str());
                 walkOk = false;
                 break;
             }
         }
    }
    if (walkOk && !walkRects.empty()) {
         storeAnimation(textureAtlasId + "_Walk", texture, walkRects, walkDurationsSec, false);
         animationsStored++;
    } else {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create complete Walk animation for %s", 
                     textureAtlasId.c_str());
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager: Finished processing '%s'. Stored %d animation definitions.", 
                jsonPath.c_str(), animationsStored);
    return animationsStored > 0;
}


// Retrieves a pointer to a previously loaded AnimationData definition.
const AnimationData* AnimationManager::getAnimationData(const std::string& animationId) const {
    auto it = loadedAnimations_.find(animationId);
    if (it != loadedAnimations_.end()) {
        return &(it->second);
    }
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager::getAnimationData: Animation definition with ID '%s' not found.", animationId.c_str());
    return nullptr;
}