#pragma once

#include <SDL.h>
#include <memory>
#include <vector>

// Forward declarations
class PCDisplay;
class InputManager;
class PlayerData;

/**
 * @brief Base class for all UI elements in the Digivice interface
 * 
 * Provides a foundation for creating structured UI components that can be
 * composed together to build complex interfaces. Supports layout management,
 * input handling, and hierarchical rendering.
 */
class UIElement {
public:
    /**
     * @brief Construct a new UIElement
     * @param x X position relative to parent (or screen if root)
     * @param y Y position relative to parent (or screen if root)
     * @param width Width of the element
     * @param height Height of the element
     */
    UIElement(int x, int y, int width, int height);
    
    virtual ~UIElement() = default;

    // Core lifecycle methods
    virtual void update(float deltaTime) {}
    virtual void render(SDL_Renderer* renderer) {}
    virtual bool handleInput(InputManager& inputManager) { return false; }

    // Position and size management
    void setPosition(int x, int y);
    void setSize(int width, int height);
    SDL_Point getPosition() const { return {x_, y_}; }
    SDL_Point getSize() const { return {width_, height_}; }
    SDL_Rect getBounds() const { return {x_, y_, width_, height_}; }

    // Absolute position calculation (considering parent hierarchy)
    SDL_Point getAbsolutePosition() const;

    // Visibility and interaction
    void setVisible(bool visible) { visible_ = visible; }
    bool isVisible() const { return visible_; }
    void setEnabled(bool enabled) { enabled_ = enabled; }
    bool isEnabled() const { return enabled_; }

    // Hierarchy management
    void addChild(std::shared_ptr<UIElement> child);
    void removeChild(std::shared_ptr<UIElement> child);
    void clearChildren();
    const std::vector<std::shared_ptr<UIElement>>& getChildren() const { return children_; }

    // Parent management (weak reference to avoid circular dependencies)
    void setParent(UIElement* parent) { parent_ = parent; }
    UIElement* getParent() const { return parent_; }

protected:
    // Position and dimensions
    int x_, y_;
    int width_, height_;
    
    // State
    bool visible_ = true;
    bool enabled_ = true;
    
    // Hierarchy
    UIElement* parent_ = nullptr;
    std::vector<std::shared_ptr<UIElement>> children_;

    // Helper methods for derived classes
    void renderChildren(SDL_Renderer* renderer);
    void updateChildren(float deltaTime);
    bool handleChildrenInput(InputManager& inputManager);
};
