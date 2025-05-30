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
      displayMode_(DisplayMode::AllItems),
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
      cursorHeight_(16),      cursorOffsetX_(-20),
      cursorOffsetY_(0),
      showCursor_(true),
      menuBarTexture_(nullptr),
      menuBarWidth_(0),
      menuBarHeight_(0),
      useMenuBar_(false),
      animating_(false),
      previousIndex_(0),
      animationProgress_(0.0f),
      animationDuration_(0.2f),  // Default: 200ms animation
      animationDirection_(0),
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
    
    // Update animation if active
    if (animating_) {
        animationProgress_ += deltaTime / animationDuration_;
        
        // Animation complete?
        if (animationProgress_ >= 1.0f) {
            animating_ = false;
            animationProgress_ = 0.0f;
            animationDirection_ = 0;
        }
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

    // Render menu items based on display mode
    if (displayMode_ == DisplayMode::Carousel) {
        // Carousel mode: only render the selected item
        if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(items_.size())) {
            // If animating, render both previous and current items
            if (animating_ && previousIndex_ >= 0 && previousIndex_ < static_cast<int>(items_.size())) {
                renderAnimatedItem(renderer, previousIndex_);
                renderAnimatedItem(renderer, selectedIndex_);
            } else {
                // Not animating, just render the selected item
                renderItem(renderer, selectedIndex_);
            }
        }
    } else {
        // AllItems mode: render all items (original behavior)
        for (size_t i = 0; i < items_.size(); ++i) {
            renderItem(renderer, static_cast<int>(i));
        }
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
    if (index < 0 || index >= static_cast<int>(items_.size()) || index == selectedIndex_) {
        return;
    }
    
    // Start animation if in carousel mode and animation duration > 0
    if (displayMode_ == DisplayMode::Carousel && animationDuration_ > 0.0f) {
        previousIndex_ = selectedIndex_;
        animating_ = true;
        animationProgress_ = 0.0f;
        
        // Determine animation direction (1 = down, -1 = up)
        int newIdx = index;
        int oldIdx = selectedIndex_;
        
        // For wrapping, maintain the natural direction of movement
        // This makes the menu feel like a continuous loop
        
        // If wrapping from bottom to top (pressing down at the end of the list)
        if (newIdx == 0 && oldIdx == static_cast<int>(items_.size()) - 1) {
            animationDirection_ = 1; // Continue moving down, new item comes from top
        }
        // If wrapping from top to bottom (pressing up at the beginning of the list)
        else if (newIdx == static_cast<int>(items_.size()) - 1 && oldIdx == 0) {
            animationDirection_ = -1; // Continue moving up, new item comes from bottom
        }
        else {
            animationDirection_ = (newIdx > oldIdx) ? 1 : -1;
        }
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
    if (items_.empty() || animating_) {
        return;
    }
    
    int nextIndex = (selectedIndex_ + 1) % static_cast<int>(items_.size());
    setSelectedIndex(nextIndex);
}

void MenuList::selectPrevious() {
    if (items_.empty() || animating_) {
        return;
    }
    
    int prevIndex = selectedIndex_ - 1;
    if (prevIndex < 0) {
        prevIndex = static_cast<int>(items_.size()) - 1;
    }
    setSelectedIndex(prevIndex);
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

void MenuList::setMenuBarTexture(SDL_Texture* texture, int width, int height) {
    menuBarTexture_ = texture;
    menuBarWidth_ = width;
    menuBarHeight_ = height;
    useMenuBar_ = (texture != nullptr);
}

void MenuList::renderItem(SDL_Renderer* renderer, int index) {
    if (index < 0 || index >= static_cast<int>(items_.size()) || !textRenderer_) {
        return;
    }

    const std::string& text = items_[index];
    bool isSelected = (index == selectedIndex_);
      // Get item bounds
    SDL_Rect itemRect = getItemBounds(index);
      // Render menu bar texture for selected item if available
    if (isSelected && useMenuBar_ && menuBarTexture_) {
        // Get text dimensions for better scaling
        SDL_Point textDimensions = textRenderer_->getTextDimensions(text, textKerning_);
        int textWidth = static_cast<int>(textDimensions.x * textScale_ * textRenderer_->getGlobalTextScale());
        
        // Make the bar wider and a bit taller than the text
        SDL_Rect barRect;
        barRect.w = std::min(width_, std::max(textWidth + 24, menuBarWidth_));  // Add padding, but use menuBarWidth_ if larger
        barRect.h = std::min(itemRect.h + 4, static_cast<int>(menuBarHeight_ * 0.8f)); // Slightly taller than text
        
        // Center horizontally, align with text vertically
        barRect.x = itemRect.x + (itemRect.w - barRect.w) / 2;
        barRect.y = itemRect.y + (itemRect.h - barRect.h) / 2;
        
        SDL_RenderCopy(renderer, menuBarTexture_, nullptr, &barRect);
        
        // Debug output to verify rendering
        SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Rendering menu bar at (%d, %d) with size %dx%d",
                    barRect.x, barRect.y, barRect.w, barRect.h);
    }
    // Otherwise, render colored background if needed
    else if (isSelected && selectedBackgroundColor_.a > 0) {
        SDL_SetRenderDrawColor(renderer, selectedBackgroundColor_.r, selectedBackgroundColor_.g, 
                              selectedBackgroundColor_.b, selectedBackgroundColor_.a);
        SDL_RenderFillRect(renderer, &itemRect);
    } else if (!isSelected && backgroundColor_.a > 0) {
        SDL_SetRenderDrawColor(renderer, backgroundColor_.r, backgroundColor_.g, 
                              backgroundColor_.b, backgroundColor_.a);
        SDL_RenderFillRect(renderer, &itemRect);
    }    // Calculate text position based on alignment
    SDL_Point textPos = getItemPosition(index);
    
    // Set the appropriate text color
    SDL_Color color = isSelected ? selectedTextColor_ : textColor_;
    
    // Set the color modulation on the font texture if it's available
    SDL_Texture* fontTexture = textRenderer_->getFontTexture();
    if (fontTexture) {
        SDL_SetTextureColorMod(fontTexture, color.r, color.g, color.b);
    }
    
    // Render text
    textRenderer_->drawText(renderer, text, textPos.x, textPos.y, textScale_, textKerning_);
    
    // Reset color back to default (white) after drawing
    if (fontTexture) {
        SDL_SetTextureColorMod(fontTexture, 255, 255, 255);
    }
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

void MenuList::renderAnimatedItem(SDL_Renderer* renderer, int index) {
    if (index < 0 || index >= static_cast<int>(items_.size()) || !textRenderer_) {
        return;
    }

    const std::string& text = items_[index];
    bool isSelected = (index == selectedIndex_);
    
    // Get animated position for this item
    SDL_Point textPos = getAnimatedItemPosition(index, animationProgress_);
      // Render menu bar texture for selected item if available
    if (isSelected && useMenuBar_ && menuBarTexture_) {
        // Get text dimensions for better scaling
        SDL_Point textDimensions = textRenderer_->getTextDimensions(text, textKerning_);
        int textWidth = static_cast<int>(textDimensions.x * textScale_ * textRenderer_->getGlobalTextScale());
        int textHeight = getItemHeight();
        
        // Make the bar wider and a bit taller than the text
        SDL_Rect barRect;
        barRect.w = std::min(width_, std::max(textWidth + 24, menuBarWidth_));  // Add padding, but use menuBarWidth_ if larger
        barRect.h = std::min(textHeight + 4, static_cast<int>(menuBarHeight_ * 0.8f)); // Slightly taller than text
        
        // Center the bar on the animated text position
        barRect.x = textPos.x - (barRect.w - textWidth) / 2;
        barRect.y = textPos.y - (barRect.h - textHeight) / 2;
        
        SDL_RenderCopy(renderer, menuBarTexture_, nullptr, &barRect);
        
        // Debug output to verify rendering during animation
        SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Rendering animated menu bar at (%d, %d) with size %dx%d",
                    barRect.x, barRect.y, barRect.w, barRect.h);
    }
      // Use the appropriate color based on selection state
    SDL_Color textColor = isSelected ? selectedTextColor_ : textColor_;
    
    // Set the color modulation on the font texture if it's available
    SDL_Texture* fontTexture = textRenderer_->getFontTexture();
    if (fontTexture) {
        SDL_SetTextureColorMod(fontTexture, textColor.r, textColor.g, textColor.b);
    }
      
    // Render text at the animated position
    textRenderer_->drawText(renderer, text, textPos.x, textPos.y, textScale_, textKerning_);
    
    // Reset color back to default (white) after drawing
    if (fontTexture) {
        SDL_SetTextureColorMod(fontTexture, 255, 255, 255);
    }
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
    }    SDL_Rect itemRect = getItemBounds(index);
    SDL_Point textDimensions = textRenderer_->getTextDimensions(items_[index], textKerning_);
    
    // Account for both local scale and global text scale for proper centering (like original MenuState)
    float globalTextScale = textRenderer_->getGlobalTextScale();
    float finalScale = textScale_ * globalTextScale;
    int scaledTextWidth = static_cast<int>(static_cast<float>(textDimensions.x) * finalScale);
    int scaledTextHeight = static_cast<int>(static_cast<float>(textDimensions.y) * finalScale);
    
    int textX = itemRect.x;
    int textY = itemRect.y + (itemRect.h - scaledTextHeight) / 2; // Use scaled height for vertical centering
    
    // Apply alignment with scaled width
    switch (alignment_) {
        case Alignment::Center:
            textX = itemRect.x + (itemRect.w - scaledTextWidth) / 2; // Use scaled width for horizontal centering
            break;
        case Alignment::Right:
            textX = itemRect.x + itemRect.w - scaledTextWidth; // Use scaled width for right alignment
            break;
        case Alignment::Left:
        default:
            // Already set to left
            break;
    }
    
    return {textX, textY};
}

