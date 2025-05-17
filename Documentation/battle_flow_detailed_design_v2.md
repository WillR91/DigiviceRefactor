# Detailed Battle Flow Design V2

This document outlines the step-by-step sequence and visual/interactive elements for the battle system.

## Phase 1: Battle Start & Enemy Introduction

1.  **Trigger**: The battle automatically initiates when the player's character achieves a predefined step goal specific to the current in-game area.
2.  **Transition to Enemy**:
    *   The current game screen (e.g., Adventure State) fades completely to black.
    *   The black screen then fades back into clarity, revealing the enemy Digimon.
    *   The enemy Digimon is positioned in the center of the screen, animated in its idle stance.
    *   The name of the enemy Digimon is rendered beneath its sprite.
3.  **Player Confirmation**: This screen remains static until the player presses the ENTER button, confirming they are ready to proceed with the battle.

## Phase 2: Battle Instructions Screen

1.  **Transition**: Upon player confirmation, a "jagged tooth" like transition occurs:
    *   The screen appears to be consumed by jagged edges closing in from the top and bottom.
    *   These edges then open like a mouth to reveal the next screen.
2.  **Instruction Screen Content**: This screen will display information about the battle's objectives or mechanics.
    *   *(For initial implementation, this can be a placeholder screen. Detailed content and design to be developed later.)*
3.  **Player Confirmation**: The player must press ENTER on this screen to continue.

## Phase 3: Player Digimon Selection Screen

