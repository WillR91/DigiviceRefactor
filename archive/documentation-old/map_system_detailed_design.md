# Map System Detailed Design Document

## 1. Overview

This document outlines the design and implementation details for the in-game map system. The system allows players to navigate continents, select specific areas (nodes) within those continents, and then choose events or battles within those nodes.

The primary goal is to emulate a multi-layered menu system with distinct visual and interactive elements at each layer.

## 2. System Emulation Goal

The system emulates a classic JRPG-style world map interaction:

1.  **Continent View:** Player sees an overworld map of a continent.
    *   Scrolling (e.g., up/down) switches to different continents.
    *   Each continent has a horizontally scrolling text label displaying its name.
2.  **Node View (Area Selection):** Pressing "Enter" on a continent reveals area nodes.
    *   Nodes are visually represented on the continent map.
    *   Initially, one node is unlocked; completing it unlocks the next in sequence.
    *   Each node has a horizontally scrolling text label. When a new node is highlighted, its name replaces the previous one.
3.  **Event/Battle View (Node Interior):** Pressing "Enter" on a node opens a new menu.
    *   This menu lists events and battles within that node.
    *   Navigation is typically vertical (e.g., up/down).
    *   Events/battles unlock sequentially as the player completes them.

## 3. Current Focus: "File Island" Continent - Node Selection and Event Menu

We are currently focused on implementing the map system for the first continent, "File Island," specifically:
*   The transition from the continent view to the node selection view.
*   The node selection menu itself.
*   The transition from a selected node to its event/battle selection menu.

## 4. Detailed Menu Breakdown

### 4.1. Continent View (Map Menu - Top Level)

*   **Visuals:**
    *   Full-screen image of the current continent (e.g., `file_island_map.png` from `assets/backgrounds/environments/file_island/`).
    *   Horizontally scrolling text label for the continent name.
*   **Interaction:**
    *   Scroll up/down: Changes to the next/previous continent.
        *   A fade-to-black transition occurs between continent changes (consistent with other menu transitions in the game).
    *   Press "Enter": Selects the current continent and transitions to the Node View.

### 4.2. Node View (Continent's Area Selection)

*   **Transition In:** After pressing "Enter" on a continent in the Continent View.
*   **Visuals:**
    *   The continent map image remains as the background.
    *   Node markers (assets) appear at relevant positions on the map.
        *   **Locked Nodes:** Greyed-out marker.
        *   **Unlocked Nodes:** White marker.
    *   Horizontally scrolling text label for the currently highlighted node's name. When a new node is highlighted, its name pops up and replaces the previous name.
*   **Interaction:**
    *   Cycle through nodes (e.g., left/right, or following a predefined path).
    *   Press "Enter": Selects the highlighted node and transitions to the Event/Battle View for that node.
*   **Progression:**
    *   Nodes unlock sequentially. Completing one (winning the final battle within it) unlocks the next node on that continent.
    *   Upon returning from a completed node, the Node View reflects the newly unlocked node.
*   **Nodes for "File Island" (in order of progression):**
    1.  Tropical Jungle (initially unlocked)
    2.  LAKE
    3.  GEAR SAVANNAH
    4.  FACTORIAL TOWN
    5.  TOY TOWN
    6.  INFINITY MOUNTAIN
*   **Asset Association:** Each node is associated with a specific folder within `assets/backgrounds/environments/<continent_name>/` (e.g., `assets/backgrounds/environments/file_island/0_tropical_jungle/`, `assets/backgrounds/environments/file_island/1_lake/`, etc.). These folders contain the environment assets for the "adventure mode state" within that node.

### 4.3. Event/Battle View (Node Interior Menu)

*   **Transition In:** After pressing "Enter" on a node in the Node View.
*   **Visuals:**
    *   **Menu Panel:** A semi-transparent blue screen (`map.png` from `assets/ui/backgrounds/`) overlaid on the screen.
    *   **Background:** The specific environment background for the selected node (composed of layers from its asset folder, e.g., `0_tropical_jungle/layer_0`, `layer_1`, `layer_2`) is visible *behind* the semi-transparent menu panel.
    *   **Enemy Sprite:** An idling sprite of the final enemy for that node is displayed on the menu.
    *   **Step Goal/Progress:** Text displaying the number of steps required for that node and any progress made (e.g., "0/400", "200/300").
*   **Interaction:**
    *   Vertically scroll up/down to select an event or battle within that node.
    *   Events/battles unlock sequentially as they are completed.
*   **Progression:** Completing an event/battle unlocks the next one in the list for that node.

## 5. Asset Structure for Node Environments

*   **Location:** `assets/backgrounds/environments/<continent_name>/<node_identifier>/`
    *   Example: `assets/backgrounds/environments/file_island/0_tropical_jungle/`
*   **Contents of each node folder:**
    *   `layer_0`: Foreground layer image(s).
    *   `layer_1`: Middleground layer image(s).
    *   `layer_2`: Background layer image(s).
    *   Variants may exist (e.g., `Layer_0_1.png`). These layers are parallax scrolled during the "adventure mode state" associated with the node.
*   **Continent Map Image:** The main map image for the continent is also stored in the continent's environment folder (e.g., `assets/backgrounds/environments/file_island/file_island_map.png`).

## 6. Current Implementation Plan (Node Selection for "File Island")

1.  **Define Node Data Structure:**
    *   Create a structure (e.g., list of objects/dictionaries) to hold data for each node on "File Island":
        *   `name`: (e.g., "Tropical Jungle")
        *   `asset_folder_name`: (e.g., "0_tropical_jungle")
        *   `unlocked_status`: (boolean)
        *   `node_marker_image_locked`: Path to locked marker asset.
        *   `node_marker_image_unlocked`: Path to unlocked marker asset.
        *   `position_on_map`: (x, y coordinates for the marker on the continent map).
        *   `final_enemy_sprite_path`: Path to the idling sprite for the Event/Battle View.
        *   `step_goal`: Total steps for the node.

2.  **Update Continent Map UI Logic (Node View):**
    *   Modify existing code to iterate through the node data structure.
    *   Display appropriate node markers based on `unlocked_status` at `position_on_map`.
    *   Implement cycling/highlighting of nodes.
    *   Display the highlighted node's `name` as scrolling text.

3.  **Asset Preparation:**
    *   Ensure folders for all "File Island" nodes exist in `assets/backgrounds/environments/file_island/` (e.g., `1_lake`, `2_gear_savannah`, etc.), even with placeholder layer images initially.
    *   Prepare or identify placeholder/actual node marker assets (locked/unlocked).

4.  **State Management:**
    *   Implement logic to track and update `unlocked_status` for each node. This status should persist (though in-memory is fine for initial development).

## 7. Future Considerations / Next Steps After Node Selection

*   Implementing the Event/Battle View menu for a selected node.
*   Loading and displaying the correct multi-layered parallaxing background for the selected node in the Event/Battle View (visible behind the semi-transparent menu).
*   Displaying the correct enemy idling sprite and step goal in the Event/Battle View.
*   Logic for event/battle selection and progression within a node.
*   Actual "adventure mode state" gameplay for each node.

This document aims to capture the current understanding and immediate goals for the map system.
