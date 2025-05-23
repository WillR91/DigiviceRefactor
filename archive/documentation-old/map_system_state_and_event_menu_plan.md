# MapSystemState and Event Selection Menu Implementation Plan

## 1. Overview & Rationale

This document outlines the plan to implement the multi-layered map system, including continent view, node selection, and the detailed "Event Selection Menu." The core of this system will reside in a dedicated `MapSystemState`.

**Rationale for `MapSystemState`:**
- **Centralization:** Consolidates all map-related UI and logic (continent display, node markers, event lists within nodes) into a single, cohesive state.
- **Separation of Concerns:** Keeps `MenuState` for general game menus and `AdventureState` for active exploration/gameplay, preventing these states from becoming overly complex with map-specific UI logic.
- **Scalability:** Provides a clear structure for adding more continents, nodes, or map features in the future.
- **Clarity:** The existing inclusion of `"states/MapSystemState.h"` in `MenuState.cpp` suggests this was an intended architectural direction.

## 2. `MapSystemState` Views and Responsibilities

`MapSystemState` will manage several distinct views or sub-modes:

1.  **`CONTINENT_VIEW`:**
    *   Displays the full image of the current continent.
    *   Allows scrolling between different continents.
    *   Displays the continent name.
    *   Handles input to select a continent and transition to `NODE_SELECTION_VIEW`.

2.  **`NODE_SELECTION_VIEW`:**
    *   Displays the selected continent map as a background.
    *   Overlays node markers (locked/unlocked) at their respective positions.
    *   Allows cycling through available nodes on the continent.
    *   Displays the highlighted node's name.
    *   Handles input to select a node and transition to `EVENT_SELECTION_VIEW`.

3.  **`EVENT_SELECTION_VIEW` (Current Focus for UI Refinement):**
    *   **Objective:** Display a menu for the player to choose an event/battle within the selected node.
    *   **Rendering Order & Visuals:**
        1.  **Node-Specific Parallax Background:** The multi-layered environment for the selected node (e.g., "Tropical Jungle" layers) is rendered first, providing a thematic backdrop. This will require `MapSystemState` to load and manage these layer textures and their scroll offsets (initially can be static, later dynamic).
        2.  **Semi-Transparent UI Panel:** The `assets/ui/backgrounds/map.png` image is rendered over the node background.
            *   **Transparency:** Set to ~50% alpha (e.g., alpha value of 128) to allow the node background to be visible.
            *   **Positioning:** Likely a large, centered rectangle or fullscreen.
        3.  **UI Elements on Panel:**
            *   **Enemy Idling Sprite:** An animated sprite of the node's final/representative enemy.
            *   **Node Name:** Displayed clearly.
            *   **Step Goal Text:** e.g., "Steps: 0/400".
            *   **Event List:** A vertically scrollable list of available events/battles in that node. The currently selected event should be highlighted.
    *   **Text Rendering:** All text elements on this panel (node name, step goal, event list items) will be rendered using the existing `TextRenderer` with a **scale factor of `0.6f`** to ensure they fit well and are legible. Kerning (e.g., -15) should also be applied as used elsewhere.
    *   **Input Handling:**
        *   Navigate up/down the event list.
        *   Confirm selection to start an event (likely transitioning to `BattleState` or another gameplay state).
        *   Cancel to return to `NODE_SELECTION_VIEW`.
    *   **Data Management:** `MapSystemState` will need to access data for the selected node, including:
        *   Paths to its background layer textures.
        *   ID/path for the enemy sprite and its animations.
        *   Step goal value.
        *   A list of events (names, and potentially IDs for triggering them).

## 3. High-Level Structure for `MapSystemState`

**File Locations:**
- Header: `z:\DigiviceRefactor\include\States\MapSystemState.h`
- Source: `z:\DigiviceRefactor\src\States\MapSystemState.cpp`

**Key Components (`MapSystemState.h`):**

