# DigiviceRefactor - Virtual Pet Simulator

A modern C++ implementation of a Digimon-style virtual pet device with cross-platform support and authentic retro gameplay mechanics.

## Project Overview

DigiviceRefactor is a comprehensive virtual pet simulator that recreates the classic Digimon virtual pet experience with modern graphics and cross-platform compatibility. The project features a state-based architecture, multi-layered rendering system, and authentic battle mechanics.

## Current Build Status

**✅ Stable Build (January 2025)**
- All core systems operational
- Text rendering working via sprite-based font atlas
- Multi-state navigation functional
- Asset loading and rendering stable
- Sound system integrated
- Cross-platform builds working

## Quick Start

### Prerequisites
- CMake 3.20+
- C++17 compatible compiler
- SDL2 development libraries
- SDL2_mixer for audio

### Building
```bash
mkdir build
cd build
cmake ..
make
```

### Running
```bash
./DigiviceRefactor
```

## Core Features

### 🎮 Virtual Pet Mechanics
- **Pet Care**: Feeding, training, and caring for your digital companion
- **Evolution**: Growth stages from baby to ultimate forms
- **Stats**: Health, hunger, happiness, and battle statistics
- **Time-based**: Real-time pet needs and aging

### ⚔️ Battle System
- Turn-based combat with authentic mechanics
- Type advantages and special attacks
- Training to improve battle stats
- Victory rewards and progression

### 🗺️ Adventure Mode
- Multi-layered background environments
- Exploration and event discovery
- Dynamic map navigation
- Environmental storytelling

### 🎨 Graphics & Audio
- Pixel-perfect sprite rendering
- Multi-layer parallax backgrounds
- Retro-styled UI elements
- Authentic sound effects and music

## Documentation Structure

- `architecture.md` - Technical architecture overview
- `build-system.md` - Build configuration and dependencies  
- `game-systems.md` - Core gameplay mechanics
- `graphics-system.md` - Rendering and visual systems
- `audio-system.md` - Sound and music implementation
- `asset-pipeline.md` - Asset organization and loading
- `development-guide.md` - Development workflow and conventions

## Project Structure

```
DigiviceRefactor/
├── src/                    # Source code
│   ├── core/              # Core game systems
│   ├── states/            # Game state implementations
│   ├── graphics/          # Rendering and graphics
│   ├── audio/             # Sound system
│   ├── platform/          # Platform-specific code
│   └── utils/             # Utility functions
├── include/               # Header files
├── assets/                # Game assets
│   ├── sprites/          # Sprite graphics
│   ├── backgrounds/      # Background images
│   ├── ui/               # UI elements and fonts
│   └── audio/            # Sound effects and music
├── build/                 # Build output
└── documentation/         # Project documentation
```

## Recent Changes

This documentation represents a fresh start as of January 2025, reflecting the current stable build state. Previous documentation has been archived in `archive/documentation-2025-05-23/` and `archive/memory-bank-2025-05-23/`.

## Contributing

See `development-guide.md` for development workflow, coding standards, and contribution guidelines.

## License

[License information to be added]
