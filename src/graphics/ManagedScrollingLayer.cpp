#include "graphics/ManagedScrollingLayer.h" 
#include "core/AssetManager.h" // For AssetManager
#include <SDL_log.h> // For logging
#include <algorithm> // For std::remove_if if needed for texture cleanup (though AssetManager handles it)
#include <cmath> // Added for std::fmod

ManagedScrollingLayer::ManagedScrollingLayer() :
    assetManager_(nullptr),
    currentVariantIndex_(0),
    scrollOffset_(0.0f),
    scrollSpeed_(0.0f),
    currentTextureWidth_(0),
    currentTextureHeight_(0)
{
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ManagedScrollingLayer: Constructor called.");
}

ManagedScrollingLayer::~ManagedScrollingLayer() {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ManagedScrollingLayer: Destructor called.");
    // SDL_Texture pointers in textureVariants_ are managed by AssetManager,
    // so no need to SDL_DestroyTexture here. Clearing the vector is good practice.
    textureVariants_.clear();
}

void ManagedScrollingLayer::initialize(AssetManager* assetManager, float scrollSpeed) {
    if (!assetManager) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ManagedScrollingLayer::initialize - AssetManager is null!");
        // Potentially throw an exception or handle error appropriately
        return;
    }
    assetManager_ = assetManager;
    scrollSpeed_ = scrollSpeed;
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ManagedScrollingLayer: Initialized with scroll speed: %f", scrollSpeed);
}

void ManagedScrollingLayer::loadVariants(const std::vector<std::string>& texturePaths, const std::string& defaultTexturePath) {
    if (!assetManager_) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ManagedScrollingLayer::loadVariants - AssetManager is not initialized!");
        return;
    }

    textureVariants_.clear();
    currentVariantIndex_ = 0;

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ManagedScrollingLayer: Loading variants. Provided paths count: %zu, Default path: '%s'", texturePaths.size(), defaultTexturePath.c_str());

    for (const std::string& path : texturePaths) {
        if (path.empty()) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ManagedScrollingLayer::loadVariants - Encountered empty texture path, skipping.");
            continue;
        }
        // Use the path itself as the texture ID for AssetManager
        // AssetManager::loadTexture will handle if it's already loaded.
        if (assetManager_->loadTexture(path, path)) { // path is used for both filePath and textureId
            SDL_Texture* tex = assetManager_->getTexture(path);
            if (tex) {
                textureVariants_.push_back(tex);
                SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ManagedScrollingLayer::loadVariants - Successfully loaded and added texture: '%s'", path.c_str());
            } else {
                // This case should ideally not be hit if loadTexture succeeded and used 'path' as ID.
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ManagedScrollingLayer::loadVariants - Texture loaded for path '%s' but getTexture returned null.", path.c_str());
            }
        } else {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ManagedScrollingLayer::loadVariants - Failed to load texture: '%s'", path.c_str());
        }
    }

    if (textureVariants_.empty()) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ManagedScrollingLayer::loadVariants - No specific variants loaded or provided. Attempting to load default: '%s'", defaultTexturePath.c_str());
        if (!defaultTexturePath.empty()) {
            if (assetManager_->loadTexture(defaultTexturePath, defaultTexturePath)) {
                SDL_Texture* tex = assetManager_->getTexture(defaultTexturePath);
                if (tex) {
                    textureVariants_.push_back(tex);
                    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ManagedScrollingLayer::loadVariants - Successfully loaded and added default texture: '%s'", defaultTexturePath.c_str());
                } else {
                    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ManagedScrollingLayer::loadVariants - Default texture loaded for path '%s' but getTexture returned null.", defaultTexturePath.c_str());
                }
            } else {
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ManagedScrollingLayer::loadVariants - Failed to load default texture: '%s'", defaultTexturePath.c_str());
            }
        } else {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ManagedScrollingLayer::loadVariants - Default texture path is also empty. Layer will have no textures.");
        }
    }
    
    if (textureVariants_.empty()) {
         SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ManagedScrollingLayer::loadVariants - Layer has no textures after all attempts.");
    }

    updateCurrentTextureDimensions(); // Update dimensions based on the first loaded texture (or set to 0 if none)
}

void ManagedScrollingLayer::setScrollSpeed(float speed) {
    scrollSpeed_ = speed;
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ManagedScrollingLayer::setScrollSpeed - Speed set to: %f", speed);
}

