# DigiviceRefactor - Next Implementation Tasks

Based on our improved understanding of the original Digivice -25th COLOR EVOLUTION- mechanics and hardware specifications (32MB flash memory), this document outlines the specific implementation tasks we'll tackle next to build a solid foundation for the project.

## Immediate Focus: Core Systems (1-2 Weeks)

### Hardware Considerations

While implementing these systems, we'll keep in mind the following hardware-inspired constraints:

- **Memory Efficiency**: Design data structures to be compact and efficient
- **Asset Management**: Implement resource loading/unloading to optimize memory usage
- **Performance**: Ensure all systems would perform well within the constraints of embedded hardware

### 1. Map & Area System Foundation

#### Goal:
Create the fundamental structure for maps, stages, and areas that form the backbone of the game's progression system.

#### Tasks:
1. **Design Data Structures**
   ```cpp
   // Map class to represent one of the 7 game maps
   class Map {
   public:
       Map(const std::string& id, const std::string& name);
       void addStage(const Stage& stage);
       std::vector<Stage>& getStages();
       std::string getName() const;
       
   private:
       std::string id_;
       std::string name_;
       std::vector<Stage> stages_;
   };

   // Stage class to represent locations within a map
   class Stage {
   public:
       Stage(const std::string& id, const std::string& name);
       void addArea(const Area& area);
       std::vector<Area>& getAreas();
       std::string getName() const;
       
   private:
       std::string id_;
       std::string name_;
       std::vector<Area> areas_;
   };

   // Area class to represent individual events/locations
   class Area {
   public:
       enum class EventType { CUTSCENE, BATTLE };
       
       Area(const std::string& id, const std::string& name, int stepsRequired, EventType type);
       int getStepsRequired() const;
       int getRemainingSteps() const;
       void addSteps(int steps);
       bool isCompleted() const;
       EventType getEventType() const;
       
   private:
       std::string id_;
       std::string name_;
       int stepsRequired_;
       int currentSteps_;
       bool completed_;
       EventType eventType_;
   };
   ```

2. **Implement Area Selection**
   - Create a map screen that shows available areas
   - Implement area selection mechanism
   - Add functionality to change destination
   - Handle progress reset when changing destinations

3. **Create Progress Tracking**
   - Track steps toward the current area
   - Implement B button functionality to display remaining steps
   - Create visual progress indicator

### 2. Step Counter System

#### Goal:
Create the step counting system that drives progression in the game.

#### Tasks:
1. **Design Step Counter Class**
   ```cpp
   class StepCounter {
   public:
       StepCounter();
       
       // Add steps (from physical movement or simulation)
       void addSteps(int count);
       
       // For testing on PC
       void simulateStep();
       
       // Get total and session counts
       int getTotalSteps() const;
       int getSessionSteps() const;
       void resetSessionSteps();
       
       // Step speed multiplier (1 physical step = 5 game steps)
       void setSpeedMultiplier(bool enabled);
       bool isSpeedMultiplierEnabled() const;
       
   private:
       int totalSteps_;
       int sessionSteps_;
       bool speedMultiplierEnabled_;
   };
   ```

2. **Implement PC Simulation**
   - Add key binding for simulating steps
   - Create visual feedback for step counting
   - Connect step counter to current area progress

3. **Add Step Speed Option**
   - Create setting to multiply step count (1 step = 5 steps)
   - Implement toggle functionality
   - Store preference persistently

### 3. Basic Event System

#### Goal:
Create the framework for triggering and handling events when an area is reached.

#### Tasks:
1. **Design Event Framework**
   ```cpp
   class EventManager {
   public:
       EventManager();
       
       // Register event types
       void registerEventHandler(Area::EventType type, std::function<void(const Area&)> handler);
       
       // Trigger event for an area
       void triggerEvent(const Area& area);
       
       // Check if area event is complete
       bool isEventComplete(const Area& area);
       
   private:
       std::map<Area::EventType, std::function<void(const Area&)>> eventHandlers_;
       std::set<std::string> completedEvents_;
   };
   ```

