// include/core/AnimationManager.h
#ifndef ANIMATIONMANAGER_H
#define ANIMATIONMANAGER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <SDL_render.h>             // <<<--- ADDED for SDL_Texture*
#include "vendor/nlohmann/json.hpp" // <<<--- ADDED for nlohmann::json

// Forward Declarations
struct AnimationData; // From graphics/AnimationData.h
class AssetManager; // Needed to get texture pointers
struct SDL_Rect;    // SDL_Rect is needed for the vector type

class AnimationManager {
public:
    AnimationManager(AssetManager* assetMgr);
    ~AnimationManager();

    bool loadAnimationDataFromFile(const std::string& jsonPath, const std::string& textureAtlasId);
    const AnimationData* getAnimationData(const std::string& animationId) const;

private:
    AssetManager* assetManager_ = nullptr;
    std::map<std::string, AnimationData> loadedAnimations_;

    // <<<--- MODIFIED Declaration: Use full type name ---<<<
    bool parseFrameRects(const nlohmann::json& framesNode, std::vector<SDL_Rect>& outFrameRects);

    // <<<--- MODIFIED Declaration: Use SDL_Texture* ---<<<
    void storeAnimation(
        const std::string& animId,
        SDL_Texture* texture, // Use SDL_Texture* here
        const std::vector<SDL_Rect>& frameRects,
        const std::vector<float>& durationsSec,
        bool loops
    );

};

#endif // ANIMATIONMANAGER_H