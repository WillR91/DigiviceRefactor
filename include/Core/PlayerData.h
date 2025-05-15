// include/Core/PlayerData.h
#ifndef PLAYERDATA_H
#define PLAYERDATA_H

#include <string> // For std::string
#include "entities/Digimon.h" // Includes the enum definition
#include "Core/MapData.h" // Include MapData for NodeData struct

// --- REMOVED TEMPORARY DIAGNOSTIC ---

class PlayerData {
public:
    // --- Constructor ---
    PlayerData() :
        currentPartner(DIGI_AGUMON), // Uses the enum member
        stepsTakenThisChapter(0),
        totalSteps(0),
        stepGoal(0),
        dPower(0),
        currentMapNode("Chapter1_Area1")
    {}

    // --- Member Variables ---
    DigimonType currentPartner; // Uses the enum type
    int stepsTakenThisChapter;
    int totalSteps;
    int stepGoal;  // Goal for current node/chapter
    int dPower;
    std::string currentMapNode;
    Digivice::NodeData currentNodeData; // Store the full NodeData for the current map node
    
    // --- Methods for Map System Integration ---
    const std::string& getCurrentMapNode() const { return currentMapNode; }
    void setCurrentMapNode(const std::string& nodeId) { currentMapNode = nodeId; }
    
    // New methods to handle NodeData
    const Digivice::NodeData& getCurrentNodeData() const { return currentNodeData; }
    void setCurrentMapNode(const Digivice::NodeData& nodeData) { 
        currentNodeData = nodeData; 
        currentMapNode = nodeData.id; 
        stepGoal = nodeData.totalSteps;
    }
    
    int getStepGoal() const { return stepGoal; }
    void setStepGoal(int goal) { stepGoal = goal; }
    
    // ... Potential Future Methods ...
};

#endif // PLAYERDATA_H