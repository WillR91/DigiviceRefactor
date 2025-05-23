# Hardware Reference Information

This document captures information about the original Digivice hardware that may inform our implementation decisions, as well as potential target hardware for a physical implementation.

## Original Digivice Hardware Specifications

Based on device teardown information:

### Memory
- **Flash Memory**: 32MB Macronix MX25L25645GM2I-08G flash chip
- **Pre-programmed**: Paint stripe suggests pre-programming before mounting

### Hardware Design
- **Programming Port**: Available for firmware updates
- **Test Points**: Numerous test points on the PCB
- **PCB Design**: Features curvy tracks (likely for signal integrity or space optimization)

## Potential Implementation Hardware

We're considering the Waveshare RP2350-Touch-AMOLED-1.43 development board as a potential target for physical implementation:

### Key Specifications
- **Processor**: Dual-core ARM Cortex-M33 + dual-core RISC-V (150 MHz)
- **Memory**: 520KB SRAM, 16MB on-chip Flash
- **Display**: 1.43-inch AMOLED, 466×466 resolution, 16.7M colors
- **Input**: Capacitive touchscreen + side buttons
- **Sensors**: QMI8658 6-axis IMU (accelerometer + gyroscope) for step counting
- **Additional**: RTC chip, MicroSD slot, battery support, buzzer

### Compatibility Assessment
- **Memory**: 16MB on-chip (vs 32MB original) but with MicroSD expansion
- **Display**: Higher resolution and color depth than original
- **Input**: Touch + buttons vs original button-only interface
- **Sensors**: Modern IMU suitable for pedometer functionality

See `hardware_implementation_rp2350.md` for detailed analysis of this hardware option.

## Implications for Our Implementation

### Memory Budgeting
- Stay within reasonable memory constraints that would be feasible on the original hardware
- Allocate memory budget appropriately across:
  - Sprite assets
  - Background images
  - Audio resources
  - Game state data

### Asset Design
- Optimize assets for size and performance
- Consider hardware-appropriate color depths and formats
- Use compression techniques that would be feasible on embedded hardware

### Architecture Decisions
- Use efficient data structures appropriate for limited memory environments
- Consider memory pooling techniques to avoid fragmentation
- Implement resource management to load/unload assets as needed

### Future Hardware Considerations
- If transitioning to physical hardware:
  - Target similar memory specifications (32MB or less)
  - Plan for programming/firmware update capability
  - Consider power consumption optimization

## Performance Targets

To maintain authenticity when emulating the original device:

1. **Frame Rate**: Target similar frame rates to the original hardware (likely 30fps or lower)
2. **Response Time**: Match button response times for authentic feel
3. **Animation Speeds**: Maintain similar timing for character animations and transitions

These hardware insights will help ensure our implementation remains faithful to the original device's capabilities while allowing for appropriate enhancements.
