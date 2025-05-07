// src/graphics/Animator.cpp
#include "graphics/Animator.h"
#include <SDL_log.h>

Animator::Animator()
    : currentAnimation_(nullptr)
    , currentFrameIndex_(0)
    , currentFrameTimerSec_(0.0f)
    , finished_(false)
{
}

void Animator::setAnimation(const AnimationData* animation, bool resetPlayback)
{
    if (animation && animation->isValid() && currentAnimation_ != animation) {
        currentAnimation_ = animation;
        if (resetPlayback) {
            this->resetPlayback();
        }
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Animator: Set animation to '%s'", currentAnimation_->id.c_str());
    } else if (!animation) {
        stop();
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Animator: setAnimation called with null, stopping.");
    } else if (currentAnimation_ == animation) {
        if (resetPlayback) {
            this->resetPlayback();
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Animator: Resetting animation '%s'", currentAnimation_->id.c_str());
        }
    } else if (animation && !animation->isValid()) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Animator: Attempted to set invalid AnimationData (ID: '%s')", animation->id.c_str());
        stop();
    }
}

void Animator::resetPlayback()
{
    currentFrameIndex_ = 0;
    currentFrameTimerSec_ = 0.0f;
    finished_ = false;
}

void Animator::update(float deltaTime) {
    if (!currentAnimation_ || finished_) {
        return;
    }

    size_t frameCount = currentAnimation_->getFrameCount();
    if (frameCount == 0) {
        finished_ = true;
        return;
    }

    if (currentFrameIndex_ >= frameCount) {
        currentFrameIndex_ = 0;
        currentFrameTimerSec_ = 0.0f;
    }

    currentFrameTimerSec_ += deltaTime;

    float currentFrameDuration = currentAnimation_->frameDurationsSec[currentFrameIndex_];

    while (currentFrameTimerSec_ >= currentFrameDuration) {
        currentFrameTimerSec_ -= currentFrameDuration;

        currentFrameIndex_++;

        if (currentFrameIndex_ >= frameCount) {
            if (currentAnimation_->loops) {
                currentFrameIndex_ = 0;
                currentFrameDuration = currentAnimation_->frameDurationsSec[currentFrameIndex_];
            } else {
                currentFrameIndex_ = frameCount - 1;
                finished_ = true;
                currentFrameTimerSec_ = 0.0f;
                break;
            }
        } else {
            currentFrameDuration = currentAnimation_->frameDurationsSec[currentFrameIndex_];
        }

        if (currentFrameDuration <= 0.0f) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Animator: Animation '%s' frame %zu has zero or negative duration!", 
                currentAnimation_->id.c_str(), currentFrameIndex_);
            if (currentAnimation_->loops) {
                break;
            }
        }
    }
}

SDL_Rect Animator::getCurrentFrameRect() const
{
    if (!currentAnimation_ || currentFrameIndex_ >= currentAnimation_->getFrameCount())
    {
        return SDL_Rect{0, 0, 0, 0};
    }
    return currentAnimation_->frameRects[currentFrameIndex_];
}

SDL_Texture* Animator::getCurrentTexture() const
{
    if (!currentAnimation_)
    {
        return nullptr;
    }
    return currentAnimation_->textureAtlas;
}

void Animator::stop()
{
    currentAnimation_ = nullptr;
    currentFrameIndex_ = 0;
    currentFrameTimerSec_ = 0.0f;
    finished_ = true;
}