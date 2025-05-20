// src/ui/TextRenderer.cpp
#include "ui/TextRenderer.h"      // Include own header
#include "vendor/nlohmann/json.hpp" // Full JSON header needed for implementation
#include "utils/ConfigManager.h"  // For loading global text scale from config
#include <SDL_log.h>
#include <fstream>
#include <stdexcept>               // For errors during loading maybe
#include <cctype>                  // For std::islower and std::toupper


TextRenderer::TextRenderer(SDL_Texture* fontTexture) :
    fontTexture_(fontTexture),
    defaultKerning_(-15), // Set default based on previous usage in MenuState
    globalTextScale_(1.0f) // Default scale
{
    if (!fontTexture_) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer created with null font texture!");
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer instance created.");
    
    // Load the global text scale from config
    updateGlobalTextScaleFromConfig();
}

// Update the global text scale from game config
void TextRenderer::updateGlobalTextScaleFromConfig() {
    if (ConfigManager::isInitialized()) {
        // Get the global text scale from config, default to 1.0 if not found
        globalTextScale_ = ConfigManager::getValue<float>("ui.textScale", 1.0f);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: Global text scale set to %.2f", globalTextScale_);
    } else {
        globalTextScale_ = 1.0f;
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: ConfigManager not initialized, using default text scale (1.0)");
    }
}

// Loads the character mapping from the JSON file
// (Based on MenuState::loadFontDataFromJson)
bool TextRenderer::loadFontData(const std::string& jsonPath) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer::loadFontData - Loading from: %s", jsonPath.c_str());
    fontCharMap_.clear();
    bool success = false;
    std::ifstream f(jsonPath);

    if (!f.is_open()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer::loadFontData Error: Failed to open file stream: %s", jsonPath.c_str());
        return false;
    }

    try {
        nlohmann::json data = nlohmann::json::parse(f);
        f.close(); // Close file after parsing

        if (!data.contains("frames") || !data["frames"].is_object()) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer::loadFontData Error: JSON missing 'frames' object or it's not an object: %s", jsonPath.c_str());
            return false;
        }

        const auto& frames = data["frames"];
        int loadedCount = 0;
        for (auto& [key, value] : frames.items()) {
            if (!value.contains("frame") || !value["frame"].is_object()) { continue; } // Skip if frame object missing

            const auto& frameData = value["frame"];
            if (!(frameData.contains("x") && frameData.contains("y") && frameData.contains("w") && frameData.contains("h"))) { continue; } // Skip if rect data missing

            char character = '\0';
            // Handle single character keys and descriptive keys
            if (key.length() == 1) {
                 character = key[0];
            } else {
                 // Map descriptive keys (add more as needed)
                 if (key == "QUESTION") character = '?'; else if (key == "apostrophe") character = '\''; else if (key == "colon") character = ':'; else if (key == "comma") character = ','; else if (key == "dash") character = '-'; else if (key == "divide") character = '/'; else if (key == "equals") character = '='; else if (key == "exclamation") character = '!'; else if (key == "forwardslash") character = '/'; else if (key == "period") character = '.'; else if (key == "plus") character = '+'; else if (key == "roundbracketleft") character = '('; else if (key == "roundbracketright") character = ')'; else if (key == "speech") character = '"'; else if (key == "times") character = '*'; else if (key == "weirdbracketleft") character = '['; else if (key == "weirdbracketright") character = ']';
                 else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer::loadFontData WARN: Skipping unrecognized descriptive key: '%s'", key.c_str()); continue; }
            }

            if (character != '\0') {
                int x = frameData.value("x", -1);
                int y = frameData.value("y", -1);
                int w = frameData.value("w", -1);
                int h = frameData.value("h", -1);

                if (x < 0 || y < 0 || w <= 0 || h <= 0) {
                    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer::loadFontData WARN: Skipping char '%c' (key '%s'): Invalid rect data (x=%d, y=%d, w=%d, h=%d)", character, key.c_str(), x, y, w, h);
                    continue;
                }
                fontCharMap_[character] = SDL_Rect{ x, y, w, h };
                loadedCount++;
            }
        }
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer::loadFontData: Finished processing frames. Loaded map for %d characters.", loadedCount);
        success = !fontCharMap_.empty();

    } catch (nlohmann::json::parse_error& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer::loadFontData ***ERROR (JSON PARSE)***: %s - %s (byte %zu)", jsonPath.c_str(), e.what(), e.byte);
    } catch (std::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer::loadFontData ***ERROR (STD EXCEPTION)***: %s - %s", jsonPath.c_str(), e.what());
    } catch (...) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer::loadFontData ***ERROR (UNKNOWN)***: An unknown exception occurred.");
    }

    if (f.is_open()) { f.close(); } // Ensure closed even on exception
    return success;
}


