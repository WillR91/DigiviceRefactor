#include "ui/UIElement.h"
#include "core/InputManager.h"
#include <algorithm>

UIElement::UIElement(int x, int y, int width, int height)
    : x_(x), y_(y), width_(width), height_(height) {
}

void UIElement::setPosition(int x, int y) {
    x_ = x;
    y_ = y;
}

void UIElement::setSize(int width, int height) {
    width_ = width;
    height_ = height;
}

SDL_Point UIElement::getAbsolutePosition() const {
    SDL_Point pos = {x_, y_};
    
    if (parent_) {
        SDL_Point parentPos = parent_->getAbsolutePosition();
        pos.x += parentPos.x;
        pos.y += parentPos.y;
    }
    
    return pos;
}

void UIElement::addChild(std::shared_ptr<UIElement> child) {
    if (child) {
        child->setParent(this);
        children_.push_back(child);
    }
}

void UIElement::removeChild(std::shared_ptr<UIElement> child) {
    auto it = std::find(children_.begin(), children_.end(), child);
    if (it != children_.end()) {
        (*it)->setParent(nullptr);
        children_.erase(it);
    }
}

void UIElement::clearChildren() {
    for (auto& child : children_) {
        child->setParent(nullptr);
    }
    children_.clear();
}

void UIElement::renderChildren(SDL_Renderer* renderer) {
    for (auto& child : children_) {
        if (child && child->isVisible()) {
            child->render(renderer);
        }
    }
}

void UIElement::updateChildren(float deltaTime) {
    for (auto& child : children_) {
        if (child) {
            child->update(deltaTime);
        }
    }
}

bool UIElement::handleChildrenInput(InputManager& inputManager) {
    // Process children in reverse order (top-most first for input handling)
    for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        auto& child = *it;
        if (child && child->isVisible() && child->isEnabled()) {
            if (child->handleInput(inputManager)) {
                return true; // Input was consumed by this child
            }
        }
    }
    return false; // No child consumed the input
}
