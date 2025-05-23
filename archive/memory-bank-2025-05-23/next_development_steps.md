# DigiviceRefactor - Next Development Steps

## Current Project Status

The DigiviceRefactor project currently has a stable foundation with:
- A functioning state management system
- Basic menu implementation
- Sprite-based character rendering
- Animation system
- Text rendering using sprite font atlas

## Immediate Next Steps (2-4 Week Focus)

### 1. Enhanced Menu System (1 week)

#### Tasks:
- Refactor MenuState to implement the carousel-style navigation
- Add smooth scrolling animations between menu items
- Create visual distinction for the currently selected item
- Implement proper menu hierarchy with sub-menus
- Add transition effects between menu states

#### Implementation Approach:
- Extend the current MenuState class
- Add animation properties for smooth scrolling
- Create a MenuOption class with positioning and scale properties
- Implement easing functions for smooth transitions

### 2. Step Counter Implementation (1 week)

#### Tasks:
- Create a StepCounter class to track and manage steps
- Implement keyboard/mouse input for simulating steps on PC
- Add visual feedback for accumulated steps
- Create step-to-progress conversion system
- Implement step history and statistics

#### Implementation Approach:
- Create new StepCounter class in Core directory
- Add step simulation to InputManager
- Implement simple step visualization in Adventure state
- Create helper functions for step-to-progress conversion

### 3. Basic Chapter System (1-2 weeks)

#### Tasks:
- Design and implement Chapter data structure
- Create a ChapterManager class
- Implement chapter loading and progression
- Add chapter completion triggers and rewards
- Create simple test chapters to validate the system

#### Implementation Approach:
- Create new Chapter and ChapterManager classes
- Define JSON format for chapter data
- Implement chapter state tracking
- Add visualization of chapter progress
- Create chapter transition effects

### 4. Initial D-Power System (1 week)

#### Tasks:
- Create D-Power counter and manager
- Implement accumulation from steps (1 per 10-15 steps)
- Add visual display for current D-Power
- Implement storage and persistence
- Create simple animations for D-Power gain

#### Implementation Approach:
- Create new DPowerManager class
- Integrate with StepCounter for automatic accumulation
- Add D-Power display to UI
- Implement save/load for D-Power values

## Technical Implementation Details

### New Classes to Create:

1. **StepCounter**
   ```cpp
   // StepCounter.h
   class StepCounter {
   public:
       StepCounter();
       void addSteps(int count);
       void simulateStep(); // For PC testing
       int getTotalSteps() const;
       int getSessionSteps() const;
       void resetSessionSteps();
       
       // Events
       void registerStepCallback(StepCallback callback);
       
   private:
       int totalSteps_;
       int sessionSteps_;
       std::vector<StepCallback> callbacks_;
   };
   ```

2. **Chapter**
   ```cpp
   // Chapter.h
   class Chapter {
   public:
       Chapter(const std::string& id, int stepsRequired);
       
       std::string getId() const;
       int getStepsRequired() const;
       int getCurrentSteps() const;
       float getProgress() const; // 0.0 to 1.0
       
       void addSteps(int steps);
       bool isComplete() const;
       void complete(); // Force completion
       
   private:
       std::string id_;
       int stepsRequired_;
       int currentSteps_;
       bool completed_;
   };
   ```

3. **ChapterManager**
   ```cpp
   // ChapterManager.h
   class ChapterManager {
   public:
       ChapterManager();
       
       void loadChapters(const std::string& dataFile);
       Chapter* getCurrentChapter();
       void advanceToNextChapter();
       
       void addStepsToCurrentChapter(int steps);
       float getCurrentProgress() const;
       
   private:
       std::vector<Chapter> chapters_;
       int currentChapterIndex_;
   };
   ```

4. **DPowerManager**
   ```cpp
   // DPowerManager.h
   class DPowerManager {
   public:
       DPowerManager(int initialAmount = 0, int maxCapacity = 500);
       
       int getCurrentAmount() const;
       int getMaxCapacity() const;
       
       bool add(int amount);
       bool spend(int amount);
       void setMaxCapacity(int capacity);
       
       float getPercentFull() const; // 0.0 to 1.0
       
   private:
       int currentAmount_;
       int maxCapacity_;
   };
   ```

### Class Integration:

1. **Game Class Updates**
   - Add StepCounter instance
   - Add ChapterManager instance
   - Add DPowerManager instance
   - Connect these systems in the update loop

2. **AdventureState Updates**
   - Display step count and chapter progress
   - Visualize D-Power accumulation
   - Handle chapter completion events

3. **MenuState Updates**
   - Implement carousel navigation
   - Add D-Power display
   - Create chapter/map access points

## Testing Plan

1. **Menu System Testing**
   - Verify smooth scrolling between options
   - Test menu hierarchy navigation
   - Ensure proper highlighting of selected items
   - Check transition animations

2. **Step Counter Testing**
   - Validate step accumulation accuracy
   - Test step simulation controls
   - Verify proper event triggering on milestones
   - Check statistics tracking

3. **Chapter System Testing**
   - Test chapter loading from data
   - Validate progress tracking
   - Check completion triggers
   - Test chapter transitions

4. **D-Power System Testing**
   - Verify accumulation rates
   - Test maximum capacity behavior
   - Check persistence of values
   - Validate visualization
