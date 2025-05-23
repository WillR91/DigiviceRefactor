# V-Pet Style Battle Encounter Sequence

This document outlines the specific sequence and mechanics for a battle encounter, emulating a V-Pet style interaction.

## 1. Battle Initiation

*   **Trigger**: The battle begins automatically when the player's character (Digimon) reaches a predefined step goal specific to the current in-game area.

## 2. Transition to Battle Scene

*   **Phase 1: Fade to Black**: The current game screen (e.g., Adventure State) fades completely to black.
*   **Phase 2: Fade Out (Reveal)**: The black screen fades out, revealing the battle scene with the enemy Digimon.

## 3. Enemy Introduction

*   **Visual**: The enemy Digimon's sprite is displayed prominently.
*   **Information**: The name of the enemy Digimon is printed on screen, typically positioned beneath its sprite.

## 4. Attack Phase Animations

*   **Player's Attack**: The player's Digimon performs its attack animation.
*   **Enemy's Attack**: The enemy Digimon performs its attack animation.
*   **Clash**: The visual representations of both attacks travel towards the center of the screen and meet or clash.

## 5. Player Interaction Minigame

*   Immediately following the attack clash, a minigame is initiated to determine the outcome of the attack.
*   The type of minigame can vary. Potential types include:
    *   **Tap Race**: The player must rapidly press a designated button within a short time limit to reach a target number of taps.
    *   **Shake Race**: The player must simulate a shaking motion (e.g., rapidly alternating two specific key presses) to meet a threshold.
    *   **QTE (Quick Time Event)**: The player must press a sequence of buttons as they are prompted on screen, with precise timing.

## 6. Outcome Resolution

*   **Player Success**: If the player successfully meets the minigame's objective/threshold:
    *   The player's attack is considered successful.
    *   The player's attack animation completes, showing a hit on the enemy Digimon.
*   **Player Failure**: If the player fails to meet the minigame's objective/threshold:
    *   The enemy's attack is considered successful.
    *   The enemy's attack animation completes, showing a hit on the player's Digimon.

## 7. Post-Attack (Implicit for Prototype)

*   **Damage Calculation**: Based on the outcome, damage would be calculated and applied to the respective Digimon's HP.
*   **HP Update**: Visual update of HP bars/stats.
*   **Battle Continuation/End**:
    *   If a Digimon's HP reaches zero, the battle ends (victory or defeat).
    *   Otherwise, the battle may loop back to a new attack phase or a different state.
    *   For the initial prototype, one round demonstrating the sequence might be sufficient before transitioning out of the battle.

## Key Characteristics to Emulate:

*   **Automatic Initiation**: Battles are not player-chosen encounters but triggered by game progression (step count).
*   **Cinematic Feel**: Emphasis on the visual sequence of fades, reveals, and attack animations.
*   **Skill-Based Outcome**: The success of an attack is determined by player performance in a real-time minigame, not just stats.
