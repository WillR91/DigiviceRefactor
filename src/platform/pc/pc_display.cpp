// Full code for src/platform/pc/pc_display.cpp - For Scrolling Backgrounds

#include "pc_display.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_log.h>
#include <iostream>
#include <vector> // Keep include just in case


// Constructor
PCDisplay::PCDisplay() {}

// Destructor
PCDisplay::~PCDisplay() { close(); }

// init Function (remains the same as before)
bool PCDisplay::init(const char* title, int width, int height) {
    if (initialized_) { /*...*/ return true; }
    if (SDL_Init(SDL_INIT_VIDEO) < 0) { /*...*/ return false; }
    window_ = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (window_ == nullptr) { /*...*/ return false; }
    // --- Add Hint BEFORE CreateRenderer if testing software rendering ---
    // SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    // ---
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"); // Use nearest pixel sampling
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer_ == nullptr) { /*...*/ return false; }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay initialized successfully.");
    SDL_RendererInfo info;
    if (SDL_GetRendererInfo(renderer_, &info) == 0) { SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Created renderer: %s", info.name ? info.name : "Unknown"); } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Could not get renderer info."); }
    initialized_ = true;
    return true;
}

// convert_rgb565_to_sdl_color Function (remains the same)
SDL_Color PCDisplay::convert_rgb565_to_sdl_color(uint16_t color565) {
    // ... (same code as before) ...
    uint8_t r5 = (color565 >> 11) & 0x1F; uint8_t g6 = (color565 >> 5) & 0x3F; uint8_t b5 = color565 & 0x1F;
    uint8_t r8 = (r5 * 255) / 31; uint8_t g8 = (g6 * 255) / 63; uint8_t b8 = (b5 * 255) / 31;
    SDL_Color sdlColor = {r8, g8, b8, 255}; return sdlColor;
}

// clear Function (remains the same)
void PCDisplay::clear(uint16_t color565) {
    // ... (same code as before) ...
    if (!initialized_) return; SDL_Color sdlColor = convert_rgb565_to_sdl_color(color565);
    SDL_SetRenderDrawColor(renderer_, sdlColor.r, sdlColor.g, sdlColor.b, sdlColor.a); SDL_RenderClear(renderer_);
}

// --- MODIFIED drawPixels Implementation ---
// Now takes source rectangle parameters
void PCDisplay::drawPixels(int dstX, int dstY,
                            int width, int height, // Size of rect to draw on screen
                            const uint16_t* srcData, int srcDataW, int srcDataH, // Source buffer info
                            int srcX, int srcY) // Top-left corner in source buffer
{
    if (!initialized_ || !srcData || width <= 0 || height <= 0 || srcDataW <= 0 || srcDataH <= 0) {
        return; // Basic validation
    }

    // --- NO Byte Swapping ---

    // Create surface from the *entire* source data buffer
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(
        (void*)srcData,           // Pointer to the start of the full source data
        srcDataW,                 // FULL width of the source data
        srcDataH,                 // FULL height of the source data
        16,                       // Depth
        srcDataW * 2,             // Pitch (based on full source width)
        SDL_PIXELFORMAT_RGB565 ); // Explicit format

    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "drawPixels: Failed to create surface! SDL Error: %s", SDL_GetError());
        return;
    }

    // Set the color key (Magenta 0xF81F) for transparency
    Uint32 magenta_key_rgb565 = 0xF81F;
    if (SDL_SetColorKey(surface, SDL_TRUE, magenta_key_rgb565) != 0) {
         SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "drawPixels: Failed to set color key! SDL Error: %s", SDL_GetError());
    }

    // Create texture from the *entire* surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    SDL_FreeSurface(surface);
    surface = nullptr;

    if (!texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "drawPixels: Failed to create texture! SDL Error: %s", SDL_GetError());
        return;
    }

    // --- Define Source and Destination Rectangles ---
    SDL_Rect srcRect; // The rectangle portion FROM the source texture
    srcRect.x = srcX;
    srcRect.y = srcY;
    srcRect.w = width;  // Width of the portion to copy
    srcRect.h = height; // Height of the portion to copy

    SDL_Rect dstRect; // The rectangle portion ON the screen
    dstRect.x = dstX;
    dstRect.y = dstY;
    dstRect.w = width;  // Width on screen
    dstRect.h = height; // Height on screen
    // ---

    // Copy only the specified part of the texture (srcRect) to the destination (dstRect)
    SDL_RenderCopy(renderer_, texture, &srcRect, &dstRect);

    // Clean up texture
    SDL_DestroyTexture(texture);
    texture = nullptr;
}
// --- End of drawPixels function ---


// present Function (remains the same)
void PCDisplay::present() { if (!initialized_) return; SDL_RenderPresent(renderer_); }

// close Function (remains the same)
void PCDisplay::close() {
    // ... (same close code as before) ...
     if (!initialized_) return; SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Closing PCDisplay.");
    if (renderer_) { SDL_DestroyRenderer(renderer_); renderer_ = nullptr; }
    if (window_) { SDL_DestroyWindow(window_); window_ = nullptr; } SDL_Quit(); initialized_ = false;
}

// isInitialized Function (remains the same)
bool PCDisplay::isInitialized() const { return initialized_; }

// getWindow Function (remains the same)
SDL_Window* PCDisplay::getWindow() const { return window_; }