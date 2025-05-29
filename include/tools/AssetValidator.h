#pragma once

#include "core/AssetManager.h"
#include <string>
#include <vector>
#include <unordered_map>

/**
 * @brief Development tool for validating and managing game assets
 * 
 * This class provides comprehensive asset validation, path resolution,
 * and diagnostic tools for development-time asset management.
 */
class AssetValidator {
public:
    struct ValidationReport {
        AssetManager::AssetValidationResult overallResult;
        std::unordered_map<std::string, std::vector<std::string>> suggestedAlternatives;
        std::vector<std::string> recommendedActions;
        std::string reportTimestamp;
        
        void printReport() const;
        void saveToFile(const std::string& filePath) const;
    };

    AssetValidator() = default;
    ~AssetValidator() = default;

    /**
     * @brief Set the asset manager to validate
     * @param assetManager Pointer to the asset manager
     */
    void setAssetManager(AssetManager* assetManager);

    /**
     * @brief Load asset paths from game configuration
     * @param configPath Path to the game configuration file
     * @return true if configuration was loaded successfully
     */
    bool loadAssetConfiguration(const std::string& configPath);

    /**
     * @brief Register common asset paths based on directory structure
     * @param assetsRootPath Root path to the assets directory
     */
    void autoDiscoverAssets(const std::string& assetsRootPath);

    /**
     * @brief Perform comprehensive asset validation
     * @return Detailed validation report
     */
    ValidationReport performFullValidation();

    /**
     * @brief Validate a specific asset category
     * @param category Asset category (e.g., "digimon", "ui", "backgrounds")
     * @return Validation report for the category
     */
    ValidationReport validateCategory(const std::string& category);

    /**
     * @brief Attempt to fix missing asset paths automatically
     * @param dryRun If true, only report what would be fixed without making changes
     * @return Number of issues that were or would be fixed
     */
    int autoFixAssetPaths(bool dryRun = true);

    /**
     * @brief Generate fallback path suggestions for missing assets
     * @param missingAssetId ID of the missing asset
     * @return Vector of suggested fallback paths
     */
    std::vector<std::string> generateFallbackSuggestions(const std::string& missingAssetId);

private:
    AssetManager* assetManager_ = nullptr;
    std::unordered_map<std::string, std::vector<std::string>> discoveredPaths_;
    std::string assetsRootPath_;

    // Helper methods
    void scanDirectory(const std::string& dirPath, const std::string& category);
    std::string generateAssetId(const std::string& filePath, const std::string& category);
    std::vector<std::string> findSimilarAssets(const std::string& assetId);
    bool tryAutoFixPath(const std::string& assetId);
    std::string normalizeAssetId(const std::string& assetId);
};
