#include "graphics/SeamlessBackgroundRenderer.h"
#include "graphics/SeamlessBackgroundRenderer.h"
#include "platform/pc/pc_display.h"
#include <algorithm>
#include <cmath>
#include <iostream>

SeamlessBackgroundRenderer::SeamlessBackgroundRenderer(PCDisplay* display, SDL_Renderer* renderer)
    : display_(display)
    , renderer_(renderer)
    , targetWidth_(0)
    , targetHeight_(0)
    , cacheDirty_(false)
    , stats_{}
{
    if (display_) {
        display_->getWindowSize(targetWidth_, targetHeight_);
    }
}

SeamlessBackgroundRenderer::~SeamlessBackgroundRenderer() {
    clearLayers();
}

void SeamlessBackgroundRenderer::addLayer(SDL_Texture* texture, float scrollSpeed) {
    if (!texture || !renderer_) {
        std::cerr << "SeamlessBackgroundRenderer::addLayer: Invalid texture or renderer" << std::endl;
        return;
    }
    
    CachedLayer layer;
    layer.originalTexture = texture;
    layer.scaledTexture = nullptr;
    layer.scrollPosition = 0.0f;
    layer.scrollSpeed = scrollSpeed;
    layer.needsUpdate = true;
    
    // Get original texture dimensions
    if (SDL_QueryTexture(texture, nullptr, nullptr, &layer.scaledWidth, &layer.scaledHeight) != 0) {
        std::cerr << "SeamlessBackgroundRenderer::addLayer: Failed to query texture: " << SDL_GetError() << std::endl;
        return;
    }
    
    layers_.push_back(layer);
    cacheDirty_ = true;
}

void SeamlessBackgroundRenderer::updateScroll(float deltaTime) {
    bool anyUpdated = false;
    
    for (auto& layer : layers_) {
        float oldPosition = layer.scrollPosition;
        layer.scrollPosition += layer.scrollSpeed * deltaTime;
        
        // Normalize to prevent floating point precision issues
        if (layer.scaledWidth > 0) {
            layer.scrollPosition = fmod(layer.scrollPosition, static_cast<float>(layer.scaledWidth));
            if (layer.scrollPosition < 0) {
                layer.scrollPosition += layer.scaledWidth;
            }
        }
        
        if (oldPosition != layer.scrollPosition) {
            anyUpdated = true;
        }
    }
    
    if (anyUpdated) {
        stats_.lastFrameTime = deltaTime;
    }
}

void SeamlessBackgroundRenderer::render() {
    if (!renderer_ || layers_.empty()) {
        return;
    }
    
    stats_.renderCalls++;
    
    // Update scaled textures if needed
    if (cacheDirty_) {
        updateLayerScaling();
        cacheDirty_ = false;
    }
    
    // Debug: Log layer count occasionally
    static int debugRenderCount = 0;
    debugRenderCount++;
    if (debugRenderCount % 300 == 1) { // Log every 300 frames (5 seconds at 60fps)
        std::cerr << "SeamlessBackgroundRenderer::render: Rendering " << layers_.size() << " layers" << std::endl;
    }
    
    for (size_t i = 0; i < layers_.size(); ++i) {
        const auto& layer = layers_[i];
        if (debugRenderCount % 300 == 1) {
            std::cerr << "  Layer " << i << ": texture=" << (layer.scaledTexture ? "VALID" : "NULL") 
                      << ", dimensions=" << layer.scaledWidth << "x" << layer.scaledHeight 
                      << ", speed=" << layer.scrollSpeed << std::endl;
        }
        renderLayer(layer);
    }
}

void SeamlessBackgroundRenderer::renderLayer(const CachedLayer& layer) {
    if (!layer.scaledTexture || layer.scaledWidth <= 0 || layer.scaledHeight <= 0) {
        std::cerr << "SeamlessBackgroundRenderer::renderLayer: Invalid layer data - texture: " 
                  << (layer.scaledTexture ? "VALID" : "NULL") 
                  << ", dimensions: " << layer.scaledWidth << "x" << layer.scaledHeight << std::endl;
        return;
    }
    
    // Set blend mode for proper layer compositing
    SDL_SetTextureBlendMode(layer.scaledTexture, SDL_BLENDMODE_BLEND);
    
    // Calculate seamless tiling positions
    float normalizedOffset = fmod(layer.scrollPosition, static_cast<float>(layer.scaledWidth));
    if (normalizedOffset < 0) normalizedOffset += layer.scaledWidth;
    
    int offsetX = static_cast<int>(normalizedOffset);
    int tilesNeeded = (targetWidth_ / layer.scaledWidth) + 2; // +2 for seamless coverage
    
    // Center vertically - this ensures backgrounds aren't stuck to the top of the screen
    int drawY = (targetHeight_ - layer.scaledHeight) / 2;
    
    for (int i = -1; i <= tilesNeeded; ++i) {
        int drawX = (i * layer.scaledWidth) - offsetX;
        
        // Only render tiles that are visible
        if (drawX + layer.scaledWidth >= 0 && drawX < targetWidth_) {
            SDL_Rect destRect = {
                drawX,
                drawY,
                layer.scaledWidth,
                layer.scaledHeight
            };
            
            SDL_RenderCopy(renderer_, layer.scaledTexture, nullptr, &destRect);
        }
    }
}

