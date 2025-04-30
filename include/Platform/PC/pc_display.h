#pragma once

#include "platform/idisplay.h" // <<< CORRECTED path relative to include dir
#include <SDL.h>               // <<< CORRECTED SDL Include >>>

// Forward declare SDL types used as pointers/references
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture; // For drawTexture method added in Phase 2

class PCDisplay : public IDisplay {
public:
    PCDisplay();
    ~PCDisplay() override;

    bool init(const char* title, int width, int height) override;
    void clear(uint16_t color) override;
    // Keep drawPixels definition for IDisplay interface
    void drawPixels(int dstX, int dstY,
                    int width, int height,
                    const uint16_t* srcData, int srcDataW, int srcDataH,
                    int srcX, int srcY) override;
    void present() override;
    void close() override;

    // Added for AssetManager and texture rendering
    SDL_Renderer* getRenderer() const;
    void drawTexture(SDL_Texture* texture, const SDL_Rect* srcRect, const SDL_Rect* dstRect, SDL_RendererFlip flip = SDL_FLIP_NONE);


    // Optional helpers, keep if used
    bool isInitialized() const;
    SDL_Window* getWindow() const;


private:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    bool initialized_ = false;
    // Keep helper if drawPixels implementation needs it
    SDL_Color convert_rgb565_to_sdl_color(uint16_t color565);
};