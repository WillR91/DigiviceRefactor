# Hardware Schematic: Button Implementation for Digivice

This document provides schematic diagrams and technical specifications for implementing physical buttons on the RP2350-Touch-AMOLED-1.43 board for our Digivice project.

## GPIO Button Circuit

### Single Button Circuit Diagram

```
RP2350
+-------+
|       |
|    GPIO Pin ----+--- 10kΩ Pullup Resistor --- 3.3V
|       |         |
|       |         |
|       |        +++
|       |        | | Tactile Switch
|       |        +++
|       |         |
|       |         |
|    GND ---------+
|       |
+-------+
```

Note: Internal pull-up resistors can be used instead of external ones.

### Complete Button Matrix

```
                  RP2350
                +-------+
                |       |
                |    GPIO2 --- [A Button]  --- GND
                |       |
                |    GPIO3 --- [B Button]  --- GND
                |       |
                |    GPIO18 -- [D-Pad Up]  --- GND
                |       |
                |    GPIO19 -- [D-Pad Down] -- GND
                |       |
                |    GPIO20 -- [D-Pad Left] -- GND
                |       |
                |    GPIO21 -- [D-Pad Right] - GND
                |       |
                |    GND -------------------+
                |       |                   |
                +-------+                   |
                                            |
                        All Buttons ---------+
```

## Button PCB Layout

### Top View

```
+-------------------------------------------------------+
|                                                       |
|    +-------+    +-------+    +-------+    +-------+   |
|    |       |    |       |    |       |    |       |   |
|    |   U   |    |   D   |    |   L   |    |   R   |   |
|    |       |    |       |    |       |    |       |   |
|    +-------+    +-------+    +-------+    +-------+   |
|                                                       |
|                                                       |
|    +-------+    +-------+                             |
|    |       |    |       |                             |
|    |   A   |    |   B   |                             |
|    |       |    |       |                             |
|    +-------+    +-------+                             |
|                                                       |
|                                       +------------+  |
|                                       |            |  |
|                                       | GPIO Header|  |
|                                       |            |  |
|                                       +------------+  |
|                                                       |
+-------------------------------------------------------+
```

### Button Breakout PCB Specifications

- **Board Dimensions**: 60mm × 40mm
- **Layers**: 2-layer PCB
- **Mounting Holes**: 4x M2 holes at corners
- **Components**:
  - 6× 4.5mm × 4.5mm tactile switches (Omron B3F or equivalent)
  - 1× 2×7 pin header (1.27mm pitch, matching RP2350 GPIO header)
  - Optional: 6× 0.1μF ceramic capacitors for hardware debouncing

## Electrical Specifications

### Tactile Switch Requirements
- **Type**: SPST (Single Pole, Single Throw) momentary tactile switch
- **Actuation Force**: 160-200gf for good tactile feedback
- **Lifetime**: 100,000+ operations
- **Size**: 4.5mm × 4.5mm with 3.5mm height
- **Operating Voltage**: 3.3V
- **Current**: <10mA

### GPIO Configuration
- **Input Mode**: GPIO configured as INPUT_PULLUP
- **Voltage Level**: 3.3V logic
- **Interrupt Mode**: Edge triggered on falling edge (button press)
- **Debounce Time**: 20-50ms in software

## Physical Layout

### Button Dimensions and Spacing

```
   ┌─────┐     ┌─────┐
   │  U  │     │     │   Button Cap Size: 8mm × 8mm
   └─────┘     │     │
┌─────┬─────┐  │     │   D-Pad Size: 24mm × 24mm
│  L  │  R  │  │     │
└─────┴─────┘  │     │   A/B Button Size: 10mm diameter
   ┌─────┐     │     │
   │  D  │     │     │   Spacing between A and B: 15mm
   └─────┘     │     │
               │     │   Distance from D-Pad to A/B: 20mm
┌─────┐ ┌─────┐│     │
│  A  │ │  B  ││     │
└─────┘ └─────┘│     │
               └─────┘
```

### 3D Case Integration

The PCB should be mounted in the case with:
- Button caps protruding through case cutouts
- 3-5mm travel distance for comfortable pressing
- Secure mounting to prevent movement
- Alignment guides to ensure proper button placement

## Connection to RP2350

### GPIO Header Pinout

| Pin # | RP2350 Pin | Button Function |
|-------|------------|----------------|
| 1     | 3.3V       | VCC            |
| 2     | GND        | Ground         |
| 3     | GPIO2      | A Button       |
| 4     | GPIO3      | B Button       |
| 5     | GPIO18     | D-Pad Up       |
| 6     | GPIO19     | D-Pad Down     |
| 7     | GPIO20     | D-Pad Left     |
| 8     | GPIO21     | D-Pad Right    |

### Wiring Harness

- Use a short ribbon cable or pre-wired connector
- Keep wire length under 10cm to avoid noise issues
- Consider strain relief in the case design
- Use keyed connectors to prevent incorrect insertion

## Prototyping Strategy

1. **Breadboard Prototype**:
   - Test with external buttons connected directly to GPIO
   - Validate software reading and debouncing

2. **Perfboard Prototype**:
   - Create hand-wired version for testing with case prototype
   - Test ergonomics and button feel

3. **PCB Manufacturing**:
   - Send final design to PCB manufacturer
   - Assemble with selected components

4. **Final Integration**:
   - Install in 3D printed case
   - Test with complete system

## Bill of Materials (Detailed)

| Item | Description | Quantity | Notes |
|------|-------------|----------|-------|
| Tactile Switch | 4.5mm × 4.5mm, 160gf | 6 | Omron B3F-1000 or equivalent |
| PCB | Button breakout board, 2-layer | 1 | Custom design |
| Button Caps | 8mm × 8mm for D-pad, 10mm dia. for A/B | 6 | 3D printed or injection molded |
| GPIO Header | 2×7 female header, 1.27mm pitch | 1 | For connection to RP2350 |
| Ribbon Cable | 14-conductor, 1.27mm pitch | 1 | 10cm length |
| Mounting Screws | M2×6mm | 4 | For securing PCB to case |
| Standoffs | M2, 3mm height | 4 | For proper button height |
| Capacitors (optional) | 0.1μF ceramic | 6 | For hardware debouncing |

This schematic and specification document provides the technical details needed to implement a physical button system for our Digivice project on the RP2350 platform.
