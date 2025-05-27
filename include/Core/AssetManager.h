#pragma once

#include <string>
#include <map>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <SDL.h>

// Forward declare SDL_Texture and SDL_Renderer
struct SDL_Texture;
struct SDL_Renderer;

class AssetManager {
public:
    AssetManager() = default;
    ~AssetManager();

    // Core initialization and management
    bool init(SDL_Renderer* renderer);
    void shutdown();
    
    // Asset loading and retrieval
    bool loadTexture(const std::string& textureId, const std::string& filePath);
    SDL_Texture* getTexture(const std::string& textureId) const;
    bool hasTexture(const std::string& textureId) const;
    
    // New memory management features
    SDL_Texture* requestTexture(const std::string& textureId, const std::string& filePath = "");
    void releaseTexture(const std::string& textureId);
    void releaseUnusedAssets();
    void setMemoryLimit(size_t maxMegabytes);
    size_t getCurrentMemoryUsage() const;
    
    // Asset registration for lazy loading
    void registerAssetPath(const std::string& textureId, const std::string& filePath);
    void preloadAsset(const std::string& textureId);
    
    // Cleanup and optimization
    void cleanupOldAssets(int maxAgeSeconds = 300); // 5 minutes default
    void forceGarbageCollection();

private:
    struct AssetInfo {
        SDL_Texture* texture;
        std::string filePath;
        int referenceCount;
        std::chrono::steady_clock::time_point lastUsed;
        size_t estimatedSize; // In bytes
        bool isPreloaded; // Assets that should stay loaded
        
        AssetInfo() : texture(nullptr), referenceCount(0), estimatedSize(0), isPreloaded(false) {}
    };

    SDL_Renderer* renderer_ptr = nullptr;
    std::unordered_map<std::string, AssetInfo> assets_;
    std::unordered_map<std::string, std::string> assetPaths_; // For lazy loading
    size_t memoryLimit_ = 100 * 1024 * 1024; // 100MB default
    size_t currentMemoryUsage_ = 0;
    
    // Helper methods
    bool loadTextureInternal(const std::string& textureId, const std::string& filePath);
    void unloadTexture(const std::string& textureId);
    size_t estimateTextureSize(SDL_Texture* texture) const;
    void updateLastUsed(const std::string& textureId) const;
    bool isMemoryLimitExceeded() const;
    void freeMemoryIfNeeded();
    std::vector<std::string> findCandidatesForUnloading() const;

    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;
};