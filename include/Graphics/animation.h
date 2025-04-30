// File: include/graphics/Animation.h (Assuming location)
#pragma once

#include <SDL.h>     // <<< CORRECTED SDL Include (for SDL_Texture, SDL_Rect, Uint32) >>>
#include <vector>    // Standard library - OK
#include <cstdint>   // Standard library - OK

// Represents a single frame using a texture atlas
struct SpriteFrame {
    SDL_Texture* texturePtr = nullptr; // Non-owning pointer to the texture atlas/sheet
    SDL_Rect sourceRect = {0, 0, 0, 0}; // Defines the frame's location and size on the texture sheet

    SpriteFrame(SDL_Texture* tex = nullptr, SDL_Rect src = {0,0,0,0})
        : texturePtr(tex), sourceRect(src) {}
};

// Represents a sequence of frames for an animation
class Animation {
public:
    std::vector<SpriteFrame> frames;
    std::vector<Uint32> frame_durations_ms; // Duration for each frame in milliseconds
    bool loops = true; // Does the animation loop?

    void addFrame(const SpriteFrame& frame, Uint32 duration_ms) {
        frames.push_back(frame);
        frame_durations_ms.push_back(duration_ms);
    }

    const SpriteFrame* getFrame(size_t frameIndex) const {
        if (frameIndex < frames.size()) {
            return &frames[frameIndex];
        }
        return nullptr;
    }

     size_t getFrameCount() const {
         return frames.size();
     }
};