# Project Analysis - Menu System

## Current Menu Implementation

The menu system is currently implemented with the following design:

1. **MenuState Class**:
   - Handles a list of menu options
   - Only displays the currently selected option (carousel style)
   - Navigation moves between options with up/down keys
   - Has support for a cursor texture that's not currently being used

2. **Visual Approach**:
   - Centers the current menu option in the middle of the screen
   - Uses TextRenderer to render the option text
   - Background is a blue texture

3. **Navigation Flow**:
   - Options trigger specific actions when confirmed
   - Some options create sub-menus with their own options
   - Some options transition to other game states (PartnerSelectState, etc.)
   - BACK option typically returns to previous menu state

## Cursor Implementation

The code attempts to load a cursor texture but doesn't actually use it:

```cpp
cursorTexture_ = assets->getTexture("menu_cursor"); // Loading occurs
if (!cursorTexture_) { 
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "MenuState Warning: Cursor texture 'menu_cursor' not found."); 
}

// Later in render() there's commented code that would use the cursor:
// if (cursorTexture_ && !menuOptions_.empty()) {
//     int cursorX = MENU_START_X - CURSOR_WIDTH - CURSOR_OFFSET_X; // Define these constants
//     int cursorY = MENU_START_Y + (currentSelection_ * MENU_ITEM_HEIGHT) + (MENU_ITEM_HEIGHT / 2) - (CURSOR_HEIGHT / 2);
//     SDL_Rect dst = { cursorX, cursorY, CURSOR_WIDTH, CURSOR_HEIGHT };
//     SDL_RenderCopy(renderer, cursorTexture_, NULL, &dst);
// }
```

This suggests the cursor functionality was planned but not fully implemented or was disabled for the carousel style.

## Improvement Opportunities

1. **Remove Unused Cursor Code**:
   - Remove the cursor texture loading for carousel menus
   - Only load and use cursor for node-based menus (Map) when implemented

2. **Menu Class Hierarchy**:
   - Create a base MenuState class with common functionality
   - Extend with CarouselMenuState (current style) and NodeMenuState (for map)

3. **Optional Enhancement**:
   - Add visual indicators for navigation (up/down arrows)
   - Show "x of y" item count to indicate position in the menu options