SDL_Point MenuList::getAnimatedItemPosition(int index, float progress) const {
    if (!textRenderer_ || index < 0 || index >= static_cast<int>(items_.size())) {
        return {0, 0};
    }

    // Get the base position without animation
    SDL_Point basePos = getItemPosition(index);
    
    // If not animating, just return the base position
    if (!animating_ || animationDirection_ == 0) {
        return basePos;
    }
      // Calculate screen height and item height for animation
    SDL_Point screenSize = {width_, height_};
    int itemHeight = getItemHeight();
    
    // Calculate vertical offset for animation
    int offset = 0;
    float easedProgress = 0.0f;
    
    // Smoother easing function: cubic ease in/out
    if (progress < 0.5f) {
        easedProgress = 4.0f * progress * progress * progress;
    } else {
        float f = (progress - 1);
        easedProgress = 1.0f + 4.0f * f * f * f;
    }
      // Calculate offset based on direction and progress
    if (index == selectedIndex_) {
        // Current item is sliding in
        if (animationDirection_ > 0) {
            // Sliding in from top - start completely off-screen
            offset = -static_cast<int>((1.0f - easedProgress) * height_);
        } else {
            // Sliding in from bottom - start completely off-screen
            offset = static_cast<int>((1.0f - easedProgress) * height_);
        }
    } else if (index == previousIndex_) {
        // Previous item is sliding out
        if (animationDirection_ > 0) {
            // Sliding out to bottom - go completely off-screen
            offset = static_cast<int>(easedProgress * height_);
        } else {
            // Sliding out to top - go completely off-screen
            offset = -static_cast<int>(easedProgress * height_);
        }
    }
    
    // Apply offset to base position
    SDL_Point animatedPos = {basePos.x, basePos.y + offset};
    return animatedPos;
}

void MenuList::updateLayout() const {
    itemBounds_.clear();
    itemBounds_.reserve(items_.size());
    
    if (items_.empty()) {
        layoutDirty_ = false;
        return;
    }

    int itemHeight = getItemHeight();
    
    if (displayMode_ == DisplayMode::Carousel) {
        // Carousel mode: center only the selected item in the entire area
        for (size_t i = 0; i < items_.size(); ++i) {
            SDL_Rect itemRect;
            itemRect.x = 0;
            itemRect.y = (height_ - itemHeight) / 2;  // Center vertically in entire area
            itemRect.w = width_;
            itemRect.h = itemHeight;
            
            itemBounds_.push_back(itemRect);
        }
    } else if (layout_ == Layout::Vertical) {
        // Vertical layout for AllItems mode
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
        // Horizontal layout for AllItems mode
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
    
    // Apply the same scale calculation as in getItemPosition for consistency
    float globalTextScale = textRenderer_->getGlobalTextScale();
    float finalScale = textScale_ * globalTextScale;
    int scaledMaxHeight = static_cast<int>(static_cast<float>(maxHeight) * finalScale);
    
    return scaledMaxHeight + 4; // Add some padding
}
