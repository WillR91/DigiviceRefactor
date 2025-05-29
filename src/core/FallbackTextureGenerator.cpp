#include "core/FallbackTextureGenerator.h"
#include <SDL_log.h>
#include <cstring>
#include <algorithm>
#include <cmath>

FallbackTextureGenerator::~FallbackTextureGenerator() {
    cleanup();
}

bool FallbackTextureGenerator::init(SDL_Renderer* renderer) {
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "FallbackTextureGenerator::init failed: Renderer is null");
        return false;
    }
    
    renderer_ = renderer;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "FallbackTextureGenerator initialized");
    return true;
}

SDL_Texture* FallbackTextureGenerator::generateFallbackTexture(TextureType type, int width, int height, 
                                                             const std::string& identifier) {
    if (!renderer_) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "FallbackTextureGenerator not initialized");
        return nullptr;
    }

    // Check cache first
    std::string cacheKey = getCacheKey(type, width, height, identifier);
    auto it = cachedFallbacks_.find(cacheKey);
    if (it != cachedFallbacks_.end()) {
        return it->second;
    }

    SDL_Texture* texture = nullptr;
    
    switch (type) {
        case TextureType::MISSING_TEXTURE:
            texture = createMissingTexture(width, height);
            break;
        case TextureType::DIGIMON_PLACEHOLDER:
            texture = createDigimonPlaceholder(width, height, identifier);
            break;
        case TextureType::UI_PLACEHOLDER:
            texture = createUIPlaceholder(width, height);
            break;
        case TextureType::SPRITE_PLACEHOLDER:
            texture = createSpritePlaceholder(width, height, identifier);
            break;
    }

    if (texture) {
        cachedFallbacks_[cacheKey] = texture;
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Generated fallback texture: %s", cacheKey.c_str());
    }

    return texture;
}

SDL_Texture* FallbackTextureGenerator::generateFallbackForAsset(const std::string& assetId, int width, int height) {
    TextureType type = determineTypeFromAssetId(assetId);
    return generateFallbackTexture(type, width, height, assetId);
}

void FallbackTextureGenerator::cleanup() {
    for (auto& pair : cachedFallbacks_) {
        SDL_DestroyTexture(pair.second);
    }
    cachedFallbacks_.clear();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "FallbackTextureGenerator cleaned up");
}

SDL_Texture* FallbackTextureGenerator::createMissingTexture(int width, int height) {
    // Create a magenta/black checkered pattern texture
    SDL_Texture* texture = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888, 
                                           SDL_TEXTUREACCESS_TARGET, width, height);
    if (!texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create missing texture: %s", SDL_GetError());
        return nullptr;
    }

    // Set texture as render target
    SDL_Texture* originalTarget = SDL_GetRenderTarget(renderer_);
    SDL_SetRenderTarget(renderer_, texture);

    // Clear with magenta
    SDL_SetRenderDrawColor(renderer_, 255, 0, 255, 255);
    SDL_RenderClear(renderer_);

    // Draw black squares in checkerboard pattern
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    int squareSize = std::max(4, std::min(width, height) / 8);
    
    for (int y = 0; y < height; y += squareSize) {
        for (int x = 0; x < width; x += squareSize) {
            if ((x / squareSize + y / squareSize) % 2 == 1) {
                SDL_Rect rect = {x, y, std::min(squareSize, width - x), std::min(squareSize, height - y)};
                SDL_RenderFillRect(renderer_, &rect);
            }
        }
    }

    // Restore original render target
    SDL_SetRenderTarget(renderer_, originalTarget);
    
    return texture;
}

SDL_Texture* FallbackTextureGenerator::createDigimonPlaceholder(int width, int height, const std::string& identifier) {
    SDL_Texture* texture = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888, 
                                           SDL_TEXTUREACCESS_TARGET, width, height);
    if (!texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create Digimon placeholder: %s", SDL_GetError());
        return nullptr;
    }

    SDL_Texture* originalTarget = SDL_GetRenderTarget(renderer_);
    SDL_SetRenderTarget(renderer_, texture);

    // Generate a color based on identifier
    SDL_Color color = generateColorFromString(identifier);
    
    // Background
    SDL_SetRenderDrawColor(renderer_, color.r / 2, color.g / 2, color.b / 2, 255);
    SDL_RenderClear(renderer_);

    // Draw a simple Digimon silhouette (oval body + smaller head)
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, 255);
    
    // Body (larger oval)
    int bodyW = width * 0.6f;
    int bodyH = height * 0.7f;
    int bodyX = (width - bodyW) / 2;
    int bodyY = height * 0.2f;
    
    SDL_Rect bodyRect = {bodyX, bodyY, bodyW, bodyH};
    SDL_RenderFillRect(renderer_, &bodyRect);
    
    // Head (smaller circle)
    int headSize = width * 0.3f;
    int headX = (width - headSize) / 2;
    int headY = height * 0.05f;
    
    SDL_Rect headRect = {headX, headY, headSize, headSize};
    SDL_RenderFillRect(renderer_, &headRect);

    // Add a "?" symbol in the center
    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
    int symbolSize = std::min(width, height) / 6;
    SDL_Rect symbolRect = {(width - symbolSize) / 2, (height - symbolSize) / 2, symbolSize, symbolSize / 2};
    SDL_RenderFillRect(renderer_, &symbolRect);

    SDL_SetRenderTarget(renderer_, originalTarget);
    return texture;
}

