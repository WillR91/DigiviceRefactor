#pragma once

#include "ui/UIElement.h"
#include <SDL.h>
#include <vector>
#include <string>
#include <functional>

// Forward declarations
class TextRenderer;

/**
 * @brief A UI component for displaying and navigating through menu options
 * 
 * MenuList provides a clean, reusable way to create menu interfaces with
 * automatic layout, navigation, and selection handling. It replaces the
 * manual positioning approach used in the original MenuState.
 */
class MenuList : public UIElement {
public:
    /**
     * @brief Callback function type for menu item selection
     * @param selectedIndex The index of the selected menu item
     * @param selectedText The text of the selected menu item
     */
    using SelectionCallback = std::function<void(int selectedIndex, const std::string& selectedText)>;

    /**
     * @brief Layout direction for menu items
     */
    enum class Layout {
        Vertical,   ///< Stack items vertically (typical for menus)
        Horizontal  ///< Arrange items horizontally
    };

    /**
     * @brief Text alignment options
     */
    enum class Alignment {
        Left,
        Center,
        Right
    };

    /**
     * @brief Construct a new MenuList
     * @param x X position relative to parent
     * @param y Y position relative to parent
     * @param width Width of the menu area
     * @param height Height of the menu area
     * @param textRenderer Pointer to text renderer for drawing text
     */
    MenuList(int x, int y, int width, int height, TextRenderer* textRenderer);

    virtual ~MenuList() = default;

    // Core UIElement overrides
    void update(float deltaTime) override;
    void render(SDL_Renderer* renderer) override;
    bool handleInput(InputManager& inputManager) override;

    // Menu item management
    void setItems(const std::vector<std::string>& items);
    void addItem(const std::string& item);
    void insertItem(int index, const std::string& item);
    void removeItem(int index);
    void clearItems();
    
    const std::vector<std::string>& getItems() const { return items_; }
    size_t getItemCount() const { return items_.size(); }

    // Selection management
    void setSelectedIndex(int index);
    int getSelectedIndex() const { return selectedIndex_; }
    std::string getSelectedItem() const;
    
    // Navigation
    void selectNext();
    void selectPrevious();
    void selectFirst();
    void selectLast();

    // Callbacks
    void setSelectionCallback(SelectionCallback callback) { selectionCallback_ = callback; }

    // Visual configuration
    void setLayout(Layout layout) { layout_ = layout; }
    void setAlignment(Alignment alignment) { alignment_ = alignment; }
    void setItemSpacing(int spacing) { itemSpacing_ = spacing; }
    void setTextScale(float scale) { textScale_ = scale; }
    void setTextKerning(int kerning) { textKerning_ = kerning; }
    
    // Colors
    void setTextColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
    void setSelectedTextColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
    void setBackgroundColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
    void setSelectedBackgroundColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);

    // Cursor management
    void setCursorTexture(SDL_Texture* texture, int width, int height);
    void setCursorOffset(int x, int y);
    void setShowCursor(bool show) { showCursor_ = show; }

protected:
    // Menu data
    std::vector<std::string> items_;
    int selectedIndex_;
    SelectionCallback selectionCallback_;

    // Rendering components
    TextRenderer* textRenderer_;

    // Layout configuration
    Layout layout_;
    Alignment alignment_;
    int itemSpacing_;
    float textScale_;
    int textKerning_;

    // Colors
    SDL_Color textColor_;
    SDL_Color selectedTextColor_;
    SDL_Color backgroundColor_;
    SDL_Color selectedBackgroundColor_;

    // Cursor
    SDL_Texture* cursorTexture_;
    int cursorWidth_;
    int cursorHeight_;
    int cursorOffsetX_;
    int cursorOffsetY_;
    bool showCursor_;

    // Helper methods
    void calculateItemPositions();
    SDL_Rect getItemBounds(int index) const;
    SDL_Point getItemPosition(int index) const;
    void renderItem(SDL_Renderer* renderer, int index);
    void renderCursor(SDL_Renderer* renderer);
    
    // Layout calculations
    int calculateTotalContentHeight() const;
    int calculateTotalContentWidth() const;
    int getItemHeight() const;

private:
    // Cached layout data
    mutable bool layoutDirty_;
    mutable std::vector<SDL_Rect> itemBounds_;
    
    void updateLayout() const;
};
