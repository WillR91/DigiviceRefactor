#pragma once

#include "platform/idisplay.h" 
#include <SDL.h>               

// Forward declare SDL types used as pointers/references
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture; 

class PCDisplay : public IDisplay {
public:
    PCDisplay();
    ~PCDisplay() override;

    bool init(const char* title, int width, int height) override;
    void clear(uint16_t color) override; // Can be updated to use SDL_Color for consistency
    void clear(Uint8 r, Uint8 g, Uint8 b, Uint8 a); // Overload for RGBA clear

    void drawPixels(int dstX, int dstY,
                    int width, int height,
                    const uint16_t* srcData, int srcDataW, int srcDataH,
                    int srcX, int srcY) override;
    void present() override;
    void close() override;

    SDL_Renderer* getRenderer() const;
    void drawTexture(SDL_Texture* texture, const SDL_Rect* srcRect, const SDL_Rect* dstRect, SDL_RendererFlip flip = SDL_FLIP_NONE);

    // --- ADDED for Fade Transitions ---
    void setDrawBlendMode(SDL_BlendMode blendMode);
    void setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
    void fillRect(const SDL_Rect* rect); // Fills with current draw color
    // --- END ADDED for Fade Transitions ---

    bool isInitialized() const;
    SDL_Window* getWindow() const;
    void getWindowSize(int& width, int& height) const;
    void setWindowSize(int width, int height); // Added to allow changing window size
    void setLogicalSize(int width, int height); // Added for scaling
    void applyMask(SDL_Texture* maskTexture); // Added for applying a mask

private:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    bool initialized_ = false;
    SDL_Color convert_rgb565_to_sdl_color(uint16_t color565);
};