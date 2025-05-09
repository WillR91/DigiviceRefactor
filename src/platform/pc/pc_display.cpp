// File: src/platform/pc/pc_display.cpp

#include "platform/pc/pc_display.h" 
#include <SDL_log.h>                
#include <stdexcept>                

PCDisplay::PCDisplay() : window_(nullptr), renderer_(nullptr), initialized_(false) {}

PCDisplay::~PCDisplay() {
    close(); 
}

bool PCDisplay::init(const char* title, int width, int height) {
    if (initialized_) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay::init called when already initialized.");
        return true;
    }

    // SDL_Init should have been called globally by Game::init()
    // if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    //     SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL could not initialize! SDL_Error: %s", SDL_GetError());
    //     return false;
    // }

    window_ = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (!window_) { 
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window could not be created! SDL_Error: %s", SDL_GetError());
        return false; 
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_) { 
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Renderer could not be created! SDL Error: %s", SDL_GetError());
        SDL_DestroyWindow(window_); 
        window_ = nullptr; 
        return false; 
    }

    SDL_SetRenderDrawColor(renderer_, 0x00, 0x00, 0x00, 0xFF); // Default draw color black
    initialized_ = true;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay initialized window and renderer.");
    return true;
}

// Original clear using RGB565 (kept for IDisplay compliance if strictly needed)
void PCDisplay::clear(uint16_t color) {
    if (!initialized_ || !renderer_) return;
    SDL_Color sdlColor = convert_rgb565_to_sdl_color(color);
    SDL_SetRenderDrawColor(renderer_, sdlColor.r, sdlColor.g, sdlColor.b, sdlColor.a);
    SDL_RenderClear(renderer_);
}

// New clear method with RGBA
void PCDisplay::clear(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    if (!initialized_ || !renderer_) return;
    SDL_SetRenderDrawColor(renderer_, r, g, b, a);
    SDL_RenderClear(renderer_);
}

void PCDisplay::drawPixels(int dstX, int dstY, int width, int height, const uint16_t* srcData, int srcDataW, int srcDataH, int srcX, int srcY) {
     if (!initialized_ || !renderer_ || !srcData) return;
     // This method is likely unused if all rendering is texture-based.
     // SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "PCDisplay::drawPixels called - consider for removal if unused.");
}

void PCDisplay::drawTexture(SDL_Texture* texture, const SDL_Rect* srcRect, const SDL_Rect* dstRect, SDL_RendererFlip flip) {
    if (!initialized_ || !renderer_ || !texture) {
        // Optionally log if texture is null, but can be spammy
        // if (!texture) SDL_LogVerbose(SDL_LOG_CATEGORY_RENDER, "PCDisplay::drawTexture called with null texture.");
        return;
    }
    SDL_RenderCopyEx(renderer_, texture, srcRect, dstRect, 0.0, NULL, flip);
}

void PCDisplay::present() {
    if (!initialized_ || !renderer_) return;
    SDL_RenderPresent(renderer_);
}

void PCDisplay::close() {
    if (!initialized_) return; // Already closed or never initialized
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Closing PCDisplay...");
    if (renderer_) { 
        SDL_DestroyRenderer(renderer_); 
        renderer_ = nullptr; 
    }
    if (window_) { 
        SDL_DestroyWindow(window_); 
        window_ = nullptr; 
    }
    initialized_ = false; // Mark as not initialized
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay closed.");
    // SDL_Quit(); // SDL_Quit should be called globally by Game::close()
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

void PCDisplay::getWindowSize(int& width, int& height) const { 
    if (window_) { 
        SDL_GetWindowSize(window_, &width, &height);
    } else {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay::getWindowSize called when window_ is null!");
        // Fallback to constants or default if GameConstants are available and make sense here
        // For now, setting to 0 to indicate an issue or that it's not available
        width = 0; 
        height = 0;
    }
}

SDL_Color PCDisplay::convert_rgb565_to_sdl_color(uint16_t color565) {
    SDL_Color color; 
    color.r = static_cast<Uint8>(((color565 >> 11) & 0x1F) * 255 / 31);
    color.g = static_cast<Uint8>(((color565 >> 5) & 0x3F) * 255 / 63);
    color.b = static_cast<Uint8>((color565 & 0x1F) * 255 / 31);
    color.a = 255; // RGB565 has no alpha, so default to opaque
    return color;
}

// --- ADDED Implementations for Fade Transitions ---
void PCDisplay::setDrawBlendMode(SDL_BlendMode blendMode) {
    if (!initialized_ || !renderer_) return;
    if (SDL_SetRenderDrawBlendMode(renderer_, blendMode) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to set render draw blend mode: %s", SDL_GetError());
    }
}

void PCDisplay::setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    if (!initialized_ || !renderer_) return;
    if (SDL_SetRenderDrawColor(renderer_, r, g, b, a) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to set render draw color: %s", SDL_GetError());
    }
}

void PCDisplay::fillRect(const SDL_Rect* rect) {
    if (!initialized_ || !renderer_) return;
    if (SDL_RenderFillRect(renderer_, rect) != 0) { // 'rect' can be NULL to fill the entire screen
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to fill rect: %s", SDL_GetError());
    }
}
// --- END ADDED Implementations ---