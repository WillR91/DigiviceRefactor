# DigiviceRefactor - Environment Asset Structure & Naming

## 1. Introduction

This document details the agreed-upon file structure and naming conventions for environment background assets used in the DigiviceRefactor project, specifically for the parallax scrolling backgrounds in the "adventure mode" segments of each game node.

A consistent approach is crucial for the data-driven system that will load and manage these assets.

## 2. Base Directory

All environment-specific background assets are located under:
`assets/backgrounds/environments/`

## 3. Continent and Node Structure

Within the `environments` directory, assets are organized hierarchically:

*   **Continent Folders:** Each continent in the game will have its own subfolder.
    *   Example: `assets/backgrounds/environments/file_island/`
    *   Naming: It's recommended to use a descriptive name (e.g., `file_island`, `server_continent`). A numeric prefix (e.g., `c01_file_island`) can also be used for ordering if desired.
    *   Contents: Besides node assets, the continent folder may also contain the main overworld map image for that continent.

*   **Node Folders:** Within each continent folder, each playable node will have its own subfolder.
    *   Example: `assets/backgrounds/environments/file_island/01_native_forest/`
    *   Naming: A numeric prefix indicates the intended order of nodes (e.g., `01_`, `02_`). A descriptive name for the node (e.g., `native_forest`, `tropical_jungle`) should follow the prefix.

## 4. Parallax Layer Assets

Each node folder contains the image assets for its three parallax scrolling layers.

*   **Layer Naming Convention:**
    *   `layer_0*.png`: **Foreground Layer**. This is the layer closest to the camera and will typically have the fastest parallax scroll speed.
    *   `layer_1*.png`: **Midground Layer**.
    *   `layer_2*.png`: **Background Layer**. This is the layer furthest from the camera and will typically have the slowest parallax scroll speed (or be static).

*   **File Format:** PNG is the preferred format, especially for layers requiring transparency (typically the foreground, but potentially others).

## 5. Layer Variants for Increased Variety

To prevent visual repetition during background scrolling, some layers can have multiple visual variants that alternate during gameplay.

*   **Variant Naming:** If a layer has variants, they are named by appending an underscore and a number to the base layer name.
    *   Example: If `layer_0.png` is the base foreground, variants would be named:
        *   `layer_0_1.png`
        *   `layer_0_2.png`
        *   (and so on, if more than two variants exist for that specific layer in that node)
    *   If there's a "base" version and then variants, the base can be `layer_X.png` and variants `layer_X_1.png`, etc. Or all can be numbered like `layer_X_0.png`, `layer_X_1.png`.

*   **Functionality:** The game's rendering system will cycle through these variants for a given layer each time the background loops. For instance, on the first scroll pass, `layer_0.png` might be used; on the second, `layer_0_1.png`; then back to `layer_0.png` on the third, and so on.

## 6. Asset Properties

*   **Dimensions:**
    *   **Height:** All layers (and their variants) for a single node should ideally share the same height, corresponding to the game's viewport or intended scrollable area height.
    *   **Width:** Layers must be wider than the game screen to allow for scrolling. The amount of extra width depends on the parallax speed and desired scroll distance.
*   **Seamless Tiling:** Each layer image (including all its variants) must be designed to tile seamlessly with itself horizontally to create a continuous scrolling effect.
*   **Transparency:** Foreground layers (and any other layers that shouldn't fully obscure those behind them) must be saved with an alpha channel (e.g., as PNGs).

## 7. Data-Driven Loading

This structured approach to asset naming and organization is designed to support a data-driven system. The game will store the paths to these assets (including all variants for each layer) in configuration files or data structures associated with each node. The `AdventureState` (or equivalent) will then dynamically load and render the appropriate backgrounds based on the current node and the defined alternation logic for variants.

*Document Updated: May 13, 2025*
