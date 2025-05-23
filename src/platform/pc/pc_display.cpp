// File: src/platform/pc/pc_display.cpp

#include "platform/pc/pc_display.h" 
#include <SDL_log.h>                
#include <stdexcept>                
#include "utils/GameConstants.h"    // Include for SPRITE_SCALE_FACTOR
#include "graphics/GraphicsConstants.h" // Include for asset scaling

// Native resolution constants
const int NATIVE_WIDTH = 466;  // Default width matching display config
const int NATIVE_HEIGHT = 466; // Default height matching display config

PCDisplay::PCDisplay() : 
    window_(nullptr), 
    renderer_(nullptr), 
    renderTarget_(nullptr),
    initialized_(false),
    width_(0),
    height_(0),
    nativeWidth_(NATIVE_WIDTH),
    nativeHeight_(NATIVE_HEIGHT) {}

PCDisplay::~PCDisplay() {
    close(); 
}

bool PCDisplay::init(const std::string& title, int width, int height, bool fullscreen) {
    if (initialized_) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay::init called when already initialized.");
        return true;
    }

    // SDL_Init should have been called globally by Game::init()
    // if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    //     SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL could not initialize! SDL_Error: %s", SDL_GetError());
    //     return false;
    // }

    width_ = width;
    height_ = height;

    window_ = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (!window_) { 
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window could not be created! SDL_Error: %s", SDL_GetError());
        return false; 
    }

    // Set up renderer flags based on vsync preference
    Uint32 rendererFlags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;
    if (fullscreen) {
        // Uncomment the following line to enable fullscreen
        // SDL_SetWindowFullscreen(window_, SDL_WINDOW_FULLSCREEN_DESKTOP);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Fullscreen enabled");
    } else {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Fullscreen disabled");
    }    renderer_ = SDL_CreateRenderer(window_, -1, rendererFlags);
    if (!renderer_) { 
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Renderer could not be created! SDL Error: %s", SDL_GetError());
        SDL_DestroyWindow(window_); 
        window_ = nullptr; 
        return false; 
    }

    // Comment out render target for now
    /*
    renderTarget_ = SDL_CreateTexture(
        renderer_,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        NATIVE_WIDTH,
        NATIVE_HEIGHT
    );
    
    if (!renderTarget_) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create render target: %s", SDL_GetError());
        return false;
    }
    
    SDL_SetTextureBlendMode(renderTarget_, SDL_BLENDMODE_BLEND);
    */
    
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

// Update the clear method call in AdventureState
void PCDisplay::clear() {
    clear(0, 0, 0, 255); // Default black clear
}

void PCDisplay::drawPixels(int dstX, int dstY, int width, int height, const uint16_t* srcData, int srcDataW, int srcDataH, int srcX, int srcY) {
     if (!initialized_ || !renderer_ || !srcData) return;
     // This method is likely unused if all rendering is texture-based.
     // SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "PCDisplay::drawPixels called - consider for removal if unused.");
}

void PCDisplay::drawTexture(SDL_Texture* texture, const SDL_Rect* srcRect, const SDL_Rect* destRect) {
    if (!texture || !renderer_) return;
    
    if (destRect) {
        // Use GraphicsConstants for scaling
        float scale = Digivice::GraphicsConstants::getAssetScale();
        SDL_Rect scaledDest = {
            destRect->x,
            destRect->y,
            static_cast<int>(destRect->w * scale),
            static_cast<int>(destRect->h * scale)
        };
        SDL_RenderCopy(renderer_, texture, srcRect, &scaledDest);
    } else {
        SDL_RenderCopy(renderer_, texture, srcRect, destRect);
    }
}

void PCDisplay::present() {
    // Simplified presentation - remove render target complexity for now
    SDL_RenderPresent(renderer_);
}

void PCDisplay::close() {
    /*
    if (renderTarget_) {
        SDL_DestroyTexture(renderTarget_);
        renderTarget_ = nullptr;
    }
    */
    
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    
    initialized_ = false;
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

void PCDisplay::setWindowSize(int width, int height) {
    if (window_) {
        SDL_SetWindowSize(window_, width, height);
        // You might want to re-center the window after resizing
        // SDL_SetWindowPosition(window_, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Window size set to %d x %d", width, height);
    }
}

void PCDisplay::setLogicalSize(int width, int height) {
    if (renderer_) {
        SDL_RenderSetLogicalSize(renderer_, width, height);
    }
}

void PCDisplay::applyMask(SDL_Texture* maskTexture) {
    if (renderer_ && maskTexture) {
        SDL_SetTextureBlendMode(maskTexture, SDL_BLENDMODE_BLEND); // Or SDL_BLENDMODE_MUL, check which looks best
        SDL_RenderCopy(renderer_, maskTexture, NULL, NULL); // Stretch mask to full render target
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

// Comment out or remove the beginFrame and endFrame methods since renderTarget_ is disabled
/*
void PCDisplay::beginFrame() {
    // Disabled - render target not created
}

void PCDisplay::endFrame() {
    // Disabled - render target not created
}
*/