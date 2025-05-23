# Event Selection Menu UI Implementation Plan

## 1. Goal

To implement the user interface for the "Event Selection Menu". This menu appears after a player selects a specific node on a continent map. The UI requires a semi-transparent panel overlaid on the selected node's environment background.

## 2. Key Visual Components

1.  **Node Environment Background:**
    *   The unique multi-layered background associated with the currently selected map node (e.g., "Tropical Jungle's" layers: `layer_0`, `layer_1`, `layer_2`).
    *   These layers should be rendered statically (non-scrolling) as the backdrop for this menu.
    *   Source: Asset paths defined in `MapData` for the current node.

2.  **Semi-Transparent Overlay Panel:**
    *   The blue background image located at `assets/ui/backgrounds/map.png`.
    *   This panel should be rendered on top of the node environment background.
    *   It needs to be semi-transparent (e.g., 50% opacity) to allow the node environment to be partially visible.

3.  **Foreground UI Elements:**
    *   Idling sprite of the node's final enemy.
    *   Text displaying the step goal for the node (e.g., "0/400").
    *   A vertically scrollable list of events/battles available within that node.
    *   Cursor/highlight for the selected event.

## 3. Target Implementation Location

*   **State Class:** `Digivice::MapSystemState` (from `include/States/MapSystemState.h` and `src/States/MapSystemState.cpp`).
*   **Specific View/Rendering Function:** Likely within a method like `render_node_detail(PCDisplay& display)` or a similar function called when `currentView_` is set to the equivalent of "node event selection".

## 4. Technical Implementation Details (SDL)

1.  **Asset Loading:**
    *   The `assets/ui/backgrounds/map.png` texture must be loaded via `AssetManager` with a distinct ID (e.g., `"ui_event_menu_panel"`).
    *   The environment layer textures for the current node (`layer_0.png`, `layer_1.png`, `layer_2.png`) must also be loaded via `AssetManager`.

2.  **Rendering Order in `render_node_detail` (or equivalent):**
    1.  Clear screen (if necessary).
    2.  Render Node Environment Background:
        *   Draw `layer_2` (background).
        *   Draw `layer_1` (middleground).
        *   (The player/character sprite is not typically shown in this menu, so `layer_0` might be drawn here or after the panel depending on desired visual depth with UI elements).
    3.  Render Semi-Transparent Overlay Panel:
        *   Get the `SDL_Renderer*`.
        *   Enable alpha blending: `SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);`
        *   Set texture alpha: `SDL_SetTextureAlphaMod(ui_panel_texture, 128);` (for 50% opacity, adjust as needed).
        *   Render the panel texture: `SDL_RenderCopy(renderer, ui_panel_texture, nullptr, &destination_rect);`
        *   Reset texture alpha: `SDL_SetTextureAlphaMod(ui_panel_texture, 255);`
        *   Optionally reset renderer blend mode if subsequent draws should not be blended by default.
    4.  Render Foreground UI Elements:
        *   Draw enemy sprite.
        *   Draw text for step goal and event names.
        *   Draw menu cursor.

## 5. Data Requirements for `MapSystemState`

*   Access to the current `ContinentData` and `NodeData` to retrieve:
    *   Paths for the node's environment layer textures.
    *   Path for the enemy sprite.
    *   Step goal value.
    *   List of events for the node.

## 6. Next Steps

1.  Verify/add loading of `assets/ui/backgrounds/map.png` in `AssetManager` within `MapSystemState`.
2.  Implement the rendering logic in the appropriate function within `MapSystemState.cpp`, ensuring correct layering and alpha blending for the panel.
3.  Fetch and display other UI elements (enemy sprite, text).
