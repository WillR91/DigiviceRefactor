#pragma once
#include <SDL.h>
#include <string>

class TextRenderer;

namespace TextLayoutUtils {
    
    /**
     * @brief Calculates centered X position for text accounting for global text scale
     * @param textRenderer The text renderer to use for dimension calculations
     * @param text The text to center
     * @param screenWidth The screen width to center within
     * @param localScale The local scale factor for the text
     * @param kerning The kerning value for the text
     * @return The X position for centered text
     */
    int getCenteredX(TextRenderer* textRenderer, const std::string& text, 
                     int screenWidth, float localScale = 1.0f, int kerning = 0);
    
    /**
     * @brief Calculates centered Y position for text accounting for global text scale
     * @param textRenderer The text renderer to use for dimension calculations
     * @param text The text to center
     * @param screenHeight The screen height to center within
     * @param localScale The local scale factor for the text
     * @param kerning The kerning value for the text
     * @return The Y position for centered text
     */
    int getCenteredY(TextRenderer* textRenderer, const std::string& text, 
                     int screenHeight, float localScale = 1.0f, int kerning = 0);
    
    /**
     * @brief Calculates both centered X and Y positions for text
     * @param textRenderer The text renderer to use for dimension calculations
     * @param text The text to center
     * @param screenWidth The screen width to center within
     * @param screenHeight The screen height to center within
     * @param localScale The local scale factor for the text
     * @param kerning The kerning value for the text
     * @return SDL_Point with x,y coordinates for centered text
     */
    SDL_Point getCenteredPosition(TextRenderer* textRenderer, const std::string& text, 
                                  int screenWidth, int screenHeight, 
                                  float localScale = 1.0f, int kerning = 0);
    
    /**
     * @brief Gets the final scaled dimensions of text accounting for global text scale
     * @param textRenderer The text renderer to use for dimension calculations
     * @param text The text to measure
     * @param localScale The local scale factor for the text
     * @param kerning The kerning value for the text
     * @return SDL_Point with width and height of the scaled text
     */
    SDL_Point getScaledTextDimensions(TextRenderer* textRenderer, const std::string& text, 
                                      float localScale = 1.0f, int kerning = 0);
}
