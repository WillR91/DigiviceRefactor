# DigiviceRefactor - Map System Prototype: Progress Update

**Date:** May 14, 2025

## 1. Overview

This document tracks the ongoing progress for implementing the map system prototype in the DigiviceRefactor project. The focus has shifted from initial setup to implementing core rendering logic for the first view and integrating the new state into the main game navigation.

## 2. Achievements Since Last Update (May 13, 2025)

1.  **Populated `load_map_data()`:**
    *   The `load_map_data()` method in `MapSystemState.cpp` has been populated with hardcoded data for the "File Island" continent.
    *   Defined the first node, "01\_tropical\_jungle," including:
        *   Paths to its adventure mode background layers (`layer_0.png`, `layer_1.png`, `layer_2.png`).
        *   Placeholder parallax factors.
        *   Paths for the continent map image (`assets/backgrounds/environments/file_island/file_island_map.png`), a placeholder node sprite (`assets/ui/node_white_placeholder.png`), and a placeholder boss sprite.
        *   Placeholder values for node name ("Tropical Jungle"), map position, and total steps (400).
        *   `isUnlocked` set to `true` for prototype testing.

2.  **Prepared `MapSystemState` for Rendering:**
    *   Updated signatures of private render helper methods (`render_continent_selection`, `render_node_selection`, `render_node_detail`) in `MapSystemState.h` to accept `PCDisplay& display`.
    *   Added necessary includes (`AssetManager.h`, `TextRenderer.h`, `PCDisplay.h`) to `MapSystemState.cpp`.
    *   The main `render(PCDisplay& display)` method in `MapSystemState.cpp` now correctly calls the appropriate private render helper method based on `currentView_`, passing the `display` object.

3.  **Implemented Continent View Rendering (`render_continent_selection`):**
    *   The `render_continent_selection(PCDisplay& display)` method in `MapSystemState.cpp` now:
        *   Attempts to load the continent map texture using `AssetManager::getInstance().loadTexture()`.
        *   Draws the loaded texture full-screen using `display.drawTexture()`.
        *   Renders the continent's name at the top-center of the screen using `TextRenderer::getInstance().renderText()`.
        *   Includes basic error handling text if the continent data or map image is missing.

4.  **Integration into Game Flow (`MenuState` and `Game`):**
    *   Modified `src/states/MenuState.cpp`:
        *   Added `#include "states/MapSystemState.h"`.
        *   Updated `handle_input()` so that selecting "VIEW MAP" from the "MAP" submenu now requests a fade transition to a new `MapSystemState` instance, popping the current submenu.
    *   Modified `src/core/Game.cpp` (`Game::init()`):
        *   Changed the initial game state from `AdventureState` to a `MenuState`.
        *   The initial `MenuState` is configured with top-level options: "DIGIMON", "MAP", "ITEMS", "SAVE", "EXIT", allowing navigation to the map system via the "MAP" -> "VIEW MAP" path.
        *   Added `#include "states/MenuState.h"` to `Game.cpp`.

## 3. Current Status & Focus

*   The `MapSystemState` is now integrated into the game's menu navigation.
*   The first view of the map system (Continent Selection) has basic rendering implemented for the map image and continent name.
*   **Current Focus: Resolving Build Errors.** After the latest integration efforts, several build errors have appeared that prevent successful compilation. These errors are primarily related to:
    *   Missing include file `Platform/PCDisplay.h` in `MapSystemState.cpp`.
    *   `MapSystemState` being an undeclared identifier in `MenuState.cpp` (likely due to a missing or incorrect include).
    *   Consequent `std::make_unique` errors in `MenuState.cpp` when trying to create `MapSystemState`.

## 4. Next Steps (After Resolving Build Errors)

1.  **Verify Map System Transition & Rendering:**
    *   Confirm that the game successfully transitions from the `MenuState` to the `MapSystemState`.
    *   Verify that the "File Island" map and name are rendered correctly as per `render_continent_selection()`.
2.  **Implement Node Selection View (`render_node_selection` and input handling):**
    *   Display node sprites (e.g., `node_white_placeholder.png`) at their `mapPositionX`, `mapPositionY` on the continent map.
    *   Implement a visual selection indicator for the currently selected node.
    *   Render the name of the selected node.
    *   Handle input for navigating between nodes on the continent map (e.g., up, down, left, right to select the nearest node in that direction or cycle through a list).
    *   Handle input for confirming a node selection, which should change `currentView_` to `NODE_DETAIL`.
3.  **Implement Node Detail View (`render_node_detail` and input handling):**
    *   Display the selected node's boss sprite (`bossSpritePath`).
    *   Display the node's name and total step count.
    *   Display player's current progress on the node (if applicable, or show 0 / totalSteps).
    *   Handle input for initiating the node (e.g., "Start Adventure" button), which should transition to `AdventureState`, passing the selected node's data.
    *   Handle input for going back to the Node Selection view.
4.  **Integrate with `AdventureState`:**
    *   Modify `AdventureState` to accept `NodeData` (or relevant parts like `adventureBackgroundLayers`).
    *   Ensure `AdventureState` correctly loads and displays the multi-layered parallax backgrounds (including variants if defined) specific to the selected node. The correct order is Layer 0 (FG), Layer 1 (MG), Layer 2 (BG).
5.  **Refine Placeholders:**
    *   Gradually replace placeholder asset paths and data values with actual game assets and correct information as they become available.
6.  **Future Enhancements (Post-Prototype):**
    *   Address multi-continent navigation.
    *   Implement node locking/unlocking based on game progression.
    *   Consider event/battle lists within nodes.
    *   Implement saving/loading of map progression.
    *   Move hardcoded map data from `MapSystemState.cpp` to external data files (e.g., JSON, XML).
    *   Implement layer variant logic for backgrounds in `AdventureState`.
