# DigiviceRefactor Project - AI Assistant Briefing Document

## Overview
This document provides a comprehensive technical briefing for AI assistants working on the DigiviceRefactor project. It covers the project structure, core systems, file organization, and key architectural patterns to enable effective code assistance.

---

## Project Summary
**DigiviceRefactor** is a digital pet simulation game inspired by Digimon virtual pets, implemented in C++ using SDL2. It features a modular state-based architecture with comprehensive animation systems, asset management, and Digimon creature registry.

**Core Technologies:** C++ (C++17), SDL2, JSON (nlohmann::json)
**Platform:** PC (Windows/Linux), cross-platform design
**Build System:** CMake (assumed from project structure)

---

## Critical File Structure Map

### Primary Source Structure
```
DigiviceRefactor/
├── main.cpp                    # Entry point - initializes Game class
├── src/                        # Implementation files
│   ├── core/                   # Core engine systems
│   │   ├── Game.cpp           # Main game loop, state management
│   │   ├── AssetManager.cpp   # Texture/resource loading
│   │   ├── InputManager.cpp   # Input handling & key mapping
│   │   ├── PlayerData.cpp     # Player state persistence
│   │   └── AnimationManager.cpp # Animation data management
│   ├── states/                # Game state implementations
│   │   ├── MenuState.cpp      # Main menu navigation
│   │   ├── AdventureState.cpp # Main game world
│   │   ├── BattleState.cpp    # Combat system
│   │   ├── PartnerSelectState.cpp # Digimon selection
│   │   ├── SettingsState.cpp  # Configuration interface
│   │   ├── TransitionState.cpp # Screen transitions
│   │   └── *TestState.cpp     # Debug/testing states
│   ├── graphics/              # Rendering & animation
│   │   ├── Animator.cpp       # Sprite animation playback
│   │   └── TextRenderer.cpp   # Bitmap font rendering
│   ├── entities/              # Game entities
│   │   └── DigimonRegistry.cpp # Digimon definitions
│   ├── ui/                    # User interface
│   └── utils/                 # Utility classes
│       └── ConfigManager.cpp  # JSON configuration
├── include/                   # Header files (mirrors src/ structure)
├── assets/                    # Game assets
│   ├── config/               # JSON configuration files
│   ├── graphics/             # Textures, sprites
│   └── animations/           # Animation data JSON
└── documentation/            # Project documentation
```

### Key Header Locations
- `include/core/Game.h` - Main game class interface
- `include/states/GameState.h` - Base state class & StateType enum
- `include/graphics/Animator.h` - Animation system interface
- `include/entities/DigimonRegistry.h` - Digimon data structures

---

## Core Architecture Patterns

### 1. State Management System
**Pattern:** State Stack with RAII
**Key Files:** `Game.cpp`, `GameState.h`, `states/*.cpp`

```cpp
// State management in Game class
std::vector<std::unique_ptr<GameState>> states_;
StateType targetStateAfterFade_;
```

**How It Works:**
- Game maintains a stack of active states
- Only top state receives input/updates
- States can push/pop other states
- Automatic cleanup via smart pointers
- Fade transitions between states

**State Types Available:**
- `MenuState` - Main menu with submenus
- `AdventureState` - Main game world
- `BattleState` - Combat encounters
- `PartnerSelectState` - Digimon selection
- `SettingsState` - Configuration
- `TransitionState` - Screen effects
- `*TestState` - Debug states

### 2. Asset Management
**Pattern:** Centralized Resource Loading
**Key Files:** `AssetManager.cpp/.h`

**Responsibilities:**
- Texture loading and caching
- SDL_Surface to SDL_Texture conversion
- Resource lifecycle management
- Cross-platform file path handling

### 3. Animation System
**Pattern:** Data-Driven Animation
**Key Files:** `AnimationManager.cpp`, `Animator.cpp`, `assets/animations/*.json`

**Components:**
- **AnimationManager**: Loads JSON animation definitions
- **Animator**: Plays animations with frame timing
- **AnimationData**: Frame sequences and timing data

**JSON Structure Example:**
```json
{
  "idle": {
    "frames": [0, 1, 2, 1],
    "frameDuration": 500,
    "loop": true
  }
}
```

### 4. Configuration Management
**Pattern:** JSON-Based Configuration
**Key Files:** `ConfigManager.cpp/.h`, `assets/config/*.json`

**Features:**
- Type-safe configuration access
- Default value fallbacks
- Runtime configuration updates
- Hierarchical JSON structure

---

## Critical Code Patterns & Conventions

### Memory Management
- **Smart Pointers**: `std::unique_ptr` for ownership, `std::shared_ptr` for shared resources
- **RAII**: Resource cleanup in destructors
- **Stack Objects**: Prefer stack allocation when possible