void SeamlessBackgroundRenderer::updateLayerScaling() {
    for (auto& layer : layers_) {
        if (layer.needsUpdate) {
            updateScaledTexture(layer);
            layer.needsUpdate = false;
            stats_.textureUpdates++;
        }
    }
}

void SeamlessBackgroundRenderer::updateScaledTexture(CachedLayer& layer) {
    if (!layer.originalTexture || !renderer_) {
        return;
    }
    
    // Clean up existing scaled texture
    if (layer.scaledTexture) {
        SDL_DestroyTexture(layer.scaledTexture);
        layer.scaledTexture = nullptr;
    }
    
    // Get original dimensions
    int originalWidth, originalHeight;
    if (SDL_QueryTexture(layer.originalTexture, nullptr, nullptr, &originalWidth, &originalHeight) != 0) {
        std::cerr << "SeamlessBackgroundRenderer::updateScaledTexture: Failed to query texture: " << SDL_GetError() << std::endl;
        return;
    }
    
    // Calculate scaling to fit the height properly
    float scaleY = static_cast<float>(targetHeight_) / static_cast<float>(originalHeight);
    float scaleX = scaleY; // Keep aspect ratio
    
    // Calculate scaled dimensions while maintaining aspect ratio
    layer.scaledWidth = static_cast<int>(originalWidth * scaleX);
    layer.scaledHeight = static_cast<int>(originalHeight * scaleY);
    
    // Create scaled texture
    layer.scaledTexture = SDL_CreateTexture(renderer_, 
                                          SDL_PIXELFORMAT_RGBA8888,
                                          SDL_TEXTUREACCESS_TARGET,
                                          layer.scaledWidth,
                                          layer.scaledHeight);
    
    if (!layer.scaledTexture) {
        std::cerr << "SeamlessBackgroundRenderer::updateScaledTexture: Failed to create scaled texture: " << SDL_GetError() << std::endl;
        return;
    }
    
    // Copy original to scaled texture
    SDL_Texture* previousTarget = SDL_GetRenderTarget(renderer_);
    SDL_SetRenderTarget(renderer_, layer.scaledTexture);
    
    SDL_Rect srcRect = { 0, 0, originalWidth, std::min(originalHeight, targetHeight_) };
    SDL_Rect dstRect = { 0, 0, layer.scaledWidth, layer.scaledHeight };
    
    SDL_RenderCopy(renderer_, layer.originalTexture, &srcRect, &dstRect);
    
    SDL_SetRenderTarget(renderer_, previousTarget);
}

void SeamlessBackgroundRenderer::cleanupLayer(CachedLayer& layer) {
    if (layer.scaledTexture) {
        SDL_DestroyTexture(layer.scaledTexture);
        layer.scaledTexture = nullptr;
    }
}

void SeamlessBackgroundRenderer::clearLayers() {
    for (auto& layer : layers_) {
        cleanupLayer(layer);
    }
    layers_.clear();
    cacheDirty_ = false;
}

void SeamlessBackgroundRenderer::setTargetResolution(int width, int height) {
    if (targetWidth_ != width || targetHeight_ != height) {
        targetWidth_ = width;
        targetHeight_ = height;
        invalidateCache();
    }
}

void SeamlessBackgroundRenderer::invalidateCache() {
    for (auto& layer : layers_) {
        layer.needsUpdate = true;
    }
    cacheDirty_ = true;
}

bool SeamlessBackgroundRenderer::removeLayer(size_t index) {
    if (index >= layers_.size()) {
        return false;
    }
    
    cleanupLayer(layers_[index]);
    layers_.erase(layers_.begin() + index);
    return true;
}

bool SeamlessBackgroundRenderer::setLayerScrollSpeed(size_t index, float speed) {
    if (index >= layers_.size()) {
        return false;
    }
    
    layers_[index].scrollSpeed = speed;
    return true;
}

float SeamlessBackgroundRenderer::getLayerScrollSpeed(size_t index) const {
    if (index >= layers_.size()) {
        return 0.0f;
    }
    
    return layers_[index].scrollSpeed;
}

const SeamlessBackgroundRenderer::PerformanceStats& SeamlessBackgroundRenderer::getPerformanceStats() const {
    return stats_;
}

void SeamlessBackgroundRenderer::resetPerformanceStats() {
    stats_ = {};
}
