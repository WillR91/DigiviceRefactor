#pragma once

#include "ui/UIElement.h"
#include <SDL.h>

/**
 * @brief A UI component to display a background bar behind menu items
 */
class MenuBar : public UIElement {
public:
    /**
     * @brief Construct a new Menu Bar
     * @param x X position relative to parent
     * @param y Y position relative to parent
     * @param width Width of the bar
     * @param height Height of the bar
     * @param texture The texture to use for the bar
     */
    MenuBar(int x, int y, int width, int height, SDL_Texture* texture);
    virtual ~MenuBar() = default;

    // Core UIElement overrides
    void render(SDL_Renderer* renderer) override;
    
    // Set the position of the bar
    void setPosition(int x, int y);
    
    // Set the size of the bar
    void setSize(int width, int height);
    
    // Set whether the bar should be visible
    void setVisible(bool visible) { visible_ = visible; }

private:
    SDL_Texture* texture_;
};
