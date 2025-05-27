#pragma once

#include <chrono>
#include <deque>
#include <memory>

/**
 * FrameRateManager - Provides consistent frame rate management and VSYNC stability
 * 
 * Key features:
 * - Adaptive frame rate targeting (30/60/120 FPS)
 * - VSYNC hitch detection and mitigation
 * - Frame time smoothing and prediction
 * - Performance monitoring and diagnostics
 */
class FrameRateManager {
public:
    enum class TargetFrameRate {
        FPS_30 = 30,
        FPS_60 = 60,
        FPS_120 = 120,
        ADAPTIVE = 0    // Automatically adjust based on display capabilities
    };
    
    enum class VSyncMode {
        OFF = 0,
        ON = 1,
        ADAPTIVE = -1   // Enable adaptive vsync if available
    };

private:
    using TimePoint = std::chrono::high_resolution_clock::time_point;
    using Duration = std::chrono::high_resolution_clock::duration;
    
    struct FrameStats {
        float averageFPS;
        float minFPS;
        float maxFPS;
        float frameTimeVariance;
        int droppedFrames;
        int vsyncHitches;
        float targetFrameTime;
        bool isStable;
    };
    
    // Configuration
    TargetFrameRate targetFrameRate_;
    VSyncMode vsyncMode_;
    bool enableFrameSkipping_;
    bool enableSmoothing_;
    
    // Timing
    TimePoint lastFrameTime_;
    TimePoint frameStartTime_;
    std::deque<float> frameTimes_;
    static constexpr size_t MAX_FRAME_HISTORY = 120;
    
    // Performance tracking
    FrameStats currentStats_;
    int frameCount_;
    float accumulatedTime_;
    
    // Adaptive behavior
    float targetFrameTime_;
    float smoothedFrameTime_;
    bool isInitialized_;
    
    // VSYNC hitch detection
    float vsyncHitchThreshold_;
    int consecutiveHitches_;
    static constexpr int MAX_CONSECUTIVE_HITCHES = 3;

public:
    FrameRateManager(TargetFrameRate targetFPS = TargetFrameRate::FPS_60, 
                     VSyncMode vsync = VSyncMode::ON);
    ~FrameRateManager() = default;
    
    // Frame timing
    void beginFrame();
    void endFrame();
    bool shouldSkipFrame() const;
    
    // Configuration
    void setTargetFrameRate(TargetFrameRate fps);
    void setVSyncMode(VSyncMode mode);
    void setFrameSkippingEnabled(bool enabled) { enableFrameSkipping_ = enabled; }
    void setSmoothingEnabled(bool enabled) { enableSmoothing_ = enabled; }
    
    // Information
    TargetFrameRate getTargetFrameRate() const { return targetFrameRate_; }
    VSyncMode getVSyncMode() const { return vsyncMode_; }
    const FrameStats& getFrameStats() const { return currentStats_; }
    float getCurrentFPS() const;
    float getTargetFrameTime() const { return targetFrameTime_; }
    float getLastFrameTime() const;
    
    // Diagnostics
    bool isFrameRateStable() const { return currentStats_.isStable; }
    bool hasVSyncHitches() const { return currentStats_.vsyncHitches > 0; }
    void resetStats();
    
    // Utility
    void waitForTargetFrameTime();
    static int getDisplayRefreshRate(); // Get display refresh rate if available

private:
    void updateFrameStats();
    void detectVSyncHitches();
    void updateTargetFrameTime();
    float calculateSmoothFrameTime() const;
    void trimFrameHistory();
};
