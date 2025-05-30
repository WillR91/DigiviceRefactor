#include "ui/MenuBar.h"
#include <SDL_log.h>

MenuBar::MenuBar(int x, int y, int width, int height, SDL_Texture* texture)
    : UIElement(x, y, width, height),
      texture_(texture)
{
    if (!texture_) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "MenuBar: Created with null texture!");
    } else {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuBar: Created with size %dx%d", width, height);
    }
}

void MenuBar::render(SDL_Renderer* renderer) {
    if (!renderer || !visible_ || !texture_) {
        return;
    }
    
    // Get absolute position (considering parent's position)
    SDL_Point pos = getAbsolutePosition();
    
    // Create destination rectangle
    SDL_Rect destRect = {
        pos.x,
        pos.y,
        width_,
        height_
    };
    
    // Render the menu bar texture
    SDL_RenderCopy(renderer, texture_, nullptr, &destRect);
    
    // Debug info
    SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "MenuBar: Rendered at (%d,%d) with size %dx%d", 
                pos.x, pos.y, width_, height_);
                
    // Render any children (if needed in the future)
    renderChildren(renderer);
}

void MenuBar::setPosition(int x, int y) {
    x_ = x;
    y_ = y;
}

void MenuBar::setSize(int width, int height) {
    width_ = width;
    height_ = height;
}
