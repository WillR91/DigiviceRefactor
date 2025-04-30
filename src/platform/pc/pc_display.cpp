// File: src/platform/pc/pc_display.cpp

#include "platform/pc/pc_display.h" // Include own header
#include <SDL_log.h>                // <<< CORRECTED SDL Include >>>
#include <stdexcept>                // Standard

// ... (rest of pc_display.cpp implementation remains the same as provided before, including the new drawTexture method) ...

PCDisplay::PCDisplay() : window_(nullptr), renderer_(nullptr), initialized_(false) {}

PCDisplay::~PCDisplay() {
    close(); // Ensure cleanup on destruction
}

bool PCDisplay::init(const char* title, int width, int height) {
    if (initialized_) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay::init called when already initialized.");
        return true;
    }
    // Assuming SDL_Init happened already

    window_ = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (!window_) { /* ... error log ... */ return false; }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_) { /* ... error log ... */ SDL_DestroyWindow(window_); window_ = nullptr; return false; }

    SDL_SetRenderDrawColor(renderer_, 0x00, 0x00, 0x00, 0xFF);
    initialized_ = true;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay initialized window and renderer.");
    return true;
}

void PCDisplay::clear(uint16_t color) {
    if (!initialized_ || !renderer_) return;
    SDL_SetRenderDrawColor(renderer_, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer_);
}

// Deprecated for textures, keep for interface compliance
void PCDisplay::drawPixels(int dstX, int dstY, int width, int height, const uint16_t* srcData, int srcDataW, int srcDataH, int srcX, int srcY) {
     if (!initialized_ || !renderer_ || !srcData) return;
     SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "PCDisplay::drawPixels called - This method is deprecated for texture rendering.");
     // Add basic implementation if needed for testing specific things, otherwise leave empty/log
}

// --- ADDED Texture Drawing Method ---
void PCDisplay::drawTexture(SDL_Texture* texture, const SDL_Rect* srcRect, const SDL_Rect* dstRect, SDL_RendererFlip flip) {
    if (!initialized_ || !renderer_ || !texture) return;
    SDL_RenderCopyEx(renderer_, texture, srcRect, dstRect, 0.0, NULL, flip);
}
// --- END Added Method ---

void PCDisplay::present() {
    if (!initialized_ || !renderer_) return;
    SDL_RenderPresent(renderer_);
}

void PCDisplay::close() {
    if (!initialized_) return;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Closing PCDisplay...");
    if (renderer_) { SDL_DestroyRenderer(renderer_); renderer_ = nullptr; }
    if (window_) { SDL_DestroyWindow(window_); window_ = nullptr; }
    initialized_ = false;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay closed.");
}

SDL_Renderer* PCDisplay::getRenderer() const {
    return renderer_;
}

bool PCDisplay::isInitialized() const {
    return initialized_;
}

SDL_Window* PCDisplay::getWindow() const {
    return window_;
}

SDL_Color PCDisplay::convert_rgb565_to_sdl_color(uint16_t color565) {
    SDL_Color color; // Calculation is correct
    color.r = ((color565 >> 11) & 0x1F) * 255 / 31;
    color.g = ((color565 >> 5) & 0x3F) * 255 / 63;
    color.b = (color565 & 0x1F) * 255 / 31;
    color.a = 255;
    return color;
}