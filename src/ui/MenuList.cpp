#include "ui/MenuList.h"
#include "ui/TextRenderer.h"
#include "core/InputManager.h"
#include "core/GameAction.h"
#include <SDL_log.h>
#include <algorithm>

MenuList::MenuList(int x, int y, int width, int height, TextRenderer* textRenderer)
    : UIElement(x, y, width, height),
      selectedIndex_(0),
      textRenderer_(textRenderer),
      layout_(Layout::Vertical),
      alignment_(Alignment::Center),
      itemSpacing_(10),
      textScale_(1.0f),
      textKerning_(0),
      textColor_{255, 255, 255, 255},        // White text
      selectedTextColor_{255, 255, 0, 255},  // Yellow selected text
      backgroundColor_{0, 0, 0, 0},          // Transparent background
      selectedBackgroundColor_{0, 0, 80, 128}, // Semi-transparent blue
      cursorTexture_(nullptr),
      cursorWidth_(16),
      cursorHeight_(16),
      cursorOffsetX_(-20),
      cursorOffsetY_(0),
      showCursor_(true),
      layoutDirty_(true) {
    
    if (!textRenderer_) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "MenuList: TextRenderer is null - text will not be rendered");
    }
}

void MenuList::update(float deltaTime) {
    // Update layout if dirty
    if (layoutDirty_) {
        updateLayout();
    }
    
    // Update children
    updateChildren(deltaTime);
}

void MenuList::render(SDL_Renderer* renderer) {
    if (!renderer || !visible_) {
        return;
    }

    // Ensure layout is up to date
    if (layoutDirty_) {
        updateLayout();
    }

    // Render all menu items
    for (size_t i = 0; i < items_.size(); ++i) {
        renderItem(renderer, static_cast<int>(i));
    }

    // Render cursor if enabled
    if (showCursor_ && cursorTexture_ && selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(items_.size())) {
        renderCursor(renderer);
    }

    // Render children
    renderChildren(renderer);
}

bool MenuList::handleInput(InputManager& inputManager) {
    if (!enabled_ || items_.empty()) {
        return false;
    }

    // Let children handle input first
    if (handleChildrenInput(inputManager)) {
        return true;
    }

    bool inputHandled = false;

    // Navigation
    if (inputManager.isActionJustPressed(GameAction::NAV_UP)) {
        selectPrevious();
        inputHandled = true;
    } else if (inputManager.isActionJustPressed(GameAction::NAV_DOWN)) {
        selectNext();
        inputHandled = true;
    } else if (inputManager.isActionJustPressed(GameAction::NAV_LEFT) && layout_ == Layout::Horizontal) {
        selectPrevious();
        inputHandled = true;
    } else if (inputManager.isActionJustPressed(GameAction::NAV_RIGHT) && layout_ == Layout::Horizontal) {
        selectNext();
        inputHandled = true;
    }

    // Selection
    if (inputManager.isActionJustPressed(GameAction::CONFIRM)) {
        if (selectionCallback_ && selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(items_.size())) {
            selectionCallback_(selectedIndex_, items_[selectedIndex_]);
        }
        inputHandled = true;
    }

    return inputHandled;
}

void MenuList::setItems(const std::vector<std::string>& items) {
    items_ = items;
    layoutDirty_ = true;
    
    // Ensure selected index is valid
    if (selectedIndex_ >= static_cast<int>(items_.size())) {
        selectedIndex_ = std::max(0, static_cast<int>(items_.size()) - 1);
    }
}

void MenuList::addItem(const std::string& item) {
    items_.push_back(item);
    layoutDirty_ = true;
}

void MenuList::insertItem(int index, const std::string& item) {
    if (index < 0 || index > static_cast<int>(items_.size())) {
        return;
    }
    
    items_.insert(items_.begin() + index, item);
    layoutDirty_ = true;
    
    // Adjust selected index if necessary
    if (selectedIndex_ >= index) {
        selectedIndex_++;
    }
}

