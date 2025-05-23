# Hardware Implementation: RP2350-Touch-AMOLED-1.43 Analysis

## Overview

The Waveshare RP2350-Touch-AMOLED-1.43 is a compact, all-in-one development board that appears highly suitable for implementing a physical version of our DigiviceRefactor project. This document analyzes its capabilities and compatibility with our project requirements.

## Key Specifications

### Processing Power
- **Processor**: Dual-core ARM Cortex-M33 + dual-core Hazard3 RISC-V (up to 150 MHz)
- **Memory**: 520KB SRAM + 16MB on-chip Flash
- **Performance Assessment**: Significantly more powerful than the original Digivice hardware, providing ample headroom for our implementation

### Display
- **Screen**: 1.43-inch AMOLED display
- **Resolution**: 466 × 466 pixels
- **Color Depth**: 16.7M colors (24-bit)
- **Interface**: SPI
- **Touch**: Capacitive touchscreen via I2C interface with interrupt support

### Motion Sensing
- **IMU**: QMI8658 6-axis IMU (3-axis accelerometer + 3-axis gyroscope)
- **Features**: Motion gesture detection, step counting
- **Compatibility**: Perfect for implementing the pedometer functionality central to Digivice gameplay

### Additional Hardware Features
- **RTC**: PCF85063 real-time clock chip
- **Storage**: MicroSD card slot
- **Connectivity**: USB Type-C
- **Battery**: 3.7V lithium battery support with charging
- **Input**: RST and BOOT side buttons
- **Sound**: Onboard buzzer
- **Expansion**: UART and I2C interfaces, 2×14PIN 1.27mm pitch interface

## Suitability Analysis

### Advantages
1. **All-in-one Solution**: Includes all hardware components needed for our Digivice implementation
2. **Step Counter**: Built-in IMU sensor perfect for pedometer functionality
3. **High-Quality Display**: Resolution and color depth exceed requirements, allowing for enhanced visuals
4. **Battery Operation**: Rechargeable battery support for portable use
5. **Storage Options**: Both on-chip Flash (16MB) and optional MicroSD expansion
6. **Time Keeping**: RTC for accurate time-based features
7. **Sound Capabilities**: Onboard buzzer for audio feedback
8. **Development Friendly**: USB interface for programming and debugging

### Potential Limitations
1. **Resolution Mismatch**: 466×466 square resolution vs. likely rectangular original Digivice display
2. **Touch Input**: Original used physical buttons, may need to adapt UI for touch
3. **Power Consumption**: AMOLED display may use more power than original LCD
4. **Size Difference**: Likely different dimensions than original Digivice
5. **Development Complexity**: May require adapting code for specific hardware

## Implementation Considerations

### Display Adaptation
- Create a viewport strategy that maintains the aspect ratio of the original Digivice
- Potentially use the extra screen space for enhanced UI elements or background extensions

### Input System
- Implement physical buttons using available GPIO pins
- Create button layout similar to original Digivice
- Use existing side buttons (RST/BOOT) and add additional buttons via GPIO
- Potentially use touch capabilities as a secondary/optional input method

### Pedometer Implementation
- Leverage the QMI8658 IMU for accurate step counting
- Implement filtering algorithms to prevent false step detection
- May need to calibrate sensitivity compared to original device

### Power Management
- Implement sleep modes to conserve battery
- Optimize rendering and processing for energy efficiency
- Consider screen timeout features

### Software Stack
- Could use C/C++ with direct hardware access
- Alternatively, MicroPython is supported for faster prototyping
- Would need to port our C++ codebase to target this specific hardware

## Memory Comparison

- **Original Digivice**: 32MB flash memory
- **RP2350 Board**: 16MB on-chip flash + MicroSD expansion
- **Assessment**: Base flash memory is half of original device, but MicroSD expansion provides ample storage if needed

## GPIO Analysis for Physical Button Implementation

Based on the board specifications and pinout diagram, the RP2350-Touch-AMOLED-1.43 offers numerous GPIO pins that can be utilized for physical button implementation:

### Available GPIO Pins
- The board has multiple GPIO pins brought out to the 2×14PIN 1.27mm pitch interface
- Key GPIO pins visible in the pinout include:
  - GPIO2-4, GPIO18-21, GPIO24-25, GPIO27-29 on main board edges
  - Additional GPIO pins available through the expansion header

### Button Implementation Strategy
1. **Button Requirements**:
   - Original Digivice typically had 3-5 buttons (direction pad + action buttons)
   - Our implementation would need a similar button count for authentic experience

2. **GPIO Configuration**:
   - Configure pins with internal pull-up resistors
   - Connect buttons between GPIO and ground
   - Use interrupt-based polling for efficient button detection

3. **Physical Layout Options**:
   - Utilize existing RST and BOOT buttons for main action buttons (A/B equivalents)
   - Add D-pad or directional buttons (Up, Down, Left, Right) using GPIO pins
   - Consider adding additional function buttons if needed

4. **Button Hardware**:
   - Tactile momentary switches compatible with handheld device ergonomics
   - Custom PCB for button breakout connected to GPIO header
   - Designed to fit within a 3D printed case

5. **Software Integration**:
   - Create abstraction layer for button input
   - Implement debouncing in software
   - Map physical buttons to game controls

This approach maintains the authentic button-based control scheme of the original Digivice while leveraging the modern capabilities of the RP2350 board. The touch screen capabilities can be reserved for future enhancements or optional features.

### GPIO and Button Implementation Diagram
```
Physical Buttons <--> GPIO Pins <--> Input Processing <--> Game Controls
      |                                     |
Tactile Hardware              Software Debouncing & Mapping
```

## Button Implementation Documentation

For detailed specifications on the physical button implementation design:
- See [Hardware Button Implementation](./hardware_button_implementation.md) for the complete implementation plan
- See [Hardware Button Schematic](./hardware_button_schematic.md) for circuit diagrams and technical specifications

## Recommendation

The RP2350-Touch-AMOLED-1.43 appears to be an excellent choice for physically implementing the DigiviceRefactor project. The hardware provides all necessary components in a compact form factor, with capabilities that meet or exceed what's required for an authentic Digivice experience.

### Next Steps if Proceeding with this Hardware
1. Create a hardware abstraction layer in our codebase
2. Develop initial proof-of-concept for display rendering and input handling
3. Implement step counter using the IMU
4. Test power consumption and optimize for battery life
5. Design and 3D print a case that resembles the original Digivice
