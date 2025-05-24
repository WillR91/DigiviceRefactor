#ifndef MAP_DATA_H
#define MAP_DATA_H

#include <string>
#include <vector>

namespace Digivice {    // Data for a single parallax scrolling background layer
    struct BackgroundLayerData {
        // Legacy support for existing single texture path system
        std::vector<std::string> texturePaths; // DEPRECATED: Use layer-specific paths instead
        
        // New variant-based system for 1x scale assets
        std::vector<std::string> foregroundPaths;    // All foreground variants (*_fg_v1.png, etc.)
        std::vector<std::string> middlegroundPaths;  // All middleground variants (*_mg_v1.png, etc.)
        std::vector<std::string> backgroundPaths;    // All background variants (*_bg_v1.png, *_bg_v2.png, etc.)
        
        float parallaxFactorX;                 // Horizontal parallax scroll speed
        float parallaxFactorY;                 // Vertical parallax scroll speed (likely 0 for horizontal scroll)
        
        // Variant selection indices
        int selectedForegroundVariant = 0;
        int selectedMiddlegroundVariant = 0;
        int selectedBackgroundVariant = 0;

        // Legacy constructor for backwards compatibility
        BackgroundLayerData(const std::vector<std::string>& paths = {}, float pX = 1.0f, float pY = 0.0f)
            : texturePaths(paths), parallaxFactorX(pX), parallaxFactorY(pY) {}
            
        // New constructor for variant-based system
        BackgroundLayerData(const std::vector<std::string>& fg, const std::vector<std::string>& mg, 
                          const std::vector<std::string>& bg, float pX = 1.0f, float pY = 0.0f)
            : foregroundPaths(fg), middlegroundPaths(mg), backgroundPaths(bg), 
              parallaxFactorX(pX), parallaxFactorY(pY) {}
    };

    // Data for a single explorable Node on a continent
    struct NodeData {
        std::string id;                         // Unique identifier, e.g., "file_island_node_01"
        std::string name;                       // Display name, e.g., "Native Forest"
        std::string continentId;                // ID of the continent this node belongs to

        // For Node Selection View
        float mapPositionX;                     // X coordinate on the continent map
        float mapPositionY;                     // Y coordinate on the continent map
        std::string unlockedSpritePath;         // Path to the 'unlocked' node sprite
        // std::string lockedSpritePath;        // Path to the 'locked' node sprite (for later)

        // For Node Detail View
        std::string bossSpritePath;             // Path to the boss/enemy sprite for the detail view
        int totalSteps;                         // Total steps required to complete the node

        // For AdventureState background configuration
        // Order: Layer 0 (Foreground), Layer 1 (Midground), Layer 2 (Background)
        std::vector<BackgroundLayerData> adventureBackgroundLayers;

        bool isUnlocked;                        // For progression (true for prototype)

        NodeData() : mapPositionX(0.0f), mapPositionY(0.0f), totalSteps(0), isUnlocked(true) {}
    };

    // Data for a Continent
    struct ContinentData {
        std::string id;                         // Unique identifier, e.g., "file_island"
        std::string name;                       // Display name, e.g., "File Island"
        std::string mapImagePath;               // Path to the full-screen continent map image
        std::vector<NodeData> nodes;            // List of nodes on this continent
    };

} // namespace Digivice

#endif // MAP_DATA_H