2. **Implement Basic Cutscene System**
   - Create simple dialog display system
   - Add character portraits/sprites
   - Implement dialog progression
   - Handle cutscene completion

3. **Create Battle Event Trigger**
   - Design battle initialization
   - Create enemy Digimon appearance
   - Set up battle state transition
   - Implement simple placeholder battle flow

### 4. Menu System Enhancement

#### Goal:
Update the menu system to match the structure of the original device.

#### Tasks:
1. **Implement Full Menu Hierarchy**
   - Create all main menu options (Digimon, Friend, Map, Recover, etc.)
   - Implement sub-menus (Partner, Change, Digivolution, Bond)
   - Design menu navigation system

2. **Create Carousel Navigation**
   - Implement smooth scrolling between menu options
   - Add visual emphasis for selected item
   - Create proper option centering
   - Add transition animations

3. **Design Menu Screens**
   - Create layouts for each menu screen
   - Implement content display for each option
   - Add proper navigation between screens
   - Design consistent UI elements

### 4. Hardware Abstraction Layer for Physical Implementation

#### Goal:
Create a hardware abstraction layer that will allow the software to run both on desktop computers during development and on the RP2350-Touch-AMOLED-1.43 board for physical implementation.

#### Tasks:
1. **Define Hardware Interface**
   ```cpp
   // Hardware abstraction interface
   class IHardware {
   public:
       virtual ~IHardware() = default;
       
       // Display functions
       virtual void initialize() = 0;
       virtual void drawPixel(int x, int y, uint32_t color) = 0;
       virtual void clear() = 0;
       virtual void update() = 0;
       
       // Input functions
       virtual bool isButtonPressed(Button button) = 0;
       virtual bool wasButtonPressed(Button button) = 0;
       virtual bool wasButtonReleased(Button button) = 0;
       
       // Step counter functions
       virtual uint32_t getStepCount() = 0;
       virtual void resetStepCount() = 0;
       
       // Time functions
       virtual uint64_t getMillis() = 0;
       virtual DateTime getCurrentTime() = 0;
   };
   ```

2. **Create Desktop Implementation**
   ```cpp
   class DesktopHardware : public IHardware {
   public:
       // Implementation for desktop development
       // Uses SDL or similar for display and input
   };
   ```

3. **Prepare for RP2350 Implementation**
   ```cpp
   class RP2350Hardware : public IHardware {
   public:
       // Implementation for RP2350 board
       // Will use GPIO for button input and SPI for display
   };
   ```

4. **Button Input System**
   - Implement button state tracking with debouncing
   - Create input mapper for game controls
   - Test with desktop interface
   - Prepare for GPIO integration on physical hardware

5. **Physical Button Implementation**
   - Design button PCB based on the hardware button schematic
   - Test physical button circuit with breadboard prototype
   - Create initial driver for GPIO button input

## Technical Implementation Plan

### Class Integration:
1. **Update Game Class**
   - Add MapManager instance
   - Add StepCounter instance
   - Add EventManager instance
   - Connect these systems in the game loop

2. **Create MapState**
   - Implement map visualization
   - Add area selection functionality
   - Create area info display
   - Handle transitions between map and other states

3. **Update AdventureState**
   - Display current area and progress
   - Visualize steps and progress
   - Handle area completion
   - Add B button functionality

4. **Enhance MenuState**
   - Implement the full menu hierarchy
   - Create proper carousel navigation
   - Add sub-menu transitions
   - Design consistent menu UI

### Testing Approach:
1. Test each system individually with simple hardcoded data
2. Create a test map with a few areas for integration testing
3. Implement basic logging to track system behavior
4. Create debug UI elements to visualize system state

## Next Steps Beyond Initial Implementation:
1. Friend Digimon system
2. Battle mechanics
3. Evolution memory bar system
4. Content creation for maps, stages, and areas
