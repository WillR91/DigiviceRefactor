// File: src/platform/pc/pc_display.cpp

// --- CORRECTED INCLUDE PATH ---
// Use angle brackets relative to main include directory
#include <Platform/PC/pc_display.h>
// ----------------------------

#include <SDL.h>      // External library - OK
#include <SDL_log.h>  // External library - OK
#include <iostream>   // Standard library - OK
#include <vector>     // Standard library - OK


// Constructor
PCDisplay::PCDisplay() {}

// Destructor
PCDisplay::~PCDisplay() { close(); }

// init Function (implementation remains the same)
bool PCDisplay::init(const char* title, int width, int height) {
    if (initialized_) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay::init called when already initialized."); return true; }
    if (SDL_Init(SDL_INIT_VIDEO) < 0) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL could not initialize! SDL_Error: %s", SDL_GetError()); return false; }
    window_ = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (window_ == nullptr) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window could not be created! SDL_Error: %s", SDL_GetError()); SDL_Quit(); return false; }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"); // Use nearest pixel sampling
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer_ == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Renderer could not be created! SDL Error: %s", SDL_GetError());
        SDL_DestroyWindow(window_); window_ = nullptr; SDL_Quit();
        // Fallback to software renderer?
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Attempting software renderer fallback...");
        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);
         if (renderer_ == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Software renderer could not be created! SDL Error: %s", SDL_GetError());
            SDL_DestroyWindow(window_); window_ = nullptr; SDL_Quit(); return false;
         }
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay initialized successfully.");
    SDL_RendererInfo info;
    if (SDL_GetRendererInfo(renderer_, &info) == 0) { SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Created renderer: %s", info.name ? info.name : "Unknown"); } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Could not get renderer info."); }
    initialized_ = true;
    return true;
}

// convert_rgb565_to_sdl_color Function (implementation remains the same)
SDL_Color PCDisplay::convert_rgb565_to_sdl_color(uint16_t color565) {
    uint8_t r5 = (color565 >> 11) & 0x1F; uint8_t g6 = (color565 >> 5) & 0x3F; uint8_t b5 = color565 & 0x1F;
    uint8_t r8 = (r5 * 255) / 31; uint8_t g8 = (g6 * 255) / 63; uint8_t b8 = (b5 * 255) / 31;
    SDL_Color sdlColor = {r8, g8, b8, 255}; return sdlColor;
}

// clear Function (implementation remains the same)
void PCDisplay::clear(uint16_t color565) {
    if (!initialized_) return; SDL_Color sdlColor = convert_rgb565_to_sdl_color(color565);
    SDL_SetRenderDrawColor(renderer_, sdlColor.r, sdlColor.g, sdlColor.b, sdlColor.a); SDL_RenderClear(renderer_);
}

// drawPixels Implementation (implementation remains the same)
void PCDisplay::drawPixels(int dstX, int dstY,
                             int width, int height, // Size of rect to draw on screen
                             const uint16_t* srcData, int srcDataW, int srcDataH, // Source buffer info
                             int srcX, int srcY) // Top-left corner in source buffer
{
    if (!initialized_ || !srcData || width <= 0 || height <= 0 || srcDataW <= 0 || srcDataH <= 0) {
        return; // Basic validation
    }

    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(
        (void*)srcData,          // Pointer to the start of the full source data
        srcDataW,                // FULL width of the source data
        srcDataH,                // FULL height of the source data
        16,                      // Depth
        srcDataW * 2,            // Pitch (based on full source width)
        SDL_PIXELFORMAT_RGB565 ); // Explicit format

    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "drawPixels: Failed to create surface! SDL Error: %s", SDL_GetError());
        return;
    }

    Uint32 magenta_key_rgb565 = 0xF81F;
    if (SDL_SetColorKey(surface, SDL_TRUE, magenta_key_rgb565) != 0) {
         SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "drawPixels: Failed to set color key! SDL Error: %s", SDL_GetError());
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    SDL_FreeSurface(surface); // Free surface immediately after texture creation
    surface = nullptr;

    if (!texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "drawPixels: Failed to create texture! SDL Error: %s", SDL_GetError());
        return;
    }

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

    SDL_RenderCopy(renderer_, texture, &srcRect, &dstRect);

    SDL_DestroyTexture(texture); // Clean up texture
    texture = nullptr;
}


// present Function (implementation remains the same)
void PCDisplay::present() { if (!initialized_) return; SDL_RenderPresent(renderer_); }

// close Function (implementation remains the same)
void PCDisplay::close() {
    if (!initialized_) return; SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Closing PCDisplay.");
    if (renderer_) { SDL_DestroyRenderer(renderer_); renderer_ = nullptr; }
    if (window_) { SDL_DestroyWindow(window_); window_ = nullptr; } SDL_Quit(); initialized_ = false;
}

// isInitialized Function (implementation remains the same)
bool PCDisplay::isInitialized() const { return initialized_; }

// getWindow Function (implementation remains the same)
SDL_Window* PCDisplay::getWindow() const { return window_; }
