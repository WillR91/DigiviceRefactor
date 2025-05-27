#pragma once

#include <vector>
#include <SDL2/SDL.h>

class PCDisplay;

/**
 * @brief Handles seamless background rendering with proper tiling
 * 
 * Replaces the problematic overlap-based tiling system with a clean
 * modulo-based approach that eliminates seaming artifacts.
 */
class SeamlessBackgroundRenderer {
private:
    struct CachedLayer {
        SDL_Texture* originalTexture;
        SDL_Texture* scaledTexture;  // Pre-scaled for target resolution
        int scaledWidth;
        int scaledHeight;
        float scrollPosition;
        float scrollSpeed;
        bool needsUpdate;
    };
    
    std::vector<CachedLayer> layers_;
    PCDisplay* display_;
    SDL_Renderer* renderer_;
    
    // Performance optimization
    int targetWidth_;
    int targetHeight_;
    bool cacheDirty_;
    
    void updateScaledTexture(CachedLayer& layer);
    void cleanupLayer(CachedLayer& layer);
    
public:
    SeamlessBackgroundRenderer(PCDisplay* display, SDL_Renderer* renderer);
    ~SeamlessBackgroundRenderer();
    
    void addLayer(SDL_Texture* texture, float scrollSpeed);
    void updateScroll(float deltaTime);
    void render();
    void clearLayers();
    
    // Configuration
    void setTargetResolution(int width, int height);
    void invalidateCache();
      // Debug and Performance
    size_t getLayerCount() const { return layers_.size(); }
    
    // Layer management
    bool removeLayer(size_t index);
    bool setLayerScrollSpeed(size_t index, float speed);
    float getLayerScrollSpeed(size_t index) const;
    
    // Performance monitoring
    struct PerformanceStats {
        int renderCalls;
        int textureUpdates;
        float lastFrameTime;
    };
    const PerformanceStats& getPerformanceStats() const { return stats_; }

private:
    PerformanceStats stats_;
    void resetPerformanceStats();
};
