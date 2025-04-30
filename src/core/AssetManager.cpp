// File: src/core/AssetManager.cpp

#include "core/AssetManager.h" // Include own header
#include <SDL_image.h>         // Use angle brackets for library headers
#include <SDL_log.h>           // <<< CORRECTED SDL Include >>>

// ... (rest of AssetManager.cpp implementation remains the same) ...

AssetManager::~AssetManager() {
    shutdown();
}

bool AssetManager::init(SDL_Renderer* renderer) {
    if (!renderer) { /* ... error log ... */ return false; }
    renderer_ptr = renderer;
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) { /* ... error log ... */ renderer_ptr = nullptr; return false; }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetManager initialized successfully (SDL_image loaded).");
    return true;
}

bool AssetManager::loadTexture(const std::string& textureId, const std::string& filePath) {
    if (!renderer_ptr) { /* ... error log ... */ return false; }
    if (textures_.count(textureId)) { /* ... warn log ... */ return true; }
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Loading texture '%s' from '%s'", textureId.c_str(), filePath.c_str());
    SDL_Surface* loadedSurface = IMG_Load(filePath.c_str());
    if (!loadedSurface) { /* ... error log ... */ return false; }
    SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderer_ptr, loadedSurface);
    SDL_FreeSurface(loadedSurface);
    if (!newTexture) { /* ... error log ... */ return false; }
    textures_[textureId] = newTexture;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Successfully loaded texture '%s'.", textureId.c_str());
    return true;
}

SDL_Texture* AssetManager::getTexture(const std::string& textureId) const {
    auto it = textures_.find(textureId);
    return (it != textures_.end()) ? it->second : nullptr;
}

void AssetManager::shutdown() {
    if (renderer_ptr == nullptr && textures_.empty()) return;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Shutting down AssetManager...");
    for (auto const& [id, texture] : textures_) {
        if (texture) SDL_DestroyTexture(texture);
    }
    textures_.clear();
    IMG_Quit();
    renderer_ptr = nullptr;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetManager shutdown complete.");
}