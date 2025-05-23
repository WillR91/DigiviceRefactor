# Project Architecture Overview

## Core Architecture
This is a Digivice simulator application built in C++ using SDL2 and SDL2_image. The project follows a state-based architecture with a game loop pattern.

### Key Components

1. **Game Class (`Game.h`/`Game.cpp`)**: 
   - Central manager class that coordinates all other components
   - Manages the game loop (init, run, update)
   - Handles state management through a state stack
   - Contains fade transition logic between states

2. **State System**:
   - `GameState` abstract base class with derived concrete states
   - States include: Adventure, Menu, PartnerSelect, Transition, Progress
   - States are managed in a stack, allowing for layered UI and state transitions

3. **Asset Management**:
   - `AssetManager` handles loading and providing access to assets
   - Assets include sprites for different Digimon characters
   - Sprite animations are organized by character name, action, and frame

4. **Input Management**:
   - `InputManager` processes user input and maps it to game actions

5. **Display System**:
   - `PCDisplay` handles rendering to the screen (PC implementation)
   - SDL2 is used for window management and rendering
   - Uses RGB565 format for sprites with magenta as a transparent color key

6. **Animation System**:
   - `AnimationManager` and `Animation` classes handle sprite animations
   - Animations defined in various action states (Idle, Walk, Run, etc.)

7. **Player Data**:
   - `PlayerData` stores the player's progress and state

## Build System
- Uses CMake for building
- SDL2 and SDL2_image dependencies
- Builds for Windows platform (possibly others)

## Asset Processing
- Python scripts (`batch_convert_sprites_mapped.py` and `convert_generic.py`) for converting sprites and other assets
- Input sprites are organized in separate files and converted to C++ header files with RGB565 data
- Animation frames are mapped to specific actions (Idle, Walk, Run, etc.)

## Project Organization
- `include/`: Header files organized by functionality
- `src/`: Source implementation files
- `assets/`: Game assets (sprites, backgrounds, UI elements)
- `build/`: Build artifacts
- `separated_sprites_input/`: Raw sprite frame images
- `generic_input/`: Background and other image inputs
