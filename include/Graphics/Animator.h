// include/graphics/Animator.h
#ifndef ANIMATOR_H
#define ANIMATOR_H

#pragma once
#include <SDL_rect.h>               // For SDL_Rect
#include "graphics/AnimationData.h" // Needs the definition of AnimationData
#include <cstddef>                  // For size_t

class Animator {
public:
    Animator();
    
    void setAnimation(const AnimationData* animation, bool resetPlayback = true);
    void update(float deltaTime);
    void stop();
    bool isFinished() const { return finished_; }
    const AnimationData* getCurrentAnimationData() const { return currentAnimation_; }
    
    SDL_Rect getCurrentFrameRect() const;  // Changed from sf::IntRect
    SDL_Texture* getCurrentTexture() const; // Changed from sf::Texture*
    void resetPlayback();
    size_t getCurrentFrameIndex() const; // New getter method

private:
    const AnimationData* currentAnimation_;
    size_t currentFrameIndex_;
    float currentFrameTimerSec_;
    bool finished_;
};

#endif // ANIMATOR_H