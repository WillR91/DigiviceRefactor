## Explanation of the AdventureState to BattleState Flash Fix

**The Problem (The Flash/Blink):**

Imagine two people, Alex (representing the `AdventureState` or adventure screen) and Bailey (representing the `BattleState` or battle screen), who are taking turns showing a picture on a screen.

1.  **Alex's Turn Ends:** Alex is showing his adventure picture. When it's time for Bailey's turn (a battle starts), Alex first fades his picture to complete black.
2.  **Telling Bailey to Start:** As soon as Alex's picture is black, he tells Bailey, "Okay, your turn!"
3.  **The Mis-timed Moment (The Flash):** Here's the tricky part. Sometimes, right after Alex says "Your turn!" but *before* Bailey has a chance to put up her first picture (which also starts with a fade-in from black), the main game system quickly asks Alex, "Hey Alex, just show your picture one last time, real quick!" This can happen because of how the game loop processes updates and rendering.
4.  **Alex's Mistake:** If Alex, thinking his job was done once he told Bailey to start, had already removed his black fade overlay (the thing making his screen black), he would accidentally show his bright adventure picture again for that split second.
5.  **Bailey Starts:** Then, Bailey would start her turn, correctly fading her battle scene in from black.

That split second where Alex accidentally showed his bright adventure picture again is the "flash" you were seeing.

**The Fix:**

The fix is to tell Alex: "Even after you've faded to black and told Bailey it's her turn, keep your black screen up until you are *officially* told you are off-duty and no longer the active screen (i.e., when your `exit()` method is called, or in this specific case, until `enter()` is called again which resets the fade)."

So now:

1.  Alex fades his adventure picture to black.
2.  Alex tells Bailey, "Your turn!"
3.  If the game system asks Alex to show his picture one last time before Bailey is fully ready, Alex *still* shows a completely black screen because he hasn't removed his black fade overlay yet. He keeps `is_fading_to_battle_` as `true` and `battle_fade_alpha_` at `255` (fully opaque black).
4.  Bailey then starts her turn, fading her battle scene in from black.

This way, the screen stays black during that tiny gap, and the flash is gone. We made sure that `AdventureState` doesn't reset its `is_fading_to_battle_` flag or its `battle_fade_alpha_` back to transparent in its `update` method after requesting the `BattleState`. Instead, these are reset when `AdventureState::enter()` is called again, ensuring it continues to render a black screen if it's rendered before `BattleState` takes over completely or if it's rendered in an inactive state.