void MenuList::removeItem(int index) {
    if (index < 0 || index >= static_cast<int>(items_.size())) {
        return;
    }
    
    items_.erase(items_.begin() + index);
    layoutDirty_ = true;
    
    // Adjust selected index
    if (selectedIndex_ >= static_cast<int>(items_.size())) {
        selectedIndex_ = std::max(0, static_cast<int>(items_.size()) - 1);
    } else if (selectedIndex_ > index) {
        selectedIndex_--;
    }
}

void MenuList::clearItems() {
    items_.clear();
    selectedIndex_ = 0;
    layoutDirty_ = true;
}

void MenuList::setSelectedIndex(int index) {
    if (index < 0 || index >= static_cast<int>(items_.size())) {
        return;
    }
    selectedIndex_ = index;
}

std::string MenuList::getSelectedItem() const {
    if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(items_.size())) {
        return items_[selectedIndex_];
    }
    return "";
}

void MenuList::selectNext() {
    if (items_.empty()) {
        return;
    }
    
    selectedIndex_ = (selectedIndex_ + 1) % static_cast<int>(items_.size());
}

void MenuList::selectPrevious() {
    if (items_.empty()) {
        return;
    }
    
    selectedIndex_--;
    if (selectedIndex_ < 0) {
        selectedIndex_ = static_cast<int>(items_.size()) - 1;
    }
}

void MenuList::selectFirst() {
    if (!items_.empty()) {
        selectedIndex_ = 0;
    }
}

void MenuList::selectLast() {
    if (!items_.empty()) {
        selectedIndex_ = static_cast<int>(items_.size()) - 1;
    }
}

void MenuList::setTextColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    textColor_ = {r, g, b, a};
}

void MenuList::setSelectedTextColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    selectedTextColor_ = {r, g, b, a};
}

void MenuList::setBackgroundColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    backgroundColor_ = {r, g, b, a};
}

void MenuList::setSelectedBackgroundColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    selectedBackgroundColor_ = {r, g, b, a};
}

void MenuList::setCursorTexture(SDL_Texture* texture, int width, int height) {
    cursorTexture_ = texture;
    cursorWidth_ = width;
    cursorHeight_ = height;
}

void MenuList::setCursorOffset(int x, int y) {
    cursorOffsetX_ = x;
    cursorOffsetY_ = y;
}

void MenuList::renderItem(SDL_Renderer* renderer, int index) {
    if (index < 0 || index >= static_cast<int>(items_.size()) || !textRenderer_) {
        return;
    }

    const std::string& text = items_[index];
    bool isSelected = (index == selectedIndex_);
    
    // Get item bounds
    SDL_Rect itemRect = getItemBounds(index);
    
    // Render background for selected item
    if (isSelected && selectedBackgroundColor_.a > 0) {
        SDL_SetRenderDrawColor(renderer, selectedBackgroundColor_.r, selectedBackgroundColor_.g, 
                              selectedBackgroundColor_.b, selectedBackgroundColor_.a);
        SDL_RenderFillRect(renderer, &itemRect);
    } else if (!isSelected && backgroundColor_.a > 0) {
        SDL_SetRenderDrawColor(renderer, backgroundColor_.r, backgroundColor_.g, 
                              backgroundColor_.b, backgroundColor_.a);
        SDL_RenderFillRect(renderer, &itemRect);
    }

    // Calculate text position based on alignment
    SDL_Point textPos = getItemPosition(index);
    
    // Render text
    textRenderer_->drawText(renderer, text, textPos.x, textPos.y, textScale_, textKerning_);
}

void MenuList::renderCursor(SDL_Renderer* renderer) {
    if (!cursorTexture_ || selectedIndex_ < 0 || selectedIndex_ >= static_cast<int>(items_.size())) {
        return;
    }

    SDL_Rect itemRect = getItemBounds(selectedIndex_);
    
    SDL_Rect cursorRect;
    cursorRect.x = itemRect.x + cursorOffsetX_;
    cursorRect.y = itemRect.y + cursorOffsetY_ + (itemRect.h - cursorHeight_) / 2;
    cursorRect.w = cursorWidth_;
    cursorRect.h = cursorHeight_;

    SDL_RenderCopy(renderer, cursorTexture_, nullptr, &cursorRect);
}

