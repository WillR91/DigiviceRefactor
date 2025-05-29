#pragma once

#include <SDL.h>
#include <string>
#include <unordered_map>

/**
 * @brief Generates fallback textures for missing assets
 * 
 * This class creates procedural textures to serve as fallbacks when
 * asset files are missing or fail to load. It helps maintain visual
 * consistency and debug-ability in the application.
 */
class FallbackTextureGenerator {
public:
    enum class TextureType {
        MISSING_TEXTURE,    // Pink/magenta checkered pattern
        DIGIMON_PLACEHOLDER, // Generic Digimon silhouette
        UI_PLACEHOLDER,     // Gray rectangle with border
        SPRITE_PLACEHOLDER  // Colored rectangle based on ID
    };

    FallbackTextureGenerator() = default;
    ~FallbackTextureGenerator();

    /**
     * @brief Initialize the generator with a renderer
     * @param renderer SDL renderer to create textures with
     * @return true if initialization succeeded
     */
    bool init(SDL_Renderer* renderer);

    /**
     * @brief Generate a fallback texture
     * @param type Type of fallback texture to generate
     * @param width Width of the texture (default: 64)
     * @param height Height of the texture (default: 64)
     * @param identifier Optional identifier for unique textures
     * @return Generated texture, or nullptr on failure
     */
    SDL_Texture* generateFallbackTexture(TextureType type, int width = 64, int height = 64, 
                                       const std::string& identifier = "");

    /**
     * @brief Generate a fallback texture based on asset ID pattern matching
     * @param assetId The asset ID that failed to load
     * @param width Width of the texture
     * @param height Height of the texture
     * @return Generated texture appropriate for the asset type
     */
    SDL_Texture* generateFallbackForAsset(const std::string& assetId, int width = 64, int height = 64);

    /**
     * @brief Clean up all generated textures
     */
    void cleanup();

private:
    SDL_Renderer* renderer_ = nullptr;
    std::unordered_map<std::string, SDL_Texture*> cachedFallbacks_;

    // Generation methods for different texture types
    SDL_Texture* createMissingTexture(int width, int height);
    SDL_Texture* createDigimonPlaceholder(int width, int height, const std::string& identifier);
    SDL_Texture* createUIPlaceholder(int width, int height);
    SDL_Texture* createSpritePlaceholder(int width, int height, const std::string& identifier);

    // Helper methods
    SDL_Color generateColorFromString(const std::string& str);
    std::string getCacheKey(TextureType type, int width, int height, const std::string& identifier);
    TextureType determineTypeFromAssetId(const std::string& assetId);
};
