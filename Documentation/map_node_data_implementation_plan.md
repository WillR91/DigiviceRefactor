# Map Node Data Implementation Plan

## 1. Purpose

This document outlines the planned approach for defining and managing data for individual selectable areas (nodes) on a continent within the game's map system. The initial focus is on the "File Island" continent and its associated nodes.

This plan follows the discussions regarding the need to expand the map system beyond the initial "Tropical Jungle" node to include all specified nodes for "File Island."

## 2. Requirement Recap

The map system requires a structured way to store and access information for each node on a continent. This information includes:

*   A unique identifier.
*   The display name shown to the player.
*   The corresponding asset folder for its environment backgrounds.
*   Its unlock status.
*   (Future) Paths to visual marker assets for the map.
*   (Future) Coordinates for placing these markers on the continent map.
*   (Future) Information relevant to the event selection screen for that node (e.g., final enemy sprite, step goal).

## 3. Proposed Data Structure: `MapNodeData`

To manage this information effectively, the following C++ structure (`MapNodeData`) was proposed:

```cpp
#include <string>
// Potentially include <vector> if events are nested later

struct MapNodeData {
    std::string id;                       // Unique internal identifier (e.g., "tropical_jungle")
    std::string displayName;              // Name for UI display (e.g., "Tropical Jungle")
    std::string assetFolderName;          // Folder name in assets (e.g., "0_tropical_jungle")
    bool isUnlocked;                    // Tracks if the node is accessible
    
    // --- Fields for future expansion ---
    // std::string nodeMarkerImageLockedPath;   // Path to locked node marker asset
    // std::string nodeMarkerImageUnlockedPath; // Path to unlocked node marker asset
    // int positionX_onMap;                   // X coordinate for node marker on map
    // int positionY_onMap;                   // Y coordinate for node marker on map
    // std::string finalEnemySpritePath;      // Sprite for the node's final enemy (event menu)
    // int stepGoal;                          // Step goal for the node (event menu)
    // std::vector<EventData> events;        // Data for events within this node
};
```

**Field Explanations:**

*   `id`: A non-displayable unique string for programmatic access.
*   `displayName`: The text that will scroll on the screen when the node is highlighted.
*   `assetFolderName`: Crucial for linking the node to its specific environmental background assets found under `assets/backgrounds/environments/file_island/`.
*   `isUnlocked`: A boolean determining if the player can select and enter this node. This will be `true` for "Tropical Jungle" initially and `false` for others, updated as the player progresses.
*   Commented-out fields: Placeholders for future enhancements like distinct map markers, their precise placement, and details for the event selection screen.

## 4. Instantiation and Usage Plan

1.  **Definition Location:** The `MapNodeData` struct will be defined in an appropriate header file, likely one related to map states or game data structures (e.g., `include/States/MapNode.h` or similar, to be decided based on existing project structure).
2.  **Data Storage:** A `std::vector<MapNodeData>` will be used to store the instances for all nodes on a given continent. For "File Island," this vector will be populated with entries for:
    *   Tropical Jungle
    *   LAKE
    *   GEAR SAVANNAH
    *   FACTORIAL TOWN
    *   TOY TOWN
    *   INFINITY MOUNTAIN
3.  **Initialization:** This vector will be initialized with the specific data for each node, setting "Tropical Jungle" as `isUnlocked = true` and others as `false` initially.
4.  **Access:** The map screen logic will iterate over this vector to:
    *   Display the node markers on the continent map (using placeholder or actual assets).
    *   Show the `displayName` of the currently highlighted node.
    *   Determine which node to transition to when the player makes a selection.

## 5. Integration with Existing System

This structured approach will replace or augment any hardcoded logic currently in place for the single "Tropical Jungle" node. It provides a scalable and maintainable way to handle an arbitrary number of nodes per continent.
The existing functionality for displaying "Tropical Jungle" will be adapted to use this data-driven method.

## 6. Next Steps in Implementation (Coding Phase)

1.  **Define `MapNodeData` Struct:** Add the struct definition to the chosen C++ header file.
2.  **Populate Node List:** Create and initialize the `std::vector<MapNodeData>` for "File Island" with the data for all its nodes.
3.  **Modify Map Screen Logic:** Update the map screen's C++ code to:
    *   Read from the new node data vector.
    *   Implement logic to cycle through these nodes.
    *   Display the correct node names and visual markers (even if placeholders initially).
    *   Handle selection and transition based on the chosen node's data.

This document reflects the plan discussed for managing map node data as of the current date.
