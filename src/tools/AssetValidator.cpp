#include "tools/AssetValidator.h"
#include <SDL_log.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <regex>

void AssetValidator::ValidationReport::printReport() const {
    std::cout << "\n=== ASSET VALIDATION REPORT ===" << std::endl;
    std::cout << "Generated: " << reportTimestamp << std::endl;
    std::cout << "Total assets checked: " << overallResult.totalAssetsChecked << std::endl;
    
    if (overallResult.hasIssues()) {
        std::cout << "\n--- ISSUES FOUND ---" << std::endl;
        
        if (!overallResult.missingAssets.empty()) {
            std::cout << "\nMISSING ASSETS (" << overallResult.missingAssets.size() << "):" << std::endl;
            for (const auto& asset : overallResult.missingAssets) {
                std::cout << "  ❌ " << asset << std::endl;
            }
        }
        
        if (!overallResult.corruptAssets.empty()) {
            std::cout << "\nCORRUPT ASSETS (" << overallResult.corruptAssets.size() << "):" << std::endl;
            for (const auto& asset : overallResult.corruptAssets) {
                std::cout << "  🔧 " << asset << std::endl;
            }
        }
        
        if (!overallResult.oversizedAssets.empty()) {
            std::cout << "\nOVERSIZED ASSETS (" << overallResult.oversizedAssets.size() << "):" << std::endl;
            for (const auto& asset : overallResult.oversizedAssets) {
                std::cout << "  📏 " << asset << std::endl;
            }
        }
        
        if (!suggestedAlternatives.empty()) {
            std::cout << "\n--- SUGGESTED ALTERNATIVES ---" << std::endl;
            for (const auto& [assetId, alternatives] : suggestedAlternatives) {
                std::cout << "\nFor '" << assetId << "':" << std::endl;
                for (const auto& alt : alternatives) {
                    std::cout << "  💡 " << alt << std::endl;
                }
            }
        }
        
        if (!recommendedActions.empty()) {
            std::cout << "\n--- RECOMMENDED ACTIONS ---" << std::endl;
            for (const auto& action : recommendedActions) {
                std::cout << "  📋 " << action << std::endl;
            }
        }
    } else {
        std::cout << "\n✅ All assets validated successfully!" << std::endl;
    }
    
    std::cout << "\n=================================" << std::endl;
}

void AssetValidator::ValidationReport::saveToFile(const std::string& filePath) const {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to save validation report to: %s", filePath.c_str());
        return;
    }
    
    file << "ASSET VALIDATION REPORT\n";
    file << "======================\n\n";
    file << "Generated: " << reportTimestamp << "\n";
    file << "Total assets checked: " << overallResult.totalAssetsChecked << "\n\n";
    
    if (overallResult.hasIssues()) {
        file << "ISSUES FOUND:\n\n";
        
        if (!overallResult.missingAssets.empty()) {
            file << "Missing Assets (" << overallResult.missingAssets.size() << "):\n";
            for (const auto& asset : overallResult.missingAssets) {
                file << "  - " << asset << "\n";
            }
            file << "\n";
        }
        
        if (!overallResult.corruptAssets.empty()) {
            file << "Corrupt Assets (" << overallResult.corruptAssets.size() << "):\n";
            for (const auto& asset : overallResult.corruptAssets) {
                file << "  - " << asset << "\n";
            }
            file << "\n";
        }
        
        if (!suggestedAlternatives.empty()) {
            file << "Suggested Alternatives:\n";
            for (const auto& [assetId, alternatives] : suggestedAlternatives) {
                file << "  " << assetId << ":\n";
                for (const auto& alt : alternatives) {
                    file << "    - " << alt << "\n";
                }
            }
            file << "\n";
        }
        
        if (!recommendedActions.empty()) {
            file << "Recommended Actions:\n";
            for (const auto& action : recommendedActions) {
                file << "  - " << action << "\n";
            }
        }
    } else {
        file << "All assets validated successfully!\n";
    }
    
    file.close();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Validation report saved to: %s", filePath.c_str());
}

void AssetValidator::setAssetManager(AssetManager* assetManager) {
    assetManager_ = assetManager;
}

