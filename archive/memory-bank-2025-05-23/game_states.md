# Game States

## Identified States
Based on the code, these are the main states in the Digivice simulator:

1. **Adventure State**
   - Main gameplay state
   - Likely shows the Digimon partner and environment
   - Handles basic interactions with the Digimon

2. **Menu State**
   - Main menu interface
   - Provides navigation to other features/states

3. **Partner Select State**
   - For choosing a Digimon partner
   - Shows different available Digimon characters

4. **Transition State**
   - Handles visual transitions between states
   - Manages fade in/out effects

5. **Progress State**
   - Tracks and displays player progress
   - May show achievements, stats, or other progress metrics

## State Management
- States are managed in a stack system
- Transitions between states use fade effects
- States can be pushed, popped, or swapped

## State Lifecycle
- `enter()`: Called when a state becomes active
- `exit()`: Called when a state is being removed or covered
- `handle_input()`: Processes user inputs specific to the state
- `update()`: Updates the state logic based on time and player data
- `render()`: Handles drawing the state's visual elements

## Transition Logic
The game uses a fade system for smooth transitions between states with these steps:
- `FADING_OUT`: Initial fade to black
- `READY_FOR_STATE_SWAP`: Preparing to change state
- `SETUP_TARGET_STATE`: Setting up the new state
- `FADING_IN`: Fading in from black to reveal the new state
