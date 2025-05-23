# DigiviceRefactor: Technical Architecture

## System Architecture Overview

The DigiviceRefactor project follows a modular, component-based architecture designed for maintainability and extensibility. The core engine provides fundamental services while game-specific logic is implemented in distinct subsystems.

## Core Engine Components

### 1. Game Class (`src/core/Game.cpp`)
**Central coordinator and main game loop**

Key Responsibilities:
- SDL2 initialization and window management
- Game state stack management  
- Asset loading coordination
- Main update/render loop
- System initialization

```cpp
class Game {
    std::stack<std::unique_ptr<GameState>> stateStack_;
    std::unique_ptr<AssetManager> assetManager_;
    std::unique_ptr<AnimationManager> animationManager_;
    std::unique_ptr<TextRenderer> textRenderer_;
    PCDisplay display_;
    InputManager inputManager_;
};
```

### 2. AssetManager (`src/core/AssetManager.cpp`)
**Resource loading and management system**

Features:
- SDL_Texture loading from PNG files
- Texture caching with string identifiers
- Automatic resource cleanup
- Error handling and logging

```cpp
// Usage example
bool success = assetManager.loadTexture("agumon", "assets/sprites/player_digimon/agumon.png");
SDL_Texture* texture = assetManager.getTexture("agumon");
```

### 3. AnimationManager (`src/core/AnimationManager.cpp`)
**Animation data loading and management**

Architecture:
- JSON-based animation definitions
- Frame rectangle parsing from sprite sheets
- Animation data caching
- Integration with AssetManager for textures

```cpp
struct AnimationData {
    std::string id;
    SDL_Texture* textureAtlas;
    std::vector<SDL_Rect> frameRects;
    std::vector<float> frameDurationsSec;
    bool loops;
};
```

JSON Format:
```json
{
  "frames": {
    "Agumon_0": {"frame": {"x": 0, "y": 0, "w": 32, "h": 32}},
    "Agumon_1": {"frame": {"x": 32, "y": 0, "w": 32, "h": 32}}
  }
}
```

### 4. State Management System
**Hierarchical game state architecture**

Base State Class:
```cpp
class GameState {
public:
    virtual void enter() = 0;
    virtual void exit() = 0;
    virtual void handle_input(InputManager&, PlayerData*) = 0;
    virtual void update(float deltaTime, PlayerData*) = 0;
    virtual void render(PCDisplay&) = 0;
    virtual StateType getType() const = 0;
protected:
    Game* game_ptr;
};
```

State Stack Operations:
- `requestPushState()`: Add new state to stack
- `requestPopState()`: Remove current state
- `requestPopUntil()`: Pop to specific state type

## Graphics and Rendering

### 1. Animator System (`src/graphics/Animator.cpp`)
**Frame-based animation playback**

Features:
- Automatic frame timing and progression
- Looping and non-looping animations
- Current frame and texture access
- Animation state management

```cpp
class Animator {
    const AnimationData* currentAnimation_;
    size_t currentFrameIndex_;
    float frameTimer_;
    bool isPlaying_;
public:
    void setAnimation(const AnimationData* animData);
    void update(float deltaTime);
    SDL_Texture* getCurrentTexture() const;
    SDL_Rect getCurrentFrameRect() const;
};
```

### 2. TextRenderer (`src/ui/TextRenderer.cpp`)
**Bitmap font rendering system**

Architecture:
- JSON-defined character mappings
- Texture atlas-based glyph storage
- Scalable text rendering
- Kerning and spacing control

Character Mapping:
```json
{
  "frames": {
    "A": {"frame": {"x": 0, "y": 0, "w": 16, "h": 16}},
    "B": {"frame": {"x": 16, "y": 0, "w": 16, "h": 16}}
  }
}
```

### 3. Display Abstraction (`src/platform/pc/pc_display.cpp`)
**Cross-platform rendering interface**

Capabilities:
- SDL2 renderer abstraction
- Texture drawing operations
- Window management
- Color and blend mode control

## Entity Management

### 1. DigimonRegistry (`src/entities/DigimonRegistry.cpp`)
**Centralized Digimon database**

Design Pattern: Singleton with registry pattern
- Thread-safe access
- JSON-based definitions
- Runtime registration support
- Classification-based queries

