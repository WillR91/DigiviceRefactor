// File: src/core/AssetManager.cpp

#include "core/AssetManager.h"
#include "core/FallbackTextureGenerator.h"
#include <SDL_image.h>
#include <SDL_render.h>
#include <SDL_surface.h>
#include <SDL_log.h>
#include <fstream>
#include <algorithm>
#include <vector>
#include <filesystem>

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
    
    // Initialize fallback texture generator
    fallbackGenerator_ = std::make_unique<FallbackTextureGenerator>();
    if (!fallbackGenerator_->init(renderer)) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize fallback texture generator");
        fallbackGenerator_.reset();
        fallbackEnabled_ = false;
    }
    
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetManager initialized with memory limit: %.2f MB, fallback: %s", 
                memoryLimit_ / (1024.0f * 1024.0f), fallbackEnabled_ ? "enabled" : "disabled");
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
    fallbackPaths_.clear();
    currentMemoryUsage_ = 0;
    
    // Clean up fallback generator
    if (fallbackGenerator_) {
        fallbackGenerator_->cleanup();
        fallbackGenerator_.reset();
    }
    
    IMG_Quit();
    renderer_ptr = nullptr;
    
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetManager shutdown complete. Freed %.2f MB.", 
               totalFreed / (1024.0f * 1024.0f));
}

// Enhanced asset loading with detailed result reporting
AssetManager::LoadResult AssetManager::loadTextureWithResult(const std::string& textureId, const std::string& filePath) {
    return loadTextureInternalWithResult(textureId, filePath);
}

AssetManager::LoadResult AssetManager::loadTextureInternalWithResult(const std::string& textureId, const std::string& filePath) {
    if (!renderer_ptr) {
        return LoadResult(false, false, "AssetManager not initialized", "");
    }
    
    // Check if already loaded
    auto it = assets_.find(textureId);
    if (it != assets_.end() && it->second.texture) {
        return LoadResult(true, it->second.isFallback, "Already loaded", it->second.filePath);
    }

    // Try to find a valid path
    std::vector<std::string> pathsToTry;
    if (!filePath.empty()) {
        pathsToTry.push_back(filePath);
    }
    
    // Add registered fallback paths
    auto fallbackIt = fallbackPaths_.find(textureId);
    if (fallbackIt != fallbackPaths_.end()) {
        pathsToTry.insert(pathsToTry.end(), fallbackIt->second.begin(), fallbackIt->second.end());
    }
    
    // Add single registered path
    auto pathIt = assetPaths_.find(textureId);
    if (pathIt != assetPaths_.end()) {
        pathsToTry.push_back(pathIt->second);
    }

    std::string validPath = findValidAssetPath(textureId, pathsToTry);
    
    if (!validPath.empty()) {
        // Try to load the actual asset
        if (loadTextureInternal(textureId, validPath)) {
            auto& asset = assets_[textureId];
            asset.isFallback = false;
            asset.lastLoadResult = LoadResult(true, false, "Successfully loaded", validPath);
            return asset.lastLoadResult;
        }
    }
    
    // If we can't load the asset, try fallback
    if (fallbackEnabled_ && fallbackGenerator_) {
        SDL_Texture* fallbackTexture = createFallbackTexture(textureId);
        if (fallbackTexture) {
            AssetInfo& asset = assets_[textureId];
            asset.texture = fallbackTexture;
            asset.filePath = "fallback:" + textureId;
            asset.lastUsed = std::chrono::steady_clock::now();
            asset.estimatedSize = estimateTextureSize(fallbackTexture);
            asset.isFallback = true;
            asset.referenceCount = 0;
            
            currentMemoryUsage_ += asset.estimatedSize;
            
            LoadResult result(true, true, "Using fallback texture", asset.filePath);
            asset.lastLoadResult = result;
            
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Created fallback texture for '%s'", textureId.c_str());
            return result;
        }
    }
    
    LoadResult result(false, false, "Failed to load asset and no fallback available", validPath);
    if (it != assets_.end()) {
        it->second.lastLoadResult = result;
    }
    return result;
}

// Fallback system methods
void AssetManager::enableFallbackTextures(bool enabled) {
    fallbackEnabled_ = enabled;
    if (enabled && !fallbackGenerator_ && renderer_ptr) {
        fallbackGenerator_ = std::make_unique<FallbackTextureGenerator>();
        fallbackGenerator_->init(renderer_ptr);
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetManager fallback textures %s", 
               enabled ? "enabled" : "disabled");
}

void AssetManager::setFallbackTextureSize(int width, int height) {
    fallbackWidth_ = width;
    fallbackHeight_ = height;
}

bool AssetManager::isUsingFallback(const std::string& textureId) const {
    auto it = assets_.find(textureId);
    return it != assets_.end() && it->second.isFallback;
}

SDL_Texture* AssetManager::createFallbackTexture(const std::string& textureId) {
    if (!fallbackGenerator_) return nullptr;
    
    return fallbackGenerator_->generateFallbackForAsset(textureId, fallbackWidth_, fallbackHeight_);
}

// Enhanced path registration
void AssetManager::registerAssetPaths(const std::vector<std::string>& fallbackPaths, const std::string& textureId) {
    fallbackPaths_[textureId] = fallbackPaths;
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Registered %zu fallback paths for '%s'", 
                fallbackPaths.size(), textureId.c_str());
}

