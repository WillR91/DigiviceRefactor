#include "core/FrameRateManager.h"
#include <SDL.h>
#include <algorithm>
#include <numeric>
#include <thread>
#include <iostream>

FrameRateManager::FrameRateManager(TargetFrameRate targetFPS, VSyncMode vsync)
    : targetFrameRate_(targetFPS)
    , vsyncMode_(vsync)
    , enableFrameSkipping_(false)
    , enableSmoothing_(true)
    , frameCount_(0)
    , accumulatedTime_(0.0f)
    , targetFrameTime_(1.0f / 60.0f) // Default to 60 FPS
    , smoothedFrameTime_(1.0f / 60.0f)
    , isInitialized_(false)
    , vsyncHitchThreshold_(0.020f) // 20ms threshold for hitch detection
    , consecutiveHitches_(0)
    , currentStats_{}
{
    updateTargetFrameTime();
    lastFrameTime_ = std::chrono::high_resolution_clock::now();
}

void FrameRateManager::beginFrame() {
    frameStartTime_ = std::chrono::high_resolution_clock::now();
    
    if (!isInitialized_) {
        lastFrameTime_ = frameStartTime_;
        isInitialized_ = true;
    }
}

void FrameRateManager::endFrame() {
    auto now = std::chrono::high_resolution_clock::now();
    auto frameDuration = now - lastFrameTime_;
    float frameTime = std::chrono::duration<float>(frameDuration).count();
    
    // Store frame time for statistics
    frameTimes_.push_back(frameTime);
    trimFrameHistory();
    
    // Update statistics
    frameCount_++;
    accumulatedTime_ += frameTime;
    updateFrameStats();
    detectVSyncHitches();
    
    // Update smoothed frame time
    if (enableSmoothing_) {
        smoothedFrameTime_ = calculateSmoothFrameTime();
    } else {
        smoothedFrameTime_ = frameTime;
    }
    
    lastFrameTime_ = now;
}

bool FrameRateManager::shouldSkipFrame() const {
    if (!enableFrameSkipping_) {
        return false;
    }
    
    // Skip frame if we're significantly behind target
    float currentFrameTime = getLastFrameTime();
    return currentFrameTime > (targetFrameTime_ * 1.5f);
}

void FrameRateManager::setTargetFrameRate(TargetFrameRate fps) {
    if (targetFrameRate_ != fps) {
        targetFrameRate_ = fps;
        updateTargetFrameTime();
        resetStats();
    }
}

void FrameRateManager::setVSyncMode(VSyncMode mode) {
    vsyncMode_ = mode;
    
    // Apply VSync setting to SDL
    int vsyncValue = static_cast<int>(mode);
    if (SDL_GL_SetSwapInterval(vsyncValue) != 0) {
        std::cerr << "FrameRateManager: Failed to set VSync mode: " << SDL_GetError() << std::endl;
    }
}

float FrameRateManager::getCurrentFPS() const {
    if (frameTimes_.empty()) {
        return 0.0f;
    }
    
    // Calculate FPS from recent frame times
    float avgFrameTime = std::accumulate(frameTimes_.begin(), frameTimes_.end(), 0.0f) / frameTimes_.size();
    return avgFrameTime > 0.0f ? 1.0f / avgFrameTime : 0.0f;
}

float FrameRateManager::getLastFrameTime() const {
    if (frameTimes_.empty()) {
        return 0.0f;
    }
    return frameTimes_.back();
}

void FrameRateManager::waitForTargetFrameTime() {
    if (vsyncMode_ != VSyncMode::OFF) {
        // VSync is handling timing, no need to wait
        return;
    }
    
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = now - frameStartTime_;
    auto elapsedSeconds = std::chrono::duration<float>(elapsed).count();
    
    float remainingTime = targetFrameTime_ - elapsedSeconds;
    if (remainingTime > 0.0f) {
        // Convert to milliseconds for more precise sleeping
        auto sleepDuration = std::chrono::duration<float>(remainingTime * 0.9f); // 90% to avoid overshooting
        std::this_thread::sleep_for(sleepDuration);
        
        // Busy wait for the remaining time for higher precision
        while (true) {
            now = std::chrono::high_resolution_clock::now();
            elapsed = now - frameStartTime_;
            elapsedSeconds = std::chrono::duration<float>(elapsed).count();
            if (elapsedSeconds >= targetFrameTime_) {
                break;
            }
        }
    }
}

