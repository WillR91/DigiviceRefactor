// include/Core/PlayerData.h
#ifndef PLAYERDATA_H
#define PLAYERDATA_H

#include <string> // For std::string
#include "entities/Digimon.h" // Includes the enum definition

// --- REMOVED TEMPORARY DIAGNOSTIC ---

class PlayerData {
public:
    // --- Constructor ---
    PlayerData() :
        currentPartner(DIGI_AGUMON), // Uses the enum member
        stepsTakenThisChapter(0),
        totalSteps(0),
        dPower(0),
        currentMapNode("Chapter1_Area1")
    {}

    // --- Member Variables ---
    DigimonType currentPartner; // Uses the enum type
    int stepsTakenThisChapter;
    int totalSteps;
    int dPower;
    std::string currentMapNode;
    // ... Potential Future Methods ...
};

#endif // PLAYERDATA_H