// File: src/core/AssetManager.cpp

#include "core/AssetManager.h"
#include <SDL_image.h>
#include <SDL_render.h>
#include <SDL_surface.h>
#include <SDL_log.h>
#include <fstream>
#include <algorithm>
#include <vector>

AssetManager::~AssetManager() {
    shutdown();
}

bool AssetManager::init(SDL_Renderer* renderer) {
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AssetManager::init failed: Renderer pointer is null.");
        return false;
    }
    renderer_ptr = renderer;
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_image could not initialize! IMG_Error: %s", IMG_GetError());
        renderer_ptr = nullptr;
        return false;
    }
    
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetManager initialized with memory limit: %.2f MB", 
                memoryLimit_ / (1024.0f * 1024.0f));
    return true;
}

bool AssetManager::loadTexture(const std::string& textureId, const std::string& filePath) {
    // Legacy method - now uses the new system internally
    return loadTextureInternal(textureId, filePath);
}

SDL_Texture* AssetManager::getTexture(const std::string& textureId) const {
    auto it = assets_.find(textureId);
    if (it != assets_.end() && it->second.texture) {
        updateLastUsed(textureId);
        return it->second.texture;
    }
    return nullptr;
}

bool AssetManager::hasTexture(const std::string& textureId) const {
    auto it = assets_.find(textureId);
    return it != assets_.end() && it->second.texture != nullptr;
}

SDL_Texture* AssetManager::requestTexture(const std::string& textureId, const std::string& filePath) {
    // Check if already loaded
    auto it = assets_.find(textureId);
    if (it != assets_.end() && it->second.texture) {
        it->second.referenceCount++;
        updateLastUsed(textureId);
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "AssetManager: Texture '%s' reference count: %d", 
                    textureId.c_str(), it->second.referenceCount);
        return it->second.texture;
    }
    
    // Try to load from provided path or registered path
    std::string pathToUse = filePath;
    if (pathToUse.empty()) {
        auto pathIt = assetPaths_.find(textureId);
        if (pathIt != assetPaths_.end()) {
            pathToUse = pathIt->second;
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AssetManager: No path available for texture '%s'", 
                        textureId.c_str());
            return nullptr;
        }
    }
    
    // Free memory if needed before loading
    freeMemoryIfNeeded();
    
    // Load the texture
    if (loadTextureInternal(textureId, pathToUse)) {
        auto& asset = assets_[textureId];
        asset.referenceCount++;
        updateLastUsed(textureId);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetManager: Loaded texture '%s' on demand (%.2f MB used)", 
                   textureId.c_str(), getCurrentMemoryUsage() / (1024.0f * 1024.0f));
        return asset.texture;
    }
    
    return nullptr;
}

void AssetManager::releaseTexture(const std::string& textureId) {
    auto it = assets_.find(textureId);
    if (it != assets_.end() && it->second.referenceCount > 0) {
        it->second.referenceCount--;
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "AssetManager: Released texture '%s', reference count: %d", 
                    textureId.c_str(), it->second.referenceCount);
    }
}

void AssetManager::registerAssetPath(const std::string& textureId, const std::string& filePath) {
    assetPaths_[textureId] = filePath;
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "AssetManager: Registered path for '%s': %s", 
                textureId.c_str(), filePath.c_str());
}

void AssetManager::preloadAsset(const std::string& textureId) {
    auto pathIt = assetPaths_.find(textureId);
    if (pathIt != assetPaths_.end()) {
        if (loadTextureInternal(textureId, pathIt->second)) {
            assets_[textureId].isPreloaded = true;
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetManager: Preloaded asset '%s'", textureId.c_str());
        }
    }
}

void AssetManager::setMemoryLimit(size_t maxMegabytes) {
    memoryLimit_ = maxMegabytes * 1024 * 1024;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetManager: Memory limit set to %.2f MB", 
               memoryLimit_ / (1024.0f * 1024.0f));
    freeMemoryIfNeeded();
}