void FrameRateManager::resetStats() {
    currentStats_ = {};
    frameCount_ = 0;
    accumulatedTime_ = 0.0f;
    frameTimes_.clear();
    consecutiveHitches_ = 0;
}

int FrameRateManager::getDisplayRefreshRate() {
    SDL_DisplayMode mode;
    if (SDL_GetCurrentDisplayMode(0, &mode) == 0) {
        return mode.refresh_rate;
    }
    return 60; // Default fallback
}

void FrameRateManager::updateFrameStats() {
    if (frameTimes_.empty()) {
        return;
    }
    
    // Calculate basic statistics
    float sum = std::accumulate(frameTimes_.begin(), frameTimes_.end(), 0.0f);
    currentStats_.averageFPS = frameTimes_.size() / sum;
    
    auto minMax = std::minmax_element(frameTimes_.begin(), frameTimes_.end());
    currentStats_.minFPS = 1.0f / *minMax.second; // Max frame time = Min FPS
    currentStats_.maxFPS = 1.0f / *minMax.first;  // Min frame time = Max FPS
    
    // Calculate variance
    float avgFrameTime = sum / frameTimes_.size();
    float variance = 0.0f;
    for (float frameTime : frameTimes_) {
        float diff = frameTime - avgFrameTime;
        variance += diff * diff;
    }
    currentStats_.frameTimeVariance = variance / frameTimes_.size();
    
    // Determine stability
    float fpsVariation = currentStats_.maxFPS - currentStats_.minFPS;
    currentStats_.isStable = (fpsVariation < 5.0f) && (currentStats_.frameTimeVariance < 0.001f);
    
    currentStats_.targetFrameTime = targetFrameTime_;
}

void FrameRateManager::detectVSyncHitches() {
    if (frameTimes_.empty() || vsyncMode_ == VSyncMode::OFF) {
        return;
    }
    
    float lastFrameTime = frameTimes_.back();
    
    // Detect hitch (frame time significantly longer than target)
    if (lastFrameTime > targetFrameTime_ + vsyncHitchThreshold_) {
        consecutiveHitches_++;
        currentStats_.vsyncHitches++;
        
        // If we have too many consecutive hitches, consider disabling VSync temporarily
        if (consecutiveHitches_ >= MAX_CONSECUTIVE_HITCHES) {
            std::cout << "FrameRateManager: Detected consecutive VSync hitches, performance may be unstable" << std::endl;
        }
    } else {
        consecutiveHitches_ = 0;
    }
}

void FrameRateManager::updateTargetFrameTime() {
    switch (targetFrameRate_) {
        case TargetFrameRate::FPS_30:
            targetFrameTime_ = 1.0f / 30.0f;
            break;
        case TargetFrameRate::FPS_60:
            targetFrameTime_ = 1.0f / 60.0f;
            break;
        case TargetFrameRate::FPS_120:
            targetFrameTime_ = 1.0f / 120.0f;
            break;
        case TargetFrameRate::ADAPTIVE:
            {
                int refreshRate = getDisplayRefreshRate();
                targetFrameTime_ = 1.0f / refreshRate;
            }
            break;
    }
}

float FrameRateManager::calculateSmoothFrameTime() const {
    if (frameTimes_.empty()) {
        return targetFrameTime_;
    }
    
    // Use a simple exponential moving average
    const float alpha = 0.1f; // Smoothing factor
    float currentFrameTime = frameTimes_.back();
    return (alpha * currentFrameTime) + ((1.0f - alpha) * smoothedFrameTime_);
}

void FrameRateManager::trimFrameHistory() {
    while (frameTimes_.size() > MAX_FRAME_HISTORY) {
        frameTimes_.pop_front();
    }
}
