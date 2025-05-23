# Menu System Implementation Details

## Phase 1: Initial Fixes

### Step 1: Update MenuState.h
```cpp
// File: include/states/MenuState.h
#pragma once

#include "states/GameState.h"
#include "ui/TextRenderer.h"
#include <vector>
#include <string>
#include <SDL.h>

// Forward declarations
class Game;
class InputManager;
class PlayerData;
class PCDisplay;

// Define menu styles
enum class MenuStyle {
    CAROUSEL,  // One item at a time, centered (current implementation)
    NODE_MAP,  // Multiple selectable points, needs cursor
    LIST       // Multiple items in a list, with selection highlight
};

class MenuState : public GameState {
public:
    MenuState(Game* game, const std::vector<std::string>& options, MenuStyle style = MenuStyle::CAROUSEL);
    ~MenuState() override;

    // Core state functions
    void handle_input(InputManager& inputManager, PlayerData* playerData) override;
    void update(float delta_time, PlayerData* playerData) override;
    void render(PCDisplay& display) override;
    StateType getType() const override;

protected:
    // Rendering helpers for different menu styles
    void renderCarouselMenu(PCDisplay& display);
    void renderNodeMapMenu(PCDisplay& display);
    void renderListMenu(PCDisplay& display);

private:
    // Menu data
    std::vector<std::string> menuOptions_;
    size_t currentSelection_;
    
    // Textures
    SDL_Texture* backgroundTexture_;
    SDL_Texture* cursorTexture_;
    
    // Menu configuration
    MenuStyle menuStyle_;
    
    // Constants
    const int MENU_START_X = 50;
    const int MENU_START_Y = 100;
    const int MENU_ITEM_HEIGHT = 30;
    const int CURSOR_WIDTH = 20;
    const int CURSOR_HEIGHT = 20;
    const int CURSOR_OFFSET_X = 10;
};
```

### Step 2: Update MenuState.cpp Constructor
```cpp
MenuState::MenuState(Game* game, const std::vector<std::string>& options, MenuStyle style) :
    GameState(game),
    menuOptions_(options),
    currentSelection_(0),
    backgroundTexture_(nullptr),
    cursorTexture_(nullptr),
    menuStyle_(style)
{
    if (!game_ptr || !game_ptr->getAssetManager()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MenuState requires valid Game pointer with initialized AssetManager!");
        return;
    }
    
    AssetManager* assets = game_ptr->getAssetManager();
    backgroundTexture_ = assets->getTexture("menu_bg_blue");
    
    if (!backgroundTexture_) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "MenuState Warning: Background texture 'menu_bg_blue' not found.");
    }
    
    // Only load cursor texture for menu styles that need it
    if (menuStyle_ == MenuStyle::NODE_MAP || menuStyle_ == MenuStyle::LIST) {
        cursorTexture_ = assets->getTexture("menu_cursor");
        if (!cursorTexture_) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "MenuState Warning: Cursor texture 'menu_cursor' not found for menu style requiring cursor.");
        }
    }
    
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState Created with %zu options, style: %d", 
                options.size(), static_cast<int>(menuStyle_));
}
```

### Step 3: Update MenuState.cpp Rendering
```cpp
void MenuState::render(PCDisplay& display) {
    // Draw background first
    SDL_Renderer* renderer = display.getRenderer();
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "MenuState Render Error: Renderer is null!");
        return;
    }
    
    // Draw background
    if (backgroundTexture_) {
        SDL_RenderCopy(renderer, backgroundTexture_, NULL, NULL);
    }
    else {
        SDL_SetRenderDrawColor(renderer, 0, 0, 80, 255); // Default dark blue
        SDL_RenderClear(renderer);
    }
    
    // Delegate to appropriate rendering method based on style
    switch (menuStyle_) {
        case MenuStyle::CAROUSEL:
            renderCarouselMenu(display);
            break;
        case MenuStyle::NODE_MAP:
            renderNodeMapMenu(display);
            break;
        case MenuStyle::LIST:
            renderListMenu(display);
            break;
        default:
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Unknown menu style: %d", static_cast<int>(menuStyle_));
            renderCarouselMenu(display); // Fallback to carousel
            break;
    }
}

// Implement the rendering methods for each style
void MenuState::renderCarouselMenu(PCDisplay& display) {
    // Current implementation of showing centered selected option
    // [Implementation code here]
}

void MenuState::renderNodeMapMenu(PCDisplay& display) {
    // Future implementation for map nodes with cursor
    SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "NodeMap menu style not fully implemented yet");
}

void MenuState::renderListMenu(PCDisplay& display) {
    // Future implementation for vertical list with highlight
    SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "List menu style not fully implemented yet");
}
```

## Phase 2: Future Specialized Menu Classes

For more complex menu types like the map, we'll eventually want dedicated classes:

```cpp
// MapMenuState.h
class MapMenuState : public MenuState {
public:
    MapMenuState(Game* game, const std::vector<MapNode>& nodes);
    
    void render(PCDisplay& display) override;
    void handle_input(InputManager& inputManager, PlayerData* playerData) override;
    
private:
    std::vector<MapNode> mapNodes_;
    // Map-specific members
};
```