size_t AssetManager::getCurrentMemoryUsage() const {
    return currentMemoryUsage_;
}

void AssetManager::releaseUnusedAssets() {
    std::vector<std::string> toRemove;
    
    for (const auto& [id, asset] : assets_) {
        if (asset.referenceCount == 0 && !asset.isPreloaded && asset.texture) {
            toRemove.push_back(id);
        }
    }
    
    size_t freedMemory = 0;
    for (const auto& id : toRemove) {
        freedMemory += assets_[id].estimatedSize;
        unloadTexture(id);
    }
    
    if (!toRemove.empty()) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetManager: Released %zu unused assets, freed %.2f MB", 
                   toRemove.size(), freedMemory / (1024.0f * 1024.0f));
    }
}

void AssetManager::cleanupOldAssets(int maxAgeSeconds) {
    auto now = std::chrono::steady_clock::now();
    std::vector<std::string> toRemove;
    
    for (const auto& [id, asset] : assets_) {
        if (asset.referenceCount == 0 && !asset.isPreloaded && asset.texture) {
            auto age = std::chrono::duration_cast<std::chrono::seconds>(now - asset.lastUsed).count();
            if (age > maxAgeSeconds) {
                toRemove.push_back(id);
            }
        }
    }
    
    size_t freedMemory = 0;
    for (const auto& id : toRemove) {
        freedMemory += assets_[id].estimatedSize;
        unloadTexture(id);
    }
    
    if (!toRemove.empty()) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetManager: Cleaned up %zu old assets (>%ds), freed %.2f MB", 
                   toRemove.size(), maxAgeSeconds, freedMemory / (1024.0f * 1024.0f));
    }
}

void AssetManager::forceGarbageCollection() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetManager: Performing garbage collection...");
    releaseUnusedAssets();
    cleanupOldAssets(0); // Remove all unreferenced assets regardless of age
}

bool AssetManager::loadTextureInternal(const std::string& textureId, const std::string& filePath) {
    if (!renderer_ptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot load texture '%s': AssetManager not initialized.", 
                    textureId.c_str());
        return false;
    }
    
    // Check if already loaded
    auto it = assets_.find(textureId);
    if (it != assets_.end() && it->second.texture) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Texture '%s' already loaded. Skipping.", textureId.c_str());
        return true;
    }

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Loading texture '%s' from '%s'", 
                textureId.c_str(), filePath.c_str());

    // Basic file check
    std::ifstream testFile(filePath);
    if (!testFile.is_open()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "File check failed for '%s'. File likely missing.", 
                    filePath.c_str());
        return false;
    }
    testFile.close();

    // Load image surface using SDL_image
    SDL_Surface* loadedSurface = IMG_Load(filePath.c_str());
    if (!loadedSurface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "IMG_Load failed for '%s'! SDL_image Error: %s", 
                    filePath.c_str(), IMG_GetError());
        return false;
    }

    // Convert surface to hardware-accelerated texture
    SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderer_ptr, loadedSurface);
    if (!newTexture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create texture from '%s'! SDL Error: %s", 
                    filePath.c_str(), SDL_GetError());
        SDL_FreeSurface(loadedSurface);
        return false;
    }

    // Calculate texture size
    size_t textureSize = estimateTextureSize(newTexture);
    
    // Free the temporary surface
    SDL_FreeSurface(loadedSurface);

    // Store the successful texture
    AssetInfo& asset = assets_[textureId];
    asset.texture = newTexture;
    asset.filePath = filePath;
    asset.lastUsed = std::chrono::steady_clock::now();
    asset.estimatedSize = textureSize;
    asset.referenceCount = 0; // Will be incremented by requestTexture if needed
    
    currentMemoryUsage_ += textureSize;
    
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Successfully loaded texture '%s' (%.2f KB, total: %.2f MB).", 
               textureId.c_str(), textureSize / 1024.0f, getCurrentMemoryUsage() / (1024.0f * 1024.0f));
    return true;
}

