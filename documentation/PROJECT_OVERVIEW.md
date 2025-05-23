# DigiviceRefactor Project Overview

## Project Description
DigiviceRefactor is a digital pet simulation game inspired by Digimon virtual pets, implemented in C++ using SDL2. The project features a modular game engine with animation systems, state management, and a comprehensive Digimon registry for managing different creatures and their properties.

## Current Architecture

### Core Systems

#### 1. Game Engine (`src/core/`)
- **Game Class**: Central game loop and state management
- **AssetManager**: Handles texture loading and resource management
- **AnimationManager**: Manages animation data from JSON files and sprite sheets
- **InputManager**: Handles input mapping and key binding configuration
- **ConfigManager**: JSON-based configuration system for settings

#### 2. State Management (`src/states/`)
The game uses a state stack system with the following implemented states:
- **MenuState**: Main menu with navigation and submenus
- **PlayerTestState**: Debug state for testing player Digimon animations
- **EnemyTestState**: Debug state for testing enemy Digimon animations  
- **PartnerSelectState**: Allows selection of partner Digimon
- **SettingsState**: Configuration interface for controls and display settings
- **AdventureState**: Main game world with partner Digimon
- **BattleState**: Combat system with animated sprites
- **TransitionState**: Screen transitions between states

#### 3. Graphics and Animation (`src/graphics/`, `include/graphics/`)
- **Animator**: Handles sprite animation playback with frame timing
- **AnimationData**: Data structure for animation sequences
- **TextRenderer**: Bitmap font rendering system using texture atlases
- **PCDisplay**: Display abstraction layer for rendering

#### 4. Entity System (`src/entities/`)
- **DigimonRegistry**: Centralized registry for all Digimon definitions
- **DigimonDefinition**: Data structure defining Digimon properties
- **PlayerData**: Player save data and partner management

#### 5. UI System (`src/ui/`)
- **TextRenderer**: Bitmap font system with JSON-defined character mappings
- **Menu navigation**: Controller and keyboard input support

## Current Features

### Working Systems

#### 1. Animation System
- **JSON-based animation definitions**: Sprite sheets with frame rectangles and timing
- **Multi-animation support**: Idle, Walk, Attack animations per character
- **Automated loading**: AnimationManager loads from `assets/sprites/` directories
- **Frame-based timing**: Configurable frame durations in milliseconds

#### 2. Digimon Registry
- **81+ enemy Digimon**: Comprehensive database of enemy creatures
- **Player Digimon**: 8 core partners (Agumon, Gabumon, Biyomon, Gatomon, Gomamon, Palmon, Tentomon, Patamon)
- **Unlockable Digimon**: Additional creatures (Veedramon, Wizardmon)
- **Classification system**: PlayerRookie, PlayerChampion, StandardEnemy, Boss, NPC classes
- **Stats and properties**: HP, Attack, Defense, Speed per Digimon

#### 3. Test States
- **PlayerTestState**: 
  - Navigate through player Digimon with LEFT/RIGHT arrows
  - Cycle between Idle/Walk/Attack animations with UP/DOWN
  - Real-time animation display with Digimon names
  - Support for registry-based definitions and fallback to hardcoded list
- **EnemyTestState**: Similar functionality for enemy Digimon

#### 4. Text Rendering
- **Bitmap font system**: Custom blue/white font atlas
- **JSON character mapping**: Flexible character-to-texture-region mapping
- **Scaling support**: Global and per-text scaling
- **Kerning control**: Adjustable character spacing
- **Fallback system**: Uppercase fallback for missing lowercase characters

#### 5. Settings and Configuration
- **Key binding system**: Customizable controls for all game actions
- **Text scale adjustment**: Real-time text size modification
- **Configuration persistence**: JSON-based config file saves settings
- **Input validation**: Prevents binding of system keys (ESC, F1, F12)

#### 6. Asset Management
- **Automatic loading**: Textures and animations loaded based on registry definitions
- **Background textures**: Castle background layers, menu backgrounds
- **UI elements**: Transition borders, mask textures, font atlases

### Menu Structure
```
MAIN MENU
├── ADVENTURE (→ AdventureState)
├── DIGIMON (→ PartnerSelectState)  
├── DEBUG
│   ├── TEST DIGIMON
│   │   ├── ENEMY DIGIMON (→ EnemyTestState)
│   │   ├── PLAYER DIGIMON (→ PlayerTestState)
│   │   └── BACK
│   ├── SETTINGS (→ SettingsState)
│   └── BACK
└── EXIT
```

## Technical Implementation

### Build System
- **CMake-based**: Cross-platform build configuration
- **Multiple build types**: Debug, Release, RelWithDebInfo configurations
- **SDL2 integration**: Graphics, input, and audio through SDL2

### File Structure
```
DigiviceRefactor/
├── src/                 # Source code
│   ├── core/           # Core engine systems
│   ├── states/         # Game states
│   ├── graphics/       # Animation and rendering
│   ├── entities/       # Digimon and game entities
│   ├── ui/             # User interface
│   ├── utils/          # Utility classes
│   └── platform/       # Platform-specific code
├── include/            # Header files (mirrors src structure)
├── assets/             # Game assets
│   ├── sprites/        # Character sprite sheets and JSON
│   ├── backgrounds/    # Background textures
│   ├── ui/             # UI elements and fonts
│   └── definitions/    # Digimon definition files
├── build_*/           # Generated build directories
└── documentation/      # Project documentation
```

### Asset Pipeline
1. **Sprite Processing**: Python script converts individual frames to sprite sheets
2. **Animation Definition**: JSON files define frame rectangles and timing
3. **Texture Loading**: AssetManager loads PNG files as SDL_Texture objects
4. **Animation Loading**: AnimationManager parses JSON and creates AnimationData

## Current Stability
The project is in a stable state with working core systems. All mentioned features are functional and tested. The build system compiles successfully with no critical errors.

## Known Limitations
1. **Map background scaling**: Adventure state backgrounds don't properly fill square display
2. **Multiple scaling systems**: Some legacy scaling code causes conflicts
3. **Asset requirements**: Many Digimon lack animation data files

## Development Tools
- **Batch sprite converter**: Python script for processing sprite sheets
- **Debug states**: Built-in tools for testing animations and Digimon
- **Configuration system**: Runtime adjustment of game settings