// Internal helper to get the source rect for a character
const SDL_Rect* TextRenderer::getCharRect(char c) const {
    auto it = fontCharMap_.find(c);
    if (it != fontCharMap_.end()) {
        // Check if the found rect is valid before returning pointer
        if (it->second.w > 0 && it->second.h > 0) {
            return &(it->second);
        } else {
            SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "TextRenderer::getCharRect WARN: Found char '%c' but rect has zero width/height!", c);
            // Fall through to try uppercase if 'c' is lowercase and its original entry was invalid
        }
    }

    // If not found, or found but invalid, try uppercase if 'c' is lowercase
    if (std::islower(static_cast<unsigned char>(c))) {
        char upper_c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        auto it_upper = fontCharMap_.find(upper_c);
        if (it_upper != fontCharMap_.end()) {
            if (it_upper->second.w > 0 && it_upper->second.h > 0) {
                return &(it_upper->second); // Return valid uppercase char
            } else {
                // Uppercase found but invalid
                SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "TextRenderer::getCharRect WARN: Found uppercase fallback '%c' for '%c' but rect has zero width/height!", upper_c, c);
                return nullptr; 
            }
        }
    }

    // If original char not found (and it wasn't lowercase or its uppercase wasn't found/invalid)
    // OR if original was found but invalid (and it wasn't lowercase or its uppercase wasn't found/invalid)
    // The calling functions (getTextDimensions, drawText) will log the "not found" if this returns nullptr.
    return nullptr; 
}


// Calculates the pixel dimensions of a string if rendered with this font
// (Based on MenuState::getTextDimensions)
SDL_Point TextRenderer::getTextDimensions(const std::string& text, int kerning) const {
    SDL_Point dimensions = {0, 0};
    int currentX = 0;
    int maxHeight = 0;
    bool firstChar = true;

    // Use default kerning if -1 is passed
    int effectiveKerning = (kerning == -1) ? defaultKerning_ : kerning;

    if (fontCharMap_.empty()) {
        SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "TextRenderer::getTextDimensions WARN: fontCharMap_ is empty!");
        return dimensions;
    }

    bool foundAnyChar = false;
    for (char c : text) {
        int charWidth = 0;
        int charHeight = 0;
        const SDL_Rect* pSrcRect = nullptr;

        if (c == ' ') {
            // Use width of 'A' or a default space width if 'A' isn't mapped
            const SDL_Rect* spaceRect = getCharRect('A');
            charWidth = (spaceRect != nullptr) ? spaceRect->w : 5; // Use 'A' width or fallback
            charHeight = (spaceRect != nullptr) ? spaceRect->h : 8; // Use 'A' height or fallback
            foundAnyChar = true;
        } else {
            pSrcRect = getCharRect(c);
            if (pSrcRect != nullptr) {
                charWidth = pSrcRect->w;
                charHeight = pSrcRect->h;
                foundAnyChar = true;
            } else {
                // Character not found - use fallback dimensions?
                charWidth = 5; // Fallback width
                charHeight = 8; // Fallback height
                SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "TextRenderer::getTextDimensions WARN: Character '%c' not found in font map!", c);
            }
        }

        // Add kerning only *between* characters
        if (!firstChar) {
             currentX += effectiveKerning;
        } else {
             firstChar = false;
        }

        currentX += charWidth;
        if (charHeight > maxHeight) {
            maxHeight = charHeight;
        }
    }

    dimensions.x = currentX; // Total width is the final X position
    dimensions.y = foundAnyChar ? maxHeight : 0; // Height is max char height found

    if (dimensions.x < 0) dimensions.x = 0; // Prevent negative width

    SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "TextRenderer::getTextDimensions for '%s' -> w=%d, h=%d", text.c_str(), dimensions.x, dimensions.y);
    return dimensions;
}


