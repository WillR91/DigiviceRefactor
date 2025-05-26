#pragma once

#include <string>
#include <map>
#include <SDL.h> // <<< CORRECTED SDL Include >>>

// Forward declare SDL_Texture and SDL_Renderer
struct SDL_Texture;
struct SDL_Renderer;

class AssetManager {
public:
    AssetManager() = default;
    ~AssetManager();    bool init(SDL_Renderer* renderer);
    bool loadTexture(const std::string& textureId, const std::string& filePath);
    SDL_Texture* getTexture(const std::string& textureId) const;
    bool hasTexture(const std::string& textureId) const; // Check if texture exists without warnings
    void shutdown();

private:
    SDL_Renderer* renderer_ptr = nullptr;
    std::map<std::string, SDL_Texture*> textures_;

    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;
};