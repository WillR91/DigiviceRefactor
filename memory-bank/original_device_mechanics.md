# Original Digivice Mechanics Reference

This document captures the core mechanics and features of the original Digivice -25th COLOR EVOLUTION- device that we're emulating in our DigiviceRefactor project.

## Core Gameplay Loop

### Progression System
- **Step Counting**: When the pedometer is shaken (either by walking or manually shaking), steps are recorded
- **Area Navigation**: The player character walks toward a destination Area within a particular stage
- **Distance Display**: Remaining steps to destination are displayed when pressing the B Button on the home screen
- **Area Events**: Each Area has a pre-set event that triggers at 0 steps remaining:
  - Cutscene events: Partner Digimon interacts with another Digimon/Chosen Child
  - Battles: Fight with an enemy Digimon
- **Area Clearing**: Areas are cleared after completing the event (cutscenes immediately clear; battles require victory)
- **Destination Selection**: After clearing an Area, player selects their next destination
- **Friend Digimon**: Winning battles generally unlocks the defeated Digimon as a Friend Digimon

### Game Structure
- **7 Maps**: The game is divided into seven distinct maps
- **Stages**: Each map contains multiple stages set in different locations
- **Areas**: Each stage contains between 4 and 13 Areas
- **Storyline**: Follows the basic plot of Digimon Adventure and Digimon Adventure: Our War Game!

## Menu System

Menus are accessed by pressing the A button while on the home screen.

### Main Menu Options

1. **Digimon**
   - **Partner**: Manage the playable partner Digimon
   - **Change**: Select an available partner Digimon to play as
   - **Digivolution**: View all unlocked evolved forms
   - **Bond**: View progress toward unlocking next evolved form
     - Progress represented in bars of "memory"
     - One memory bar filled after winning certain number of battles
     - Requirements vary by Level and sometimes by specific Digimon
     - Next evolution unlocked after filling all 5 memory bars

2. **Friend**
   - View unlocked Friend Digimon (previously defeated enemies)
   - Select one to assist in battle with additional attacks
   - Can deselect or change selected Friend Digimon

3. **Map**
   - View world map and Areas to clear
   - Select another Area as destination
   - Note: Changing destination resets step progress

4. **Recover**
   - Heal injuries sustained from losing battles

5. **Link**
   - Multiplayer functionality with compatible devices
   - **vs Digivice**: Play with another Digivice -25th COLOR EVOLUTION-
   - **vs Others**: Play with other compatible devices

6. **Playmode**
   - **Digivolution**: View evolution cutscenes
   - **Sound**: Listen to voice clips and music
   - **Ending**: View ending cutscene (after completing the game)

7. **Setting**
   - **Sound Volume**: Adjust device volume
   - **Display Brightness**: Adjust screen brightness
   - **LED Effects**: Enable/disable light-up effects for voice clips, battles, alerts
   - **Vibration**: Enable/disable device vibration for alerts and battle effects
   - **Clock Setting**: Adjust device time
   - **Step Speed**: Enable/disable multiplier (1 actual step = 5 in-game steps)
   - **Clear All**: Reset device and erase all progress

8. **Power Off**
   - Turn off the Digivice
   - All progress is saved for next time

## Evolution System

- **Memory Bars**: Progress tracked through 5 memory bars
- **Battle Requirements**: Each bar requires winning a certain number of battles
- **Variable Difficulty**: Requirements increase with higher evolution levels
- **Digimon-Specific**: Some Digimon have unique requirements
- **Evolution Unlock**: Once all 5 bars are filled, next evolution becomes available

## Battle System

- **Partner Digimon**: Player's selected partner fights
- **Friend Digimon**: Optional assist with additional attacks
- **Victory Rewards**: Winning unlocks defeated Digimon as Friends
- **Defeat Consequences**: Losing causes injuries requiring recovery
