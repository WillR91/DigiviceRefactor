# DigiviceRefactor Project Context

## Project Overview
This is a Digimon-related application that simulates a Digivice, a device from the Digimon franchise. The application is built using C++ and SDL2, following a state-based architecture with a game loop pattern. It aims to recreate the experience of the original Digivice hardware with authentic visual styling and gameplay mechanics.

## Project Structure
- The project uses CMake for building
- It is a C++ application requiring C++17 standard
- SDL2 and SDL2_image are used for graphics and window management
- Multiple sprites for different Digimon characters (Agumon, Gabumon, Biyomon, etc.)
- Background assets for environments
- Multiple workspace files for different development aspects

## Key Directories
- `assets/`: Contains game assets organized in backgrounds, sprites, and UI
- `include/`: Contains header files organized by functionality
- `src/`: Contains source code files
- `build/`: Contains build artifacts
- `separated_sprites_input/`: Contains individual sprite frames for different Digimon
- `generic_input/`: Contains background images
- `memory-bank/`: Documentation about project architecture and planning

## Dependencies
- SDL2 (version 2.32.0)
- SDL2_image (version 2.8.6)
- C++17 Standard Library

## Current Status
- Stable build with no compilation errors
- Basic game loop and state management implemented
- State transitions with fade effects working
- Asset loading system in place
- Input handling system established
- Multiple game states (Adventure, Menu, PartnerSelect, etc.) implemented
- Text rendering using sprite-based font atlas approach
- Digimon character animations working

## Development Notes
- Multiple workspace files suggest iterative development approach
- Asset conversion scripts handle sprite preparation
- Text rendering uses a sprite sheet-based approach consistent with the original device
- Source code is organized by functionality with clean separation of concerns
- The application follows authentic styling to match the original Digivice experience
