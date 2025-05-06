// src/ui/TextRenderer.cpp
#include "ui/TextRenderer.h"      // Include own header
#include "vendor/nlohmann/json.hpp" // Full JSON header needed for implementation
#include <SDL_log.h>
#include <fstream>
#include <stdexcept>               // For errors during loading maybe


TextRenderer::TextRenderer(SDL_Texture* fontTexture) :
    fontTexture_(fontTexture),
    defaultKerning_(-15) // Set default based on previous usage in MenuState
{
    if (!fontTexture_) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer created with null font texture!");
    }
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer instance created.");
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
               return nullptr; // Treat as not found if rect invalid
          }
     }
     return nullptr; // Character not in map
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
        SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "TextRenderer::drawText returning early. Texture=%p, MapEmpty=%d, Renderer=%p, Scale=%.2f", (void*)fontTexture_, fontCharMap_.empty(), (void*)renderer, scale);
        return;
    }

    int currentX = x;
    // Use default kerning if -1 is passed
    int effectiveKerning = (kerning == -1) ? defaultKerning_ : kerning;
    float scaledKerning = static_cast<float>(effectiveKerning) * scale;
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
        }

        // Apply kerning before drawing (except for first char)
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