bool AssetValidator::loadAssetConfiguration(const std::string& configPath) {
    try {
        if (!std::filesystem::exists(configPath)) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Config file not found: %s", configPath.c_str());
            return false;
        }
        
        // This is a simplified version - in a real implementation you'd parse JSON
        // For now, we'll assume the config follows the game's existing structure
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Asset configuration loaded from: %s", configPath.c_str());
        return true;
        
    } catch (const std::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error loading config: %s", e.what());
        return false;
    }
}

void AssetValidator::autoDiscoverAssets(const std::string& assetsRootPath) {
    assetsRootPath_ = assetsRootPath;
    discoveredPaths_.clear();
    
    if (!std::filesystem::exists(assetsRootPath)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Assets root path not found: %s", assetsRootPath.c_str());
        return;
    }
    
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Auto-discovering assets in: %s", assetsRootPath.c_str());
    
    // Scan common directories
    std::vector<std::pair<std::string, std::string>> commonDirs = {
        {"sprites/digimon", "digimon"},
        {"sprites/player_digimon", "player_digimon"},
        {"sprites/enemy_digimon", "enemy_digimon"},
        {"ui", "ui"},
        {"backgrounds", "backgrounds"},
        {"sprites", "sprites"}
    };
    
    for (const auto& [subPath, category] : commonDirs) {
        std::string fullPath = assetsRootPath + "/" + subPath;
        if (std::filesystem::exists(fullPath)) {
            scanDirectory(fullPath, category);
        }
    }
    
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Asset discovery complete. Found %zu categories", 
               discoveredPaths_.size());
}

void AssetValidator::scanDirectory(const std::string& dirPath, const std::string& category) {
    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(dirPath)) {
            if (entry.is_regular_file()) {
                std::string extension = entry.path().extension().string();
                std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
                
                // Check for image file extensions
                if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || 
                    extension == ".bmp" || extension == ".gif") {
                    
                    std::string assetId = generateAssetId(entry.path().string(), category);
                    discoveredPaths_[category].push_back(entry.path().string());
                    
                    // Register with asset manager if available
                    if (assetManager_) {
                        assetManager_->registerAssetPath(assetId, entry.path().string());
                    }
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Error scanning directory %s: %s", 
                   dirPath.c_str(), e.what());
    }
}

std::string AssetValidator::generateAssetId(const std::string& filePath, const std::string& category) {
    std::filesystem::path path(filePath);
    std::string filename = path.stem().string(); // filename without extension
    
    // Create a hierarchical ID based on the path structure
    std::string relativePath = std::filesystem::relative(path, assetsRootPath_).string();
    
    // Replace path separators and remove extension
    std::replace(relativePath.begin(), relativePath.end(), '\\', '/');
    std::replace(relativePath.begin(), relativePath.end(), '/', '_');
    
    // Remove the extension
    size_t lastDot = relativePath.find_last_of('.');
    if (lastDot != std::string::npos) {
        relativePath = relativePath.substr(0, lastDot);
    }
    
    return relativePath;
}

AssetValidator::ValidationReport AssetValidator::performFullValidation() {
    ValidationReport report;
    
    if (!assetManager_) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No asset manager set for validation");
        return report;
    }
    
    // Generate timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    report.reportTimestamp = ss.str();
    
    // Perform validation
    report.overallResult = assetManager_->validateRegisteredAssets();
    
    // Generate suggestions for missing assets
    for (const std::string& missingAsset : report.overallResult.missingAssets) {
        // Extract asset ID from the missing asset string
        std::string assetId = missingAsset.substr(0, missingAsset.find(' '));
        std::vector<std::string> alternatives = assetManager_->findAlternativePaths(assetId);
        
        if (!alternatives.empty()) {
            report.suggestedAlternatives[assetId] = alternatives;
        }
        
        // Add to recommended actions
        if (alternatives.empty()) {
            report.recommendedActions.push_back("Create or locate asset: " + assetId);
        } else {
            report.recommendedActions.push_back("Consider using alternative for: " + assetId);
        }
    }
    
    // Add general recommendations
    if (report.overallResult.hasIssues()) {
        report.recommendedActions.push_back("Run asset auto-discovery to find replacement files");
        report.recommendedActions.push_back("Enable fallback textures to improve user experience");
        
        if (!report.overallResult.corruptAssets.empty()) {
            report.recommendedActions.push_back("Re-export or replace corrupt image files");
        }
    }
    
    return report;
}