1.  **Transition**: The instruction screen fades to black, and then fades back into clarity for the selection screen.
2.  **Selection Screen Content**: This screen allows the player to choose which of their Digimon will participate in the battle.
    *   *(Initially, this choice will be limited to the player's available rookie Digimon partners. A different Digivolving gameplay loop is planned for later implementation, which will expand these options.)*
3.  **Player Confirmation**: The player navigates to their chosen Digimon and presses ENTER to confirm their selection.

## Phase 4: Player Digimon Enters Battle Arena

1.  **Transition**: The selection screen fades to black, and then resolves to show the chosen player Digimon on the battle screen.
2.  **Player Digimon Reveal**: The selected Digimon is centered on the screen and displayed in its idle animation.

## Phase 5: Player's Attack Sequence

1.  **Background Transition**: A few frames after the player's Digimon appears, the screen background changes to a dynamic orange visual with motion graphics.
2.  **Large Sprite Attack**: A large, animated, full-screen drawn sprite of the player's Digimon appears, executing its attack.
3.  **Return to Pixel Sprite**: This large sprite animation fades, revealing the player's smaller pixel sprite, now superimposed on the orange motion background.
4.  **HP Display**: A semi-transparent overlay appears on the very right of the screen, hosting five (5) HP blocks representing the player's current health.
5.  **Pixel Sprite Attack Animation**:
    *   The player's pixel sprite, initially stationary, jumps backwards on the screen by a few pixels.
    *   It then performs its attack animation.
    *   A projectile asset is fired from the sprite.

## Phase 6: Enemy's Attack Sequence (Mirrored)

1.  **Transition**: The screen quickly fades to white.
2.  **Enemy Reveal (Mirrored)**: The white fade reveals the enemy Digimon, now facing the opposite direction (e.g., facing left if the player faces right). This creates the visual impression that the enemy is positioned to the left of the player, and they are facing each other.
3.  **Mirrored Attack Process**: The enemy Digimon repeats the attack process described in Phase 5 (orange background, large sprite attack, small sprite animation, projectile firing), but all animations and positioning are mirrored.

## Phase 7: Attack Clash & Duel Mechanic (Novel Implementation Area)

1.  **Conceptual Basis (from reference device)**: In the device being emulated, the two attack projectile sprites would meet in the middle of a new screen. The player would then engage in a minigame or follow on-screen instructions to win the duel.
2.  **Outcome Determination**: If the player wins this duel, their attack successfully lands on the enemy. If the enemy wins the duel, their attack lands on the player.
3.  **Future Development Goal**: A novel and unique mechanic for this duel phase is desired, rather than a direct copy of the reference device.
    *   *(The specifics of how attack damage is calculated against different Digimon levels will be designed and implemented at a later stage.)*

## Phase 8: Health Points (HP) and Battle Conclusion

1.  **Damage and Defeat**: Attacks reduce the target's HP. If a Digimon's HP is reduced to zero, they are defeated.
2.  **Win Condition**: The player wins the battle if the enemy Digimon's HP is reduced to zero.
3.  **Lose Condition**: The player loses the battle if their active Digimon's HP is reduced to zero.

## Phase 9: Player Victory Sequence

1.  **Victory Display**: If the player wins, the screen shows the player's Digimon, either in its idle animation or a special celebration animation (depending on available assets).
2.  **De-Digivolution (if applicable)**: If the player's Digimon had Digivolved for the battle, a white fade transition occurs. After the fade, the Digimon is shown in its de-evolved (e.g., rookie) form.
3.  **"CLEAR" Notification**: A blue UI bar appears on screen with the text "CLEAR" prominently displayed, confirming the player's victory.

## Phase 10: Return to Overworld and Progression

1.  **Transition to Map**: The battle victory screen fades to black.
2.  **Map Reveal with Unlocks**: The screen then fades back into the continent map view. A newly unlocked map node is revealed, signifying the player's progress.

## Implementation Plan (High-Level)

1.  **Refine/Rebuild `BattleState` Class:**
    *   Review and adapt the existing `BattleState.h` and `BattleState.cpp`.
    *   Ensure the class can manage the overall flow of the battle as described in the phases above.
    *   Solidify the `VPetBattlePhase` enum to cover all necessary stages.
    *   Manage member variables for player/enemy Digimon data, HP, transition states (fading, jagged tooth), and current phase.

2.  **Integrate `BattleState` Trigger:**
    *   Confirm and refine how `BattleState` is triggered (e.g., step goal in `AdventureState`).
    *   Ensure smooth transition from the triggering state to `BattleState`, including passing necessary data like player Digimon and enemy ID/type.

3.  **Implement Phase 1: Battle Start & Enemy Introduction (Refine Existing):**
    *   Adapt existing fade-in logic.
    *   Ensure enemy Digimon sprite and name are loaded and displayed correctly based on passed-in data (not just hardcoded).
    *   Confirm ENTER key press transitions to the next phase.

4.  **Implement Core Transition Mechanics:**
    *   Standardize fade-to-black/fade-from-black logic (potentially in a utility or base class if not already).
    *   Design and implement the "jagged tooth" transition effect. This might involve:
        *   Creating specific assets (images/sprites for the teeth).
        *   Developing rendering logic to animate these assets to create the closing/opening mouth effect.

5.  **Develop Placeholder Screens (Phase 2 & 3):**
    *   Create simple, functional placeholder screens for:
        *   "Battle Instructions" (Phase 2): Displays placeholder text, waits for ENTER.
        *   "Player Digimon Selection" (Phase 3): Displays placeholder text (e.g., "Rookie Partner Selected"), waits for ENTER. (Full selection logic deferred).

6.  **Implement Attack Sequences (Phases 5 & 6 - Visuals First):**
    *   Focus on the visual sequence for both player and enemy (mirrored):
        *   Transition to orange motion background (can be a static image or simple shader/animation).
        *   Display large attack sprite (placeholder if final assets are unavailable).
        *   Fade/transition to pixel sprite on the orange background.
        *   Implement HP bar display (static initially).
        *   Animate pixel sprite (jump back, attack animation, fire projectile). Projectiles can be simple shapes initially.

7.  **Basic Battle Logic & Outcome (Placeholders for Phase 7 & 8):**
    *   After attack animations, implement a temporary outcome (e.g., player always "hits").
    *   No complex duel mechanic or HP calculation in the first pass. This is to allow flow to victory/defeat.

8.  **Implement Victory Sequence (Phase 9):**
    *   Display player Digimon (idle/celebration).
    *   Implement white fade for de-Digivolution (visual only, no logic change).
    *   Display "CLEAR" UI bar.

9.  **Implement Return to Map (Phase 10):**
    *   Fade to black.
    *   Transition state back to `MapSystemState`.
    *   (Deferred: Logic for actually unlocking a map node in `MapSystemState` based on battle outcome).

## Initial Focus Areas for Next Steps

*   **A. Refine `BattleState` Structure and Phase Enum:**
    *   Update `BattleState.h` with a comprehensive `VPetBattlePhase` enum covering all detailed phases.
    *   Add necessary member variables to `BattleState.h` to support the new phases (e.g., timers for new transitions, textures for new UI elements, state for jagged transition).
*   **B. Implement "Jagged Tooth" Transition (Phase 2):**
    *   Design the visual assets for the jagged teeth.
    *   Implement the animation logic for the transition in `BattleState::update` and `BattleState::render`.
    *   Trigger this transition after player confirms enemy intro (Phase 1).
*   **C. Implement Placeholder Instruction Screen (Phase 2):**
    *   After the jagged tooth transition completes, display a simple screen with text like "Battle Instructions: Defeat the Enemy!"
    *   Wait for an ENTER press to proceed to a (temporary) next phase.
