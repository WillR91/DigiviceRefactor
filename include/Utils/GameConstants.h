// File: include/Utils/GameConstants.h

#ifndef GAME_CONSTANTS_H
#define GAME_CONSTANTS_H

namespace Constants {

    // Define your standard background dimensions here
    // Replace 240 and 160 with the actual width and height you intend to use!
    static constexpr int BACKGROUND_WIDTH = 1421;
    static constexpr int BACKGROUND_HEIGHT = 474;    // Global sprite scaling factor
    // For memory efficiency, sprite assets will be stored at 1x scale
    // and rendered at this scale factor
    static constexpr float SPRITE_SCALE_FACTOR = 1.0f;  // No scaling for now (1x)

    // You can add other game-wide constants here later if needed
    // static constexpr int SCREEN_WIDTH = 240;
    // static constexpr int SCREEN_HEIGHT = 160;

} // namespace Constants

#endif // GAME_CONSTANTS_H