AssetValidator::ValidationReport AssetValidator::validateCategory(const std::string& category) {
    ValidationReport report;
    
    // This is a simplified implementation - you'd need to track categories in the asset manager
    // For now, we'll perform a general validation and filter results
    report = performFullValidation();
    
    // Filter results to only include assets from the specified category
    auto filterByCategory = [&category](std::vector<std::string>& assets) {
        assets.erase(std::remove_if(assets.begin(), assets.end(), 
            [&category](const std::string& asset) {
                return asset.find(category) == std::string::npos;
            }), assets.end());
    };
    
    filterByCategory(report.overallResult.missingAssets);
    filterByCategory(report.overallResult.corruptAssets);
    filterByCategory(report.overallResult.oversizedAssets);
    
    return report;
}

int AssetValidator::autoFixAssetPaths(bool dryRun) {
    if (!assetManager_) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No asset manager set for auto-fix");
        return 0;
    }
    
    auto result = assetManager_->validateRegisteredAssets();
    int fixedCount = 0;
    
    for (const std::string& missingAssetEntry : result.missingAssets) {
        std::string assetId = missingAssetEntry.substr(0, missingAssetEntry.find(' '));
        
        if (tryAutoFixPath(assetId)) {
            fixedCount++;
            if (dryRun) {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[DRY RUN] Would fix asset: %s", assetId.c_str());
            } else {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Fixed asset path for: %s", assetId.c_str());
            }
        }
    }
    
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s: %d asset paths could be fixed", 
               dryRun ? "Dry run complete" : "Auto-fix complete", fixedCount);
    
    return fixedCount;
}

bool AssetValidator::tryAutoFixPath(const std::string& assetId) {
    // Try to find a similar file in discovered paths
    std::string normalizedId = normalizeAssetId(assetId);
    
    for (const auto& [category, paths] : discoveredPaths_) {
        for (const std::string& path : paths) {
            std::string pathId = generateAssetId(path, category);
            std::string normalizedPathId = normalizeAssetId(pathId);
            
            // Check for exact match or close match
            if (normalizedPathId == normalizedId || 
                normalizedPathId.find(normalizedId) != std::string::npos ||
                normalizedId.find(normalizedPathId) != std::string::npos) {
                
                // Found a potential match
                if (assetManager_) {
                    assetManager_->registerAssetPath(assetId, path);
                    return true;
                }
            }
        }
    }
    
    return false;
}

std::string AssetValidator::normalizeAssetId(const std::string& assetId) {
    std::string normalized = assetId;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);
    
    // Remove common suffixes/prefixes
    std::vector<std::string> toRemove = {"_sprite", "_texture", "_img", "_image", "sprite_", "texture_", "img_"};
    
    for (const std::string& remove : toRemove) {
        size_t pos = normalized.find(remove);
        if (pos != std::string::npos) {
            normalized.erase(pos, remove.length());
        }
    }
    
    return normalized;
}

std::vector<std::string> AssetValidator::generateFallbackSuggestions(const std::string& missingAssetId) {
    std::vector<std::string> suggestions;
    
    // Look through discovered assets for similar ones
    std::string normalizedMissing = normalizeAssetId(missingAssetId);
    
    for (const auto& [category, paths] : discoveredPaths_) {
        for (const std::string& path : paths) {
            std::string pathId = generateAssetId(path, category);
            std::string normalizedPath = normalizeAssetId(pathId);
            
            // Calculate similarity (simple substring matching)
            if (normalizedPath.find(normalizedMissing.substr(0, 5)) != std::string::npos ||
                normalizedMissing.find(normalizedPath.substr(0, 5)) != std::string::npos) {
                suggestions.push_back(path);
            }
        }
    }
    
    // Limit suggestions to avoid overwhelming output
    if (suggestions.size() > 5) {
        suggestions.resize(5);
    }
    
    return suggestions;
}
