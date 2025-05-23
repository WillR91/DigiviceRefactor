# DigiviceRefactor - Menu & UI System Design

## Overview

The menu and UI system for DigiviceRefactor is designed to emulate the constrained yet functional interface of handheld Digivice devices while incorporating quality-of-life improvements. The primary menu system uses a vertical carousel approach rather than a traditional cursor-based selection mechanism, with special handling for the Map screen.

## Core Menu Structure

### Main Menu
- **Access**: A Button press from home screen
- **Style**: Vertical carousel
- **Options**:
  1. **DIGIMON**: Partner Digimon menu
     - **Partner**: Manage playable partner
     - **Change**: Select available partner Digimon
     - **Digivolution**: View unlocked evolved forms
     - **Bond**: View progress toward next evolution (memory bars)
  2. **FRIEND**: Manage Friend Digimon
     - View unlocked Friend Digimon (defeated enemies)
     - Select one to assist in battle
  3. **MAP**: View and select destination Areas
  4. **RECOVER**: Heal injuries from lost battles
  5. **LINK**: Multiplayer functionality (future implementation)
  6. **PLAYMODE**: View special content
     - **Digivolution**: Evolution cutscenes
     - **Sound**: Voice clips and music
     - **Ending**: Ending cutscene (after completion)
  7. **SETTING**: Configure options
     - Sound Volume
     - Display Brightness
     - Visual Effects
     - Step Speed
     - Reset Game
  8. **POWER OFF**: Save and exit

### Navigation Pattern
- **Up/Down**: Scroll to adjacent menu options (carousel wrapping)
- **Select/Confirm**: Choose the currently centered option
- **Back/Cancel**: Return to previous menu or state

## Menu Implementation

### Carousel Style Menu
- **Display Pattern**: 
  - Current selection centered
  - Previous/next items partially visible above/below
  - Smooth scrolling animation between selections
- **Visual Emphasis**:
  - Current selection: Full opacity, larger, possible highlight effect
  - Adjacent options: Slightly dimmed, smaller
  - Further options: Not visible

### Menu Transitions
- **State Transitions**: 
  - Fade effect when moving between major states
  - Duration: 0.3 seconds for standard transitions
- **Option Transitions**:
  - Smooth scroll animation when changing selection
  - Duration: 0.15 seconds for selection changes

### Menu Layout
- **Title Area**: Top of screen (20% vertical space)
- **Content Area**: Middle of screen (60% vertical space)
  - Selection carousel here
  - Information display
  - Visual elements
- **Control Hints**: Bottom of screen (20% vertical space)

## Special Interfaces

### Map Screen
- **Access**: Through "MAP" option in main menu
- **Display**: Visual representation of adventure map showing nodes and connections
- **Navigation**:
  - Up/Down: Highlight different map nodes (sequential selection)
  - Visual indicator shows currently selected node
  - Node information displayed for selection
- **Node States**:
  - Completed: Visually distinct, selectable for replay
  - Current: Highlighted as active location
  - Locked: Visible but not selectable
  - Hidden: Not visible until prerequisites met

### Digimon Status Screen
- **Access**: Through "DIGIMON" option in main menu
- **Display**: Partner Digimon with status information
- **Information Shown**:
  - Name and current form
  - D-Power level
  - Evolution progress
  - Statistics (battles won, steps taken, etc.)
  - Available actions (sub-menu carousel)

### Battle Interface
- **Access**: During encounters or through "BATTLE" option
- **Display**: 
  - Player Digimon (left side)
  - Opponent Digimon (right side)
  - Health indicators
  - Action menu (bottom)
- **Battle Actions**: Vertical carousel of available moves

### Evolution Screen
- **Access**: Through Digimon status or at evolution triggers
- **Display**: 
  - Current form (left)
  - Evolution animation (center)
  - New form (right)
  - D-Power consumption indicator
- **Interaction**: Confirmation before spending D-Power

## Visual Design Elements

### UI Components
- **Text Boxes**: 
  - Rounded corners
  - Semi-transparent backgrounds
  - Pixel font for authenticity
- **Borders**: 
  - 1-2px width
  - Contrast color to stand out from backgrounds
- **Icons**:
  - Simple pixel art
  - Consistent style across interface
  - Clear silhouettes for recognizability

### Animation Guidelines
- **Menu Scrolling**: Smooth, slight easing at end of movement
- **Selection Feedback**: Brief flash or highlight effect
- **Transitions**: Cross-fade between major screens
- **Idle Animations**: Subtle movement for menu elements when inactive

### Color Schemes
- **Main UI**: Blue/white theme reminiscent of original devices
- **Alerts**: Yellow/orange for warnings or important information
- **Confirmation**: Green for positive actions
- **Denial/Errors**: Red for negative feedback or unavailable options

## Implementation Considerations

### Technical Requirements
- **Rendering System**: 
  - Support for layered UI elements
  - Text rendering with sprite-based font
  - Simple animation capabilities
- **Input Handling**:
  - Debounced input to prevent accidental double-selections
  - Support for both momentary and held inputs
  - Consistent response timing

### Memory Constraints
- **Asset Reuse**: Shared UI elements across different screens
- **Efficient Rendering**: Minimal redraw of static elements
- **Asset Management**: Load UI elements only when needed

### Accessibility
- **Readability**: Ensure text is legible on small screens
- **Input Timing**: Adjustable timing for button presses
- **Visual Clarity**: Sufficient contrast for all UI elements

## UI States & Flow

### UI State Hierarchy
1. **Game States**: High-level application states (Adventure, Menu, Battle)
2. **UI Containers**: Screen-level organization (Main Menu, Status Screen)
3. **UI Components**: Individual interactive elements (Buttons, Displays)

### Menu Flow Diagram
```
Adventure State
    │
    ▼
Main Menu ◄────────────────────────┐
    │                               │
    ├─► Digimon Menu               │
    │       │                       │
    │       ├─► Status Screen       │
    │       │                       │
    │       ├─► Training Options    │
    │       │                       │
    │       └─► Evolution Screen    │
    │                               │
    ├─► Adventure (return to game)  │
    │                               │
    ├─► Map Screen ──► Node Select ─┘
    │       
    ├─► Battle Simulator
    │
    └─► Settings Menu
```

## Testing Guidelines

### UI Testing Priorities
- **Navigation Flow**: Ensure all screens are accessible and properly connected
- **Input Responsiveness**: Verify consistent button response
- **Visual Clarity**: Test readability under different conditions
- **Animation Smoothness**: Check for consistent frame rates during transitions

### User Experience Checks
- **Learning Curve**: New users should understand navigation within 1-2 minutes
- **Efficiency**: Regular actions should require minimal inputs
- **Feedback**: Every user action should have clear visual or audio feedback
- **Consistency**: Similar actions should behave similarly across the application