void AssetManager::unloadTexture(const std::string& textureId) {
    auto it = assets_.find(textureId);
    if (it != assets_.end() && it->second.texture) {
        SDL_DestroyTexture(it->second.texture);
        currentMemoryUsage_ -= it->second.estimatedSize;
        
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Unloaded texture '%s' (%.2f KB freed, total: %.2f MB)", 
                    textureId.c_str(), it->second.estimatedSize / 1024.0f, 
                    getCurrentMemoryUsage() / (1024.0f * 1024.0f));
        
        it->second.texture = nullptr;
        it->second.estimatedSize = 0;
    }
}

size_t AssetManager::estimateTextureSize(SDL_Texture* texture) const {
    if (!texture) return 0;
    
    int width, height;
    Uint32 format;
    if (SDL_QueryTexture(texture, &format, nullptr, &width, &height) != 0) {
        return 0;
    }
    
    // Estimate bytes per pixel based on format
    int bytesPerPixel = 4; // Default to RGBA
    switch (format) {
        case SDL_PIXELFORMAT_RGB24: bytesPerPixel = 3; break;
        case SDL_PIXELFORMAT_RGBA8888:
        case SDL_PIXELFORMAT_ARGB8888: bytesPerPixel = 4; break;
        case SDL_PIXELFORMAT_RGB565: bytesPerPixel = 2; break;
        default: bytesPerPixel = 4; break;
    }
    
    return width * height * bytesPerPixel;
}

void AssetManager::updateLastUsed(const std::string& textureId) const {
    auto it = assets_.find(textureId);
    if (it != assets_.end()) {
        // Const cast is acceptable here as we're only updating usage tracking
        const_cast<AssetInfo&>(it->second).lastUsed = std::chrono::steady_clock::now();
    }
}

bool AssetManager::isMemoryLimitExceeded() const {
    return currentMemoryUsage_ > memoryLimit_;
}

void AssetManager::freeMemoryIfNeeded() {
    if (!isMemoryLimitExceeded()) return;
    
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AssetManager: Memory limit exceeded (%.2f/%.2f MB), freeing assets...", 
               getCurrentMemoryUsage() / (1024.0f * 1024.0f), memoryLimit_ / (1024.0f * 1024.0f));
    
    // First try releasing unreferenced assets
    releaseUnusedAssets();
    
    // If still over limit, release old assets
    if (isMemoryLimitExceeded()) {
        cleanupOldAssets(60); // 1 minute
    }
    
    // If still over limit, be more aggressive
    if (isMemoryLimitExceeded()) {
        cleanupOldAssets(10); // 10 seconds
    }
}

std::vector<std::string> AssetManager::findCandidatesForUnloading() const {
    std::vector<std::pair<std::string, std::chrono::steady_clock::time_point>> candidates;
    
    for (const auto& [id, asset] : assets_) {
        if (asset.referenceCount == 0 && !asset.isPreloaded && asset.texture) {
            candidates.emplace_back(id, asset.lastUsed);
        }
    }
    
    // Sort by last used time (oldest first)
    std::sort(candidates.begin(), candidates.end(), 
              [](const auto& a, const auto& b) { return a.second < b.second; });
    
    std::vector<std::string> result;
    for (const auto& [id, _] : candidates) {
        result.push_back(id);
    }
    
    return result;
}

void AssetManager::shutdown() {
    if (renderer_ptr == nullptr && assets_.empty()) return;
    
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Shutting down AssetManager...");
    
    size_t totalFreed = 0;
    for (auto& [id, asset] : assets_) {
        if (asset.texture) {
            SDL_DestroyTexture(asset.texture);
            totalFreed += asset.estimatedSize;
        }
    }
    
    assets_.clear();
    assetPaths_.clear();
    currentMemoryUsage_ = 0;
    IMG_Quit();
    renderer_ptr = nullptr;
    
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetManager shutdown complete. Freed %.2f MB.", 
               totalFreed / (1024.0f * 1024.0f));
}