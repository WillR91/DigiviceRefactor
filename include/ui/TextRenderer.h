// include/ui/TextRenderer.h
#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include <SDL.h>
#include <string>
#include <vector>
#include <map>
#include "vendor/nlohmann/json.hpp" // <<<< INCLUDE FULL HEADER

class TextRenderer {
public:
    // Constructor: Takes the pre-loaded font texture atlas
    // Textures are managed by AssetManager, so TextRenderer doesn't own it.
    TextRenderer(SDL_Texture* fontTexture);

    // Loads the character mapping from the JSON file
    bool loadFontData(const std::string& jsonPath);

    // Calculates the pixel dimensions of a string if rendered with this font
    SDL_Point getTextDimensions(const std::string& text, int kerning = -1) const; // Default kerning? Check original code use. Use -1 to signal using default member

    // Draws the text using the loaded font data
    void drawText(SDL_Renderer* renderer, const std::string& text, int x, int y, float scale = 1.0f, int kerning = -1) const; // Default kerning?

private:
    SDL_Texture* fontTexture_; // Non-owning pointer to the font atlas
    std::map<char, SDL_Rect> fontCharMap_; // Map from character to source Rect on atlas
    int defaultKerning_; // Store default kerning (Maybe -15 based on MenuState?)

    // Internal helper to get the source rect for a character
    const SDL_Rect* getCharRect(char c) const;
};

#endif // TEXTRENDERER_H