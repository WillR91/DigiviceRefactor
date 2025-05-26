// File: src/core/AssetManager.cpp

#include "core/AssetManager.h" // Include own header
#include <SDL_image.h>         // For IMG_Load, IMG_Init, IMG_Quit, IMG_GetError
#include <SDL_render.h>        // For SDL_CreateTextureFromSurface, SDL_DestroyTexture
#include <SDL_surface.h>       // For SDL_Surface, SDL_FreeSurface
#include <SDL_log.h>           // For logging
#include <fstream>             // <<< ADDED for std::ifstream >>>


AssetManager::~AssetManager() {
    shutdown();
}

bool AssetManager::init(SDL_Renderer* renderer) {
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AssetManager::init failed: Renderer pointer is null.");
        return false;
    }
    renderer_ptr = renderer;
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_image could not initialize! IMG_Error: %s", IMG_GetError());
        renderer_ptr = nullptr;
        return false;
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetManager initialized successfully (SDL_image loaded).");
    return true;
}

bool AssetManager::loadTexture(const std::string& textureId, const std::string& filePath) {
    if (!renderer_ptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot load texture '%s': AssetManager not initialized.", textureId.c_str());
        return false;
    }
    if (textures_.count(textureId)) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Texture '%s' already loaded. Skipping.", textureId.c_str());
        return true;
    }

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Loading texture '%s' from '%s'", textureId.c_str(), filePath.c_str());

    // <<< --- ADDED BASIC FILE STREAM CHECK --- >>>
    std::ifstream testFile(filePath); // Attempt to open the file
    if (!testFile.is_open()) {
        // If the file couldn't be opened at the OS level:
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "BASIC FILE CHECK FAILED for '%s'. File likely missing/inaccessible at this path relative to CWD.", filePath.c_str());
        // IMG_GetError might not be meaningful here, but we log it just in case
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Attempted IMG_Load would likely fail. IMG_GetError() currently reports: %s", IMG_GetError());
        // testFile automatically closes when it goes out of scope here
        return false; // Exit early
    }
    // If we get here, the file was successfully opened (and immediately closed)
    testFile.close();
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Basic file check successful for '%s'. Proceeding with IMG_Load.", filePath.c_str());
    // <<< --- END ADDED FILE STREAM CHECK --- >>>

    // --- Load image surface using SDL_image ---
    SDL_Surface* loadedSurface = IMG_Load(filePath.c_str());
    if (!loadedSurface) {
        // Now log the SDL_image specific error if IMG_Load failed *after* basic check passed
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "IMG_Load failed for '%s'! SDL_image Error: %s", filePath.c_str(), IMG_GetError());
        return false;
    }

    // --- Convert surface to hardware-accelerated texture ---
    SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderer_ptr, loadedSurface);
    if (!newTexture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create texture from '%s'! SDL Error: %s", filePath.c_str(), SDL_GetError());
    }

    // --- Free the temporary surface ---
    SDL_FreeSurface(loadedSurface);

    if (!newTexture) {
        return false; // Texture creation failed
    }

    // --- Store the successful texture ---
    textures_[textureId] = newTexture;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Successfully loaded texture '%s'.", textureId.c_str());
    return true; // Success!
}

SDL_Texture* AssetManager::getTexture(const std::string& textureId) const {
    auto it = textures_.find(textureId);
    if (it != textures_.end()) {
        return it->second;
    } else {
        // Note: Removed warning log here as getTexture is often used for existence checks
        // Warning logs should be in calling code when appropriate
        return nullptr;
    }
}

bool AssetManager::hasTexture(const std::string& textureId) const {
    return textures_.find(textureId) != textures_.end();
}

void AssetManager::shutdown() {
    if (renderer_ptr == nullptr && textures_.empty()) { return; }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Shutting down AssetManager...");
    for (auto const& [id, texture] : textures_) {
        if (texture) SDL_DestroyTexture(texture);
    }
    textures_.clear();
    IMG_Quit();
    renderer_ptr = nullptr;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetManager shutdown complete.");
}