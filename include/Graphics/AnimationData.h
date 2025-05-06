// include/graphics/AnimationData.h
#ifndef ANIMATIONDATA_H
#define ANIMATIONDATA_H

#include <SDL_rect.h> // For SDL_Rect
#include <SDL_render.h> // For SDL_Texture forward declaration below
#include <vector>
#include <string>

// Forward declare SDL_Texture (actual definition is in SDL_render.h)
// This avoids including the full SDL_render.h in every file that includes AnimationData.h
struct SDL_Texture;

// Represents the definition of a single animation sequence.
// This struct holds the data loaded from JSON.
struct AnimationData {
    std::string id;                       // Unique identifier (e.g., "Agumon_Idle", "CastleBackground_Scroll")
    SDL_Texture* textureAtlas = nullptr;  // Non-owning pointer to the texture atlas (Managed by AssetManager)
    std::vector<SDL_Rect> frameRects;     // Source rectangles for each frame on the atlas
    std::vector<float> frameDurationsSec; // Duration (in seconds) to display each corresponding frame
    bool loops = false;                   // Whether the animation should loop

    // Default constructor
    AnimationData() = default;

    // Optional: Constructor for convenience? Maybe later.

    // Helper method to get frame count
    size_t getFrameCount() const {
        // Frame count is determined by the number of rectangles (should match durations)
        return frameRects.size();
    }

    // Basic validation
    bool isValid() const {
        return !id.empty() &&
               textureAtlas != nullptr &&
               !frameRects.empty() &&
               frameRects.size() == frameDurationsSec.size();
    }
};

#endif // ANIMATIONDATA_H