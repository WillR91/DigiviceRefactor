# Hardware Button Implementation for Digivice Project

## Original Digivice Button Layout

The Digivice -25th COLOR EVOLUTION- featured a simple but effective button layout:

1. **A Button** - Main action/select button
2. **B Button** - Secondary action/back button
3. **Directional Pad** - Four-way navigation (Up, Down, Left, Right)

This layout allowed for all necessary game interactions while maintaining the classic handheld feel of the device.

## RP2350 Implementation Strategy

### Button PCB Design

For our hardware implementation on the Waveshare RP2350-Touch-AMOLED-1.43, we'll create a custom PCB that:

1. Connects to the RP2350's GPIO header
2. Provides mount points for tactile switches
3. Includes necessary pull-up resistors (if not using internal ones)
4. Fits within the 3D printed case design

### GPIO Pin Assignment

Based on the available GPIO pins on the RP2350 board, we'll use the following pin assignments:

| Button        | GPIO Pin      | Notes                                    |
|---------------|---------------|------------------------------------------|
| A Button      | GPIO2         | Primary action button                    |
| B Button      | GPIO3         | Secondary action/back button             |
| D-Pad Up      | GPIO18        | Navigation up                           |
| D-Pad Down    | GPIO19        | Navigation down                         |
| D-Pad Left    | GPIO20        | Navigation left                         |
| D-Pad Right   | GPIO21        | Navigation right                        |
| Reset         | RST (built-in)| System reset (optional in gameplay)      |
| Boot/Special  | BOOT (built-in)| Special function (optional in gameplay) |

### Circuit Design

Each button will be implemented using:
- Tactile momentary switches (4.5mm × 4.5mm or similar)
- Internal pull-up resistors (enabled in software)
- Ground connection to complete the circuit when pressed

```
       3.3V
        |
        R (Pull-up, internal to RP2350)
        |
GPIO Pin -----+
        |     |
        |    ---
        |    --- Switch
        |     |
        +-----+
        |
       GND
```

### Button Debouncing

To prevent false button presses from switch bounce:

1. **Hardware Solution**:
   - Optional capacitor (0.1μF) in parallel with each switch

2. **Software Solution**:
   - Debounce interval of 20-50ms
   - State change validation requiring consistent readings
   - Interrupt-driven approach with validation routine

### Input Processing Stack

The button input system will be structured as follows:

1. **Hardware Layer**:
   - Physical buttons connected to GPIO pins

2. **Driver Layer**:
   - GPIO configuration and interrupt handling
   - Debouncing implementation
   - Raw button state detection

3. **Input Abstraction Layer**:
   - Translates physical button states to logical inputs
   - Handles combinations and special inputs
   - Provides event-based interface for the game logic

4. **Game Input Layer**:
   - Maps logical inputs to game actions
   - Contextual controls based on current game state
   - Menu navigation and selection handling

## Physical Implementation

### Button Placement

The physical placement of buttons will be designed to:
- Match the ergonomics of the original Digivice
- Be comfortable for extended play sessions
- Support natural thumb movement for D-pad and A/B buttons
- Resist accidental presses

### Case Design

The 3D printed case will include:
- Cutouts for the buttons with proper alignment to PCB
- Button caps that provide tactile feedback similar to original device
- Secure mounting for the button PCB and RP2350 board
- Space for battery and other components

### Visual Design

- Button caps will be color-coded to match original design
- D-pad will have directional indicators
- A/B buttons will be labeled appropriately
- Overall aesthetic will mimic the original Digivice

## Software Integration

### Input API

```cpp
// Button state definitions
enum class ButtonState {
    PRESSED,
    RELEASED,
    HELD
};

// Button type definitions
enum class Button {
    A,
    B,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    SPECIAL
};

// Input manager interface
class InputManager {
public:
    // Get current state of a button
    ButtonState getButtonState(Button button);
    
    // Check if a button was just pressed this frame
    bool wasButtonPressed(Button button);
    
    // Check if a button was just released this frame
    bool wasButtonReleased(Button button);
    
    // Get duration of button press in milliseconds
    uint32_t getButtonHoldTime(Button button);
    
    // Register callback for button events
    void registerButtonCallback(Button button, ButtonState state, 
                               std::function<void()> callback);
    
    // Update input states (called once per frame)
    void update();
    
private:
    // Implementation details
};
```

### Integration with Game Logic

The input system will integrate with the game logic through:

1. **Event-based triggers** for immediate actions
2. **State polling** for continuous inputs
3. **Context-specific controls** based on current game state
4. **Menu navigation** using the InputManager API

## Prototype Sequence

1. Breadboard prototype with external buttons
2. PCB design and fabrication
3. Integration with RP2350 board
4. Software driver implementation
5. Testing with game logic
6. Final integration with 3D printed case

## Bill of Materials

| Component             | Quantity | Notes                            |
|-----------------------|----------|----------------------------------|
| Tactile Switches      | 6        | 4.5mm × 4.5mm, 160gf actuation   |
| Custom PCB            | 1        | Button breakout board            |
| Button Caps           | 6        | 3D printed or commercial         |
| Ribbon Cable/Wires    | 1        | For GPIO connection              |
| Mounting Hardware     | 8        | M2 screws and standoffs          |
| 3D Printed Case       | 1        | Custom designed                  |

This implementation will provide a faithful recreation of the original Digivice button experience while leveraging the modern capabilities of the RP2350 platform.
