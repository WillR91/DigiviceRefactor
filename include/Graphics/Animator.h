// include/graphics/Animator.h
#ifndef ANIMATOR_H
#define ANIMATOR_H

#include "graphics/AnimationData.h" // Needs the definition of AnimationData
#include <cstddef>                  // For size_t
#include <SDL_rect.h>               // For SDL_Rect

class Animator {
public:
    Animator(); // Default constructor

    // Starts playing a new animation sequence.
    // Takes a non-owning pointer to the animation data definition.
    // Resets the playback state (frame index, timer).
    void setAnimation(const AnimationData* animationData);

    // Updates the animation playback based on elapsed time.
    // Should be called once per frame.
    void update(float deltaTime);

    // Gets the source rectangle on the texture atlas for the current frame.
    // Returns a default/empty rect if no animation is set or invalid.
    SDL_Rect getCurrentFrameRect() const;

    // Gets the texture atlas currently being used by the animation.
    // Returns nullptr if no animation is set.
    SDL_Texture* getCurrentTexture() const;

    // Checks if the current animation has finished playing (for non-looping anims).
    bool isFinished() const;

    // Resets the animator to its default state (no animation playing).
    void stop();

private:
    const AnimationData* currentAnimation_; // Non-owning pointer to the animation definition being played
    size_t currentFrameIndex_;              // Index of the frame currently being displayed
    float currentFrameTimerSec_;            // How long the current frame has been shown (in seconds)
    bool finished_;                         // Flag indicating if a non-looping animation has completed
};

#endif // ANIMATOR_H