### Error Handling
- **SDL_Log**: Logging system with categories (APPLICATION, RENDER, etc.)
- **Return Values**: Boolean success/failure for init functions
- **Exceptions**: Minimal use, prefer error codes

### Naming Conventions
- **Classes**: PascalCase (`GameState`, `AssetManager`)
- **Methods**: camelCase (`getCurrentState()`, `loadTexture()`)
- **Members**: snake_case with trailing underscore (`is_running_`, `states_`)
- **Constants**: UPPER_SNAKE_CASE (`DEFAULT_SMALL_SCREEN_WIDTH`)

### File Organization
- **Headers**: Forward declarations when possible
- **Includes**: Grouped by local/system, alphabetically sorted
- **Namespace**: No global using directives in headers

---

## Key Game Systems Interconnection

### Game Loop Flow
```
main() 
  → Game::init() 
    → Display init, Asset loading, State setup
  → Game::run() 
    → Input handling → State update → Rendering
  → Game::close() 
    → Cleanup resources
```

### State Transitions
```
MenuState 
  → (Partner Selection) → PartnerSelectState 
  → (Game Start) → AdventureState 
  → (Battle Encounter) → BattleState 
  → (Settings) → SettingsState
```

### Animation Pipeline
```
JSON Animation Data 
  → AnimationManager::loadAnimations() 
  → Animator::setAnimation() 
  → Frame updates in render loop 
  → Sprite rendering
```

### Asset Loading Chain
```
Asset Request 
  → AssetManager::loadTexture() 
  → File system access 
  → SDL_Surface creation 
  → SDL_Texture conversion 
  → Cache storage
```

---

## Development Guidelines for AI Assistants

### When Modifying Core Systems
1. **Game Class**: Changes affect entire application lifecycle
2. **State Classes**: Ensure proper enter/exit handling
3. **AssetManager**: Consider memory implications of texture loading
4. **Animation System**: Maintain JSON data format compatibility

### Common Modification Patterns
1. **Adding New State**: 
   - Create header in `include/states/`
   - Implement in `src/states/`
   - Add to StateType enum
   - Register in Game class

2. **Adding New Digimon**:
   - Update `DigimonRegistry.cpp`
   - Add sprite assets
   - Create animation JSON
   - Test in PlayerTestState

3. **Configuration Changes**:
   - Update JSON in `assets/config/`
   - Modify ConfigManager access
   - Add default values

### Testing Approach
- **Debug States**: Use `PlayerTestState` and `EnemyTestState` for animation testing
- **Live Reloading**: Configuration changes can be tested without rebuild
- **State Isolation**: Test individual states by direct transitions

### Performance Considerations
- **Texture Loading**: Expensive, cache appropriately
- **Animation Updates**: Frame timing affects smoothness
- **State Stack**: Deep stacks may impact performance
- **SDL Rendering**: Minimize texture switches

---

## Common Issues & Solutions

### Build Issues
- **Missing Headers**: Check include paths and forward declarations
- **Linking Errors**: Verify SDL2 library linking
- **Asset Paths**: Use relative paths from executable location

### Runtime Issues
- **Black Screen**: Check texture loading and rendering state
- **No Input Response**: Verify InputManager initialization
- **Animation Freezing**: Check JSON animation data validity
- **State Crashes**: Ensure proper state lifecycle management

### Asset Issues
- **Texture Not Loading**: Check file paths and format support
- **Animation Not Playing**: Verify JSON structure and frame indices
- **Config Not Applied**: Check JSON syntax and ConfigManager calls

---

## Quick Reference Commands

### Key Classes to Understand
```cpp
Game                    // Main application controller
GameState              // Base class for all game states  
AssetManager           // Resource loading and management
AnimationManager       // Animation data loading
Animator               // Animation playback
DigimonRegistry        // Digimon definitions and data
ConfigManager          // Configuration access
InputManager           // Input handling and mapping
```

### Essential Enums
```cpp
StateType              // Available game states
FadeSequenceStep       // Transition effects
TransitionEffectType   // Screen transition types
```

### Configuration Keys
- `display.smallScreenWidth/Height` - Screen dimensions
- `input.*` - Key bindings
- `graphics.*` - Rendering settings

---

## Project Status & Architecture Notes

### Current State
- **Stable Core**: Game loop, state management, asset loading functional
- **Animation System**: Fully implemented with JSON data loading
- **Multiple States**: Menu, adventure, battle, testing states operational
- **Configuration**: JSON-based system with type-safe access

### Areas of Active Development
- Battle system refinement
- Enhanced animation effects
- Additional Digimon implementations
- UI system improvements

### Technical Debt Areas
- Some states may need refactoring for consistency
- Asset loading could be optimized
- Error handling could be more comprehensive

This briefing should provide sufficient context for effective AI assistance on the DigiviceRefactor project. The modular architecture allows for focused development on individual systems while maintaining overall project coherence.
