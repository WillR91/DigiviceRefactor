// AdventureStateTextureHelpers.cpp
// Contains helper methods for the AdventureState class related to texture handling

#include "states/AdventureState.h"
#include "core/Game.h"
#include "core/AssetManager.h"
#include "platform/pc/pc_display.h"
#include <SDL_log.h>
#include <cmath>

// Helper method to load texture variants for a layer
void AdventureState::loadTextureVariantsForLayer(LayerVariants& layer, 
                                              const std::vector<std::string>& texturePaths, 
                                              const std::string& baseTextureId) {
    // Clear any existing data
    layer.textures.clear();
    layer.widths.clear();
    layer.effectiveWidths.clear();
    layer.currentVariantIndex = 0;
    
    // DEBUG: Log the number of texture paths we're going to process
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
        "DEBUG: Loading layer '%s' with %d texture paths", 
        baseTextureId.c_str(), (int)texturePaths.size());
        
    if (texturePaths.empty()) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
            "DEBUG: No texture paths provided for layer '%s'", baseTextureId.c_str());
        return;
    }
    
    // DEBUG: Print all paths that we're going to try to load
    for (size_t i = 0; i < texturePaths.size(); i++) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
            "DEBUG: Layer '%s' texture path[%d]: '%s'", 
            baseTextureId.c_str(), (int)i, texturePaths[i].c_str());
    }
    
    AssetManager* assets = game_ptr->getAssetManager();
    if (!assets) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AdventureState::loadTextureVariantsForLayer - AssetManager is NULL!");
        return;
    }
    
    // Loop through each texture path and load it
    for (size_t i = 0; i < texturePaths.size(); i++) {
        const std::string& texturePath = texturePaths[i];
        std::string variantTexId = baseTextureId;
        
        // For variant textures (after the first one), append the variant index to the ID
        if (i > 0) {
            variantTexId += "_" + std::to_string(i);
        }
        
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
            "DEBUG: Attempting to load variant %d for layer '%s' with ID '%s' from path: '%s'", 
            (int)i, baseTextureId.c_str(), variantTexId.c_str(), texturePath.c_str());
        
        // Check if texture is already loaded
        SDL_Texture* tex = assets->getTexture(variantTexId);
        if (tex) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                "DEBUG: Texture '%s' is already loaded in AssetManager", variantTexId.c_str());
        } else {
            // Try to load the texture
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                "DEBUG: Texture '%s' not found in AssetManager, attempting to load from '%s'", 
                variantTexId.c_str(), texturePath.c_str());
                
            bool loadResult = assets->loadTexture(variantTexId, texturePath);
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                "DEBUG: Load texture result for '%s': %s", 
                variantTexId.c_str(), loadResult ? "SUCCESS" : "FAILURE");
                
            if (loadResult) {
                tex = assets->getTexture(variantTexId);
            }
        }
        
        if (tex) {
            // Add texture to the layer's variants
            layer.textures.push_back(tex);
            
            // Get texture width and calculate effective width
            int width = 0, height = 0;
            SDL_QueryTexture(tex, nullptr, nullptr, &width, &height);
            layer.widths.push_back(width);
            
            // Set effective width to 2/3 of the actual width (for better scrolling effect)
            int effectiveWidth = width * 2/3;
            if (effectiveWidth <= 0) effectiveWidth = width;
            layer.effectiveWidths.push_back(effectiveWidth);
            
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                "DEBUG: Successfully added texture variant %d for layer '%s': %dx%d (effective width: %d)", 
                (int)i, baseTextureId.c_str(), width, height, effectiveWidth);
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                "Failed to load texture variant %d for layer %s: %s", 
                (int)i, baseTextureId.c_str(), texturePath.c_str());
        }
    }
    
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
        "DEBUG: Final count - Loaded %d texture variants for layer '%s'", 
        (int)layer.textures.size(), baseTextureId.c_str());
}

// Helper to render a background layer with its variants
void AdventureState::renderBackgroundLayerVariants(PCDisplay& display, 
                                                const LayerVariants& layer, 
                                                float scrollOffset) {
    // DEBUG: Add static counter to limit how often we log (every 60 frames)
    static int frameCounter = 0;
    frameCounter = (frameCounter + 1) % 60;
    bool shouldLog = (frameCounter == 0);
    
    if (layer.textures.empty()) {
        if (shouldLog) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                "DEBUG: Skipping rendering - No textures in layer (scrollOffset: %.2f)", 
                scrollOffset);
        }
        return;
    }
    
    const int windowW = WINDOW_WIDTH;
    const int windowH = WINDOW_HEIGHT;
    
    // Get the current variant
    size_t variantIndex = layer.currentVariantIndex % layer.textures.size();
    SDL_Texture* currentTexture = layer.textures[variantIndex];
    if (!currentTexture) {
        if (shouldLog) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                "DEBUG: Current texture at index %zu is NULL", variantIndex);
        }
        return;
    }
    
    if (shouldLog) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
            "DEBUG: Rendering variant %zu of %zu (scrollOffset: %.2f)", 
            variantIndex, layer.textures.size(), scrollOffset);
    }
    
    int texW = layer.widths[variantIndex];
    int texH = windowH; // Use window height for texture height
    int effectiveWidth = layer.effectiveWidths[variantIndex];
    
    // Calculate where to draw the current texture
    int drawX = -static_cast<int>(std::fmod(scrollOffset, (float)effectiveWidth));
    if (drawX > 0) drawX -= effectiveWidth;
    
    // Draw the current texture
    SDL_Rect dst = { drawX, 0, texW, texH };
    display.drawTexture(currentTexture, nullptr, &dst);
    
    // Draw the next variant (or wrap back to the first if this is the last)
    size_t nextVariantIndex = (variantIndex + 1) % layer.textures.size();
    SDL_Texture* nextTexture = layer.textures[nextVariantIndex];
    if (!nextTexture) {
        if (shouldLog) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                "DEBUG: Next texture at index %zu is NULL", nextVariantIndex);
        }
        return;
    }
    
    if (shouldLog && nextVariantIndex != variantIndex) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
            "DEBUG: Drawing next variant %zu after current variant %zu", 
            nextVariantIndex, variantIndex);
    }
    
    int nextTexW = layer.widths[nextVariantIndex];
    int nextEffectiveWidth = layer.effectiveWidths[nextVariantIndex];
    
    // Draw the next texture after the current one
    int drawNextX = drawX + effectiveWidth;
    SDL_Rect dstNext = { drawNextX, 0, nextTexW, texH };
    display.drawTexture(nextTexture, nullptr, &dstNext);
    
    // Draw a third texture if needed (could be the first variant again)
    if (drawNextX + nextTexW < windowW) {
        size_t thirdVariantIndex = (nextVariantIndex + 1) % layer.textures.size();
        SDL_Texture* thirdTexture = layer.textures[thirdVariantIndex];
        if (!thirdTexture) {
            if (shouldLog) {
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                    "DEBUG: Third texture at index %zu is NULL", thirdVariantIndex);
            }
            return;
        }
        
        if (shouldLog && thirdVariantIndex != variantIndex && thirdVariantIndex != nextVariantIndex) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
                "DEBUG: Drawing third variant %zu", thirdVariantIndex);
        }
        
        int thirdTexW = layer.widths[thirdVariantIndex];
        int drawThirdX = drawNextX + nextEffectiveWidth;
        SDL_Rect dstThird = { drawThirdX, 0, thirdTexW, texH };
        display.drawTexture(thirdTexture, nullptr, &dstThird);
    }
}
