# Copilot Context Briefing for DigiviceRefactor Project

**Date of Briefing:** May 16, 2025

## Project Overview
The "DigiviceRefactor" project is a C++ application, likely a game or simulator inspired by Digimon. It involves managing game states, rendering 2D graphics, handling animations, and loading assets and configurations. The user has recently reverted to a more stable build to address some persistent issues.

## Technology Stack
*   **Language:** C++ (C++17 standard)
*   **Build System:** CMake (version 3.15+)
*   **Core Libraries:**
    *   SDL2 (for windowing, input, rendering)
    *   SDL2_image (for loading various image formats)
*   **Configuration:** JSON files (e.g., `config/game_config.json`) parsed by a custom `ConfigManager`.
*   **IDE/Environment:** Visual Studio Code on Windows, using PowerShell as the default shell.

## Build Process
*   The project is built using CMake. The `CMakeLists.txt` file is located in the root directory.
*   A `rebuild.ps1` script likely exists for convenience.
*   A VS Code task "Build DigiviceSim" (`cmake --build ${workspaceFolder}/build --config Debug`) is available.
*   The primary output directory appears to be `build/Debug/`.
*   Assets (from `assets/`) and configuration files (from `config/`) are copied to the build output directory as a post-build step defined in `CMakeLists.txt`.

## Core Systems & Code Structure
*   **Entry Point:** `main.cpp`
*   **Main Game Class:** `core/Game.h` and `src/core/Game.cpp` (handles initialization, game loop, and state management).
*   **Asset Management:** `core/AssetManager.h` and `src/core/AssetManager.cpp` (responsible for loading and providing access to textures, sounds, etc.).
    *   Asset paths seem to be a recurring challenge, with the game running from `build/Debug/` and needing to access `assets/` (e.g., using `../../assets/`).
*   **Input Management:** `core/InputManager.h` and `src/core/InputManager.cpp`.
*   **Player Data:** `core/PlayerData.h` and `src/core/PlayerData.cpp` (stores game progress, current partner Digimon, current map node information).
*   **Animation:** `graphics/Animator.h`, `graphics/AnimationData.h`, `core/AnimationManager.h`.
*   **Configuration:** `utils/ConfigManager.h` and `src/utils/ConfigManager.cpp` (loads settings from `config/game_config.json`).
*   **State Machine:**
    *   Base class: `states/GameState.h`
    *   Derived states include:
        *   `AdventureState` (`states/AdventureState.h`, `src/states/AdventureState.cpp`): Core gameplay state, handles character movement, background rendering, and potentially battle encounters.
        *   `MenuState` (`states/MenuState.h`, `src/states/MenuState.cpp`): Main menu.
        *   `MapSystemState` (`states/MapSystemState.h`, `src/states/MapSystemState.cpp`): Handles map navigation, continent and node selection.
        *   `PartnerSelectState`: For choosing a Digimon partner.
        *   `BattleState`: For combat.
        *   `TransitionState`: For visual transitions between states.
*   **Map and Environment Data:**
    *   `include/Core/MapData.h`: Defines crucial data structures:
        *   `BackgroundLayerData`: Paths to texture variants for parallax scrolling layers, parallax factors.
        *   `NodeData`: Information about specific locations/nodes on the map (ID, name, sprites, steps, background layer configurations).
        *   `ContinentData`: Defines continents, their map images, and associated nodes.
    *   This data is likely loaded from JSON files within the `assets/config/` directory (e.g., `map_data.json`, or individual files per continent/node).


## Potential Areas for the New Chat to Focus On
*   **Confirming the current stable state:** Understand what functionality is working in the reverted build.
*   **Re-evaluating Asset Loading:** Given the previous errors, this is a critical area. Pay close attention to how paths are constructed and resolved by `AssetManager` and `SDL_image` (e.g., `IMG_Load`). The `SDL_GetBasePath()` often points to the executable's directory (`build/Debug/`), so relative paths to `assets/` need to be correct (e.g., `../../assets/`).
*   **Background Layer System:** If the user wants to continue with the parallax background variants, this will need careful implementation and testing, ensuring `NodeData` correctly provides texture paths and `AdventureState` loads and renders them.
*   **Map System (`MapSystemState`):** Loading map data, node details, and associated assets (icons, boss sprites, background layers for the selected node when transitioning to `AdventureState`).
*   **Decoupling of Background Movement and Character Animation:** This was a previously identified issue where background scrolling wasn't synchronized with the player's walk animation in `AdventureState`.

## Key Files for Context
*   `CMakeLists.txt` (Root directory)
*   `main.cpp` (Root directory)
*   `config/game_config.json`
*   `include/Core/MapData.h`
*   `src/core/Game.cpp` & `include/Core/Game.h`
*   `src/core/AssetManager.cpp` & `include/Core/AssetManager.h`
*   `src/states/AdventureState.cpp` & `include/States/AdventureState.h`
*   `src/states/MapSystemState.cpp` & `include/States/MapSystemState.h`
*   Any JSON files in `assets/config/` that define map/node data.

This document should provide a good starting point for a new chat session. Good luck!
