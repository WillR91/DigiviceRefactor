// include/entities/Digimon.h
#ifndef DIGIMON_H
#define DIGIMON_H

// Definition of the different Digimon types available in the game.
// Moved here from AdventureState.h to be accessible globally.
enum DigimonType {
    DIGI_AGUMON,
    DIGI_GABUMON,
    DIGI_BIYOMON,
    DIGI_GATOMON,
    DIGI_GOMAMON,
    DIGI_PALMON,
    DIGI_TENTOMON,
    DIGI_PATAMON,
    DIGI_KUWAGAMON,
    DIGI_COUNT // Helper to get the number of digimon types (useful for loops/arrays)
};

// Consider adding helper functions here later if needed, e.g.:
// std::string digimonTypeToString(DigimonType type);
// SDL_Texture* getDigimonSpriteSheet(DigimonType type, AssetManager* assets); // etc.

#endif // DIGIMON_H