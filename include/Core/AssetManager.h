#pragma once

#include <string>
#include <map>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <vector>
#include <functional>
#include <SDL.h>
#include "core/FallbackTextureGenerator.h"

// Forward declare SDL_Texture and SDL_Renderer
struct SDL_Texture;
struct SDL_Renderer;

class AssetManager {
public:
    // Asset loading result information
    struct LoadResult {
        bool success;
        bool usedFallback;
        std::string errorMessage;
        std::string actualPath;
        
        LoadResult(bool s = false, bool f = false, const std::string& err = "", const std::string& path = "")
            : success(s), usedFallback(f), errorMessage(err), actualPath(path) {}
    };

    // Asset validation information
    struct AssetValidationResult {
        std::vector<std::string> missingAssets;
        std::vector<std::string> corruptAssets;
        std::vector<std::string> oversizedAssets;
        size_t totalAssetsChecked;
        
        bool hasIssues() const {
            return !missingAssets.empty() || !corruptAssets.empty() || !oversizedAssets.empty();
        }
    };

    AssetManager() = default;
    ~AssetManager();    // Core initialization and management
    bool init(SDL_Renderer* renderer);
    void shutdown();
    
    // Fallback system configuration
    void enableFallbackTextures(bool enabled = true);
    void setFallbackTextureSize(int width, int height);
    bool isFallbackEnabled() const { return fallbackEnabled_; }
    
    // Asset loading and retrieval with enhanced error handling
    LoadResult loadTextureWithResult(const std::string& textureId, const std::string& filePath);
    bool loadTexture(const std::string& textureId, const std::string& filePath);
    SDL_Texture* getTexture(const std::string& textureId) const;
    bool hasTexture(const std::string& textureId) const;
    bool isUsingFallback(const std::string& textureId) const;
    
    // New memory management features
    SDL_Texture* requestTexture(const std::string& textureId, const std::string& filePath = "");
    void releaseTexture(const std::string& textureId);
    void releaseUnusedAssets();
    void setMemoryLimit(size_t maxMegabytes);
    size_t getCurrentMemoryUsage() const;
      // Asset registration for lazy loading
    void registerAssetPath(const std::string& textureId, const std::string& filePath);
    void registerAssetPaths(const std::vector<std::string>& fallbackPaths, const std::string& textureId);
    void preloadAsset(const std::string& textureId);
    
    // Asset validation and diagnostics
    AssetValidationResult validateRegisteredAssets();
    AssetValidationResult validateAssetPath(const std::string& textureId);
    std::vector<std::string> findAlternativePaths(const std::string& textureId);
    void setAssetValidationCallback(std::function<void(const AssetValidationResult&)> callback);
    
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
        bool isFallback; // Whether this is a fallback texture
        LoadResult lastLoadResult; // Information about the last load attempt
        
        AssetInfo() : texture(nullptr), referenceCount(0), estimatedSize(0), 
                     isPreloaded(false), isFallback(false) {}
    };

    SDL_Renderer* renderer_ptr = nullptr;
    std::unordered_map<std::string, AssetInfo> assets_;
    std::unordered_map<std::string, std::string> assetPaths_; // For lazy loading
    std::unordered_map<std::string, std::vector<std::string>> fallbackPaths_; // Multiple paths per asset
    size_t memoryLimit_ = 100 * 1024 * 1024; // 100MB default
    size_t currentMemoryUsage_ = 0;
    
    // Fallback system
    std::unique_ptr<FallbackTextureGenerator> fallbackGenerator_;
    bool fallbackEnabled_ = true;
    int fallbackWidth_ = 64;
    int fallbackHeight_ = 64;
    
    // Validation
    std::function<void(const AssetValidationResult&)> validationCallback_;
    
    // Helper methods
    bool loadTextureInternal(const std::string& textureId, const std::string& filePath);
    LoadResult loadTextureInternalWithResult(const std::string& textureId, const std::string& filePath);
    SDL_Texture* createFallbackTexture(const std::string& textureId);
    std::string findValidAssetPath(const std::string& textureId, const std::vector<std::string>& paths);
    bool validateAssetFile(const std::string& filePath);
    void unloadTexture(const std::string& textureId);
    size_t estimateTextureSize(SDL_Texture* texture) const;
    void updateLastUsed(const std::string& textureId) const;
    bool isMemoryLimitExceeded() const;
    void freeMemoryIfNeeded();
    std::vector<std::string> findCandidatesForUnloading() const;

    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;
};