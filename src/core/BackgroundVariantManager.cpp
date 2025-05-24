#include "Core/BackgroundVariantManager.h"
#include "Core/MapData.h"
#include <random>
#include <chrono>
#include <algorithm>
#include <iostream>

namespace Digivice {

    // Initialize static members
    std::mt19937 BackgroundVariantManager::rng_(std::chrono::steady_clock::now().time_since_epoch().count());
    const std::string BackgroundVariantManager::NEW_ASSET_BASE_PATH = "assets/backgrounds/environmentsnew/";

    void BackgroundVariantManager::initializeVariantsForNode(BackgroundLayerData& layerData, const std::string& environmentName) {
        // Generate variant paths for each layer type
        layerData.foregroundPaths = generateVariantPaths(environmentName, "fg", 3);
        layerData.middlegroundPaths = generateVariantPaths(environmentName, "mg", 3);
        layerData.backgroundPaths = generateVariantPaths(environmentName, "bg", 10); // Backgrounds have more variants
        
        // Randomly select variants for each layer
        if (!layerData.foregroundPaths.empty()) {
            std::uniform_int_distribution<int> fgDist(0, static_cast<int>(layerData.foregroundPaths.size() - 1));
            layerData.selectedForegroundVariant = fgDist(rng_);
        }
        
        if (!layerData.middlegroundPaths.empty()) {
            std::uniform_int_distribution<int> mgDist(0, static_cast<int>(layerData.middlegroundPaths.size() - 1));
            layerData.selectedMiddlegroundVariant = mgDist(rng_);
        }
        
        if (!layerData.backgroundPaths.empty()) {
            std::uniform_int_distribution<int> bgDist(0, static_cast<int>(layerData.backgroundPaths.size() - 1));
            layerData.selectedBackgroundVariant = bgDist(rng_);        }
        
        std::cout << "BackgroundVariantManager: Initialized variants for '" << environmentName 
                  << "' - FG:" << (layerData.selectedForegroundVariant + 1) << "/" << layerData.foregroundPaths.size()
                  << ", MG:" << (layerData.selectedMiddlegroundVariant + 1) << "/" << layerData.middlegroundPaths.size()
                  << ", BG:" << (layerData.selectedBackgroundVariant + 1) << "/" << layerData.backgroundPaths.size() << std::endl;
    }

    void BackgroundVariantManager::initializeVariantsForLayer(BackgroundLayerData& layerData) {
        // Legacy support for old texture paths system
        if (layerData.texturePaths.empty()) {
            return;
        }

        // For backward compatibility, randomly select one of the available texture paths
        std::uniform_int_distribution<int> dist(0, static_cast<int>(layerData.texturePaths.size() - 1));        int selectedIndex = dist(rng_);
        
        std::cout << "BackgroundVariantManager: Legacy mode - Selected variant " 
                  << (selectedIndex + 1) << "/" << layerData.texturePaths.size() << " for layer" << std::endl;
    }

    std::string BackgroundVariantManager::getSelectedPath(const std::vector<std::string>& variants, int selectedIndex) {        if (variants.empty()) {
            return "";
        }

        if (selectedIndex < 0 || selectedIndex >= static_cast<int>(variants.size())) {
            std::cout << "BackgroundVariantManager: Invalid index " << selectedIndex << " for " << variants.size() 
                      << " variants, using first" << std::endl;
            return variants[0];
        }

        return variants[selectedIndex];
    }

    std::vector<std::string> BackgroundVariantManager::generateVariantPaths(const std::string& environmentName,
                                                                            const std::string& layer,
                                                                            int maxVariants) {
        std::vector<std::string> variants;
        
        // Map environment names to their directory structure based on actual asset layout
        std::string continentPath;
        
        if (environmentName == "tropicaljungle") {
            continentPath = "01_file_island/01_tropicaljungle/";
        } else if (environmentName == "lake") {
            continentPath = "01_file_island/02_lake/";
        } else if (environmentName == "gearsavannah") {
            continentPath = "01_file_island/03_gearsavannah/";
        } else if (environmentName == "factorialtown") {
            continentPath = "01_file_island/04_factorialtown/";
        } else if (environmentName == "toytown") {
            continentPath = "01_file_island/05_toytown/";        } else if (environmentName == "infinitymountain") {
            continentPath = "01_file_island/06_infinitymountain/";
        } else {
            // For now, fallback to a generic structure for unmapped environments
            std::cout << "BackgroundVariantManager: Unknown environment '" << environmentName 
                      << "', using fallback path" << std::endl;
            continentPath = "01_file_island/01_tropicaljungle/"; // Fallback
        }
        
        // Generate paths based on the new naming convention
        // Example: assets/backgrounds/environmentsnew/01_file_island/01_tropicaljungle/tropicaljungle_fg_v1.png
        for (int i = 1; i <= maxVariants; ++i) {            std::string variantPath = NEW_ASSET_BASE_PATH + continentPath + environmentName + "_" + layer + "_v" + std::to_string(i) + ".png";
            variants.push_back(variantPath);
        }
        
        std::cout << "BackgroundVariantManager: Generated " << maxVariants << " variant paths for " 
                  << environmentName << "_" << layer << " in " << continentPath << std::endl;

        return variants;
    }

    void BackgroundVariantManager::setSeed(unsigned int seed) {
        rng_.seed(seed);
        std::cout << "BackgroundVariantManager: Random seed set to " << seed << std::endl;
    }

    std::string BackgroundVariantManager::mapEnvironmentName(const std::string& currentPath) {
        // Map old environment paths to new naming convention
        // Example: "assets/backgrounds/environments/01_file_island/01_tropical_jungle/layer_0.png" -> "tropicaljungle"
        
        // Extract folder name from path
        std::string folderName;
        size_t lastSlash = currentPath.find_last_of('/');
        if (lastSlash != std::string::npos) {
            // Find the parent folder (environment folder)
            size_t parentSlash = currentPath.find_last_of('/', lastSlash - 1);
            if (parentSlash != std::string::npos) {
                folderName = currentPath.substr(parentSlash + 1, lastSlash - parentSlash - 1);
            }
        }
        
        // Clean up folder name (remove numbers and underscores at start)
        size_t firstLetter = folderName.find_first_of("abcdefghijklmnopqrstuvwxyz");
        if (firstLetter != std::string::npos) {
            folderName = folderName.substr(firstLetter);
        }
        
        // Replace underscores with nothing to match new naming convention
        std::string result;
        for (char c : folderName) {
            if (c != '_') {
                result += c;
            }
        }
        
        // Map known environment names
        if (result == "tropicaljungle" || result == "nativeforest") {
            return "tropicaljungle";
        } else if (result == "gearsavannah") {
            return "gearsavannah";
        } else if (result == "lake") {
            return "lake";
        } else if (result == "factorialtown") {
            return "factorialtown";
        } else if (result == "toytown") {
            return "toytown";        } else if (result == "infinitymountain") {
            return "infinitymountain";
        } else {
            std::cout << "BackgroundVariantManager: No mapping found for '" << currentPath 
                      << "', using cleaned name: '" << result << "'" << std::endl;
        }
        
        return result;
    }

} // namespace Digivice