std::string AssetManager::findValidAssetPath(const std::string& textureId, const std::vector<std::string>& paths) {
    for (const std::string& path : paths) {
        if (validateAssetFile(path)) {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Found valid path for '%s': %s", 
                        textureId.c_str(), path.c_str());
            return path;
        }
    }
    return "";
}

bool AssetManager::validateAssetFile(const std::string& filePath) {
    try {
        if (!std::filesystem::exists(filePath)) {
            return false;
        }
        
        if (!std::filesystem::is_regular_file(filePath)) {
            return false;
        }
        
        // Check file size (reject files larger than 50MB)
        auto fileSize = std::filesystem::file_size(filePath);
        if (fileSize > 50 * 1024 * 1024) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Asset file too large: %s (%.2f MB)", 
                       filePath.c_str(), fileSize / (1024.0f * 1024.0f));
            return false;
        }
        
        // Try to open the file
        std::ifstream testFile(filePath, std::ios::binary);
        if (!testFile.is_open()) {
            return false;
        }
        
        // Check file header for common image formats
        char header[8] = {0};
        testFile.read(header, 8);
        testFile.close();
        
        // PNG signature
        if (header[0] == '\x89' && header[1] == 'P' && header[2] == 'N' && header[3] == 'G') {
            return true;
        }
        
        // JPEG signatures
        if (header[0] == '\xFF' && header[1] == '\xD8') {
            return true;
        }
        
        // BMP signature
        if (header[0] == 'B' && header[1] == 'M') {
            return true;
        }
        
        // GIF signatures
        if ((header[0] == 'G' && header[1] == 'I' && header[2] == 'F' && header[3] == '8') &&
            (header[4] == '7' || header[4] == '9') && header[5] == 'a') {
            return true;
        }
        
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unknown image format: %s", filePath.c_str());
        return false;
        
    } catch (const std::filesystem::filesystem_error& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Filesystem error validating %s: %s", 
                    filePath.c_str(), e.what());
        return false;
    }
}

// Asset validation methods
AssetManager::AssetValidationResult AssetManager::validateRegisteredAssets() {
    AssetValidationResult result;
    result.totalAssetsChecked = 0;
    
    // Validate single paths
    for (const auto& [textureId, path] : assetPaths_) {
        result.totalAssetsChecked++;
        if (!validateAssetFile(path)) {
            if (!std::filesystem::exists(path)) {
                result.missingAssets.push_back(textureId + " (" + path + ")");
            } else {
                result.corruptAssets.push_back(textureId + " (" + path + ")");
            }
        }
    }
    
    // Validate fallback paths
    for (const auto& [textureId, paths] : fallbackPaths_) {
        bool hasValidPath = false;
        for (const std::string& path : paths) {
            result.totalAssetsChecked++;
            if (validateAssetFile(path)) {
                hasValidPath = true;
                break;
            }
        }
        
        if (!hasValidPath) {
            result.missingAssets.push_back(textureId + " (all fallback paths invalid)");
        }
    }
    
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Asset validation complete: %zu total, %zu missing, %zu corrupt", 
               result.totalAssetsChecked, result.missingAssets.size(), result.corruptAssets.size());
    
    if (validationCallback_ && result.hasIssues()) {
        validationCallback_(result);
    }
    
    return result;
}

AssetManager::AssetValidationResult AssetManager::validateAssetPath(const std::string& textureId) {
    AssetValidationResult result;
    result.totalAssetsChecked = 1;
    
    std::vector<std::string> pathsToCheck;
    
    auto pathIt = assetPaths_.find(textureId);
    if (pathIt != assetPaths_.end()) {
        pathsToCheck.push_back(pathIt->second);
    }
    
    auto fallbackIt = fallbackPaths_.find(textureId);
    if (fallbackIt != fallbackPaths_.end()) {
        pathsToCheck.insert(pathsToCheck.end(), fallbackIt->second.begin(), fallbackIt->second.end());
    }
    
    bool hasValidPath = false;
    for (const std::string& path : pathsToCheck) {
        if (validateAssetFile(path)) {
            hasValidPath = true;
            break;
        }
    }
    
    if (!hasValidPath) {
        result.missingAssets.push_back(textureId);
    }
    
    return result;
}

std::vector<std::string> AssetManager::findAlternativePaths(const std::string& textureId) {
    std::vector<std::string> alternatives;
    
    // Look for similar asset IDs
    std::string lowerTextureId = textureId;
    std::transform(lowerTextureId.begin(), lowerTextureId.end(), lowerTextureId.begin(), ::tolower);
    
    for (const auto& [id, path] : assetPaths_) {
        if (id != textureId) {
            std::string lowerAssetId = id;
            std::transform(lowerAssetId.begin(), lowerAssetId.end(), lowerAssetId.begin(), ::tolower);
            
            // Check for partial matches
            if (lowerAssetId.find(lowerTextureId.substr(0, lowerTextureId.find('_'))) != std::string::npos ||
                lowerTextureId.find(lowerAssetId.substr(0, lowerAssetId.find('_'))) != std::string::npos) {
                alternatives.push_back(id + " -> " + path);
            }
        }
    }
    
    return alternatives;
}

void AssetManager::setAssetValidationCallback(std::function<void(const AssetValidationResult&)> callback) {
    validationCallback_ = callback;
}