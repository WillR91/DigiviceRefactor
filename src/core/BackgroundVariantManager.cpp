#include "Core/BackgroundVariantManager.h"
#include "Core/MapData.h"
#include <random>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <filesystem>
#include <set>

namespace Digivice {

    // Initialize static members
    std::mt19937 BackgroundVariantManager::rng_(std::chrono::steady_clock::now().time_since_epoch().count());
    const std::string BackgroundVariantManager::NEW_ASSET_BASE_PATH = "assets/backgrounds/environmentsnew/";    void BackgroundVariantManager::initializeVariantsForNode(BackgroundLayerData& layerData, const std::string& environmentName) {
        // Generate variant paths for each layer type
        layerData.foregroundPaths = generateVariantPaths(environmentName, "fg", 3);
        layerData.middlegroundPaths = generateVariantPaths(environmentName, "mg", 3);
        layerData.backgroundPaths = generateVariantPaths(environmentName, "bg", 10); // Backgrounds have more variants
        
        // Initialize selected variants to 0 (default)
        layerData.selectedForegroundVariant = 0;
        layerData.selectedMiddlegroundVariant = 0;
        layerData.selectedBackgroundVariant = 0;
        
        // Randomly select variants for each layer if variants exist
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
            layerData.selectedBackgroundVariant = bgDist(rng_);
        }
          // Only log initialization if there are issues or in debug mode
        // This reduces startup console spam significantly
        int totalVariants = layerData.foregroundPaths.size() + layerData.middlegroundPaths.size() + layerData.backgroundPaths.size();
        if (totalVariants == 0) {
            std::cout << "BackgroundVariantManager: WARNING - No variants found for '" << environmentName << "'" << std::endl;
        }
    }

    void BackgroundVariantManager::initializeVariantsForLayer(BackgroundLayerData& layerData) {
        // Legacy support for old texture paths system
        if (layerData.texturePaths.empty()) {
            return;
        }        // For backward compatibility, randomly select one of the available texture paths
        std::uniform_int_distribution<int> dist(0, static_cast<int>(layerData.texturePaths.size() - 1));        int selectedIndex = dist(rng_);
        
        // Only log legacy mode usage once to avoid spam
        static bool legacyWarningShown = false;
        if (!legacyWarningShown) {
            std::cout << "BackgroundVariantManager: Using legacy texture path system" << std::endl;
            legacyWarningShown = true;
        }
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
            continentPath = "01_file_island/05_toytown/";
        } else if (environmentName == "infinitymountain") {
            continentPath = "01_file_island/06_infinitymountain/";
        } else if (environmentName == "freezeland") {
            continentPath = "02_file_island_broken/01_freezeland/";
        } else if (environmentName == "ancientdinoregion") {
            continentPath = "02_file_island_broken/02_ancientdinoregion/";
        } else if (environmentName == "overdellcemetary") {
            continentPath = "02_file_island_broken/03_overdellcemetary/";
        } else if (environmentName == "townofbeginnings") {
            continentPath = "02_file_island_broken/04_townofbeginnings/";
        } else if (environmentName == "infinitymountain2") {
            continentPath = "02_file_island_broken/05_infinitymountain2/";
        } else if (environmentName == "opensea") {
            continentPath = "02_file_island_broken/06_opensea/";
        } else if (environmentName == "koromonsvillage") {
            continentPath = "03_server_continent/01_koromonsvillage/";
        } else if (environmentName == "colosseum") {
            continentPath = "03_server_continent/02_colosseum/";
        } else if (environmentName == "desert") {
            continentPath = "03_server_continent/03_desert/";
        } else if (environmentName == "piccolomonsforest") {
            continentPath = "03_server_continent/04_piccolomonsforest/";
        } else if (environmentName == "reversepyramid") {
            continentPath = "03_server_continent/05_reversepyramid/";
        } else if (environmentName == "amusementpark") {
            continentPath = "03_server_continent/06_amusementpark/";
        } else if (environmentName == "restaurant") {
            continentPath = "03_server_continent/07_restaurant/";
        } else if (environmentName == "differentspace") {
            continentPath = "03_server_continent/08_differentspace/";
        } else if (environmentName == "gekomonscastle") {
            continentPath = "03_server_continent/09_gekomonscastle/";
        } else if (environmentName == "myotismonscastle") {
            continentPath = "03_server_continent/10_myotismonscastle/";
        } else if (environmentName == "hikarigoaka") {
            continentPath = "04_tokyo/01_hikarigoaka/";
        } else if (environmentName == "harumi") {
            continentPath = "04_tokyo/02_harumi/";
        } else if (environmentName == "pier") {
            continentPath = "04_tokyo/03_pier/";
        } else if (environmentName == "tower") {
            continentPath = "04_tokyo/04_tower/";        } else if (environmentName == "shibuya") {
            continentPath = "04_tokyo/05_shibuya/";
        } else if (environmentName == "exhibitioncentre") {
            continentPath = "04_tokyo/06_exhibitioncentre/";
        } else if (environmentName == "jungle") {
            continentPath = "05_spiral_mountain/01_jungle/";
        } else if (environmentName == "colosseum2") {
            continentPath = "05_spiral_mountain/02_colosseum2/";
        } else if (environmentName == "digitalsea") {
            continentPath = "05_spiral_mountain/03_digitalsea/";
        } else if (environmentName == "digitalforest") {
            continentPath = "05_spiral_mountain/04_digitalforest/";
        } else if (environmentName == "digitalcity") {
            continentPath = "05_spiral_mountain/05_digitalcity/";
        } else if (environmentName == "wasteland") {
            continentPath = "05_spiral_mountain/06_wasteland/";
        } else if (environmentName == "subspace") {
            continentPath = "06_subspace/01_subspace/";
        } else if (environmentName == "network") {
            continentPath = "07_network/01_network/";        } else {
            // For now, fallback to a generic structure for unmapped environments
            // Use static set to only warn once per unknown environment
            static std::set<std::string> unknownEnvironments;
            if (unknownEnvironments.find(environmentName) == unknownEnvironments.end()) {
                std::cout << "BackgroundVariantManager: Unknown environment '" << environmentName 
                          << "', using fallback path" << std::endl;
                unknownEnvironments.insert(environmentName);
            }
            continentPath = "01_file_island/01_tropicaljungle/"; // Fallback
        }          // Dynamically detect which variants actually exist
        // Check variants from v1 up to maxVariants, but only add those that exist
        int foundVariants = 0;
        for (int i = 1; i <= maxVariants; ++i) {
            std::string variantPath = NEW_ASSET_BASE_PATH + continentPath + environmentName + "_" + layer + "_v" + std::to_string(i) + ".png";
            
            // Check if the file actually exists
            try {
                if (std::filesystem::exists(variantPath)) {
                    variants.push_back(variantPath);
                    foundVariants++;
                    // Only log if this is the first variant or if we're in debug mode
                    // Reduces console spam significantly
                }
            } catch (const std::filesystem::filesystem_error& e) {
                // Only log errors, not missing files (which are expected)
                if (i == 1) { // Only log error for the first variant to avoid spam
                    std::cout << "BackgroundVariantManager: Error checking " << environmentName << "_" << layer 
                              << " variants: " << e.what() << std::endl;
                }
            }
        }
        
        // Only log summary if no variants found (which indicates a potential problem)
        if (foundVariants == 0) {
            std::cout << "BackgroundVariantManager: No variants found for " 
                      << environmentName << "_" << layer << " (checked up to v" << maxVariants << ")" << std::endl;
        }

        // Fallback: if no variants found, try adding the first variant path anyway for compatibility
        if (variants.empty()) {
            std::string fallbackPath = NEW_ASSET_BASE_PATH + continentPath + environmentName + "_" + layer + "_v1.png";
            variants.push_back(fallbackPath);
            std::cout << "BackgroundVariantManager: WARNING - No variants found, added fallback path: " 
                      << fallbackPath << std::endl;
        }

        return variants;
    }    void BackgroundVariantManager::setSeed(unsigned int seed) {
        rng_.seed(seed);
        // Removed debug output to reduce console noise
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
            return "toytown";
        } else if (result == "infinitymountain") {
            return "infinitymountain";
        } else if (result == "freezeland") {
            return "freezeland";
        } else if (result == "ancientdinoregion") {
            return "ancientdinoregion";
        } else if (result == "overdellcemetary") {
            return "overdellcemetary";
        } else if (result == "townofbeginnings") {
            return "townofbeginnings";
        } else if (result == "infinitymountain2") {
            return "infinitymountain2";
        } else if (result == "opensea") {
            return "opensea";
        } else if (result == "koromonsvillage") {
            return "koromonsvillage";
        } else if (result == "colosseum") {
            return "colosseum";
        } else if (result == "desert") {
            return "desert";
        } else if (result == "piccolomonsforest") {
            return "piccolomonsforest";
        } else if (result == "reversepyramid") {
            return "reversepyramid";
        } else if (result == "amusementpark") {
            return "amusementpark";
        } else if (result == "restaurant") {
            return "restaurant";
        } else if (result == "differentspace") {
            return "differentspace";
        } else if (result == "gekomonscastle") {
            return "gekomonscastle";
        } else if (result == "myotismonscastle") {
            return "myotismonscastle";
        } else if (result == "hikarigoaka") {
            return "hikarigoaka";
        } else if (result == "harumi") {
            return "harumi";
        } else if (result == "pier") {
            return "pier";
        } else if (result == "tower") {
            return "tower";
        } else if (result == "shibuya") {
            return "shibuya";
        } else if (result == "jungle") {
            return "jungle";
        } else if (result == "colosseum2") {
            return "colosseum2";
        } else if (result == "digitalsea") {
            return "digitalsea";
        } else if (result == "digitalforest") {
            return "digitalforest";
        } else if (result == "digitalcity") {
            return "digitalcity";
        } else if (result == "wasteland") {
            return "wasteland";
        } else if (result == "subspace") {
            return "subspace";
        } else if (result == "network") {
            return "network";
        } else {
            std::cout << "BackgroundVariantManager: No mapping found for '" << currentPath 
                      << "', using cleaned name: '" << result << "'" << std::endl;
        }
        
        return result;
    }

} // namespace Digivice
