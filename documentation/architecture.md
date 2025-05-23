# Technical Architecture

## System Overview

DigiviceRefactor uses a state-based architecture with modular graphics, audio, and input systems. The design prioritizes maintainability, cross-platform compatibility, and authentic virtual pet mechanics.

## Core Architecture

### State Management System

The application uses a finite state machine pattern where each game mode is represented by a distinct state:

```
GameStateManager
├── MenuState          # Main menu navigation
├── VPetState         # Virtual pet care interface  
├── AdventureState    # Map exploration and events
├── BattleState       # Combat encounters
├── SettingsState     # Configuration options
└── TransitionState   # State change animations
```

**Key Components:**
- `GameStateManager` - Controls state transitions and lifecycle
- `GameState` (base class) - Defines common state interface
- State-specific implementations handle their own rendering and input

### Graphics System

**Multi-Layer Rendering Pipeline:**
1. **Background Layer** - Environment textures with parallax support
2. **Sprite Layer** - Character and object sprites with animations
3. **UI Layer** - Interface elements and text overlays
4. **Effect Layer** - Particles and screen effects

**Key Classes:**
- `PCDisplay` - Platform-specific rendering implementation
- `SpriteManager` - Sprite loading and animation control
- `BackgroundRenderer` - Multi-layer background system
- `TextRenderer` - Font atlas-based text rendering

### Asset Management

**Unified Asset Pipeline:**
- JSON-based asset definitions
- Automatic resource loading and caching
- Platform-agnostic file path handling
- Memory-efficient texture management

**Asset Structure:**
```
assets/
├── sprites/
│   ├── pets/           # Pet sprites and animations
│   ├── ui/             # Interface elements
│   └── effects/        # Visual effects
├── backgrounds/
│   ├── environments/   # Multi-layer backgrounds
│   └── patterns/       # Tiled textures
├── ui/
│   └── fonts/          # Font atlases and definitions
└── audio/
    ├── sfx/            # Sound effects
    └── music/          # Background music
```

## Key Systems

### Virtual Pet Engine

**Core Components:**
- `VPet` - Pet state and behavior logic
- `PetStats` - Health, hunger, happiness tracking
- `EvolutionSystem` - Growth and transformation logic
- `CareSystem` - Feeding, training, and interaction

**Data Flow:**
```
User Input → VPetState → VPet Logic → Stat Updates → Rendering
```

### Battle System

**Combat Architecture:**
- Turn-based state machine
- Type advantage calculations
- Animation queuing system
- Result processing and rewards

**Key Classes:**
- `BattleState` - Combat state controller
- `BattleEngine` - Combat logic and calculations
- `AttackSystem` - Move execution and effects
- `BattleAnimator` - Combat visual effects

### Save System

**Data Persistence:**
- JSON-based save format
- Automatic state serialization
- Cross-platform file handling
- Backup and recovery support

## Platform Abstraction

### Display System
- `PCDisplay` - Desktop implementation (SDL2)
- `RP2350Display` - Hardware implementation (planned)
- Common interface for rendering operations

### Input Handling
- Abstract input events
- Platform-specific input mapping
- Hardware button simulation

### Audio System
- SDL2_mixer integration
- Sound effect management
- Background music streaming
- Volume and settings control

## Performance Considerations

### Memory Management
- Asset caching strategies
- Texture memory optimization
- State-specific resource loading
- Garbage collection for unused assets

### Rendering Optimization
- Sprite batching
- Dirty region updates
- Frame rate limiting
- Resolution scaling

## Build System

### CMake Configuration
- Cross-platform build support
- Dependency management
- Asset pipeline integration
- Debug/release configurations

### Dependencies
- **SDL2** - Graphics, input, and audio
- **SDL2_mixer** - Advanced audio features
- **nlohmann/json** - JSON parsing
- **fmt** - String formatting

## Code Organization

### Namespace Structure
```cpp
namespace DigiviceRefactor {
    namespace Core { /* Core game systems */ }
    namespace Graphics { /* Rendering components */ }
    namespace Audio { /* Sound systems */ }
    namespace States { /* Game state implementations */ }
    namespace Utils { /* Utility functions */ }
}
```

### Header Organization
- Core interfaces in `include/core/`
- Platform code in `include/platform/`
- System-specific headers in respective directories
- Forward declarations to minimize dependencies

## Development Patterns

### Error Handling
- Exception-based error reporting
- Graceful degradation for missing assets
- Comprehensive logging system
- Debug assertions for development

### Testing Strategy
- Unit tests for core logic
- Integration tests for systems
- Asset validation tools
- Platform-specific testing

This architecture provides a solid foundation for the virtual pet simulator while maintaining flexibility for future enhancements and platform ports.