// Draws the text using the loaded font data
// (Based on MenuState::drawText)
void TextRenderer::drawText(SDL_Renderer* renderer, const std::string& text, int x, int y, float scale, int kerning) const {
    if (!fontTexture_ || fontCharMap_.empty() || !renderer || scale <= 0.0f) {
        SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "TextRenderer::drawText returning early. Texture=%p, MapEmpty=%d, Renderer=%p, Scale=%.2f", 
                   static_cast<const void*>(fontTexture_), fontCharMap_.empty(), static_cast<const void*>(renderer), scale);
        return;
    }
    
    // Apply global text scale
    float finalScale = scale * globalTextScale_;
    
    int currentX = x;
    // Use default kerning if -1 is passed
    int effectiveKerning = (kerning == -1) ? defaultKerning_ : kerning;
    float scaledKerning = static_cast<float>(effectiveKerning) * finalScale;
    bool firstChar = true;

    for (char c : text) {
        int charWidth = 0;
        const SDL_Rect* pSrcRect = nullptr;

        if (c == ' ') {
            const SDL_Rect* spaceRect = getCharRect('A');
            charWidth = (spaceRect != nullptr) ? spaceRect->w : 5; // Use 'A' width or fallback
        } else {
            pSrcRect = getCharRect(c);
            if (pSrcRect != nullptr) {
                charWidth = pSrcRect->w;
            } else {
                charWidth = 5; // Fallback width for unknown chars
                SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "TextRenderer::drawText: Char '%c' not found in map.", c);
            }
        }        // Apply kerning before drawing (except for first char)
        if (!firstChar) {
            currentX += static_cast<int>(scaledKerning);
        } else {
            firstChar = false;
        }

        int scaledW = static_cast<int>(static_cast<float>(charWidth) * scale);

        // Only attempt to draw if we found a source rect (i.e., not space or unknown)
        if (pSrcRect != nullptr) {
             int charHeight = pSrcRect->h;
             int scaledH = static_cast<int>(static_cast<float>(charHeight) * scale);

             if (scaledW > 0 && scaledH > 0) {
                 SDL_Rect destRect = { currentX, y, scaledW, scaledH };
                 // SDL_RenderCopy handles drawing
                 SDL_RenderCopy(renderer, fontTexture_, pSrcRect, &destRect);
             }
        }

        // Advance X position for the next character
        currentX += scaledW;
    }
}

// Implementation for renderTextToTexture
SDL_Texture* TextRenderer::renderTextToTexture(SDL_Renderer* renderer, const std::string& text, SDL_Color color, float scale, int kerning) {
    if (!fontTexture_ || fontCharMap_.empty() || !renderer || scale <= 0.0f || text.empty()) {
        SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "TextRenderer::renderTextToTexture returning early. Texture=%p, MapEmpty=%d, Renderer=%p, Scale=%.2f, TextEmpty=%d",
                   static_cast<const void*>(fontTexture_), fontCharMap_.empty(), static_cast<const void*>(renderer), scale, text.empty());
        return nullptr;
    }
    
    // Apply global text scale
    float finalScale = scale * globalTextScale_;

    // 1. Calculate dimensions
    SDL_Point dimensions = getTextDimensions(text, kerning);
    if (dimensions.x <= 0 || dimensions.y <= 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "TextRenderer::renderTextToTexture: Calculated text dimensions are invalid (w=%d, h=%d) for text: %s", dimensions.x, dimensions.y, text.c_str());
        return nullptr;
    }

    int finalWidth = static_cast<int>(static_cast<float>(dimensions.x) * finalScale);
    int finalHeight = static_cast<int>(static_cast<float>(dimensions.y) * finalScale);

    if (finalWidth <= 0 || finalHeight <= 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "TextRenderer::renderTextToTexture: Scaled text dimensions are invalid (w=%d, h=%d) for text: %s", finalWidth, finalHeight, text.c_str());
        return nullptr;
    }

    // 2. Create a new texture
    SDL_Texture* newTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, finalWidth, finalHeight);
    if (!newTexture) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "TextRenderer::renderTextToTexture: Failed to create target texture: %s", SDL_GetError());
        return nullptr;
    }

    // 3. Set the new texture as render target and make it transparent
    SDL_SetRenderTarget(renderer, newTexture);
    SDL_SetTextureBlendMode(newTexture, SDL_BLENDMODE_BLEND); // Enable alpha blending for the texture itself
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0); // Transparent background
    SDL_RenderClear(renderer);

    // Optional: Modulate the font texture color if your font atlas is white/grayscale
    // This allows you to "colorize" the text. If your font atlas already has colors, you might skip this.
    SDL_SetTextureColorMod(fontTexture_, color.r, color.g, color.b);
    // Note: Alpha modulation (SDL_SetTextureAlphaMod) might also be useful if the color.a is intended to make the whole text semi-transparent.
    // However, for typical text rendering, color.a in SDL_Color is often ignored unless specifically handled.
    // The created texture (newTexture) will handle overall transparency via its pixel data and blend mode.    // 4. Draw the text onto the new texture
    // The drawText method handles scaling internally, so we pass the scale.
    // We draw at (0,0) on the new texture.
    drawText(renderer, text, 0, 0, scale, kerning);

    // Reset color modulation on the font atlas if you changed it
    SDL_SetTextureColorMod(fontTexture_, 255, 255, 255); // Reset to white (no modulation)

    // 5. Reset render target back to the default (screen or previous target)
    SDL_SetRenderTarget(renderer, nullptr);

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer::renderTextToTexture - Successfully created texture for text: %s", text.c_str());
    return newTexture;
}