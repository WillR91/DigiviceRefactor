# Menu Enhancement Plan

## Objectives
1. Remove benign "menu_cursor not found" warnings for carousel menus
2. Prepare for proper cursor implementation in node-based menus
3. Keep the code clean and maintainable

## Implementation Plan

### Phase 1: Clean Up Current MenuState
1. Make cursor texture optional
   - Only load cursor for menu types that need it
   - Add a flag/enum to indicate menu rendering style

2. Remove dead code
   - Delete commented-out cursor rendering code if not needed
   - Clean up any other unused code paths

### Phase 2: Menu Style Differentiation
1. Add menu style enum
   ```cpp
   enum class MenuStyle {
       CAROUSEL,  // One item at a time, centered (current implementation)
       NODE_MAP,  // Multiple selectable points, needs cursor
       LIST       // Multiple items in a list, with selection highlight
   };
   ```

2. Update MenuState constructor
   ```cpp
   MenuState(Game* game, const std::vector<std::string>& options, MenuStyle style = MenuStyle::CAROUSEL);
   ```

3. Modify rendering based on style
   - Carousel: Current implementation (centered single option)
   - Node Map: Load cursor, show all options with cursor
   - List: Show multiple options with highlight on current selection

### Phase 3: Prepare Map Screen Implementation
1. Create map node data structure
   ```cpp
   struct MapNode {
       std::string name;
       SDL_Point position;
       bool unlocked;
       std::vector<size_t> connections; // Indices to connected nodes
   };
   ```

2. Create specialized MapMenuState class
   - Inherit from MenuState
   - Override render() to draw map with nodes
   - Implement cursor movement between nodes

## Future Enhancements
1. Add visual indicators for navigation
2. Implement smooth transitions between menu options
3. Add pagination for long menu lists
