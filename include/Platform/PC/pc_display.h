// File: include/Platform/PC/pc_display.h

#pragma once

// --- INCLUDE PATH for idisplay.h ---
#include <Platform/idisplay.h>// -----------------------------------

#include <SDL.h> // External library - OK

class PCDisplay : public IDisplay {
public:
    PCDisplay();
    ~PCDisplay() override;

    bool init(const char* title, int width, int height) override;
    void clear(uint16_t color) override;
    // --- MODIFIED drawPixels declaration (Matches interface) ---
    void drawPixels(int dstX, int dstY,
                      int width, int height,
                      const uint16_t* srcData, int srcDataW, int srcDataH,
                      int srcX, int srcY) override;
    // --- End of modification ---
    void present() override;
    void close() override;

    bool isInitialized() const;
    SDL_Window* getWindow() const;
    // Add getRenderer() if needed by other parts
    SDL_Renderer* getRenderer() const;


private:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    bool initialized_ = false;
    SDL_Color convert_rgb565_to_sdl_color(uint16_t color565);
};

// Inline implementation for getRenderer() example
inline SDL_Renderer* PCDisplay::getRenderer() const {
    return renderer_;
}