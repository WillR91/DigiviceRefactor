# DigiviceRefactor - Map System Design Summary

## 1. Overview & Core Goal

This document outlines the design for the map and area navigation system in the DigiviceRefactor project. The primary goal is to faithfully emulate the hierarchical map and area selection mechanics of the Digivice -25th COLOR EVOLUTION- device, allowing players to navigate through different game worlds (continents), select specific locations (nodes), and access the events or battles within those nodes.

A key aspect is the dynamic management of background environment assets for the "adventure mode" gameplay that occurs within each node.

For initial development and testing, all continents, nodes, and events/battles will be treated as "unlocked" or accessible. The progression gating (locking/unlocking based on player progress) will be implemented in a later phase.

## 2. Hierarchical Structure & Navigation

The map system is structured in three main hierarchical levels:

### 2.1. Continent Level

*   **Access:** Players access this level by selecting a "Map" option from the game's main menu system.
*   **Visuals:**
    *   A full-screen image representing the currently selected continent is displayed.
    *   When transitioning between continents, a fade-to-black effect (consistent with other menu transitions in the game) will be used.
*   **Navigation:**
    *   Players can scroll Up or Down to cycle through the available continents in the game world. Each scroll action changes the displayed full-screen continent image.
*   **Text Display:**
    *   The name of the currently displayed continent is shown as a text label. This text is intended to scroll horizontally across the screen.
*   **Action:**
    *   Pressing an "Enter" or "Confirm" button selects the currently displayed continent and transitions the player to the Node Level for that continent.

### 2.2. Node Level (within a selected Continent)

*   **Visuals:**
    *   The full-screen image of the selected continent persists as the background.
    *   A series of "node" assets (sprites or icons) are overlaid onto the continent map. These nodes appear at predefined coordinates, corresponding to specific locations on that continent where game events occur.
    *   **Node Appearance & State:**
        *   **Unlocked/Available Nodes:** Displayed as white-colored assets.
        *   **Locked Nodes:** Displayed as grey-colored assets. These will change to white as they become unlocked through player progression in the final system. (For initial development, all may appear as white/available).
*   **Navigation:**
    *   The player can navigate (e.g., using directional input) to highlight different nodes visible on the current continent map.
*   **Text Display:**
    *   When a node is highlighted, its specific name is displayed as a text label. This name "pops up" (likely replacing any previous node name or a general continent name) and is also intended to scroll horizontally across the screen.
*   **Progression (Final System):**
    *   Typically, one node on a continent will be unlocked initially.
    *   Completing the events/battles within a node will unlock the next sequential node on that continent.
*   **Action:**
    *   Pressing "Enter" or "Confirm" while a node is highlighted and available selects that node and transitions the player to the Event/Battle Level for that specific node.

### 2.3. Event/Battle Level (within a selected Node)

*   **Visuals:**
    *   This level presents as a menu screen, consistent in style with other menu interfaces in the game.
    *   **Enemy Sprite:** An idling sprite of the primary enemy Digimon (often the "boss") that the player will encounter at the end of this node's sequence of events/battles is displayed.
    *   **Step Information:**
        *   The total number of steps required to complete the node (i.e., reach its final event/battle) is displayed.
        *   The player's current progress in steps for that node is also shown (e.g., "0/400 steps", "250/500 steps").
*   **Navigation:**
    *   The player can scroll Up or Down to navigate through a list of events and battles contained within the selected node.
*   **Progression (Final System):**
    *   The first event/battle in the list for a node will typically be unlocked initially.
    *   Successfully completing an event or winning a battle will unlock the next item in the sequence for that specific node.
*   **Action:**
    *   Selecting an available event or battle from the menu will initiate that specific game segment (e.g., trigger a cutscene, start a combat encounter).

## 3. Adventure Mode Backgrounds (per Node)

A critical feature is the display of unique environments when the player is in "adventure mode" (the state where they are accumulating steps towards an area's completion).

*   **Requirement:** Each playable "node" must be associated with its own unique set of background environment assets that are displayed during adventure mode gameplay within that node.
*   **Asset Reusability:** While some individual background elements or layers might be reused across different scenes or nodes, each node will have a distinct, defined configuration for its complete background scene.
*   **System Design Approach (Data-Driven):**
    *   The `AdventureState` (or the equivalent game state responsible for step-based exploration) will contain generic logic for rendering layered backgrounds.
    *   The specific background assets for any given node will be defined in data structures associated with that node. This data will include:
        *   Paths to the texture files for each background layer (e.g., far background, mid-ground, foreground).
        *   Optionally, properties for each layer, such as parallax scrolling factors (to create a sense of depth), Z-order for layering, and any animation details if backgrounds are dynamic.
    *   When the player enters a node in adventure mode, the `AdventureState` will read the background configuration for that specific node and load/render the appropriate assets.
    *   This approach ensures flexibility: changing a node's environment involves modifying its data configuration, not altering the core rendering code of the `AdventureState`. It also makes adding new nodes with new environments straightforward.

## 4. Development Phasing

The implementation of the map system will occur in two main phases:

*   **Phase 1 (Current/Initial Focus):**
    *   Implement the basic hierarchical structure (Continent -> Node -> Event/Battle).
    *   Develop the UI and navigation for each level as described.
    *   Integrate the display of specified assets (continent images, node sprites, enemy sprites).
    *   For this phase, all continents, nodes, and events/battles will be treated as "unlocked" and accessible by default. This facilitates easier development, testing of individual areas, and asset integration without being blocked by unimplemented progression logic.
*   **Phase 2 (Later Development):**
    *   Implement the full progression logic. This includes:
        *   Locking and unlocking continents based on overall game progress.
        *   Locking and unlocking nodes sequentially based on the completion of prerequisite nodes.
        *   Locking and unlocking events/battles within a node based on the completion of preceding events/battles in that node's list.
        *   Integrating this progression with the game's overall save/load system.
