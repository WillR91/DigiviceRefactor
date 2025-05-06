// src/graphics/Animator.cpp
#include "graphics/Animator.h"
#include <SDL_log.h> // For logging potential issues

Animator::Animator() :
    currentAnimation_(nullptr),
    currentFrameIndex_(0),
    currentFrameTimerSec_(0.0f),
    finished_(false)
{}

void Animator::setAnimation(const AnimationData* animationData) {
    // Check if the new animation data is valid and different from the current one
    if (animationData && animationData->isValid() && currentAnimation_ != animationData) {
        currentAnimation_ = animationData;
        currentFrameIndex_ = 0;
        currentFrameTimerSec_ = 0.0f;
        finished_ = false; // Start fresh, not finished
        SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "Animator: Set animation to '%s'", currentAnimation_->id.c_str());
    } else if (!animationData) {
        // If passed null, stop the animation
        stop();
        SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "Animator: setAnimation called with null, stopping.");
    } else if (currentAnimation_ == animationData) {
        // Optionally reset if setting the same animation again? Or do nothing?
        // For now, let's reset it if explicitly set again.
        currentFrameIndex_ = 0;
        currentFrameTimerSec_ = 0.0f;
        finished_ = false;
         SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "Animator: Resetting animation '%s'", currentAnimation_->id.c_str());
    } else if (animationData && !animationData->isValid()) {
         SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Animator: Attempted to set invalid AnimationData (ID: '%s')", animationData->id.c_str());
         stop(); // Stop if invalid data provided
    }
}

void Animator::update(float deltaTime) {
    // Only update if we have a valid animation and it hasn't finished (for non-looping)
    if (!currentAnimation_ || finished_) {
        return;
    }

    size_t frameCount = currentAnimation_->getFrameCount();
    // Need at least one frame to animate
    if (frameCount == 0) {
        finished_ = true; // Consider it finished if there are no frames
        return;
    }

    // Ensure frame index is valid (should be, but safety check)
    if (currentFrameIndex_ >= frameCount) {
        currentFrameIndex_ = 0; // Should not happen if logic below is correct
        currentFrameTimerSec_ = 0.0f;
    }

    // Add elapsed time
    currentFrameTimerSec_ += deltaTime;

    // Get the duration for the current frame
    float currentFrameDuration = currentAnimation_->frameDurationsSec[currentFrameIndex_];

    // Check if we need to advance frames (handle cases where deltaTime > frame duration)
    while (currentFrameTimerSec_ >= currentFrameDuration) {
        // Subtract the duration we just used up
        currentFrameTimerSec_ -= currentFrameDuration;

        // Move to the next frame
        currentFrameIndex_++;

        // Check if we reached the end of the animation sequence
        if (currentFrameIndex_ >= frameCount) {
            if (currentAnimation_->loops) {
                // Loop back to the first frame
                currentFrameIndex_ = 0;
                // Get the duration of the new frame (frame 0) for the next check in the while loop
                currentFrameDuration = currentAnimation_->frameDurationsSec[currentFrameIndex_];
            } else {
                // Not looping: clamp to the last frame and mark as finished
                currentFrameIndex_ = frameCount - 1;
                finished_ = true;
                currentFrameTimerSec_ = 0.0f; // Optional: reset timer on finish
                break; // Exit the while loop, animation is done
            }
        } else {
             // If we didn't reach the end, get duration for the new current frame
             currentFrameDuration = currentAnimation_->frameDurationsSec[currentFrameIndex_];
        }

         // Safety check for potentially zero duration frames after advancing
         if (currentFrameDuration <= 0.0f) {
              SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Animator: Animation '%s' frame %zu has zero or negative duration!", currentAnimation_->id.c_str(), currentFrameIndex_);
              // If we are looping and hit a zero duration frame, break the while to avoid infinite loop
              if (currentAnimation_->loops) {
                   break;
              }
              // If not looping, continue the while loop to potentially skip past it (might get stuck if many zero frames)
         }

    } // End while (timer >= duration)
}

SDL_Rect Animator::getCurrentFrameRect() const {
    if (currentAnimation_ && currentFrameIndex_ < currentAnimation_->frameRects.size()) {
        return currentAnimation_->frameRects[currentFrameIndex_];
    }
    // Return an empty rect if no valid animation/frame
    return {0, 0, 0, 0};
}

SDL_Texture* Animator::getCurrentTexture() const {
     if (currentAnimation_) {
          return currentAnimation_->textureAtlas;
     }
     return nullptr;
}

bool Animator::isFinished() const {
    return finished_;
}

void Animator::stop() {
    currentAnimation_ = nullptr;
    currentFrameIndex_ = 0;
    currentFrameTimerSec_ = 0.0f;
    finished_ = true; // Treat stopped as finished
}