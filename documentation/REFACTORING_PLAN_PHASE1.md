# Phase 1: Performance & Stability Refactoring

## 1.1 Background Scrolling System Refactor

### Current Problems
- **Seaming Issues**: Overlap-based tiling causes visual artifacts
- **Performance Hitches**: Complex scaling calculations each frame  
- **VSYNC Problems**: Frame rate instability during scrolling

### Solutions

#### Replace Overlap Tiling with Seamless Tiling
```cpp
// Current problematic approach in AdventureState::renderScaledBackgroundLayer
float effectiveTileWidth = static_cast<float>(originalTextureWidth) * (2.0f / 3.0f);

// New seamless approach
class SeamlessScrollingRenderer {
    struct LayerCache {
        SDL_Texture* scaledTexture = nullptr;
        int scaledWidth = 0;
        int scaledHeight = 0;
        float scrollPosition = 0.0f;
    };
    
    std::vector<LayerCache> layerCaches_;
    
public:
    void preScaleTextures(const std::vector<SDL_Texture*>& textures, float targetScale);
    void renderSeamlessLayer(PCDisplay& display, int layerIndex, float scrollOffset);
    void updateScrollPositions(float deltaTime);
};
```

#### Pre-Scaled Texture Caching
- Cache scaled textures at startup instead of scaling each frame
- Use render targets for pre-scaled backgrounds
- Implement texture streaming for large environments

#### Optimized Scrolling Algorithm
```cpp
// New efficient scrolling with pre-calculated positions
void SeamlessScrollingRenderer::renderSeamlessLayer(PCDisplay& display, int layerIndex, float scrollOffset) {
    auto& cache = layerCaches_[layerIndex];
    
    // Simple modulo-based positioning (no overlap calculations)
    float normalizedOffset = fmod(scrollOffset, static_cast<float>(cache.scaledWidth));
    if (normalizedOffset < 0) normalizedOffset += cache.scaledWidth;
    
    // Draw only 2 tiles maximum with simple positioning
    int baseX = static_cast<int>(-normalizedOffset);
    
    SDL_Rect dest1 = {baseX, 0, cache.scaledWidth, cache.scaledHeight};
    SDL_Rect dest2 = {baseX + cache.scaledWidth, 0, cache.scaledWidth, cache.scaledHeight};
    
    display.drawTexture(cache.scaledTexture, nullptr, &dest1);
    if (dest2.x < display.getWidth()) {
        display.drawTexture(cache.scaledTexture, nullptr, &dest2);
    }
}
```

## 1.2 Frame Rate Stabilization

### VSYNC Issues Resolution
```cpp
// Add to Game.cpp initialization
class FrameRateManager {
    static constexpr double TARGET_FPS = 60.0;
    static constexpr double TARGET_FRAME_TIME = 1000.0 / TARGET_FPS;
    
    Uint64 lastFrameTime_;
    double frameTimeAccumulator_;
    
public:
    void beginFrame();
    void endFrame();
    bool shouldUpdate() const;
    double getDeltaTime() const;
};

// In Game::run()
FrameRateManager frameManager;
while (running_) {
    frameManager.beginFrame();
    
    if (frameManager.shouldUpdate()) {
        handleInput();
        update(frameManager.getDeltaTime());
    }
    
    render();
    frameManager.endFrame();
}
```

### GPU Performance Optimization
- Implement texture atlasing for sprites
- Batch sprite rendering calls
- Use hardware-accelerated scaling when possible
- Add configurable quality settings

## 1.3 Memory Management Improvements

### Asset Lifecycle Management
```cpp
class AssetManager {
    // Add reference counting for textures
    struct TextureEntry {
        SDL_Texture* texture;
        std::string path;
        int refCount;
        size_t memorySize;
        bool persistent; // Never unload
    };
    
    std::unordered_map<std::string, TextureEntry> textures_;
    size_t totalMemoryUsage_ = 0;
    size_t maxMemoryLimit_ = 512 * 1024 * 1024; // 512MB
    
public:
    void addRef(const std::string& id);
    void removeRef(const std::string& id);
    void garbageCollect(); // Remove unused non-persistent textures
    void setMemoryLimit(size_t limit);
    size_t getMemoryUsage() const { return totalMemoryUsage_; }
};
```

### Background Variant Memory Optimization
- Load only currently needed variants
- Implement variant streaming for large environments
- Add memory pressure callbacks

## 1.4 Enhanced Error Handling

### Centralized Error System
```cpp
class ErrorManager {
public:
    enum class Severity { Info, Warning, Error, Critical };
    
    struct ErrorInfo {
        Severity severity;
        std::string component;
        std::string message;
        std::chrono::system_clock::time_point timestamp;
    };
    
    static void reportError(Severity severity, const std::string& component, 
                          const std::string& message);
    static std::vector<ErrorInfo> getRecentErrors(int count = 50);
    static void setErrorCallback(std::function<void(const ErrorInfo&)> callback);
};

// Usage throughout codebase
if (!texture) {
    ErrorManager::reportError(ErrorManager::Severity::Error, 
                            "AssetManager", 
                            "Failed to load texture: " + path);
    return false;
}
```

### Graceful Degradation
- Fallback assets for missing textures
- Default animations for missing animation data
- Recovery strategies for corrupted saves

## 1.5 Configuration System Enhancement

### Runtime Configuration
```cpp
// Enhanced ConfigManager with change notifications
class ConfigManager {
    struct ConfigValue {
        json value;
        std::vector<std::function<void(const json&)>> callbacks;
    };
    
    std::unordered_map<std::string, ConfigValue> config_;
    
public:
    template<typename T>
    void addChangeListener(const std::string& key, std::function<void(T)> callback);
    
    void saveConfig() const;
    void reloadConfig();
    bool isConfigDirty() const;
};

// Performance settings
struct GraphicsSettings {
    bool vsyncEnabled = true;
    float renderScale = 1.0f;
    int maxFPS = 60;
    bool textureFiltering = true;
    
    void apply(PCDisplay& display);
    static GraphicsSettings fromConfig();
};
```

## Implementation Timeline

### Week 1: Core Performance
- [ ] Implement SeamlessScrollingRenderer
- [ ] Add FrameRateManager to Game class
- [ ] Create pre-scaled texture caching system
- [ ] Basic error reporting integration

### Week 2: Memory & Error Handling  
- [ ] Enhanced AssetManager with reference counting
- [ ] Implement ErrorManager throughout codebase
- [ ] Add graceful degradation for missing assets
- [ ] Memory usage monitoring and limits

### Week 3: Configuration & Polish
- [ ] Enhanced ConfigManager with change notifications
- [ ] Graphics settings system
- [ ] Performance profiling tools
- [ ] Testing and validation

## Success Metrics

1. **Performance**: Stable 60 FPS with no VSYNC hitches
2. **Visual Quality**: Eliminate background scrolling seaming
3. **Memory**: Keep total memory usage under 512MB
4. **Stability**: Zero crashes during 30-minute gameplay sessions
5. **Configuration**: All settings changeable without restart

## Risk Mitigation

- Implement changes incrementally with feature flags
- Maintain backward compatibility with legacy asset system
- Add comprehensive logging for performance debugging
- Create automated performance regression tests
