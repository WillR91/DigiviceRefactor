# Menu Cursor Implementation Plan

## Issue Summary
During runtime, the application logs the following warning:
```
WARN: Texture 'menu_cursor' not found in AssetManager.
WARN: MenuState Warning: Cursor texture 'menu_cursor' not found.
```

This indicates that the MenuState is attempting to use a texture called 'menu_cursor' that has not been loaded into the AssetManager.

## Implementation Steps

### 1. Asset Creation/Acquisition
- Create a simple cursor image (e.g., arrow or selection indicator)
- Format should match other UI elements (likely PNG format with transparency)
- Size should be appropriate for menu selection (e.g., 16x16 or 32x32 pixels)
- Color scheme should match the UI theme

### 2. Asset Integration
- Place the new cursor image in the appropriate assets directory (likely `assets/ui/`)
- Follow naming convention: `menu_cursor.png`

### 3. Asset Loading
- Ensure the texture is loaded during initialization in the AssetManager
- Add the appropriate load call in the initialization sequence
- Verify the asset path is correct

### 4. Testing
- Run the application and verify that the warning no longer appears
- Check that the cursor appears correctly in the menu
- Verify cursor behavior when navigating menu items

## Code Changes Required

### AssetManager Initialization
Look for where other UI assets are loaded and add:
```cpp
// Load menu cursor texture
if (!loadTexture("menu_cursor", "assets/ui/menu_cursor.png")) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Failed to load menu cursor texture");
}
```

### Verification
Check the MenuState implementation to ensure it handles the cursor texture correctly, including fallback behavior if the texture is missing.

## Expected Outcome
- No more "menu_cursor not found" warnings in the console
- Visual cursor element appears in menus
- Menu navigation is more intuitive with visual feedback
