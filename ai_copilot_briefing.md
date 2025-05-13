# AI Copilot Project Briefing: DigiviceRefactor

## 1. Project Overview

*   **Project Name**: DigiviceRefactor
*   **Core Goal**: To faithfully recreate the core mechanics and gameplay experience of the Digivice -25th COLOR EVOLUTION- toy, while incorporating selective enhancements and porting it to PC (and potentially other platforms).
*   **Application Type**: This is a game/simulator application.
*   **Key Inspirations**: Digimon Adventure, Digivice -25th COLOR EVOLUTION- virtual pet.

## 2. Core Mandate for AI Copilot

**IMPORTANT**: This document is a living brief. As we work together, you are tasked with **actively maintaining and updating this document**. This includes:
*   Logging **current development tasks and focus areas**.
*   Noting **key decisions made** during our sessions.
*   Documenting **challenges encountered and their resolutions or current status**.
*   Keeping track of **progress made on features and bug fixes**.
*   Adding new sections or details as the project evolves and more context becomes available.

Your primary role is to assist in the development of DigiviceRefactor. Use this document as your foundational understanding of the project. If information is missing or unclear, please ask for clarification.

## 3. Current Development Status & Focus

*(AI Copilot: Please update this section at the beginning and end of our work sessions, or as major tasks shift.)*

*   **Last Updated**: May 13, 2025
*   **Current High-Level Goal**: Implement the map and area system.
*   **Specific Tasks Currently Underway**:
    *   Defining data structures for Maps, Stages, and Areas.
    *   Planning for initial implementation with all areas unlocked for development.
*   **Upcoming Priorities**:
    *   Implementing the defined data structures in C++.
    *   Creating sample map data.
    *   Integrating map/area selection into the Adventure State.
*   **Key Challenges We Are Facing**:
    *   [e.g., Ensuring faithful recreation of original Digivice timing for events]

## 4. Key Systems & Features (High-Level)

This project involves several interconnected systems. Refer to specific design documents in the `memory-bank` directory for detailed specifications.

*   **Progression System**:
    *   **Map & Area Structure**: 7 maps based on Digimon Adventure, each with stages and areas. Navigation is step-based.
    *   **Step & Event System**: Physical (or simulated) steps unlock events (cutscenes, battles) in areas.
    *   **Evolution System**:
        *   Memory Bars: 5 bars filled by winning battles.
        *   Levels: Rookie, Champion, Ultimate, Mega, with varying requirements (battles, map progression, story progression).
        *   Battle Win Progression: Digimon-specific win counters for evolution.
    *   **Progression Pacing**: Difficulty and rewards scale through early, mid, late, and end-game phases.
*   **Battle System**:
    *   Turn-based combat (details to be fleshed out).
    *   Enemy Digimon encounters.
    *   Winning battles is crucial for evolution and unlocking Friend Digimon.
*   **Engagement Mechanics**:
    *   Daily Bonuses
    *   Achievement System (steps, battles, collection, evolution mastery)
    *   Replay Incentives (partner selection, alternate evolutions, challenge modes)
*   **User Interface (UI)**:
    *   Menus for Digimon status, map, items, options.
    *   Visual feedback for progression, battles, and events.
*   **Asset Management**:
    *   Sprites, backgrounds, sound effects, music.

## 5. Technical Stack (To Be Confirmed/Expanded)

*   **Primary Language**: C++ (evident from `main.cpp`, `CMakeLists.txt`)
*   **Build System**: CMake
*   **Graphics Library**: [e.g., SDL2, SFML, custom - *AI Copilot: Please help identify and confirm*]
*   **Platform**: Initially PC (Windows), potential for cross-platform.
*   **Key Dependencies**: [e.g., nlohmann/json for config files - *AI Copilot: Please help identify and list*]

## 6. Development Principles & Guidelines

*   **Faithful Recreation**: Prioritize adherence to the original Digivice mechanics where specified.
*   **Selective Enhancement**: Improvements should be purposeful and enhance the player experience without diluting the core feel.
*   **Modularity**: Design systems to be as independent as possible to facilitate easier development, testing, and modification.
*   **Clarity and Readability**: Code should be well-commented and easy to understand.
*   **Iterative Development**: We will build and refine features incrementally.

## 7. How to Use This Document

*   **AI Copilot**: Before starting a new task or session, review this document, especially Section 3, to understand the current context.
*   **Human User**: Refer to this document to ensure the AI has the correct understanding of the project state. Update it manually if the AI's updates are insufficient or incorrect.

---

*This document was initiated on May 13, 2025.*
