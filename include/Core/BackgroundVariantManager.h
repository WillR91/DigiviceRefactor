#ifndef BACKGROUND_VARIANT_MANAGER_H
#define BACKGROUND_VARIANT_MANAGER_H

#include <vector>
#include <string>
#include <random>
#include "core/MapData.h"  // Include full definition instead of forward declaration

namespace Digivice {
    /**
     * BackgroundVariantManager handles random selection and management of background variants.
     * Supports the new asset structure with variant paths like:
     * - assets/backgrounds/environmentsnew/tropicaljungle_fg_v1.png
     * - assets/backgrounds/environmentsnew/tropicaljungle_bg_v2.png
     */
    class BackgroundVariantManager {
    public:
        /**
         * Initializes variant paths and randomly selects variants for all layers of a node.
         * Populates foregroundPaths, middlegroundPaths, backgroundPaths and sets selected variants.
         */
        static void initializeVariantsForNode(BackgroundLayerData& layerData, const std::string& environmentName);

        /**
         * Legacy method for backward compatibility with old single-layer system.
         * Randomly selects variants for a layer's texture paths and stores selection.
         */
        static void initializeVariantsForLayer(BackgroundLayerData& layerData);

        /**
         * Returns the selected path from a list of variant paths based on stored index.
         * If selectedIndex is out of bounds, returns the first path or empty string.
         */
        static std::string getSelectedPath(const std::vector<std::string>& variants, int selectedIndex);

        /**
         * Generates variant paths for a given environment name and layer using the new naming convention.
         * Example: ("tropicaljungle", "fg") -> ["assets/backgrounds/environmentsnew/tropicaljungle_fg_v1.png"]
         * Example: ("tropicaljungle", "bg") -> ["assets/backgrounds/environmentsnew/tropicaljungle_bg_v1.png", 
         *                                       "assets/backgrounds/environmentsnew/tropicaljungle_bg_v2.png", ...]
         */
        static std::vector<std::string> generateVariantPaths(const std::string& environmentName, 
                                                             const std::string& layer, 
                                                             int maxVariants = 10);

        /**
         * Sets the random seed for variant selection (useful for testing or deterministic behavior).
         */
        static void setSeed(unsigned int seed);

        /**
         * Maps current environment folder names to new asset naming convention.
         * Example: "01_file_island/01_tropical_jungle" -> "tropicaljungle"
         */
        static std::string mapEnvironmentName(const std::string& currentPath);

    private:
        static std::mt19937 rng_;
        static const std::string NEW_ASSET_BASE_PATH;
    };

} // namespace Digivice

#endif // BACKGROUND_VARIANT_MANAGER_H
