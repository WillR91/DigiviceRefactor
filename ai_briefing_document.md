# AI Collaboration Briefing: DigiviceRefactor Project

**Last Updated:** May 13, 2025

**Instruction for AI Assistant:**
*This document serves as the primary context for our collaboration on the DigiviceRefactor project. Please familiarize yourself thoroughly with its contents. As we work together, if we make significant progress, complete major tasks, encounter new challenges, or shift our focus, it is crucial that this document is updated. Please either remind the user to update it or, if appropriate, suggest updates to the 'Current Focus/Tasks', 'Recent Developments/Challenges', and 'Key Milestones Achieved' sections to ensure this briefing remains accurate and relevant.*

## 1. Project Overview

**Project Name:** DigiviceRefactor

**Core Concept:** DigiviceRefactor is a C++ and SDL2-based emulator/simulator that aims to faithfully recreate the experience of the Digimon virtual pet devices, specifically the **Digivice -25th COLOR EVOLUTION-** model, while also incorporating elements from the classic 90s pedometer toys. The project focuses on authentic gameplay mechanics, visual styling, and the Digimon Adventure storyline.

**Primary Goal:** To create a robust and engaging simulation of the Digivice, eventually with an eye towards potential porting to a physical, custom-hardware device.

**Key Technologies:**
*   **Language:** C++ (Standard: C++17)
*   **Graphics/Windowing:** SDL2 (version 2.32.0), SDL2_image (version 2.8.6)
*   **Build System:** CMake
*   **Operating System (Development):** Windows (targeting cross-platform eventually)

## 2. Core Gameplay & Design Pillars

*   **Pedometer Simulation:** Step counting is the primary progression mechanism. On PC, this is simulated via keyboard/mouse input. Steps reduce the distance to the next "Area event."
*   **Map & Area Structure:**
    *   The game follows the Digimon Adventure storyline, structured into 7 main maps.
    *   Each map contains multiple stages, and each stage has 4-13 "Areas."
    *   Areas trigger events (cutscenes or battles) upon arrival.
*   **Evolution System:**
    *   Digimon evolve through levels (Rookie, Champion, Ultimate, Mega).
    *   Evolution progress is tracked by "Memory Bars" filled by winning battles.
    *   Requirements vary by Digimon and evolution level, sometimes tied to map/story progression.
*   **Battle System:** Turn-based combat against enemy Digimon. Winning battles is crucial for evolution and unlocking "Friend Digimon."
*   **State-Based Architecture:** The game uses a state machine to manage different modes like Adventure, Menu, Battle, Partner Select, etc.
*   **Authentic Experience:** A strong emphasis is placed on recreating the look, feel, and mechanics of the original Digivice hardware.

## 3. Project Structure & Key Files/Directories

*   **`CMakeLists.txt`:** Defines the build process, dependencies, and links SDL2/SDL2_image.
*   **`main.cpp`:** Entry point of the application, initializes the `Game` object and game loop.
*   **`src/`:** Contains the core C++ source code, organized into subdirectories like `core/`, `states/`, `graphics/`, `ui/`, etc.
*   **`include/`:** Contains header files, mirroring the `src/` structure.
*   **`assets/`:** Stores all game assets (sprites, backgrounds, UI elements, fonts).
*   **`memory-bank/`:** Contains all design documents, roadmaps, technical assessments, and project context files (like this one, and the ones used to generate it). This is a crucial source of truth for project direction and history.

## 4. Current Development Status (as of May 13, 2025)

*   **Build System:** Stable CMake setup, project compiles successfully.
*   **Core Loop:** Basic game loop, state management (with fade transitions), and input handling are implemented.
*   **Asset Management:** System for loading textures (sprites, backgrounds) is in place.
*   **Graphics & Animation:** Sprite-based character animation and text rendering (using a sprite-sheet font) are functional.
*   **Implemented Game States:**
    *   `AdventureState`
    *   `MenuState`
    *   `PartnerSelectState`
    *   `ProgressState`
    *   `BattleState` (likely in early stages)
    *   `TransitionState`
*   **Key Design Documents Available:**
    *   `core_design_document.md`
    *   `progression_system_design.md`
    *   `project_context.md`
    *   `development_roadmap.md`

## 5. Current Focus / Immediate Goals (from `development_roadmap.md`)

*   **Text Rendering Enhancements:**
    *   Expand font atlas, add alignment options, implement word wrapping.
    *   Optimize text rendering (e.g., batching).
*   **Refactor Error Handling:**
    *   Implement a consistent error reporting system.
    *   Replace boolean returns with more descriptive error codes/types.
    *   Enhance logging.
*   **Configuration System:**
    *   Create a JSON-based configuration system for game settings.
    *   Move hard-coded values to config files.
    *   Implement persistence for user settings.
*   **Memory Management Optimization:**
    *   Implement on-demand asset loading.
    *   Expand texture atlas usage.
    *   Create an asset caching system.

## 6. How the AI Assistant Can Help

*   **C++ Code Generation & Refactoring:** Assisting with implementing new features, classes, and functions in C++ and SDL2, following existing patterns and the design documents.
*   **Problem Solving & Debugging:** Helping to identify and fix bugs, optimize code, and resolve technical challenges.
*   **Implementing Game Mechanics:** Translating design specifications from the `memory-bank/` documents into functional code (e.g., specific evolution requirements, battle mechanics, UI interactions).
*   **CMake & Build System Adjustments:** Modifying `CMakeLists.txt` as new files are added or dependencies change.
*   **Best Practices:** Providing advice on C++ best practices, game development patterns, and SDL2 usage.
*   **Documentation:** Assisting in keeping this document and potentially other technical notes updated.

## 7. Key Milestones Achieved

*(To be filled as the project progresses)*

## 8. Recent Developments / Challenges Encountered

*(To be filled as the project progresses)*

## 9. Long-Term Vision (from `development_roadmap.md`)

*   Cross-platform support (mobile, other OS).
*   Asset pipeline automation.
*   Advanced performance optimization.
*   Feature expansion: Multiplayer, online connectivity, expanded game world.
*   Potential hardware implementation.

---

This document should provide a comprehensive starting point for any AI assistant.
