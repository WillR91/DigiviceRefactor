#include "States/MapSystemState.h"
// Potentially include other necessary headers here later, e.g., for AssetManager, InputManager, GameManager
// #include "Platform/PCDisplay.h" // Temporarily removed to resolve build error; will re-add correctly later.

#include "Core/Game.h" // Assuming Game.h is needed for Game* in constructor
#include "Core/InputManager.h" // For handle_input parameter
#include "Core/PlayerData.h"   // For handle_input/update parameters

namespace Digivice {

    MapSystemState::MapSystemState(Game* game) // Uncommented Game* game parameter
        : GameState(game), // Uncommented game, passing it to GameState constructor
          currentView_(MapView::CONTINENT_SELECTION),
          currentContinentIndex_(0),
          currentNodeIndex_(0) {
        // Constructor body
    }

    MapSystemState::~MapSystemState() {
        // Destructor body
    }

    void MapSystemState::enter() {
        load_map_data();
        currentView_ = MapView::CONTINENT_SELECTION;
    }

    void MapSystemState::exit() {
        // Cleanup if needed
    }

    // Corrected signature
    void MapSystemState::handle_input(InputManager& inputManager, PlayerData* playerData) {
        switch (currentView_) {
            case MapView::CONTINENT_SELECTION:
                // handle_input_continent_selection(inputManager, playerData);
                break;
            case MapView::NODE_SELECTION:
                // handle_input_node_selection(inputManager, playerData);
                break;
            case MapView::NODE_DETAIL:
                // handle_input_node_detail(inputManager, playerData);
                break;
        }
    }

    // Corrected signature
    void MapSystemState::update(float dt, PlayerData* playerData) {
        switch (currentView_) {
            case MapView::CONTINENT_SELECTION:
                // update_continent_selection(dt, playerData);
                break;
            case MapView::NODE_SELECTION:
                // update_node_selection(dt, playerData);
                break;
            case MapView::NODE_DETAIL:
                // update_node_detail(dt, playerData);
                break;
        }
    }

    // Corrected signature
    void MapSystemState::render(PCDisplay& display) {
        // Clear screen or prepare rendering context if needed
        switch (currentView_) {
            case MapView::CONTINENT_SELECTION:
                // render_continent_selection(display);
                break;
            case MapView::NODE_SELECTION:
                // render_node_selection(display);
                break;
            case MapView::NODE_DETAIL:
                // render_node_detail(display);
                break;
        }
    }
    
    // Corrected signature
    StateType MapSystemState::getType() const {
        return StateType::MAP_SYSTEM; // Assuming you'll add MAP_SYSTEM to your StateType enum
    }

    // --- Private Helper Methods --- 

    void MapSystemState::load_map_data() {
        continents_.clear(); 

        // --- Define File Island ---
        ContinentData fileIsland;
        fileIsland.id = "file_island";
        fileIsland.name = "File Island";
        fileIsland.mapImagePath = "assets/backgrounds/environments/file_island/file_island_map.png";

        // --- Define Node: 01_tropical_jungle ---
        NodeData tropicalJungleNode;
        tropicalJungleNode.id = "file_island_node_01";
        tropicalJungleNode.name = "Tropical Jungle";
        tropicalJungleNode.continentId = fileIsland.id;
        tropicalJungleNode.mapPositionX = 100.0f; // Placeholder X on continent map
        tropicalJungleNode.mapPositionY = 150.0f; // Placeholder Y on continent map
        tropicalJungleNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png"; // Placeholder
        tropicalJungleNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_tj_idle.png"; // Placeholder
        tropicalJungleNode.totalSteps = 400;    // Placeholder
        tropicalJungleNode.isUnlocked = true;   // Unlocked for prototype

        // Background Layers for Tropical Jungle (Order: FG, MG, BG as per docs)
        // Layer 0: Foreground
        tropicalJungleNode.adventureBackgroundLayers.push_back(
            BackgroundLayerData(
                {"assets/backgrounds/environments/file_island/01_tropical_jungle/layer_0.png"}, // texturePaths
                0.5f,  // parallaxFactorX
                0.0f   // parallaxFactorY (assuming horizontal scroll only)
            )
        );
        // Layer 1: Midground
        tropicalJungleNode.adventureBackgroundLayers.push_back(
            BackgroundLayerData(
                {"assets/backgrounds/environments/file_island/01_tropical_jungle/layer_1.png"}, // texturePaths
                0.25f, // parallaxFactorX
                0.0f   // parallaxFactorY
            )
        );
        // Layer 2: Background
        tropicalJungleNode.adventureBackgroundLayers.push_back(
            BackgroundLayerData(
                {"assets/backgrounds/environments/file_island/01_tropical_jungle/layer_2.png"}, // texturePaths
                0.1f,  // parallaxFactorX
                0.0f   // parallaxFactorY
            )
        );
        fileIsland.nodes.push_back(tropicalJungleNode);

        // TODO: Add other nodes for File Island here later (Lake, Gear Savannah, etc.)

        continents_.push_back(fileIsland);

        // Initialize indices
        if (!continents_.empty()) {
            currentContinentIndex_ = 0;
            if (!continents_[0].nodes.empty()) {
                currentNodeIndex_ = 0;
            } else {
                currentNodeIndex_ = -1; // No nodes on the current continent
            }
        } else {
            currentContinentIndex_ = -1; // No continents
            currentNodeIndex_ = -1;    // No nodes
        }
    }

    void MapSystemState::handle_input_continent_selection(float dt) {}
    void MapSystemState::handle_input_node_selection(float dt) {}
    void MapSystemState::handle_input_node_detail(float dt) {}

    void MapSystemState::update_continent_selection(float dt) {}
    void MapSystemState::update_node_selection(float dt) {}
    void MapSystemState::update_node_detail(float dt) {}

    void MapSystemState::render_continent_selection() {}
    void MapSystemState::render_node_selection() {}
    void MapSystemState::render_node_detail() {}

} // namespace Digivice
