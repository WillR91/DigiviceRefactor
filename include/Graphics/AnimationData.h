// include/graphics/AnimationData.h
#ifndef ANIMATIONDATA_H
#define ANIMATIONDATA_H

#include <SDL_rect.h>    // For SDL_Rect
#include <SDL_render.h>  // For SDL_Texture definition
#include <vector>
#include <string>

struct AnimationData {
public:
    std::string id;                              // Animation identifier
    SDL_Texture* textureAtlas;                   // Pointer to texture atlas
    std::vector<SDL_Rect> frameRects;           // Frame rectangles
    std::vector<float> frameDurationsSec;        // Duration of each frame
    bool loops;                                  // Whether animation loops

    bool isValid() const {
        return textureAtlas != nullptr && 
               !frameRects.empty() && 
               frameRects.size() == frameDurationsSec.size();
    }

    size_t getFrameCount() const { 
        return frameRects.size(); 
    }
};

#endif // ANIMATIONDATA_H