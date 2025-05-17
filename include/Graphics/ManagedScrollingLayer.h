\
#pragma once

#include <vector>
#include <string>
#include <SDL.h> // For SDL_Texture and SDL_Rect

// Forward declarations
class AssetManager;

class ManagedScrollingLayer {
public:
    ManagedScrollingLayer();
    ~ManagedScrollingLayer();

    void initialize(AssetManager* assetManager, float scrollSpeed);
    void loadVariants(const std::vector<std::string>& texturePaths, const std::string& defaultTexturePath);
    void setScrollSpeed(float speed);
    void update(float deltaTime);
    SDL_Texture* getCurrentTexture() const;
    float getScrollOffset() const;
    void getCurrentDimensions(int& width, int& height) const;
    void reset();

private:
    void updateCurrentTextureDimensions();

    AssetManager* assetManager_;
    std::vector<SDL_Texture*> textureVariants_;
    std::vector<std::string> texturePaths_; // To store the paths for cycling logic
    size_t currentVariantIndex_;
    float scrollOffset_;
    float scrollSpeed_;
    int currentTextureWidth_;
    int currentTextureHeight_;
};
