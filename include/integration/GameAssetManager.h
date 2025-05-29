/**
 * @file AssetManagerIntegration.h
 * @brief Example integration of enhanced AssetManager with error handling
 * 
 * This example shows how to integrate the enhanced AssetManager with fallback
 * textures and validation into the existing game systems.
 */

#pragma once

#include "core/AssetManager.h"
#include "tools/AssetValidator.h"
#include <functional>
#include <memory>

/**
 * @brief Enhanced asset management wrapper for game integration
 * 
 * This class provides a higher-level interface for asset management
 * with automatic validation, error handling, and development tools.
 */
class GameAssetManager {
public:
    // Callback for asset loading issues during gameplay
    using AssetErrorCallback = std::function<void(const std::string& assetId, const std::string& error)>;
    
    GameAssetManager();
    ~GameAssetManager();

    /**
     * @brief Initialize the asset manager with game configuration
     * @param renderer SDL renderer
     * @param assetsPath Path to assets directory
     * @param configPath Path to game configuration file
     * @return true if initialization succeeded
     */
    bool initialize(SDL_Renderer* renderer, const std::string& assetsPath, const std::string& configPath);

    /**
     * @brief Load essential assets for the current game state
     * @param stateAssets List of asset IDs needed for the current state
     * @return true if all essential assets loaded successfully
     */
    bool loadStateAssets(const std::vector<std::string>& stateAssets);

    /**
     * @brief Get texture with automatic fallback handling
     * @param assetId Asset identifier
     * @param reportError Whether to report missing assets via callback
     * @return Texture pointer (may be fallback) or nullptr
     */
    SDL_Texture* getTexture(const std::string& assetId, bool reportError = true);

    /**
     * @brief Check if an asset is using a fallback texture
     * @param assetId Asset identifier
     * @return true if using fallback
     */
    bool isUsingFallback(const std::string& assetId) const;

    /**
     * @brief Preload assets for the next game state
     * @param stateAssets List of asset IDs for the next state
     */
    void preloadForNextState(const std::vector<std::string>& stateAssets);

    /**
     * @brief Set callback for asset error reporting
     * @param callback Function to call when asset errors occur
     */
    void setErrorCallback(AssetErrorCallback callback);

    /**
     * @brief Perform asset validation (development only)
     * @return Validation report
     */
    AssetValidator::ValidationReport validateAssets();

    /**
     * @brief Get asset loading statistics
     * @return Statistics about loaded assets and fallbacks
     */
    struct AssetStats {
        size_t totalAssetsLoaded;
        size_t fallbacksUsed;
        size_t memoryUsedMB;
        size_t memoryLimitMB;
        std::vector<std::string> recentErrors;
    };
    AssetStats getStatistics() const;

    /**
     * @brief Configure asset management for different performance profiles
     */
    enum class PerformanceProfile {
        DEVELOPMENT,    // Extensive validation, detailed logging
        TESTING,        // Moderate validation, error reporting
        PRODUCTION      // Minimal validation, fast loading
    };
    void setPerformanceProfile(PerformanceProfile profile);

    /**
     * @brief Cleanup and shutdown
     */
    void shutdown();

private:
    std::unique_ptr<AssetManager> assetManager_;
    std::unique_ptr<AssetValidator> validator_;
    AssetErrorCallback errorCallback_;
    PerformanceProfile currentProfile_;
    std::vector<std::string> recentErrors_;
    
    // Asset state tracking
    std::vector<std::string> currentStateAssets_;
    std::vector<std::string> preloadedAssets_;
    
    // Helper methods
    void setupValidation();
    void registerCommonAssets(const std::string& assetsPath);
    void handleAssetError(const std::string& assetId, const std::string& error);
    std::vector<std::string> getDigimonAssets();
    std::vector<std::string> getUIAssets();
    std::vector<std::string> getBackgroundAssets();
};
