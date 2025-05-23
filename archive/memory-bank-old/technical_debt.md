# Technical Debt

## Current Issues

### 1. Current Active Issues

#### Menu Cursor Warning
- **Status**: Low-priority improvement opportunity
- **Severity**: Very Low (benign warning)
- **Description**: The application attempts to load a 'menu_cursor' texture that is not found in the asset manager, causing runtime warnings. After investigation:
  - This is expected behavior for carousel-style menus that don't need a cursor
  - The warning only affects visual node-based menus (like the Map screen) that would benefit from a cursor
  - The code was designed to handle both menu styles, but only one style is currently implemented

### 2. Resolved Issues

#### TextRenderer Implementation
- **Status**: Resolved in current build
- **Severity**: None (previously high)
- **Description**: The TextRenderer.cpp implementation is working correctly in the current build with:
  - Proper member variable access
  - Correct instance method calls
  - Functional text rendering

#### OverworldMapState References
- **Status**: Confirmed non-existent
- **Severity**: None (outdated reference)
- **Description**: Previous documentation referenced an OverworldMapState.cpp file that isn't present in the current project structure. This was likely:
  - A planned feature that was never implemented
  - A file that was removed in a code cleanup
  - Reference from an older branch or version that is no longer relevant

### 2. Architecture Issues

#### Error Handling
- **Status**: Ongoing
- **Severity**: Medium
- **Description**: Inconsistent error handling across the codebase:
  - Mix of boolean return values, exceptions, and SDL_Log calls
  - No centralized error reporting system
  - Limited recovery mechanisms for failures

#### Configuration Management
- **Status**: Ongoing
- **Severity**: Medium
- **Description**: Hard-coded values scattered throughout the codebase:
  - Screen dimensions, file paths, and game parameters embedded in code
  - No central configuration system
  - Limited ability to change settings without recompilation

#### Memory Management
- **Status**: Ongoing
- **Severity**: Medium
- **Description**: Suboptimal asset and memory management:
  - Loading all assets at startup rather than on-demand
  - Limited resource cleanup mechanisms
  - No texture atlas optimization for sprites

## Improvement Opportunities

### 1. Short-term Fixes
- Fix TextRenderer implementation errors
- Investigate and address OverworldMapState references
- Address size conversion warnings

### 2. Medium-term Improvements
- Implement consistent error handling strategy
- Create a centralized configuration system
- Optimize memory usage with better asset management

### 3. Long-term Vision
- Comprehensive test coverage
- Build system improvements
- Documentation enhancement

## Action Items
1. Prioritize fixing TextRenderer implementation to unblock builds
2. Scan codebase for references to OverworldMapState
3. Create a plan for addressing other technical debt items in priority order
