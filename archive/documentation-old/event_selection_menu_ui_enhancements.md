# Event Selection Menu UI Implementation Plan

## 1. Date
May 14, 2025

## 2. Current Objective
Refine the user interface for the "Event Selection Menu." This is the menu that appears after a player selects a specific node (e.g., "Tropical Jungle") on the continent map. It allows the player to choose which event or battle within that node to engage in.

## 3. Key UI Elements & Tasks

### 3.1. Semi-Transparent Background Panel
*   **Asset:** `assets/ui/backgrounds/map.png`
*   **Appearance:** This image should be rendered as a panel covering a significant portion of the screen (potentially fullscreen or a large centered rectangle).
*   **Transparency:** The panel must be semi-transparent (e.g., 50% opacity) so that the underlying node-specific environment background (e.g., parallaxing layers of "Tropical Jungle") is still visible through it.
*   **Implementation:**
    *   Ensure the `map.png` asset is loaded via `AssetManager`.
    *   In the rendering logic for the Event Selection Menu state/screen:
        1.  Render the node's environment background layers.
        2.  Render the `map.png` texture on top, using SDL's alpha blending capabilities (`SDL_SetRenderDrawBlendMode`, `SDL_SetTextureAlphaMod`).

### 3.2. Text Rendering Refinements
*   **Problem:** Current text elements (e.g., enemy name, step goal, event list items) are described as a "big jumble."
*   **Initial Action - Font Scaling:** Scale all fonts used specifically for this menu's text elements to `0.6f` of their original size. This is a first step to improve readability and layout.
*   **Future Actions - Placement:** After scaling, assess and adjust the (x, y) positions of text elements for better organization and clarity.
*   **Implementation:**
    *   Identify the code sections responsible for rendering text in this menu.
    *   Locate the font loading/scaling parameters for these text elements.
    *   Modify the scaling factor to `0.6f`.

### 3.3. Other UI Elements (to be integrated with the above)
*   **Enemy Sprite:** An idling sprite of the node's final enemy should be displayed on this menu.
*   **Step Goal:** Text indicating the step requirement for the node (e.g., "0/400") needs to be clearly displayed.
*   **Event List:** A vertically scrollable list of available events/battles within the node.

## 4. Code Investigation & Implementation Steps

1.  **Identify Target State:** Determine the C++ game state class responsible for managing and rendering the Event Selection Menu. This might be an existing state like `MapState` (if it handles multiple map views), `AdventureState` (if this menu is a sub-mode), or a new dedicated state (e.g., `EventMenuState`).
2.  **Asset Loading:** Verify/add `assets/ui/backgrounds/map.png` to `AssetManager`'s loading sequence, noting its texture ID.
3.  **Render Logic Modification (Background):** Update the `render()` method of the target state to include drawing the node environment, then the semi-transparent `map.png`.
4.  **Render Logic Modification (Text):** Locate text rendering calls within the target state and adjust font scaling parameters.

## 5. Expected Outcome
A visually cleaner Event Selection Menu where:
*   The blue panel provides a distinct UI layer while still allowing the thematic environment background to be seen.
*   Text elements are smaller and more manageable, paving the way for better layout and readability.
