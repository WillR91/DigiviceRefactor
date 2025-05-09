// include/core/AnimationManager.h
#ifndef ANIMATIONMANAGER_H
#define ANIMATIONMANAGER_H

#include <string>
#include <vector>
#include <map>                      // <<< REQUIRED for std::map usage
#include <memory>
#include <SDL_render.h>             // For SDL_Texture*
#include <SDL_rect.h>               // <<< REQUIRED for SDL_Rect usage
#include "vendor/nlohmann/json.hpp" // <<<< INCLUDE FULL HEADER

// Forward Declarations
struct AnimationData; // From graphics/AnimationData.h
class AssetManager;   // Needed to get texture pointers

class AnimationManager {
public:
    AnimationManager(AssetManager* assetMgr);
    ~AnimationManager();

    bool loadAnimationDataFromFile(const std::string& jsonPath, const std::string& textureAtlasId);
    const AnimationData* getAnimationData(const std::string& animationId) const;

private:
    AssetManager* assetManager_ = nullptr;
    std::map<std::string, AnimationData> loadedAnimations_;

    // Declaration uses const nlohmann::json& which is fine with forward declaration
    bool parseFrameRects(const nlohmann::json& framesNode, std::map<int, SDL_Rect>& outFrameRectsMap); 

    // Declaration using SDL_Texture*
    void storeAnimation(
        const std::string& animId,
        SDL_Texture* texture, // Use SDL_Texture* here
        const std::vector<SDL_Rect>& frameRects, // StoreAnimation still builds the vector
        const std::vector<float>& durationsSec,
        bool loops
    );

};

#endif // ANIMATIONMANAGER_H