```cpp
class DigimonRegistry {
public:
    static DigimonRegistry& getInstance();
    const DigimonDefinition* getDefinitionById(const std::string& id);
    std::vector<const DigimonDefinition*> getDefinitionsByClass(DigimonClass);
private:
    std::map<std::string, std::unique_ptr<DigimonDefinition>> definitions_;
};
```

### 2. DigimonDefinition (`include/entities/DigimonDefinition.h`)
**Digimon data structure**

```cpp
struct DigimonDefinition {
    std::string id;                    // Unique identifier
    std::string displayName;           // User-visible name
    DigimonClass digimonClass;         // Classification
    DigimonStats stats;                // Combat statistics
    std::string spriteBaseId;          // Animation asset reference
    std::string description;           // Flavor text
    std::vector<std::string> animations; // Supported animations
    DigimonType legacyEnum;            // Backward compatibility
};
```

## Input and Configuration

### 1. InputManager (`src/core/InputManager.cpp`)
**Input abstraction and key binding**

Features:
- Action-based input mapping
- Customizable key bindings
- State tracking (pressed, just pressed, released)
- Configuration persistence

```cpp
enum class GameAction {
    NAV_UP, NAV_DOWN, NAV_LEFT, NAV_RIGHT,
    CONFIRM, CANCEL, MENU_TOGGLE,
    TOGGLE_SCREEN_SIZE
};
```

### 2. ConfigManager (`src/utils/ConfigManager.cpp`)
**JSON-based configuration system**

Architecture:
- Thread-safe singleton
- Template-based value access
- Automatic file persistence
- Default value support

```cpp
// Usage examples
float textScale = ConfigManager::getValue<float>("ui.textScale", 1.0f);
ConfigManager::setValue("graphics.fullscreen", true);
ConfigManager::saveChanges();
```

## Data Flow Architecture

### 1. Initialization Sequence
1. **SDL2 Setup**: Graphics, audio, input systems
2. **Asset Loading**: Textures, fonts, backgrounds
3. **Registry Setup**: Load Digimon definitions
4. **Animation Loading**: Parse animation JSON files
5. **Initial State**: Push MenuState onto stack

### 2. Game Loop
```
Input → State Stack → Update → Render → Present
  ↓         ↓           ↓        ↓        ↓
Handle → Current → Animation → Graphics → Display
Events   State    Updates     Drawing    Buffer
```

### 3. State Transitions
```
MenuState → PartnerSelectState → AdventureState
    ↓              ↓                  ↓
  Debug         Choose             Game World
 Submenus       Partner           + Battle States
```

## Memory Management

### 1. Resource Ownership
- **AssetManager**: Owns all SDL_Texture objects
- **AnimationManager**: Owns AnimationData instances
- **DigimonRegistry**: Owns DigimonDefinition objects
- **Game**: Owns all manager instances

### 2. Smart Pointers
- `std::unique_ptr` for single ownership
- `std::shared_ptr` where resource sharing needed
- Raw pointers for non-owning references

### 3. RAII Principles
- Automatic resource cleanup
- Exception-safe resource management
- Clear ownership semantics

## Threading Model

**Single-threaded architecture** with potential for:
- Asynchronous asset loading
- Background audio processing
- Network communication (future)

## Error Handling

### 1. Logging System
SDL2 logging with categories:
- `SDL_LOG_CATEGORY_APPLICATION`: General game events
- `SDL_LOG_CATEGORY_RENDER`: Graphics operations
- `SDL_LOG_CATEGORY_INPUT`: Input handling

### 2. Graceful Degradation
- Missing assets trigger warnings but don't crash
- Unknown configurations use default values
- Failed state transitions are logged and handled

## Performance Considerations

### 1. Asset Loading
- All textures loaded at startup
- Animation data cached in memory
- No runtime file I/O during gameplay

### 2. Rendering Optimization
- Texture sharing between objects
- Minimal state changes
- Efficient sprite batching

### 3. Update Efficiency
- Delta-time based animations
- State-specific update logic
- Input polling optimization

This architecture provides a solid foundation for the digital pet simulation while maintaining clean separation of concerns and extensibility for future features.
