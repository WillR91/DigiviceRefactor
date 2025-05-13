# DigiviceRefactor - Map System Prototype: Initial Setup Progress

**Date:** May 13, 2025

## 1. Overview

This document tracks the initial setup and progress for implementing the map system prototype in the DigiviceRefactor project. The goal of this phase was to establish the foundational C++ structures and new game state required for the map system, ensuring they integrate correctly into the existing build process.

## 2. Achievements in this Phase

1.  **Data Structures Defined (`MapData.h`):
    *   Created `z:\DigiviceRefactor\include\Core\MapData.h`.
    *   Defined `struct ContinentData` to hold information about a continent (ID, name, map image path, list of nodes).
    *   Defined `struct NodeData` to hold information for each explorable node (ID, name, continent ID, map position, sprite paths, step requirements, and a list of `BackgroundLayerData` for its adventure mode environment).
    *   Defined `struct BackgroundLayerData` to store texture paths (supporting variants) and parallax factors for individual background layers.

2.  **New Game State Created (`MapSystemState`):
    *   Created header file `z:\DigiviceRefactor\include\States\MapSystemState.h`.
    *   Declared the `MapSystemState` class, inheriting from `GameState`.
    *   Included an `enum class MapView` to manage internal views (Continent Selection, Node Selection, Node Detail).
    *   Declared standard state methods (`enter`, `exit`, `handle_input`, `update`, `render`) and private helper methods for each view.
    *   Created source file `z:\DigiviceRefactor\src\States\MapSystemState.cpp`.
    *   Provided basic implementations for the constructor, destructor, and declared state methods, including stubs for view-specific logic and `load_map_data()`.

3.  **Build System Integration (`CMakeLists.txt`):
    *   Successfully added `src\States\MapSystemState.cpp` to the `add_executable` target in `CMakeLists.txt`.

4.  **Compilation Success:
    *   Iteratively resolved initial compilation errors, including:
        *   Incorrect include paths for `GameState.h` in `MapSystemState.h`.
        *   Missing comma in the `StateType` enum within `GameState.h`.
        *   Incorrect `GameState` constructor call in `MapSystemState` (requiring the `Game*` argument).
        *   Temporarily addressed a missing include for `PCDisplay.h` (to be properly handled when rendering is implemented).
    *   The project now successfully compiles with the new map system files and data structures integrated.

## 3. Current Status

*   The codebase now contains the foundational C++ structures for the map system (`MapData.h`, `MapSystemState.h`, `MapSystemState.cpp`).
*   **Build is Stable:** The project successfully compiles after integrating these basic structures and resolving initial compilation errors (related to includes and constructor calls).
*   The `MapSystemState` exists but is not yet transitioned to by the game logic, nor does it have any visual rendering implemented.
*   The `load_map_data()` function within `MapSystemState.cpp` is currently a stub and does not yet contain specific data for continents or nodes.
*   There are **no runtime changes** to the game's behavior at this point.

## 4. Next Immediate Steps

1.  **Populate `load_map_data()` (Current Focus):**
    *   Add hardcoded data to `MapSystemState.cpp` for the "File Island" continent.
    *   Define the first node ("01_tropical_jungle"), including:
        *   Paths to its adventure mode background layers (Foreground: `layer_0.png`, Midground: `layer_1.png`, Background: `layer_2.png`).
        *   Placeholder parallax factors.
        *   Paths for continent map image, node sprite, and boss sprite (placeholders acceptable).
        *   Placeholder values for node name, map position, and total steps.
2.  **Implement Basic Continent View Rendering:**
    *   Modify `MapSystemState::render_continent_selection()` to load and display the "File Island" map image.
    *   Render the "File Island" name.
3.  **Integrate `MapSystemState` into Game Flow:**
    *   Update the main `Game` class (or state manager) to be able to create and switch to `MapSystemState`.
    *   Add a temporary trigger (e.g., a menu option or key press) to transition into the `MapSystemState`.