void ManagedScrollingLayer::update(float deltaTime) {
    if (currentTextureWidth_ == 0) return; // Avoid division by zero or issues if texture not loaded

    float scrollAmount = scrollSpeed_ * deltaTime;
    scrollOffset_ -= scrollAmount; // Player moves right, so background elements move from R to L on screen

    bool variantChanged = false;

    // Check if the texture has scrolled at least one full width to the left
    if (scrollOffset_ <= -static_cast<float>(currentTextureWidth_)) {
        // Normalize scrollOffset_ using fmod.
        // For a negative scrollOffset_ and positive currentTextureWidth_,
        // std::fmod will return a value in (-currentTextureWidth_, 0].
        // Example: fmod(-301.0, 300.0) -> -1.0
        // Example: fmod(-300.0, 300.0) -> 0.0 (or -0.0)
        scrollOffset_ = std::fmod(scrollOffset_, static_cast<float>(currentTextureWidth_));

        if (texturePaths_.size() > 1) {
            currentVariantIndex_ = (currentVariantIndex_ + 1) % texturePaths_.size();
            variantChanged = true;
        }
    }
    // It is highly unlikely that scrollOffset_ will become positive if it starts at 0
    // and scrollSpeed_ is positive, as scrollAmount is subtracted.
    // However, if there was a case where scrollOffset_ could become > 0 (e.g. scrollSpeed_ becomes negative briefly),
    // a similar fmod and variant change logic might be needed:
    // else if (scrollOffset_ > 0.0f) { // Scrolled "backwards" past the origin
    //     scrollOffset_ = std::fmod(scrollOffset_, static_cast<float>(currentTextureWidth_));
    //     // If it was positive, fmod might keep it positive, e.g. fmod(1.0, 300.0) -> 1.0.
    //     // We need it in (-width, 0]. So, if it's positive, subtract width.
    //     if (scrollOffset_ > 0.0f) { // This condition might seem redundant with the else-if, but fmod could be 0
    //         scrollOffset_ -= static_cast<float>(currentTextureWidth_);
    //     }
    //     if (texturePaths_.size() > 1) {
    //         currentVariantIndex_ = (currentVariantIndex_ - 1 + texturePaths_.size()) % texturePaths_.size(); // Cycle backwards
    //         variantChanged = true;
    //     }
    // }


    if (variantChanged) {
        // int oldTextureWidth = currentTextureWidth_; // For proportional scaling if needed
        updateCurrentTextureDimensions();
        // Optional: If texture widths can vary, scale scrollOffset_ proportionally
        // to maintain visual position. This is complex and might not be needed if widths are similar.
        // if (currentTextureWidth_ != oldTextureWidth && oldTextureWidth != 0) {
        //     scrollOffset_ = (scrollOffset_ / static_cast<float>(oldTextureWidth)) * static_cast<float>(currentTextureWidth_);
        // }
    }
}

SDL_Texture* ManagedScrollingLayer::getCurrentTexture() const {
    if (textureVariants_.empty() || currentVariantIndex_ >= textureVariants_.size()) {
        // SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ManagedScrollingLayer::getCurrentTexture - No texture available or index out of bounds.");
        return nullptr;
    }
    return textureVariants_[currentVariantIndex_];
}

float ManagedScrollingLayer::getScrollOffset() const {
    return scrollOffset_;
}

void ManagedScrollingLayer::getCurrentDimensions(int& width, int& height) const {
    width = currentTextureWidth_;
    height = currentTextureHeight_;
}

void ManagedScrollingLayer::reset() {
    scrollOffset_ = 0.0f;
    currentVariantIndex_ = 0;
    if (!textureVariants_.empty()) {
        updateCurrentTextureDimensions(); // Ensure dimensions are for the first variant
    }
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ManagedScrollingLayer::reset - Layer reset.");
}

void ManagedScrollingLayer::updateCurrentTextureDimensions() {
    SDL_Texture* currentTex = getCurrentTexture();
    if (currentTex) {
        if (SDL_QueryTexture(currentTex, nullptr, nullptr, &currentTextureWidth_, &currentTextureHeight_) != 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ManagedScrollingLayer::updateCurrentTextureDimensions - SDL_QueryTexture failed: %s", SDL_GetError());
            currentTextureWidth_ = 0;
            currentTextureHeight_ = 0;
        }
    } else {
        currentTextureWidth_ = 0;
        currentTextureHeight_ = 0;
    }
    // SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "ManagedScrollingLayer: Updated current texture dimensions to W: %d, H: %d", currentTextureWidth_, currentTextureHeight_);
}