```cpp
#pragma once
#include "states/GameState.h"
#include "graphics/Animator.h" // For enemy sprite
#include <vector>
#include <string>
#include <map> // For storing node data or background textures

// Forward declarations
class Game;
class InputManager;
class PlayerData;
class PCDisplay;
struct SDL_Texture;
struct MapNodeData; // Assumed from previous discussions for node properties

enum class MapViewMode {
    CONTINENT_SELECT,
    NODE_SELECT,
    EVENT_SELECT
};

class MapSystemState : public GameState {
public:
    MapSystemState(Game* game);
    ~MapSystemState() override;

    void enter() override;
    void exit() override;
    void handle_input(InputManager& inputManager, PlayerData* playerData) override;
    void update(float delta_time, PlayerData* playerData) override;
    void render(PCDisplay& display) override;
    StateType getType() const override { return StateType::MapSystem; } // Assuming new StateType

private:
    // View Management
    MapViewMode currentView_;
    void transitionToView(MapViewMode newView);

    // Common Data
    std::string currentContinentID_;
    std::string selectedNodeID_;
    // const MapNodeData* activeNodeData_; // Pointer to the full data of the selected node

    // Assets (pointers, managed by AssetManager)
    SDL_Texture* uiPanelTexture_; // For the semi-transparent blue panel
    
    // --- EVENT_SELECTION_VIEW Specific Members ---
    std::vector<SDL_Texture*> eventNodeBackgroundLayers_;
    float eventNodeBackgroundOffsets_[3]; // Example for 3 layers
    Animator eventEnemyAnimator_;
    std::string eventNodeName_;
    std::string eventStepGoalText_;
    std::vector<std::string> eventListOptions_;
    int currentEventSelectionIndex_;
    // SDL_Texture* enemySpriteTexture_; // Managed by Animator

    // --- NODE_SELECTION_VIEW Specific Members ---
    SDL_Texture* continentMapTexture_; // For the main map of the continent
    // std::vector<NodeMarkerUIData> nodeMarkers_; // Data for drawing markers

    // --- CONTINENT_SELECT Specific Members ---
    // std::vector<ContinentData> continents_;
    // int currentContinentIndex_;

    // Helper methods for each view's logic
    void handleInputContinentSelect(InputManager& inputManager, PlayerData* playerData);
    void handleInputNodeSelect(InputManager& inputManager, PlayerData* playerData);
    void handleInputEventSelect(InputManager& inputManager, PlayerData* playerData);

    void updateEventSelect(float delta_time);
    // ... other update helpers

    void renderContinentSelect(PCDisplay& display);
    void renderNodeSelect(PCDisplay& display);
    void renderEventSelect(PCDisplay& display);
    
    void loadDataForEventSelect(const std::string& nodeID);
    void loadDataForNodeSelect(const std::string& continentID);
};
```

## 4. Implementation Steps for Event Selection View UI

1.  **Create/Verify `MapSystemState` Files:** Ensure `MapSystemState.h` and `MapSystemState.cpp` exist. If not, create basic skeletons.
2.  **Asset Loading (`AssetManager`):**
    *   In `AssetManager::init` (or a similar central loading spot), ensure `assets/ui/backgrounds/map.png` is loaded with a texture ID like `"ui_event_menu_panel"`.
3.  **`MapSystemState` Constructor & Basic Setup:**
    *   Initialize `currentView_` (e.g., to `CONTINENT_SELECT` or `NODE_SELECT` initially).
    *   In the constructor, get the `uiPanelTexture_` from `AssetManager`.
4.  **Data Loading for Event View (`loadDataForEventSelect`):**
    *   This method will be called when transitioning to `EVENT_SELECT`.
    *   It will take a `nodeID` (e.g., "tropical_jungle").
    *   Fetch `MapNodeData` for this `nodeID` (this implies `MapNodeData` structures are defined and accessible, as per previous discussions).
    *   From `MapNodeData`:
        *   Load background layer textures (e.g., `assets/backgrounds/environments/file_island/0_tropical_jungle/layer_0.png`, etc.) into `eventNodeBackgroundLayers_`.
        *   Set up `eventEnemyAnimator_` using the enemy sprite ID from `MapNodeData`.
        *   Populate `eventNodeName_`, `eventStepGoalText_` (e.g., "0/400" - progress might come from `PlayerData`), and `eventListOptions_`.
        *   Reset `currentEventSelectionIndex_ = 0`.
5.  **Rendering Logic (`renderEventSelect`):**
    *   Get `SDL_Renderer*` and `TextRenderer*`.
    *   **Render Parallax Background:** Iterate through `eventNodeBackgroundLayers_`, drawing each with its offset (similar to `AdventureState`'s background rendering).
    *   **Render Semi-Transparent Panel:**
        *   Set `SDL_BLENDMODE_BLEND` on the renderer.
        *   Set `SDL_SetTextureAlphaMod(uiPanelTexture_, 128);`
        *   Define destination rectangle for the panel.
        *   `SDL_RenderCopy` the `uiPanelTexture_`.
        *   Reset alpha mod on texture and blend mode on renderer.
    *   **Render UI Elements on Panel:**
        *   Draw `eventEnemyAnimator_.getCurrentTexture()` at a designated spot.
        *   Use `textRenderer->drawText(...)` for `eventNodeName_`, `eventStepGoalText_`, and each item in `eventListOptions_`, passing `0.6f` as the scale argument. Highlight `eventListOptions_[currentEventSelectionIndex_]`.
6.  **Input Handling (`handleInputEventSelect`):**
    *   Process `NAV_UP`, `NAV_DOWN` to change `currentEventSelectionIndex_`.
    *   Process `CONFIRM` to trigger the selected event (e.g., `game_ptr->requestPushState(std::make_unique<BattleState>(...));`).
    *   Process `CANCEL` to call `transitionToView(MapViewMode::NODE_SELECT);`.
7.  **Update Logic (`updateEventSelect`):**
    *   Call `eventEnemyAnimator_.update(delta_time);`.
8.  **State Transitions:**
    *   Modify `MenuState` (or wherever "VIEW MAP" is handled) to transition to `MapSystemState` (e.g., `game_ptr->requestFadeToState(std::make_unique<MapSystemState>(game_ptr), 0.3f, true);`).
    *   Implement `MapSystemState::transitionToView` to handle cleanup/setup when switching between `CONTINENT_SELECT`, `NODE_SELECT`, and `EVENT_SELECT`.

This plan provides a clear path forward. The next step would be to start implementing the `MapSystemState` skeleton and then the `EVENT_SELECTION_VIEW` specifics.
