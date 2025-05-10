# DigiviceRefactor - Progression System Design

## Overview

The progression system in DigiviceRefactor faithfully recreates the mechanics of the Digivice -25th COLOR EVOLUTION- device while incorporating selective enhancements. This document details how players advance through the game, how events are triggered, and how the evolution system works.

## Map & Area Structure

### Map Organization
- **7 Maps Structure**: Game divided into seven distinct maps following the Digimon Adventure storyline
- **Stages**: Each map contains multiple stages set in different locations
- **Areas**: Each stage contains between 4 and 13 Areas
- **Area Types**:
  - **Cutscene Areas**: Partner Digimon interacts with characters
  - **Battle Areas**: Fight with enemy Digimon

### Area Navigation
- **Area Selection**: Player selects destination Area from the map
- **Step Requirements**: Each Area has a preset number of steps required to reach
- **Progress Check**: B Button displays remaining steps to destination
- **Destination Changes**: Player can select a different Area, but this resets progress
- **Area Completion**: Events trigger automatically upon reaching 0 steps remaining
- **Post-Completion**: After clearing an Area, player selects next destination

## Step & Event System

### Step Mechanics
- **Base Step Counter**: Tracks physical steps (or simulated steps on PC)
- **Step Display**: Shows steps remaining to reach current destination Area
- **Step Speed Option**: Optional setting to count 1 physical step as 5 game steps
- **Step Visualization**: Progress indicator showing advancement toward Area event

### Event System
- **Predetermined Events**: Each Area has a fixed event that triggers upon arrival
- **Event Types**:
  - **Cutscenes**: Brief story moments with character interactions
  - **Battles**: Combat encounters with enemy Digimon
- **Event Completion**: 
  - Cutscene events complete automatically
  - Battle events complete upon victory
- **Friend Digimon**: Defeating an enemy Digimon typically unlocks it as a Friend Digimon

## Evolution System

### Memory Bar System
- **Progress Tracking**: Evolution progress represented by 5 memory bars
- **Accumulation Method**: Bars filled by winning battles
- **Progress Display**: Viewed in the "Bond" option under "Digimon" menu

### Evolution Requirements
- **Rookie Level**: 
  - Available from start
  - Base form for all partner Digimon
- **Champion Level**:
  - Complete certain number of battles to fill memory bars
  - Requirements vary by specific Digimon
- **Ultimate Level**:
  - Higher battle win requirements
  - May require progression to certain map stage
- **Mega Level**:
  - Highest battle win requirements
  - May require specific story progression

### Battle Win Progression
- **Battle Counter**: Each Digimon tracks wins toward evolution
- **Variable Requirements**:
  - Different Digimon may need different numbers of wins
  - Higher evolution levels require more wins
  - Some special Digimon may have unique requirements
- **Persistence**: Battle win counts maintained even when switching partner Digimon

## Progression Pacing

### Early Game (Nodes 1-2)
- **Focus**: Learning game mechanics, initial exploration
- **Challenge Level**: Low to moderate
- **Evolution State**: Rookie to early Champion
- **D-Power Accumulation**: Slower, more effort required
- **Step Requirements**: 500-1000 steps per chapter

### Mid Game (Nodes 3-5)
- **Focus**: Expanding the adventure, more varied challenges
- **Challenge Level**: Moderate to challenging
- **Evolution State**: Champion to early Ultimate
- **D-Power Accumulation**: Moderate, balanced effort/reward
- **Step Requirements**: 750-1500 steps per chapter

### Late Game (Nodes 6-8)
- **Focus**: Major story developments, challenging encounters
- **Challenge Level**: High
- **Evolution State**: Ultimate to early Mega
- **D-Power Accumulation**: Faster, more rewarding
- **Step Requirements**: 1000-2000 steps per chapter

### End Game (Nodes 9+)
- **Focus**: Final challenges, mastery of the game
- **Challenge Level**: Very high
- **Evolution State**: Mega forms with special abilities
- **D-Power Accumulation**: Rapid, power fantasy enabled
- **Step Requirements**: 1500-2500 steps per chapter

## Engagement Mechanics

### Daily Bonuses
- **First Login**: Bonus D-Power (amount scales with progression)
- **Step Streaks**: Consecutive days with minimum steps provide multipliers
- **Special Days**: Birthday, holidays, etc. provide unique encounters or bonuses

### Achievement System
- **Step Milestones**: Total accumulated steps (10K, 50K, 100K, etc.)
- **Battle Achievements**: Victories, perfect wins, specific opponents
- **Collection Goals**: Encountering specific Digimon types
- **Evolution Mastery**: Achieving all possible evolutions

### Replay Incentives
- **Partner Selection**: Different starting Digimon provide unique experiences
- **Evolution Paths**: Alternate evolution choices affect gameplay
- **Challenge Modes**: Unlockable difficulty increases for experienced players
- **Speedrun Tracking**: Records for fastest chapter/node completions

## Implementation Notes

### Technical Requirements
- **Step Tracking System**: Interface for step input (simulated on PC)
- **Progress Persistence**: Save system for long-term advancement
- **Chapter/Node Management**: System to track state of all map elements
- **Encounter Generation**: Algorithm for creating appropriate random encounters
- **D-Power Economy**: Balanced input/output of the primary progression currency

### User Experience Considerations
- **Progress Visibility**: Clear indicators of advancement and next goals
- **Satisfying Milestones**: Meaningful rewards at regular intervals
- **Play Session Design**: Achievements possible in short play sessions (5-15 minutes)
- **Long-term Goals**: Deeper objectives for dedicated players
