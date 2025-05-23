# Battle Flow Design Document

This document outlines the intended sequence of events and player interactions for the battle system.

## 1. Initiation & Enemy Introduction
*   **Trigger**: Battle commences automatically when the player's character achieves a predefined step goal specific to the current in-game area.
*   **Transition**:
    1.  The current game screen (e.g., Adventure State) fades completely to black.
    2.  The black screen fades back into clarity, revealing the enemy Digimon.
*   **Scene**:
    *   The enemy Digimon's sprite is displayed, centered and in its idle animation.
    *   The name of the enemy Digimon is rendered beneath its sprite.
*   **Player Interaction**: The scene remains static until the player presses the ENTER button to confirm they are ready.

## 2. Battle Instructions Screen
*   **Transition**: Upon player confirmation (ENTER press), a "jagged tooth" like transition occurs:
    1.  The effect closes in from the top and bottom edges of the screen.
    2.  It then opens like a mouth to reveal the battle instructions screen.
*   **Scene**: This screen will display information about how to win the battle. (For initial implementation, this can be a placeholder).
*   **Player Interaction**: Player presses ENTER to continue.

## 3. Player's Digimon Selection
*   **Transition**:
    1.  The instruction screen fades to black.
    2.  The black screen fades back into clarity, revealing the Digimon selection screen.
*   **Scene**: This screen allows the player to choose which of their Digimon will participate in the battle.
    *   **Note**: While the original device involved Digivolution at this stage, this implementation will initially limit the choice to the player's rookie Digimon partners. A different Digivolution gameplay loop is planned for later.
*   **Player Interaction**: Player navigates and presses ENTER on the Digimon they wish to use.

## 4. Player's Digimon Appears on Battle Screen
*   **Transition**: The selection screen fades, then resolves to the main battle screen.
*   **Scene**: The player's chosen Digimon is now visible, centered on their side of the battle screen, in its idle animation.

## 5. Player's Attack Phase
*   **Visual Shift**: After a few frames of the player's Digimon idling:
    1.  The background abruptly changes to a dynamic orange display with motion graphics.
    2.  A large, animated, full-screen drawn sprite of the player's Digimon executes its attack.
*   **Transition**: This full-screen attack animation fades, revealing the player's smaller pixel sprite now superimposed on the orange motion background.
*   **UI Element**: A semi-transparent overlay appears on the far right of the screen, displaying 5 HP blocks representing the player's health.
*   **Attack Animation (Pixel Sprite)**:
    1.  The sprite remains in its initial position.
    2.  It jumps backwards on the screen by a few pixels.
    3.  It performs its attack animation.
    4.  A projectile asset is fired from the sprite.

## 6. Enemy's Attack Phase (Mirrored)
*   **Transition**: The screen quickly fades to white.
*   **Scene Shift**: The white fade reveals the enemy Digimon, now facing the opposite direction (e.g., facing left if the player is on the right and facing right). This creates the visual impression that the enemy is to the left of the player, and they are facing each other.
*   **Mirrored Process**: The enemy Digimon then repeats the attack sequence described in section 5 (visual shift to orange background, large sprite attack, fade to pixel sprite, HP display if applicable for enemy, attack animation with projectile), but all actions and positioning are mirrored.

## 7. Attack Resolution / Duel (Novel Mechanic - To Be Designed)
*   **Current State**: In the device being referenced, the two attack projectile sprites would meet on a new screen, and the player would engage in a minigame to determine the outcome of the clash.
*   **Future Goal**: A novel and unique gameplay mechanic will be designed and implemented for this phase, replacing the direct copy of the existing device's duel.
*   **Outcome**: The result of this novel mechanic will determine which Digimon's attack successfully lands.
*   **Damage**: How much damage attacks deal, and considerations for Digimon levels, will be determined at a later stage of development.

## 8. Victory and Defeat
*   **Player Wins (Enemy HP reaches 0)**:
    1.  A screen shows the player's Digimon, either in its idle animation or a victory/celebration animation (asset dependent).
    2.  If the player's Digimon had Digivolved for the battle, a white fade transition occurs, after which the Digimon is shown in its de-evolved (rookie) form.
    3.  A blue UI bar appears with the text "CLEAR" prominently displayed, signifying victory.
*   **Player Loses (Player HP reaches 0)**:
    *   (The specific screen flow for player defeat will be detailed later.)

## 9. Post-Battle - Return to Map
*   **Transition**:
    1.  The battle outcome screen (or final battle scene) fades to black.
    2.  The black screen fades back into clarity, revealing the continent map screen.
*   **Map Update**: A newly unlocked map node is now visible and accessible to the player on the continent map.
