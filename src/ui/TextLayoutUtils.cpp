#include "ui/TextLayoutUtils.h"
#include "ui/TextRenderer.h"
#include <SDL_log.h>

namespace TextLayoutUtils {
    
    int getCenteredX(TextRenderer* textRenderer, const std::string& text, 
                     int screenWidth, float localScale, int kerning) {
        if (!textRenderer || text.empty() || screenWidth <= 0) {
            SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "TextLayoutUtils::getCenteredX: Invalid parameters");
            return 0;
        }
        
        SDL_Point baseDimensions = textRenderer->getTextDimensions(text, kerning);
        if (baseDimensions.x <= 0) {
            SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "TextLayoutUtils::getCenteredX: Invalid text dimensions for: %s", text.c_str());
            return 0;
        }
        
        float globalTextScale = textRenderer->getGlobalTextScale();
        float finalScale = localScale * globalTextScale;
        int scaledWidth = static_cast<int>(static_cast<float>(baseDimensions.x) * finalScale);
        
        return (screenWidth / 2) - (scaledWidth / 2);
    }
    
    int getCenteredY(TextRenderer* textRenderer, const std::string& text, 
                     int screenHeight, float localScale, int kerning) {
        if (!textRenderer || text.empty() || screenHeight <= 0) {
            SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "TextLayoutUtils::getCenteredY: Invalid parameters");
            return 0;
        }
        
        SDL_Point baseDimensions = textRenderer->getTextDimensions(text, kerning);
        if (baseDimensions.y <= 0) {
            SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "TextLayoutUtils::getCenteredY: Invalid text dimensions for: %s", text.c_str());
            return 0;
        }
        
        float globalTextScale = textRenderer->getGlobalTextScale();
        float finalScale = localScale * globalTextScale;
        int scaledHeight = static_cast<int>(static_cast<float>(baseDimensions.y) * finalScale);
        
        return (screenHeight / 2) - (scaledHeight / 2);
    }
    
    SDL_Point getCenteredPosition(TextRenderer* textRenderer, const std::string& text, 
                                  int screenWidth, int screenHeight, 
                                  float localScale, int kerning) {
        SDL_Point position = {0, 0};
        
        if (!textRenderer || text.empty() || screenWidth <= 0 || screenHeight <= 0) {
            SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "TextLayoutUtils::getCenteredPosition: Invalid parameters");
            return position;
        }
        
        position.x = getCenteredX(textRenderer, text, screenWidth, localScale, kerning);
        position.y = getCenteredY(textRenderer, text, screenHeight, localScale, kerning);
        
        return position;
    }
    
    SDL_Point getScaledTextDimensions(TextRenderer* textRenderer, const std::string& text, 
                                      float localScale, int kerning) {
        SDL_Point dimensions = {0, 0};
        
        if (!textRenderer || text.empty()) {
            SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "TextLayoutUtils::getScaledTextDimensions: Invalid parameters");
            return dimensions;
        }
        
        SDL_Point baseDimensions = textRenderer->getTextDimensions(text, kerning);
        if (baseDimensions.x <= 0 || baseDimensions.y <= 0) {
            SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "TextLayoutUtils::getScaledTextDimensions: Invalid text dimensions for: %s", text.c_str());
            return dimensions;
        }
        
        float globalTextScale = textRenderer->getGlobalTextScale();
        float finalScale = localScale * globalTextScale;
        
        dimensions.x = static_cast<int>(static_cast<float>(baseDimensions.x) * finalScale);
        dimensions.y = static_cast<int>(static_cast<float>(baseDimensions.y) * finalScale);
        
        return dimensions;
    }
}