SDL_Texture* FallbackTextureGenerator::createUIPlaceholder(int width, int height) {
    SDL_Texture* texture = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888, 
                                           SDL_TEXTUREACCESS_TARGET, width, height);
    if (!texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create UI placeholder: %s", SDL_GetError());
        return nullptr;
    }

    SDL_Texture* originalTarget = SDL_GetRenderTarget(renderer_);
    SDL_SetRenderTarget(renderer_, texture);

    // Light gray background
    SDL_SetRenderDrawColor(renderer_, 200, 200, 200, 255);
    SDL_RenderClear(renderer_);

    // Dark gray border
    SDL_SetRenderDrawColor(renderer_, 100, 100, 100, 255);
    SDL_Rect borderRect = {0, 0, width, height};
    SDL_RenderDrawRect(renderer_, &borderRect);

    // Inner border
    if (width > 4 && height > 4) {
        SDL_Rect innerBorder = {2, 2, width - 4, height - 4};
        SDL_RenderDrawRect(renderer_, &innerBorder);
    }

    SDL_SetRenderTarget(renderer_, originalTarget);
    return texture;
}

SDL_Texture* FallbackTextureGenerator::createSpritePlaceholder(int width, int height, const std::string& identifier) {
    SDL_Texture* texture = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888, 
                                           SDL_TEXTUREACCESS_TARGET, width, height);
    if (!texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create sprite placeholder: %s", SDL_GetError());
        return nullptr;
    }

    SDL_Texture* originalTarget = SDL_GetRenderTarget(renderer_);
    SDL_SetRenderTarget(renderer_, texture);

    // Generate color based on identifier
    SDL_Color color = generateColorFromString(identifier);
    
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, 255);
    SDL_RenderClear(renderer_);

    // Add a diagonal pattern
    SDL_SetRenderDrawColor(renderer_, 255 - color.r, 255 - color.g, 255 - color.b, 100);
    for (int i = -height; i < width; i += 8) {
        SDL_RenderDrawLine(renderer_, i, 0, i + height, height);
    }

    SDL_SetRenderTarget(renderer_, originalTarget);
    return texture;
}

SDL_Color FallbackTextureGenerator::generateColorFromString(const std::string& str) {
    // Simple hash-based color generation
    size_t hash = 0;
    for (char c : str) {
        hash = hash * 31 + static_cast<size_t>(c);
    }
    
    SDL_Color color;
    color.r = (hash & 0xFF0000) >> 16;
    color.g = (hash & 0x00FF00) >> 8;
    color.b = (hash & 0x0000FF);
    color.a = 255;
    
    // Ensure minimum brightness
    int brightness = color.r + color.g + color.b;
    if (brightness < 200) {
        float factor = 200.0f / brightness;
        color.r = std::min(255, static_cast<int>(color.r * factor));
        color.g = std::min(255, static_cast<int>(color.g * factor));
        color.b = std::min(255, static_cast<int>(color.b * factor));
    }
    
    return color;
}

std::string FallbackTextureGenerator::getCacheKey(TextureType type, int width, int height, 
                                                 const std::string& identifier) {
    return std::to_string(static_cast<int>(type)) + "_" + std::to_string(width) + "x" + 
           std::to_string(height) + "_" + identifier;
}

FallbackTextureGenerator::TextureType FallbackTextureGenerator::determineTypeFromAssetId(const std::string& assetId) {
    std::string lowerAssetId = assetId;
    std::transform(lowerAssetId.begin(), lowerAssetId.end(), lowerAssetId.begin(), ::tolower);
    
    if (lowerAssetId.find("digimon") != std::string::npos) {
        return TextureType::DIGIMON_PLACEHOLDER;
    } else if (lowerAssetId.find("ui") != std::string::npos || 
               lowerAssetId.find("menu") != std::string::npos ||
               lowerAssetId.find("button") != std::string::npos) {
        return TextureType::UI_PLACEHOLDER;
    } else if (lowerAssetId.find("sprite") != std::string::npos) {
        return TextureType::SPRITE_PLACEHOLDER;
    }
    
    return TextureType::MISSING_TEXTURE;
}
