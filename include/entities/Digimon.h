// include/entities/Digimon.h
#ifndef DIGIMON_H
#define DIGIMON_H

// Definition of the different Digimon types available in the game.
// Moved here from AdventureState.h to be accessible globally.
enum DigimonType {
    NONE = -1,          // Used as default/invalid value
    // Player Digimon
    DIGI_AGUMON,
    DIGI_GABUMON,
    DIGI_BIYOMON,
    DIGI_GATOMON,
    DIGI_GOMAMON,
    DIGI_PALMON,
    DIGI_TENTOMON,
    DIGI_PATAMON,
    
    // Enemy Digimon - Part 1
    DIGI_KUWAGAMON,
    DIGI_ANDROMON,
    DIGI_APOCALYMON,
    DIGI_BAKEMON,
    DIGI_BLOSSOMON,
    DIGI_CENTAURMON,
    DIGI_CHERRYMON,
    DIGI_COCKATRIMON,
    DIGI_COLOSSEUMGREYMON,
    DIGI_DARKTYRANOMON,
    DIGI_DEMIDEVIMON,
    DIGI_DERAMON,
    
    // Enemy Digimon - Part 2
    DIGI_DEVIDRAMON,
    DIGI_DEVIMON,
    DIGI_DEVIWOMON,
    DIGI_DIABLOMON,
    DIGI_DIGITAMAMON,
    DIGI_DIVERMON,
    DIGI_DOKUGUMON,
    DIGI_DRIMOGEMON,
    DIGI_ELECMON,
    DIGI_ETEMON,
    DIGI_EVILMON,
    DIGI_FLORAMON,
    DIGI_FLYMON,
    DIGI_FRIGIMON,
    DIGI_GARBAGEMON,
    DIGI_GAZIMON,
    DIGI_GEKOMON,
    DIGI_GESOMON,
    DIGI_GIGADRAMON,
    DIGI_GIZAMON,
    
    // Enemy Digimon - Part 3
    DIGI_GOTSUMON,
    DIGI_HAGURUMON,
    DIGI_INFERMON,
    DIGI_KERAMON,
    DIGI_KIWIMON,
    DIGI_LEOMON,
    DIGI_MACHINEDRAMON,
    DIGI_MAMMOTHMON,
    DIGI_MEGADRAMON,
    DIGI_MEGASEADRAMON,
    DIGI_MEKANORIMON,
    DIGI_MERAMON,
    
    // Enemy Digimon - Part 4
    DIGI_METALETEMON,
    DIGI_METALSEADRAMON,
    DIGI_MOJYAMON,
    DIGI_MONOCHROMON,
    DIGI_MONZAEMON,
    DIGI_MUSHMON,
    DIGI_MYOTISMON,
    DIGI_NANIMON,
    DIGI_NANOMON,
    DIGI_NUMEMON,
    DIGI_OGREMON,
    DIGI_OTAMAMON,
    DIGI_PARROTMON,
    DIGI_PHANTOMON,
    DIGI_PIEDMON,
    
    // Enemy Digimon - Part 5
    DIGI_PIXIMON,
    DIGI_PUMPMON,
    DIGI_PUPPETMON,
    DIGI_RAREMON,
    DIGI_REDVEGIMON,
    DIGI_SABRELEOMON,
    DIGI_SCORPIOMON,
    DIGI_SEADRAMON,
    DIGI_SHELLMON,
    DIGI_SHOGUNGEKOMON,
    DIGI_SKULLMERAMON,
    DIGI_SNIMON,
    DIGI_SUKAMON,
    DIGI_TANKMON,
    DIGI_TUSKMON,
    DIGI_TYRANOMON,
    DIGI_UNIMON,
    DIGI_VADEMON,
    DIGI_VEGIMON,
    DIGI_VENOMMYOTISMON,
    DIGI_WARUMONZAEMON,
    DIGI_WHAMON,
    
    DIGI_COUNT // Helper to get the number of digimon types (useful for loops/arrays)
};

// Consider adding helper functions here later if needed, e.g.:
// std::string digimonTypeToString(DigimonType type);
// SDL_Texture* getDigimonSpriteSheet(DigimonType type, AssetManager* assets); // etc.

#endif // DIGIMON_H