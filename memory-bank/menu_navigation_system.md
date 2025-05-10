# Menu Navigation System

## Current Implementation

The Digivice simulator uses a carousel-style menu system for most screens:

- Only one menu option is typically visible at a time
- Users scroll up/down to cycle through available options
- The current option is centered on screen
- No cursor is required for this type of menu

## Special Cases

The Map screen is an exception to the carousel approach:
- Shows multiple selectable nodes at once
- Requires a highlight/cursor to indicate the currently selected node
- Navigation is spatial rather than sequential

## Current Issue

The code is attempting to load a `menu_cursor` texture that doesn't exist:
```
WARN: Texture 'menu_cursor' not found in AssetManager.
WARN: MenuState Warning: Cursor texture 'menu_cursor' not found.
```

This is likely because:
1. The MenuState class was designed to handle both types of menus (carousel and node-based)
2. The cursor is only needed for the node-based menu (Map screen)
3. The warning is harmless for carousel-style menus that don't need a cursor

## Potential Solutions

1. **Keep the current warning** - It's harmless and doesn't affect functionality
2. **Add cursor checking** - Only attempt to load the cursor for menu types that need it
3. **Create the missing texture** - Add a cursor texture for completeness
4. **Separate menu implementations** - Create distinct classes for different menu styles