SDL_Rect MenuList::getItemBounds(int index) const {
    if (layoutDirty_) {
        updateLayout();
    }
    
    if (index < 0 || index >= static_cast<int>(itemBounds_.size())) {
        return {0, 0, 0, 0};
    }
    
    SDL_Point absPos = getAbsolutePosition();
    SDL_Rect bounds = itemBounds_[index];
    bounds.x += absPos.x;
    bounds.y += absPos.y;
    
    return bounds;
}

SDL_Point MenuList::getItemPosition(int index) const {
    if (!textRenderer_ || index < 0 || index >= static_cast<int>(items_.size())) {
        return {0, 0};
    }

    SDL_Rect itemRect = getItemBounds(index);
    SDL_Point textDimensions = textRenderer_->getTextDimensions(items_[index], textKerning_);
    
    int textX = itemRect.x;
    int textY = itemRect.y + (itemRect.h - textDimensions.y) / 2;
    
    // Apply alignment
    switch (alignment_) {
        case Alignment::Center:
            textX = itemRect.x + (itemRect.w - textDimensions.x) / 2;
            break;
        case Alignment::Right:
            textX = itemRect.x + itemRect.w - textDimensions.x;
            break;
        case Alignment::Left:
        default:
            // Already set to left
            break;
    }
    
    return {textX, textY};
}

void MenuList::updateLayout() const {
    itemBounds_.clear();
    itemBounds_.reserve(items_.size());
    
    if (items_.empty()) {
        layoutDirty_ = false;
        return;
    }

    int itemHeight = getItemHeight();
    
    if (layout_ == Layout::Vertical) {
        // Vertical layout
        int totalHeight = calculateTotalContentHeight();
        int startY = (height_ - totalHeight) / 2;  // Center vertically
        
        for (size_t i = 0; i < items_.size(); ++i) {
            SDL_Rect itemRect;
            itemRect.x = 0;
            itemRect.y = startY + static_cast<int>(i) * (itemHeight + itemSpacing_);
            itemRect.w = width_;
            itemRect.h = itemHeight;
            
            itemBounds_.push_back(itemRect);
        }
    } else {
        // Horizontal layout
        int totalWidth = calculateTotalContentWidth();
        int startX = (width_ - totalWidth) / 2;  // Center horizontally
        
        int currentX = startX;
        for (size_t i = 0; i < items_.size(); ++i) {
            if (!textRenderer_) {
                break;
            }
            
            SDL_Point textDim = textRenderer_->getTextDimensions(items_[i], textKerning_);
            int itemWidth = textDim.x;
            
            SDL_Rect itemRect;
            itemRect.x = currentX;
            itemRect.y = (height_ - itemHeight) / 2;  // Center vertically
            itemRect.w = itemWidth;
            itemRect.h = itemHeight;
            
            itemBounds_.push_back(itemRect);
            currentX += itemWidth + itemSpacing_;
        }
    }
    
    layoutDirty_ = false;
}

int MenuList::calculateTotalContentHeight() const {
    if (items_.empty()) {
        return 0;
    }
    
    int itemHeight = getItemHeight();
    return static_cast<int>(items_.size()) * itemHeight + (static_cast<int>(items_.size()) - 1) * itemSpacing_;
}

int MenuList::calculateTotalContentWidth() const {
    if (items_.empty() || !textRenderer_) {
        return 0;
    }
    
    int totalWidth = 0;
    for (size_t i = 0; i < items_.size(); ++i) {
        SDL_Point textDim = textRenderer_->getTextDimensions(items_[i], textKerning_);
        totalWidth += textDim.x;
        
        if (i < items_.size() - 1) {
            totalWidth += itemSpacing_;
        }
    }
    
    return totalWidth;
}

int MenuList::getItemHeight() const {
    if (!textRenderer_ || items_.empty()) {
        return 20; // Default height
    }
    
    // Find the tallest item
    int maxHeight = 0;
    for (const auto& item : items_) {
        SDL_Point textDim = textRenderer_->getTextDimensions(item, textKerning_);
        maxHeight = std::max(maxHeight, textDim.y);
    }
    
    return maxHeight + 4; // Add some padding
}
