// Full code for src/platform/pc/pc_display.h - CORRECTED INCLUDE PATH

#pragma once

#include "idisplay.h" // <<< CORRECTED: Removed "hal/" prefix
#include <SDL.h>

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


private:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    bool initialized_ = false;
    SDL_Color convert_rgb565_to_sdl_color(uint16_t color565);
};