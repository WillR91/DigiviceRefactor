# DigiviceRefactor - Core Design Document

## Game Overview

DigiviceRefactor is an emulator/simulator that recreates the experience of the Digimon virtual pet devices, specifically the Digivice -25th COLOR EVOLUTION- along with elements from the 90s pedometer toys. The project aims to faithfully recreate the core mechanics while adding enhancements and personal touches.

## Core Gameplay Mechanics

### Pedometer Simulation
- **Step Counting**: Primary gameplay progression mechanism
- **PC Implementation**: Will use keyboard/mouse input to simulate steps
- **Hardware Implementation**: Will eventually support actual pedometer input when ported to a physical device
- **Steps-to-Progress Conversion**: Steps reduce the distance to the next Area event
- **Progress Check**: B Button displays remaining steps to destination

### Map System
- **7-Map Structure**: Game divided into seven main maps following Digimon Adventure storyline
- **Stages**: Each map contains multiple stages set in different locations
- **Areas**: Each stage contains 4-13 Areas with preset events
- **Visual Representation**: Map view showing player position and available Areas
- **Area Selection**: After clearing an Area, player selects next destination Area
- **Selection Reset**: Changing destination resets step progress

### Area Structure
- **Area Types**:
  - Cutscene areas: Partner Digimon interacts with another character
  - Battle areas: Fight with an enemy Digimon
- **Progress Tracking**: Visual indicator of steps remaining to Area event
- **Completion Rewards**: Friend Digimon unlocks, story progression, etc.

### Encounter System
- **Preset Encounters**: Each Area has a predetermined event
- **Battle System**: Combat with enemy Digimon
- **Non-Combat Events**: Cutscenes and character interactions
- **Friend System**: Defeated Digimon become available as Friend Digimon that can assist in battles

### Evolution System
- **Memory Bar System**: Progress tracked through 5 memory bars
- **Battle Requirements**: Bars filled by winning specific number of battles
- **Variable Difficulty**: Requirements vary by evolution Level and specific Digimon
- **Evolution Unlock**: Once all 5 bars are filled, next evolution becomes available

## Game Flow & Progression

### Overall Structure
1. **Initial Setup**: Player selects starting partner Digimon
2. **Chapter Progression**: Complete chapters within map nodes by accumulating steps
3. **Evolution**: Partner evolves as player accumulates D-Power and progresses through story
4. **Map Expansion**: New nodes unlock as player completes chapters and defeats key battles
5. **Story Advancement**: Overall narrative unfolds across map nodes and chapters

### Progression Balancing
- **Early Game**: Higher D-Power requirements relative to player capabilities
- **Mid Game**: More efficiency in D-Power accumulation as player progresses
- **Late Game**: Power fantasy enabled by high efficiency and evolved forms

### Replay Value
- **Partner Variety**: Different starting Digimon provide unique experiences
- **Evolution Choices**: Different evolution paths based on player decisions
- **Milestone Achievements**: Special challenges and recognition for accomplished players

## Technical Implementation

### State Management
- **Adventure State**: Main gameplay state showing Digimon and environment
- **Menu State**: Carousel-style menu for accessing different functions
- **Map State**: Visual representation of adventure map with selectable nodes
- **Battle State**: Interface for combat encounters
- **Evolution State**: Visual sequence and interface for evolution events

### Visual Design
- **Sprite-Based Graphics**: Character and environment represented through sprite animations
- **Menu System**: Vertical carousel interface where options scroll into view
- **Map Interface**: Node-based visual with highlighting for selection
- **Battle Interface**: Simple representation of player Digimon and opponent

### Interface Navigation
- **Button Controls**: Simple controls mapped to device buttons or keyboard
- **Menu Navigation**: Up/down to scroll options, confirm to select centered option
- **Map Navigation**: Up/down to highlight different nodes, confirm to select

## Future Considerations

### Hardware Implementation
- **Target Device**: Portable handheld with pedometer capability
- **Hardware Requirements**: Display, input buttons, step counter, storage
- **Power Management**: Efficiency considerations for battery life

### Feature Expansion
- **Multiplayer Capabilities**: Local connection between devices
- **Additional Partner Digimon**: Expanded roster of available partners
- **Enhanced Battle System**: More strategic elements and special abilities
- **Achievement System**: Tracking and rewarding player milestones

### Content Growth
- **Extended Map**: More nodes, chapters, and overall adventure length
- **Special Events**: Seasonal or rare encounters
- **Alternate Story Paths**: Different narrative experiences based on choices

## Implementation Priorities

1. **Core Loop**: Step counting, chapter progression, basic encounters
2. **Menu System**: Functional carousel-style interface
3. **Map System**: Node-based adventure map with navigation
4. **Evolution System**: D-Power accumulation and Digimon evolution
5. **Battle System**: Basic combat encounters
6. **Content Expansion**: Additional chapters, encounters, and evolution paths
7. **Hardware Considerations**: Preparation for potential physical implementation
