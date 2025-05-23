# Refactoring Ideas for DigiviceRefactor Project

*Created on May 14, 2025*

## Overview
This document outlines recommended refactoring opportunities for the DigiviceSim project to improve code quality, maintainability, and set a strong foundation for future development without breaking existing functionality.

## 1. Configuration System

### Current State
The project has hard-coded values scattered throughout (screen dimensions, file paths, animation parameters).

### Recommendation
Implement a JSON-based configuration system:

```cpp
// include/utils/ConfigManager.h
class ConfigManager {
public:
    static bool initialize(const std::string& configPath = "assets/config/game_config.json");
    static void shutdown();
    
    template<typename T>
    static T getValue(const std::string& key, const T& defaultValue);
    
    static bool setValue(const std::string& key, const nlohmann::json& value);
    static bool saveChanges();
};

// Usage example:
int screenWidth = ConfigManager::getValue<int>("display.width", 466);
```

### Benefits
- Adjust game parameters without recompilation
- Easier testing of different configurations
- Better separation of code and configuration
- User-configurable settings become possible

## 2. Error Handling System

### Current State
Error handling is inconsistent, relying on boolean returns and SDL_Log calls.

### Recommendation
Create a centralized error handling system:

```cpp
// include/utils/ErrorHandler.h
class ErrorHandler {
public:
    enum class Severity { INFO, WARNING, ERROR, FATAL };
    
    static void logMessage(Severity severity, const std::string& component, const std::string& message);
    static void setLogLevel(Severity minLevel);
    
    // Optional: error collection for UI display
    static std::vector<std::string> getRecentErrors();
};

// Usage:
ErrorHandler::logMessage(ErrorHandler::Severity::ERROR, "AssetManager", "Failed to load texture: " + filename);
```

### Benefits
- Consistent error reporting across the codebase
- Centralized control of error logging behavior
- Foundation for error recovery mechanisms
- Better debugging experience

## 3. Asset Management Enhancements

### Current State
The AssetManager loads all assets at startup, which can lead to memory inefficiency.

### Recommendation
Implement on-demand loading and resource caching:

```cpp
// Enhanced AssetManager with resource reference counting
class AssetManager {
public:
    // Existing methods...
    
    // New methods:
    bool preloadAsset(const std::string& id, const std::string& path);
    void releaseUnusedAssets();
    void setAssetCacheSize(size_t maxMegabytes);
    
private:
    struct AssetInfo {
        SDL_Texture* texture;
        int referenceCount;
        std::time_t lastUsed;
    };
    std::unordered_map<std::string, AssetInfo> assets_;
};
```

### Benefits
- Reduced memory usage
- Faster startup time
- Better performance on memory-constrained devices
- Foundation for level/area streaming

## 4. Consistent Component Lifecycle

### Current State
Classes have inconsistent initialization and cleanup patterns.

### Recommendation
Standardize component lifecycle with init/shutdown pattern:

```cpp
// Abstract base class for components with consistent lifecycle
class GameComponent {
public:
    virtual ~GameComponent() = default;
    
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    
    bool isInitialized() const { return initialized_; }
    
protected:
    bool initialized_ = false;
};
```

### Benefits
- More predictable component behavior
- Easier to identify initialization issues
- Better cleanup in error scenarios
- Simplified integration of new components

## 5. Input System Enhancement

### Current State
The InputManager is tightly coupled to specific key bindings and lacks flexibility for customization.

### Recommendation
Create an action-based input system with configurable bindings:

```cpp
// Enhanced InputManager
class InputManager {
public:
    // Existing methods...
    
    // New methods:
    bool loadBindingsFromConfig(const std::string& configPath = "assets/config/input_bindings.json");
    bool saveBindingToConfig();
    bool rebindAction(GameAction action, SDL_Scancode key);
    std::string getBindingName(GameAction action);
};
```

### Benefits
- User-configurable controls
- Easier to adapt for different input devices
- Better separation of input detection and game actions
- Foundation for controller support

## 6. Standardized Logging System

### Current State
The codebase uses SDL_Log extensively but with inconsistent categories and verbosity levels.

### Recommendation
Create a custom logging wrapper with consistent categories:

```cpp
// include/utils/Logger.h
class Logger {
public:
    enum class Category { CORE, RENDER, INPUT, AUDIO, STATE, ASSET, UI, NETWORK };
    enum class Level { VERBOSE, DEBUG, INFO, WARNING, ERROR, CRITICAL };
    
    static void setLevel(Level level);
    static void log(Category category, Level level, const std::string& message);
    
    // Helper methods
    static void info(Category category, const std::string& message);
    static void warn(Category category, const std::string& message);
    static void error(Category category, const std::string& message);
};

// Usage:
Logger::info(Logger::Category::ASSET, "Loaded texture: " + filename);
```

### Benefits
- Consistent log format and categories
- Better filtering of log messages
- Potential for log output to multiple destinations
- Improved debugging experience

## 7. State Management Refactoring

### Current State
The game state system works well but has complex transition logic with fade sequences.

### Recommendation
Implement a state transition manager:

```cpp
// include/core/StateTransitionManager.h
class StateTransitionManager {
public:
    struct TransitionOptions {
        float duration = 0.3f;
        TransitionEffectType effect = TransitionEffectType::FADE;
        bool popCurrent = true;
    };
    
    StateTransitionManager(Game* game);
    
    void transition(std::unique_ptr<GameState> newState, const TransitionOptions& options = {});
    void update(float deltaTime);
    bool isTransitioning() const;
    
private:
    // Implementation details
};
```

### Benefits
- Cleaner state transition code in Game class
- More maintainable transition effects
- Easier to add new transition types
- Better separation of concerns

## 8. SDL Include Standardization

### Current State
Inconsistent SDL include styles (`<SDL.h>` vs `<SDL_render.h>`, etc.)

### Recommendation
Standardize to either using the umbrella `<SDL.h>` or consistently use specific headers.

### Benefits
- Ensures consistent compilation across all files
- Reduces potential for missing dependencies
- Makes future SDL upgrades easier

## 9. Unit Testing Framework

### Current State
No apparent testing framework or automated tests.

### Recommendation
Set up a testing framework (like Catch2 or Google Test) and add unit tests.

### Benefits
- Confirms code works as expected
- Makes refactoring safer
- Documents expected behavior

## 10. Precompiled Headers

### Current State
Many files include the same headers repeatedly.

### Recommendation
Set up precompiled headers for frequently used headers like SDL.

### Benefits
- Faster compilation times
- Reduced risk of header-related issues

## Implementation Priority

Based on impact vs. effort:

1. **Configuration System** - High impact, moderate effort
2. **Error Handler** - High impact, relatively low effort 
3. **Logger Wrapper** - Medium impact, low effort
4. **Asset Management Enhancements** - High impact, higher effort

## Next Steps

To proceed with implementation, it's recommended to:

1. Start with the Configuration System or Error Handler as they provide significant benefits with reasonable effort
2. Implement the Logger as an early win that supports the other systems
3. Address the other refactorings as time permits, focusing on those that enable subsequent improvements
