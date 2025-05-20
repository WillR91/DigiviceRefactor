// include/Core/PlayerData.h
#ifndef PLAYERDATA_H
#define PLAYERDATA_H

#include <string> // For std::string
#include "Core/MapData.h" // Include MapData for NodeData struct
#include "entities/DigimonDefinition.h" // <<< ADDED for DigimonDefinition
#include "entities/DigimonRegistry.h"   // <<< ADDED for DigimonRegistry access

// --- REMOVED TEMPORARY DIAGNOSTIC ---

class PlayerData {
public:
    // --- Constructor ---
    PlayerData() :
        currentPartnerId_("agumon"), // Default to Agumon's ID
        stepsTakenThisChapter(0),
        totalSteps(0),
        stepGoal(0),
        dPower(0),
        currentMapNode("Chapter1_Area1")
    {}

    // --- Member Variables ---
    // DigimonType currentPartner; // <<< REPLACED
    std::string currentPartnerId_; // Stores the ID of the current partner Digimon
    int stepsTakenThisChapter;
    int totalSteps;
    int stepGoal;  // Goal for current node/chapter
    int dPower;
    std::string currentMapNode;
    Digivice::NodeData currentNodeData; // Store the full NodeData for the current map node
    
    // --- Methods for Partner Digimon ---
    const std::string& getCurrentPartnerId() const { return currentPartnerId_; }
    void setCurrentPartnerId(const std::string& partnerId) { currentPartnerId_ = partnerId; }

    // Retrieves the full definition of the current partner.
    // Requires the DigimonRegistry to look up the ID.
    const Digimon::DigimonDefinition* getCurrentPartnerDefinition(const Digimon::DigimonRegistry* registry) const {
        if (!registry) {
            // SDL_LogWarn or similar for error handling if registry is null
            return nullptr;
        }
        return registry->getDefinitionById(currentPartnerId_);
    }

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