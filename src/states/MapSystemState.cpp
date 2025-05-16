#include "states/MapSystemState.h"
#include "Core/Game.h" 
#include "Core/InputManager.h" 
#include "Core/PlayerData.h"   
#include "Core/AssetManager.h" // Added for loading textures
#include "UI/TextRenderer.h"   // Added for rendering text
#include "platform/pc/pc_display.h" // Corrected path for PCDisplay.h
#include "../../include/states/adventurestate.h" // Ensure AdventureState is included

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
        // Handle CANCEL first, as it's common to all views
        if (inputManager.isActionJustPressed(GameAction::CANCEL)) { // Corrected: isActionJustPressed and GameAction::
            if (currentView_ == MapView::NODE_DETAIL) {
                currentView_ = MapView::NODE_SELECTION;
                SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Map: CANCELLED from Node Detail to Node Selection.");
                return; 
            } else if (currentView_ == MapView::NODE_SELECTION) {
                currentView_ = MapView::CONTINENT_SELECTION;
                SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Map: CANCELLED from Node Selection to Continent Selection.");
                return; 
            } else if (currentView_ == MapView::CONTINENT_SELECTION) {
                if (game_ptr) {
                    SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Map: CANCELLED from Continent Selection. Exiting MapSystemState.");
                    game_ptr->requestPopState(); 
                }
                return; 
            }
        }

        switch (currentView_) {
            case MapView::CONTINENT_SELECTION:
                handle_input_continent_selection(inputManager); // Pass inputManager
                break;
            case MapView::NODE_SELECTION:
                handle_input_node_selection(inputManager);    // Pass inputManager
                break;
            case MapView::NODE_DETAIL:
                handle_input_node_detail(inputManager);       // Pass inputManager
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
        // display.clear(0, 0, 0); // Example: Clear to black
        switch (currentView_) {
            case MapView::CONTINENT_SELECTION:
                render_continent_selection(display); // Pass display
                break;
            case MapView::NODE_SELECTION:
                render_node_selection(display);    // Pass display
                break;
            case MapView::NODE_DETAIL:
                render_node_detail(display);       // Pass display
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
        fileIsland.id = "01_file_island"; // Changed to match folder name
        fileIsland.name = "FILE ISLAND"; 
        fileIsland.mapImagePath = "assets/backgrounds/environments/01_file_island/file_island_map.png"; // Adjusted path
        // --- Define Node: 01_tropical_jungle ---
        NodeData tropicalJungleNode;
        tropicalJungleNode.id = "01_fi_node_01_tropical_jungle"; // Adjusted ID prefix
        tropicalJungleNode.name = "TROPICAL JUNGLE"; 
        tropicalJungleNode.continentId = fileIsland.id;
        tropicalJungleNode.mapPositionX = 120.0f; 
        tropicalJungleNode.mapPositionY = 150.0f; 
        tropicalJungleNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png"; 
        tropicalJungleNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_tj_idle.png"; 
        tropicalJungleNode.totalSteps = 20; 
        tropicalJungleNode.isUnlocked = true;

        tropicalJungleNode.adventureBackgroundLayers.push_back(
            BackgroundLayerData(
                {"assets/backgrounds/environments/01_file_island/01_tropical_jungle/layer_0.png"}, 
                0.5f,  
                0.0f   
            )
        );
        tropicalJungleNode.adventureBackgroundLayers.push_back(
            BackgroundLayerData(
                {"assets/backgrounds/environments/01_file_island/01_tropical_jungle/layer_1.png"}, 
                0.25f, 
                0.0f   
            )
        );
        tropicalJungleNode.adventureBackgroundLayers.push_back(
            BackgroundLayerData(
                {"assets/backgrounds/environments/01_file_island/01_tropical_jungle/layer_2.png"}, 
                0.1f,  
                0.0f   
            )
        );
        fileIsland.nodes.push_back(tropicalJungleNode);
        // --- Define Node: 02_lake ---
        NodeData lakeNode;
        lakeNode.id = "01_fi_node_02_lake"; // Adjusted ID prefix
        lakeNode.name = "LAKE";
        lakeNode.continentId = fileIsland.id;
        lakeNode.mapPositionX = 200.0f; 
        lakeNode.mapPositionY = 100.0f; 
        lakeNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        lakeNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png"; 
        lakeNode.totalSteps = 20; 
        lakeNode.isUnlocked = true; 
        lakeNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/01_file_island/02_lake/layer_0.png"}, 0.5f, 0.0f));
        // Note: Lake has multiple layer_1 and layer_2 files, this is a simplified representation.
        // You might need a more complex loading or data structure if these are used simultaneously or for animation.
        // For now, picking one of each for simplicity.
        lakeNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/01_file_island/02_lake/layer_1_0.png"}, 0.25f, 0.0f)); 
        lakeNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/01_file_island/02_lake/layer_2_0.png"}, 0.1f, 0.0f));
        fileIsland.nodes.push_back(lakeNode);
        // --- Define Node: 03_gear_savannah ---
        NodeData gearSavannahNode;
        gearSavannahNode.id = "01_fi_node_03_gear_savannah"; // Adjusted ID prefix
        gearSavannahNode.name = "GEAR SAVANNAH";
        gearSavannahNode.continentId = fileIsland.id;
        gearSavannahNode.mapPositionX = 300.0f; 
        gearSavannahNode.mapPositionY = 180.0f; 
        gearSavannahNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        gearSavannahNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png"; 
        gearSavannahNode.totalSteps = 20; 
        gearSavannahNode.isUnlocked = true; 
        gearSavannahNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/01_file_island/03_gear_savannah/layer_0.png"}, 0.5f, 0.0f));
        gearSavannahNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/01_file_island/03_gear_savannah/layer_1.png"}, 0.25f, 0.0f));
        gearSavannahNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/01_file_island/03_gear_savannah/layer_2.png"}, 0.1f, 0.0f));
        fileIsland.nodes.push_back(gearSavannahNode);
        // --- Define Node: 04_factorial_town ---
        NodeData factorialTownNode;
        factorialTownNode.id = "01_fi_node_04_factorial_town"; // Adjusted ID prefix
        factorialTownNode.name = "FACTORIAL TOWN";
        factorialTownNode.continentId = fileIsland.id;
        factorialTownNode.mapPositionX = 350.0f; 
        factorialTownNode.mapPositionY = 120.0f; 
        factorialTownNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        factorialTownNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png"; 
        factorialTownNode.totalSteps = 20; 
        factorialTownNode.isUnlocked = true; 
        factorialTownNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/01_file_island/04_factorial_town/layer_0.png"}, 0.5f, 0.0f));
        factorialTownNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/01_file_island/04_factorial_town/layer_1.png"}, 0.25f, 0.0f));
        factorialTownNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/01_file_island/04_factorial_town/layer_2.png"}, 0.1f, 0.0f));
        fileIsland.nodes.push_back(factorialTownNode);
        // --- Define Node: 05_toy_town ---
        NodeData toyTownNode;
        toyTownNode.id = "01_fi_node_05_toy_town"; // Adjusted ID prefix
        toyTownNode.name = "TOY TOWN";
        toyTownNode.continentId = fileIsland.id;
        toyTownNode.mapPositionX = 250.0f; 
        toyTownNode.mapPositionY = 220.0f; 
        toyTownNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        toyTownNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png"; 
        toyTownNode.totalSteps = 20; 
        toyTownNode.isUnlocked = true; 
        toyTownNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/01_file_island/05_toy_town/layer_0.png"}, 0.5f, 0.0f));
        // Note: Toy Town has multiple layer_2 files. Using one for simplicity.
        toyTownNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/01_file_island/05_toy_town/layer_1.png"}, 0.25f, 0.0f));
        toyTownNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/01_file_island/05_toy_town/layer_2_0.png"}, 0.1f, 0.0f));
        fileIsland.nodes.push_back(toyTownNode);
        // --- Define Node: 06_infinity_mountain ---
        NodeData infinityMountainNode;
        infinityMountainNode.id = "01_fi_node_06_infinity_mountain"; // Adjusted ID prefix
        infinityMountainNode.name = "INFINITY MOUNTAIN";
        infinityMountainNode.continentId = fileIsland.id;
        infinityMountainNode.mapPositionX = 180.0f; 
        infinityMountainNode.mapPositionY = 280.0f; 
        infinityMountainNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        infinityMountainNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png"; 
        infinityMountainNode.totalSteps = 20; 
        infinityMountainNode.isUnlocked = true; 
        infinityMountainNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/01_file_island/06_infinity_mountain/layer_0.png"}, 0.5f, 0.0f));
        infinityMountainNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/01_file_island/06_infinity_mountain/layer_1.png"}, 0.25f, 0.0f));
        infinityMountainNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/01_file_island/06_infinity_mountain/layer_2.png"}, 0.1f, 0.0f));
        fileIsland.nodes.push_back(infinityMountainNode);
        continents_.push_back(fileIsland);

        // --- Define File Island Broken ---
        ContinentData fileIslandBroken;
        fileIslandBroken.id = "02_file_island_broken";
        fileIslandBroken.name = "FILE ISLAND BROKEN";
        fileIslandBroken.mapImagePath = "assets/backgrounds/environments/02_file_island_broken/file_island_broken_map.png";

        // Node: 01_freezeland
        NodeData freezelandNode;
        freezelandNode.id = "02_fib_node_01_freezeland"; 
        freezelandNode.name = "FREEZELAND";
        freezelandNode.continentId = fileIslandBroken.id;
        freezelandNode.mapPositionX = 100.0f; 
        freezelandNode.mapPositionY = 100.0f;
        freezelandNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        freezelandNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        freezelandNode.totalSteps = 20;
        freezelandNode.isUnlocked = true;
        freezelandNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/02_file_island_broken/01_freezeland/layer_0.png"}, 0.5f, 0.0f));
        freezelandNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/02_file_island_broken/01_freezeland/layer_1.png"}, 0.25f, 0.0f));
        freezelandNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/02_file_island_broken/01_freezeland/layer_2.png"}, 0.1f, 0.0f));
        fileIslandBroken.nodes.push_back(freezelandNode);

        // Node: 02_ancientdinoregion
        NodeData ancientDinoRegionNode;
        ancientDinoRegionNode.id = "02_fib_node_02_ancientdinoregion";
        ancientDinoRegionNode.name = "ANCIENT DINO REGION";
        ancientDinoRegionNode.continentId = fileIslandBroken.id;
        ancientDinoRegionNode.mapPositionX = 200.0f; 
        ancientDinoRegionNode.mapPositionY = 100.0f;
        ancientDinoRegionNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        ancientDinoRegionNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        ancientDinoRegionNode.totalSteps = 20;
        ancientDinoRegionNode.isUnlocked = true;
        ancientDinoRegionNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/02_file_island_broken/02_ancientdinoregion/layer_0.png"}, 0.5f, 0.0f));
        ancientDinoRegionNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/02_file_island_broken/02_ancientdinoregion/layer_1.png"}, 0.25f, 0.0f));
        ancientDinoRegionNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/02_file_island_broken/02_ancientdinoregion/layer_2.png"}, 0.1f, 0.0f));
        fileIslandBroken.nodes.push_back(ancientDinoRegionNode);
        
        // Node: 03_overdellcemetary
        NodeData overdellCemetaryNode;
        overdellCemetaryNode.id = "02_fib_node_03_overdellcemetary";
        overdellCemetaryNode.name = "OVERDELL CEMETARY";
        overdellCemetaryNode.continentId = fileIslandBroken.id;
        overdellCemetaryNode.mapPositionX = 300.0f;
        overdellCemetaryNode.mapPositionY = 100.0f;
        overdellCemetaryNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        overdellCemetaryNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        overdellCemetaryNode.totalSteps = 20;
        overdellCemetaryNode.isUnlocked = true;
        overdellCemetaryNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/02_file_island_broken/03_overdellcemetary/layer_0.png"}, 0.5f, 0.0f));
        overdellCemetaryNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/02_file_island_broken/03_overdellcemetary/layer_1.png"}, 0.25f, 0.0f));
        overdellCemetaryNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/02_file_island_broken/03_overdellcemetary/layer_2.png"}, 0.1f, 0.0f));
        fileIslandBroken.nodes.push_back(overdellCemetaryNode);

        // Node: 04_townofbeginnings
        NodeData townOfBeginningsNode;
        townOfBeginningsNode.id = "02_fib_node_04_townofbeginnings";
        townOfBeginningsNode.name = "TOWN OF BEGINNINGS";
        townOfBeginningsNode.continentId = fileIslandBroken.id;
        townOfBeginningsNode.mapPositionX = 100.0f;
        townOfBeginningsNode.mapPositionY = 200.0f;
        townOfBeginningsNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        townOfBeginningsNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        townOfBeginningsNode.totalSteps = 20;
        townOfBeginningsNode.isUnlocked = true;
        townOfBeginningsNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/02_file_island_broken/04_townofbeginnings/layer_0.png"}, 0.5f, 0.0f));
        townOfBeginningsNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/02_file_island_broken/04_townofbeginnings/layer_1.png"}, 0.25f, 0.0f));
        townOfBeginningsNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/02_file_island_broken/04_townofbeginnings/layer_2.png"}, 0.1f, 0.0f));
        fileIslandBroken.nodes.push_back(townOfBeginningsNode);
        
        // Node: 05_infinitymountain2
        NodeData infinityMountain2Node;
        infinityMountain2Node.id = "02_fib_node_05_infinitymountain2";
        infinityMountain2Node.name = "INFINITY MOUNTAIN 2";
        infinityMountain2Node.continentId = fileIslandBroken.id;
        infinityMountain2Node.mapPositionX = 200.0f;
        infinityMountain2Node.mapPositionY = 200.0f;
        infinityMountain2Node.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        infinityMountain2Node.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        infinityMountain2Node.totalSteps = 20;
        infinityMountain2Node.isUnlocked = true;
        infinityMountain2Node.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/02_file_island_broken/05_infinitymountain2/layer_0.png"}, 0.5f, 0.0f));
        infinityMountain2Node.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/02_file_island_broken/05_infinitymountain2/layer_1.png"}, 0.25f, 0.0f));
        infinityMountain2Node.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/02_file_island_broken/05_infinitymountain2/layer_2.png"}, 0.1f, 0.0f));
        fileIslandBroken.nodes.push_back(infinityMountain2Node);

        // Node: 06_opensea
        NodeData openSeaNode;
        openSeaNode.id = "02_fib_node_06_opensea";
        openSeaNode.name = "OPEN SEA";
        openSeaNode.continentId = fileIslandBroken.id;
        openSeaNode.mapPositionX = 300.0f;
        openSeaNode.mapPositionY = 200.0f;
        openSeaNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        openSeaNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        openSeaNode.totalSteps = 20;
        openSeaNode.isUnlocked = true;
        openSeaNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/02_file_island_broken/06_opensea/layer_0.png"}, 0.5f, 0.0f));
        openSeaNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/02_file_island_broken/06_opensea/layer_1.png"}, 0.25f, 0.0f));
        openSeaNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/02_file_island_broken/06_opensea/layer_2.png"}, 0.1f, 0.0f));
        fileIslandBroken.nodes.push_back(openSeaNode);

        continents_.push_back(fileIslandBroken);

        // --- Define Server Continent ---
        ContinentData serverContinent;
        serverContinent.id = "03_server_continent";
        serverContinent.name = "SERVER CONTINENT";
        serverContinent.mapImagePath = "assets/backgrounds/environments/03_server_continent/server_continent_map.png";
        
        NodeData koromonsVillageNode;
        koromonsVillageNode.id = "03_sc_node_01_koromonsvillage";
        koromonsVillageNode.name = "KOROMONS VILLAGE";
        koromonsVillageNode.continentId = serverContinent.id;
        koromonsVillageNode.mapPositionX = 100.0f; 
        koromonsVillageNode.mapPositionY = 100.0f;
        koromonsVillageNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        koromonsVillageNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        koromonsVillageNode.totalSteps = 20;
        koromonsVillageNode.isUnlocked = true;
        koromonsVillageNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/01_koromonsvillage/layer_0.png"}, 0.5f, 0.0f));
        koromonsVillageNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/01_koromonsvillage/layer_1.png"}, 0.25f, 0.0f));
        koromonsVillageNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/01_koromonsvillage/layer_2.png"}, 0.1f, 0.0f));
        serverContinent.nodes.push_back(koromonsVillageNode);

        NodeData scColosseumNode; 
        scColosseumNode.id = "03_sc_node_02_colosseum";
        scColosseumNode.name = "COLOSSEUM";
        scColosseumNode.continentId = serverContinent.id;
        scColosseumNode.mapPositionX = 150.0f; 
        scColosseumNode.mapPositionY = 100.0f;
        scColosseumNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        scColosseumNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        scColosseumNode.totalSteps = 20;
        scColosseumNode.isUnlocked = true;
        scColosseumNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/02_colosseum/layer_0.png"}, 0.5f, 0.0f));
        scColosseumNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/02_colosseum/layer_1.png"}, 0.25f, 0.0f));
        scColosseumNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/02_colosseum/layer_2.png"}, 0.1f, 0.0f));
        serverContinent.nodes.push_back(scColosseumNode);

        NodeData desertNode;
        desertNode.id = "03_sc_node_03_desert";
        desertNode.name = "DESERT";
        desertNode.continentId = serverContinent.id;
        desertNode.mapPositionX = 200.0f; 
        desertNode.mapPositionY = 100.0f;
        desertNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        desertNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        desertNode.totalSteps = 20;
        desertNode.isUnlocked = true;
        desertNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/03_desert/layer_0.png"}, 0.5f, 0.0f));
        desertNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/03_desert/layer_1.png"}, 0.25f, 0.0f));
        desertNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/03_desert/layer_2.png"}, 0.1f, 0.0f));
        serverContinent.nodes.push_back(desertNode);
        
        NodeData piccolomonsForestNode;
        piccolomonsForestNode.id = "03_sc_node_04_piccolomonsforest";
        piccolomonsForestNode.name = "PICCOLOMONS FOREST";
        piccolomonsForestNode.continentId = serverContinent.id;
        piccolomonsForestNode.mapPositionX = 250.0f; 
        piccolomonsForestNode.mapPositionY = 100.0f;
        piccolomonsForestNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        piccolomonsForestNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        piccolomonsForestNode.totalSteps = 20;
        piccolomonsForestNode.isUnlocked = true;
        piccolomonsForestNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/04_piccolomonsforest/layer_0.png"}, 0.5f, 0.0f));
        piccolomonsForestNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/04_piccolomonsforest/layer_1.png"}, 0.25f, 0.0f));
        piccolomonsForestNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/04_piccolomonsforest/layer_2.png"}, 0.1f, 0.0f));
        serverContinent.nodes.push_back(piccolomonsForestNode);

        NodeData reversePyramidNode;
        reversePyramidNode.id = "03_sc_node_05_reversepyramid";
        reversePyramidNode.name = "REVERSE PYRAMID";
        reversePyramidNode.continentId = serverContinent.id;
        reversePyramidNode.mapPositionX = 300.0f; 
        reversePyramidNode.mapPositionY = 100.0f;
        reversePyramidNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        reversePyramidNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        reversePyramidNode.totalSteps = 20;
        reversePyramidNode.isUnlocked = true;
        reversePyramidNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/05_reversepyramid/layer_0.png"}, 0.5f, 0.0f));
        reversePyramidNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/05_reversepyramid/layer_1.png"}, 0.25f, 0.0f));
        reversePyramidNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/05_reversepyramid/layer_2.png"}, 0.1f, 0.0f));
        serverContinent.nodes.push_back(reversePyramidNode);

        NodeData amusementParkNode;
        amusementParkNode.id = "03_sc_node_06_amusementpark";
        amusementParkNode.name = "AMUSEMENT PARK";
        amusementParkNode.continentId = serverContinent.id;
        amusementParkNode.mapPositionX = 100.0f; 
        amusementParkNode.mapPositionY = 200.0f;
        amusementParkNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        amusementParkNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        amusementParkNode.totalSteps = 20;
        amusementParkNode.isUnlocked = true;
        amusementParkNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/06_amusementpark/layer_0.png"}, 0.5f, 0.0f));
        amusementParkNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/06_amusementpark/layer_1.png"}, 0.25f, 0.0f));
        amusementParkNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/06_amusementpark/layer_2.png"}, 0.1f, 0.0f));
        serverContinent.nodes.push_back(amusementParkNode);
        
        NodeData restaurantNode;
        restaurantNode.id = "03_sc_node_07_restaurant";
        restaurantNode.name = "RESTAURANT";
        restaurantNode.continentId = serverContinent.id;
        restaurantNode.mapPositionX = 150.0f; 
        restaurantNode.mapPositionY = 200.0f;
        restaurantNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        restaurantNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        restaurantNode.totalSteps = 20;
        restaurantNode.isUnlocked = true;
        restaurantNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/07_restaurant/layer_0.png"}, 0.5f, 0.0f));
        restaurantNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/07_restaurant/layer_1.png"}, 0.25f, 0.0f));
        restaurantNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/07_restaurant/layer_2.png"}, 0.1f, 0.0f));
        serverContinent.nodes.push_back(restaurantNode);

        NodeData differentSpaceNode;
        differentSpaceNode.id = "03_sc_node_08_differentspace";
        differentSpaceNode.name = "DIFFERENT SPACE";
        differentSpaceNode.continentId = serverContinent.id;
        differentSpaceNode.mapPositionX = 200.0f; 
        differentSpaceNode.mapPositionY = 200.0f;
        differentSpaceNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        differentSpaceNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        differentSpaceNode.totalSteps = 20;
        differentSpaceNode.isUnlocked = true;
        differentSpaceNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/08_differentspace/layer_0.png"}, 0.5f, 0.0f));
        differentSpaceNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/08_differentspace/layer_1.png"}, 0.25f, 0.0f));
        differentSpaceNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/08_differentspace/layer_2.png"}, 0.1f, 0.0f));
        serverContinent.nodes.push_back(differentSpaceNode);
        
        NodeData gekomonsCastleNode;
        gekomonsCastleNode.id = "03_sc_node_09_gekomonscastle";
        gekomonsCastleNode.name = "GEKOMONS CASTLE";
        gekomonsCastleNode.continentId = serverContinent.id;
        gekomonsCastleNode.mapPositionX = 250.0f; 
        gekomonsCastleNode.mapPositionY = 200.0f;
        gekomonsCastleNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        gekomonsCastleNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        gekomonsCastleNode.totalSteps = 20;
        gekomonsCastleNode.isUnlocked = true;
        gekomonsCastleNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/09_gekomonscastle/layer_0.png"}, 0.5f, 0.0f));
        gekomonsCastleNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/09_gekomonscastle/layer_1.png"}, 0.25f, 0.0f));
        gekomonsCastleNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/09_gekomonscastle/layer_2.png"}, 0.1f, 0.0f));
        serverContinent.nodes.push_back(gekomonsCastleNode);

        NodeData myotismonsCastleNode;
        myotismonsCastleNode.id = "03_sc_node_10_myotismonscastle";
        myotismonsCastleNode.name = "MYOTISMONS CASTLE";
        myotismonsCastleNode.continentId = serverContinent.id;
        myotismonsCastleNode.mapPositionX = 300.0f; 
        myotismonsCastleNode.mapPositionY = 200.0f;
        myotismonsCastleNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        myotismonsCastleNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        myotismonsCastleNode.totalSteps = 20;
        myotismonsCastleNode.isUnlocked = true;
        myotismonsCastleNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/10_myotismonscastle/layer_0.png"}, 0.5f, 0.0f));
        myotismonsCastleNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/10_myotismonscastle/layer_1.png"}, 0.25f, 0.0f));
        myotismonsCastleNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/03_server_continent/10_myotismonscastle/layer_2.png"}, 0.1f, 0.0f));
        serverContinent.nodes.push_back(myotismonsCastleNode);

        continents_.push_back(serverContinent);

        // --- Define Tokyo ---
        ContinentData tokyo;
        tokyo.id = "04_tokyo";
        tokyo.name = "TOKYO";
        tokyo.mapImagePath = "assets/backgrounds/environments/04_tokyo/tokyo_map.png";

        NodeData hikarigoakaNode;
        hikarigoakaNode.id = "04_tk_node_01_hikarigoaka";
        hikarigoakaNode.name = "HIKARIGOAKA";
        hikarigoakaNode.continentId = tokyo.id;
        hikarigoakaNode.mapPositionX = 100.0f; 
        hikarigoakaNode.mapPositionY = 100.0f;
        hikarigoakaNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        hikarigoakaNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        hikarigoakaNode.totalSteps = 20;
        hikarigoakaNode.isUnlocked = true;
        hikarigoakaNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/04_tokyo/01_hikarigoaka/layer_0.png"}, 0.5f, 0.0f));
        hikarigoakaNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/04_tokyo/01_hikarigoaka/layer_1.png"}, 0.25f, 0.0f));
        hikarigoakaNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/04_tokyo/01_hikarigoaka/layer_2.png"}, 0.1f, 0.0f));
        tokyo.nodes.push_back(hikarigoakaNode);
        
        NodeData harumiNode;
        harumiNode.id = "04_tk_node_02_harumi";
        harumiNode.name = "HARUMI";
        harumiNode.continentId = tokyo.id;
        harumiNode.mapPositionX = 200.0f; 
        harumiNode.mapPositionY = 100.0f;
        harumiNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        harumiNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        harumiNode.totalSteps = 20;
        harumiNode.isUnlocked = true;
        harumiNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/04_tokyo/02_harumi/layer_0.png"}, 0.5f, 0.0f));
        harumiNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/04_tokyo/02_harumi/layer_1.png"}, 0.25f, 0.0f));
        harumiNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/04_tokyo/02_harumi/layer_2.png"}, 0.1f, 0.0f));
        tokyo.nodes.push_back(harumiNode);

        NodeData pierNode;
        pierNode.id = "04_tk_node_03_pier";
        pierNode.name = "PIER";
        pierNode.continentId = tokyo.id;
        pierNode.mapPositionX = 300.0f; 
        pierNode.mapPositionY = 100.0f;
        pierNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        pierNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        pierNode.totalSteps = 20;
        pierNode.isUnlocked = true;
        pierNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/04_tokyo/03_pier/layer_0.png"}, 0.5f, 0.0f));
        pierNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/04_tokyo/03_pier/layer_1.png"}, 0.25f, 0.0f));
        pierNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/04_tokyo/03_pier/layer_2.png"}, 0.1f, 0.0f));
        tokyo.nodes.push_back(pierNode);

        NodeData towerNode;
        towerNode.id = "04_tk_node_04_tower";
        towerNode.name = "TOWER";
        towerNode.continentId = tokyo.id;
        towerNode.mapPositionX = 100.0f; 
        towerNode.mapPositionY = 200.0f;
        towerNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        towerNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        towerNode.totalSteps = 20;
        towerNode.isUnlocked = true;
        towerNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/04_tokyo/04_tower/layer_0.png"}, 0.5f, 0.0f));
        towerNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/04_tokyo/04_tower/layer_1.png"}, 0.25f, 0.0f));
        towerNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/04_tokyo/04_tower/layer_2.png"}, 0.1f, 0.0f));
        tokyo.nodes.push_back(towerNode);
        
        NodeData shibuyaNode;
        shibuyaNode.id = "04_tk_node_05_shibuya";
        shibuyaNode.name = "SHIBUYA";
        shibuyaNode.continentId = tokyo.id;
        shibuyaNode.mapPositionX = 200.0f; 
        shibuyaNode.mapPositionY = 200.0f;
        shibuyaNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        shibuyaNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        shibuyaNode.totalSteps = 20;
        shibuyaNode.isUnlocked = true;
        shibuyaNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/04_tokyo/05_shibuya/layer_0.png"}, 0.5f, 0.0f));
        shibuyaNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/04_tokyo/05_shibuya/layer_1.png"}, 0.25f, 0.0f));
        shibuyaNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/04_tokyo/05_shibuya/layer_2.png"}, 0.1f, 0.0f));
        tokyo.nodes.push_back(shibuyaNode);

        NodeData exhibitionCentreNode;
        exhibitionCentreNode.id = "04_tk_node_06_exhibitioncentre";
        exhibitionCentreNode.name = "EXHIBITION CENTRE";
        exhibitionCentreNode.continentId = tokyo.id;
        exhibitionCentreNode.mapPositionX = 300.0f; 
        exhibitionCentreNode.mapPositionY = 200.0f;
        exhibitionCentreNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        exhibitionCentreNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        exhibitionCentreNode.totalSteps = 20;
        exhibitionCentreNode.isUnlocked = true;
        exhibitionCentreNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/04_tokyo/06_exhibitioncentre/layer_0.png"}, 0.5f, 0.0f));
        exhibitionCentreNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/04_tokyo/06_exhibitioncentre/layer_1.png"}, 0.25f, 0.0f));
        exhibitionCentreNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/04_tokyo/06_exhibitioncentre/layer_2.png"}, 0.1f, 0.0f));
        tokyo.nodes.push_back(exhibitionCentreNode);

        continents_.push_back(tokyo);

        // --- Define Spiral Mountain ---
        ContinentData spiralMountain;
        spiralMountain.id = "05_spiral_mountain";
        spiralMountain.name = "SPIRAL MOUNTAIN";
        spiralMountain.mapImagePath = "assets/backgrounds/environments/05_spiral_mountain/spiral_mountain_map.png";

        NodeData smJungleNode; 
        smJungleNode.id = "05_sm_node_01_jungle";
        smJungleNode.name = "JUNGLE";
        smJungleNode.continentId = spiralMountain.id;
        smJungleNode.mapPositionX = 100.0f; 
        smJungleNode.mapPositionY = 100.0f;
        smJungleNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        smJungleNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        smJungleNode.totalSteps = 20;
        smJungleNode.isUnlocked = true;
        smJungleNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/05_spiral_mountain/01_jungle/layer_0.png"}, 0.5f, 0.0f));
        smJungleNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/05_spiral_mountain/01_jungle/layer_1.png"}, 0.25f, 0.0f));
        smJungleNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/05_spiral_mountain/01_jungle/layer_2.png"}, 0.1f, 0.0f));
        spiralMountain.nodes.push_back(smJungleNode);
        
        NodeData smColosseum2Node;
        smColosseum2Node.id = "05_sm_node_02_colosseum2";
        smColosseum2Node.name = "COLOSSEUM 2";
        smColosseum2Node.continentId = spiralMountain.id;
        smColosseum2Node.mapPositionX = 200.0f; 
        smColosseum2Node.mapPositionY = 100.0f;
        smColosseum2Node.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        smColosseum2Node.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        smColosseum2Node.totalSteps = 20;
        smColosseum2Node.isUnlocked = true;
        smColosseum2Node.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/05_spiral_mountain/02_colosseum2/layer_0.png"}, 0.5f, 0.0f));
        smColosseum2Node.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/05_spiral_mountain/02_colosseum2/layer_1.png"}, 0.25f, 0.0f));
        smColosseum2Node.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/05_spiral_mountain/02_colosseum2/layer_2.png"}, 0.1f, 0.0f));
        spiralMountain.nodes.push_back(smColosseum2Node);

        NodeData digitalSeaNode;
        digitalSeaNode.id = "05_sm_node_03_digitalsea";
        digitalSeaNode.name = "DIGITAL SEA";
        digitalSeaNode.continentId = spiralMountain.id;
        digitalSeaNode.mapPositionX = 300.0f; 
        digitalSeaNode.mapPositionY = 100.0f;
        digitalSeaNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        digitalSeaNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        digitalSeaNode.totalSteps = 20;
        digitalSeaNode.isUnlocked = true;
        digitalSeaNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/05_spiral_mountain/03_digitalsea/layer_0.png"}, 0.5f, 0.0f));
        digitalSeaNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/05_spiral_mountain/03_digitalsea/layer_1.png"}, 0.25f, 0.0f));
        digitalSeaNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/05_spiral_mountain/03_digitalsea/layer_2.png"}, 0.1f, 0.0f));
        spiralMountain.nodes.push_back(digitalSeaNode);

        NodeData digitalForestNode;
        digitalForestNode.id = "05_sm_node_04_digitalforest";
        digitalForestNode.name = "DIGITAL FOREST";
        digitalForestNode.continentId = spiralMountain.id;
        digitalForestNode.mapPositionX = 100.0f; 
        digitalForestNode.mapPositionY = 200.0f;
        digitalForestNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        digitalForestNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        digitalForestNode.totalSteps = 20;
        digitalForestNode.isUnlocked = true;
        digitalForestNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/05_spiral_mountain/04_digitalforest/layer_0.png"}, 0.5f, 0.0f));
        digitalForestNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/05_spiral_mountain/04_digitalforest/layer_1.png"}, 0.25f, 0.0f));
        digitalForestNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/05_spiral_mountain/04_digitalforest/layer_2.png"}, 0.1f, 0.0f));
        spiralMountain.nodes.push_back(digitalForestNode);
        
        NodeData digitalCityNode;
        digitalCityNode.id = "05_sm_node_05_digitalcity";
        digitalCityNode.name = "DIGITAL CITY";
        digitalCityNode.continentId = spiralMountain.id;
        digitalCityNode.mapPositionX = 200.0f; 
        digitalCityNode.mapPositionY = 200.0f;
        digitalCityNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        digitalCityNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        digitalCityNode.totalSteps = 20;
        digitalCityNode.isUnlocked = true;
        digitalCityNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/05_spiral_mountain/05_digitalcity/layer_0.png"}, 0.5f, 0.0f));
        digitalCityNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/05_spiral_mountain/05_digitalcity/layer_1.png"}, 0.25f, 0.0f));
        digitalCityNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/05_spiral_mountain/05_digitalcity/layer_2.png"}, 0.1f, 0.0f));
        spiralMountain.nodes.push_back(digitalCityNode);

        NodeData wastelandNode;
        wastelandNode.id = "05_sm_node_06_wasteland";
        wastelandNode.name = "WASTELAND";
        wastelandNode.continentId = spiralMountain.id;
        wastelandNode.mapPositionX = 300.0f; 
        wastelandNode.mapPositionY = 200.0f;
        wastelandNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        wastelandNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        wastelandNode.totalSteps = 20;
        wastelandNode.isUnlocked = true;
        wastelandNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/05_spiral_mountain/06_wasteland/layer_0.png"}, 0.5f, 0.0f));
        wastelandNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/05_spiral_mountain/06_wasteland/layer_1.png"}, 0.25f, 0.0f));
        wastelandNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/05_spiral_mountain/06_wasteland/layer_2.png"}, 0.1f, 0.0f));
        spiralMountain.nodes.push_back(wastelandNode);
        
        continents_.push_back(spiralMountain);

        // --- Define Subspace ---
        ContinentData subspace;
        subspace.id = "06_subspace";
        subspace.name = "SUBSPACE";
        subspace.mapImagePath = "assets/backgrounds/environments/06_subspace/subspace_map.png";

        NodeData subspaceNode;
        subspaceNode.id = "06_ss_node_01_subspace";
        subspaceNode.name = "SUBSPACE";
        subspaceNode.continentId = subspace.id;
        subspaceNode.mapPositionX = 150.0f; 
        subspaceNode.mapPositionY = 150.0f; 
        subspaceNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        subspaceNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        subspaceNode.totalSteps = 20;
        subspaceNode.isUnlocked = true;
        subspaceNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/06_subspace/01_subspace/layer_0.png"}, 0.5f, 0.0f));
        subspaceNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/06_subspace/01_subspace/layer_1.png"}, 0.25f, 0.0f));
        subspaceNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/06_subspace/01_subspace/layer_2.png"}, 0.1f, 0.0f));
        subspace.nodes.push_back(subspaceNode);
        continents_.push_back(subspace);

        // --- Define Network ---
        ContinentData network;
        network.id = "07_network";
        network.name = "NETWORK";
        network.mapImagePath = "assets/backgrounds/environments/07_network/network_map.png";

        NodeData networkNode;
        networkNode.id = "07_nw_node_01_network";
        networkNode.name = "NETWORK";
        networkNode.continentId = network.id;
        networkNode.mapPositionX = 150.0f; 
        networkNode.mapPositionY = 150.0f; 
        networkNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        networkNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png";
        networkNode.totalSteps = 20;
        networkNode.isUnlocked = true;
        networkNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/07_network/01_network/layer_0.png"}, 0.5f, 0.0f));
        networkNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/07_network/01_network/layer_1.png"}, 0.25f, 0.0f));
        networkNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/07_network/01_network/layer_2.png"}, 0.1f, 0.0f));
        network.nodes.push_back(networkNode);
        continents_.push_back(network);

        // Log number of continents loaded
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Loaded %zu continents.", continents_.size());
        for (const auto& cont : continents_) {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Continent: %s (%s), Nodes: %zu", cont.id.c_str(), cont.name.c_str(), cont.nodes.size());
            for (const auto& node_entry : cont.nodes) {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "  Node: %s (%s), Layers: %zu", node_entry.id.c_str(), node_entry.name.c_str(), node_entry.adventureBackgroundLayers.size());
            }
        }
    }

    // Replaced helper function definitions with corrected signatures and added input logic
    void MapSystemState::handle_input_continent_selection(InputManager& inputManager) {
        if (continents_.empty()) return;

        if (inputManager.isActionJustPressed(GameAction::NAV_UP)) {
            currentContinentIndex_ = (currentContinentIndex_ == 0) ? static_cast<int>(continents_.size()) - 1 : currentContinentIndex_ - 1;
            SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Map: Continent NAV_UP. New Index: %d (%s)", currentContinentIndex_, continents_[currentContinentIndex_].name.c_str());
        } else if (inputManager.isActionJustPressed(GameAction::NAV_DOWN)) {
            currentContinentIndex_ = (currentContinentIndex_ + 1) % static_cast<int>(continents_.size());
            SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Map: Continent NAV_DOWN. New Index: %d (%s)", currentContinentIndex_, continents_[currentContinentIndex_].name.c_str());
        } else if (inputManager.isActionJustPressed(GameAction::CONFIRM)) {
            if (currentContinentIndex_ >= 0 && currentContinentIndex_ < static_cast<int>(continents_.size())) {
                if (!continents_[currentContinentIndex_].nodes.empty()){
                    currentView_ = MapView::NODE_SELECTION;
                    currentNodeIndex_ = 0; 
                    SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Map: Continent '%s' CONFIRMED. Transitioning to Node Selection. Node: %s", continents_[currentContinentIndex_].name.c_str(), continents_[currentContinentIndex_].nodes[currentNodeIndex_].name.c_str());
                } else {
                    SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Map: Continent '%s' CONFIRMED, but has no nodes.", continents_[currentContinentIndex_].name.c_str());
                }
            }
        }
    }

    void MapSystemState::handle_input_node_selection(InputManager& inputManager) {
        if (currentContinentIndex_ < 0 || currentContinentIndex_ >= static_cast<int>(continents_.size()) || continents_[currentContinentIndex_].nodes.empty()) return;

        const auto& currentNodes = continents_[currentContinentIndex_].nodes;
        if (currentNodes.empty()) return; 

        if (inputManager.isActionJustPressed(GameAction::NAV_UP)) {
            currentNodeIndex_ = (currentNodeIndex_ == 0) ? static_cast<int>(currentNodes.size()) - 1 : currentNodeIndex_ - 1;
            SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Map: Node NAV_UP. New Index: %d (%s)", currentNodeIndex_, currentNodes[currentNodeIndex_].name.c_str());
        } else if (inputManager.isActionJustPressed(GameAction::NAV_DOWN)) {
            currentNodeIndex_ = (currentNodeIndex_ + 1) % static_cast<int>(currentNodes.size());
            SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Map: Node NAV_DOWN. New Index: %d (%s)", currentNodeIndex_, currentNodes[currentNodeIndex_].name.c_str());
        } else if (inputManager.isActionJustPressed(GameAction::NAV_LEFT)) {
            currentNodeIndex_ = (currentNodeIndex_ == 0) ? static_cast<int>(currentNodes.size()) - 1 : currentNodeIndex_ - 1;
            SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Map: Node NAV_LEFT (acts as UP). New Index: %d (%s)", currentNodeIndex_, currentNodes[currentNodeIndex_].name.c_str());
        } else if (inputManager.isActionJustPressed(GameAction::NAV_RIGHT)) {
            currentNodeIndex_ = (currentNodeIndex_ + 1) % static_cast<int>(currentNodes.size());
            SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Map: Node NAV_RIGHT (acts as DOWN). New Index: %d (%s)", currentNodeIndex_, currentNodes[currentNodeIndex_].name.c_str());
        } else if (inputManager.isActionJustPressed(GameAction::CONFIRM)) {
            if (currentNodeIndex_ >= 0 && currentNodeIndex_ < static_cast<int>(currentNodes.size())) {
                if (currentNodes[currentNodeIndex_].isUnlocked) {
                    currentView_ = MapView::NODE_DETAIL;
                    SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Map: Node '%s' CONFIRMED. Transitioning to Node Detail.", currentNodes[currentNodeIndex_].name.c_str());
                } else {
                    SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Map: Node '%s' CONFIRMED, but it is locked.", currentNodes[currentNodeIndex_].name.c_str());
                }
            }
        }
    }

    void MapSystemState::handle_input_node_detail(InputManager& inputManager) {
    // Check for cancel to return to node selection
    if (inputManager.isActionJustPressed(GameAction::CANCEL)) {
        currentView_ = MapView::NODE_SELECTION;
        SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Map: CANCELLED from Node Detail to Node Selection.");
        return;
    }

    // Check for confirm button press to enter the selected node
    if (inputManager.isActionJustPressed(GameAction::CONFIRM)) {
        if (currentContinentIndex_ >= 0 && currentContinentIndex_ < static_cast<int>(continents_.size()) &&
            currentNodeIndex_ >= 0 && currentNodeIndex_ < static_cast<int>(continents_[currentContinentIndex_].nodes.size())) {
            
            // Get the selected node
            const NodeData& selectedNode = continents_[currentContinentIndex_].nodes[currentNodeIndex_];
            
            // Only proceed if the node is unlocked
            if (selectedNode.isUnlocked) {
                // Save the selected node to player data for the AdventureState to use
                if (game_ptr && game_ptr->getPlayerData()) {
                    game_ptr->getPlayerData()->setCurrentMapNode(selectedNode);
                }
                
                // Create and transition to an AdventureState using the normal constructor
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Map: Transitioning to AdventureState for node: %s", selectedNode.name.c_str());
                auto newState = std::make_unique<AdventureState>(game_ptr);
                game_ptr->requestFadeToState(std::move(newState));
            } else {
                SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Map: Node '%s' is locked. Cannot enter.", selectedNode.name.c_str());
            }
        }
    }
}

    void MapSystemState::update_continent_selection(float dt) {}
    void MapSystemState::update_node_selection(float dt) {}
    void MapSystemState::update_node_detail(float dt) {}

    void MapSystemState::render_continent_selection(PCDisplay& display) {
        if (!game_ptr) { 
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MapSystemState::render_continent_selection - game_ptr is null!");
            return;
        }

        AssetManager* assetManager = game_ptr->getAssetManager();
        TextRenderer* textRenderer = game_ptr->getTextRenderer();

        if (!assetManager) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MapSystemState::render_continent_selection - AssetManager is null!");
            return;
        }
        // TextRenderer can be null if font fails to load, handle gracefully

        if (currentContinentIndex_ < 0 || currentContinentIndex_ >= static_cast<int>(continents_.size())) {
            if (textRenderer) {
                // Corrected: renderText to drawText, added display.getRenderer() if drawText needs it (assuming it does based on previous context)
                // drawText signature from TextRenderer.h: void drawText(SDL_Renderer* renderer, const std::string& text, int x, int y, float scale = 1.0f, int kerning = -1) const;
                textRenderer->drawText(display.getRenderer(), "NO CONTINENTS LOADED", 10, 10, 1.0f); // Changed to uppercase
            } else {
                 SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MapSystemState::render_continent_selection - TextRenderer is null, cannot display 'NO CONTINENTS LOADED'.");
            }
            return;
        }

        const auto& continent = continents_[currentContinentIndex_];
        // Create a unique ID for the texture, e.g., by appending "_map" to the continent ID.
        std::string continentMapTextureId = continent.id + "_map_texture"; 

        // Corrected: loadTexture to take ID and path.
        // Only load if not already loaded. AssetManager::getTexture can check this.
        if (!assetManager->getTexture(continentMapTextureId)) { 
            if (!assetManager->loadTexture(continentMapTextureId, continent.mapImagePath)) { // Pass ID and path
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load texture: %s for ID: %s", continent.mapImagePath.c_str(), continentMapTextureId.c_str());
                // Texture will be nullptr if loading fails, handled below
            }
        }
        SDL_Texture* continentMapTexture = assetManager->getTexture(continentMapTextureId);
        
        int screenWidth = 0, screenHeight = 0;
        display.getWindowSize(screenWidth, screenHeight);

        if (continentMapTexture) { 
            SDL_Rect dstRect = {0, 0, screenWidth, screenHeight}; // Define destination rectangle to fill the screen
            // Corrected: drawTexture arguments. PCDisplay::drawTexture takes (texture, srcRect, dstRect, flip)
            // Assuming we want to draw the whole texture, srcRect can be nullptr.
            display.drawTexture(continentMapTexture, nullptr, &dstRect); 
        } else {
            if (textRenderer) {
                // Corrected: renderText to drawText
                std::string errMsg = "ERROR: MAP MISSING FOR " + continent.name; // Changed to uppercase
                // Corrected: getTextWidth to getTextDimensions().x
                SDL_Point errTextSize = textRenderer->getTextDimensions(errMsg); // getTextDimensions returns SDL_Point {w, h}
                textRenderer->drawText(display.getRenderer(), errMsg, screenWidth / 2 - errTextSize.x / 2, screenHeight / 2, 1.0f);
            } else {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MapSystemState::render_continent_selection - TextRenderer is null, cannot display 'ERROR: MAP MISSING'.");
            }
        }

        if (textRenderer) {
            float scale = 0.65f; // Define the scale factor
            SDL_Point unscaled_text_size = textRenderer->getTextDimensions(continent.name);
            float scaled_text_width = static_cast<float>(unscaled_text_size.x) * scale;
            float scaled_text_height = static_cast<float>(unscaled_text_size.y) * scale;

            int textX = (screenWidth - static_cast<int>(scaled_text_width)) / 2;
            int textY = static_cast<int>(static_cast<float>(screenHeight) * 0.75f - scaled_text_height / 2.0f);
            textRenderer->drawText(display.getRenderer(), continent.name, textX, textY, scale);
        }
    }

    void MapSystemState::render_node_selection(PCDisplay& display) {
        if (!game_ptr) return; 
        AssetManager* assetManager = game_ptr->getAssetManager(); // Added for node sprites
        TextRenderer* textRenderer = game_ptr->getTextRenderer();
        
        if (!assetManager) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MapSystemState::render_node_selection - AssetManager is null!");
            // Optionally, display an error using textRenderer if it's available
            if (textRenderer) {
                textRenderer->drawText(display.getRenderer(), "ERROR: ASSET MANAGER NULL", 10, 10, 1.0f);
            }
            return;
        }
        if (!textRenderer) {
             SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MapSystemState::render_node_selection - TextRenderer is null!");
            return; // Ensure textRenderer is not null for subsequent operations
        }

        if (currentContinentIndex_ < 0 || currentContinentIndex_ >= static_cast<int>(continents_.size())) { // Corrected static_cast
            textRenderer->drawText(display.getRenderer(), "NODE SELECTION: NO CONTINENT DATA", 10, 50, 1.0f); // Changed to uppercase
            return;
        }

        const auto& continent = continents_[currentContinentIndex_];
        
        // First, render the continent map as the background for node selection
        std::string continentMapTextureId = continent.id + "_map_texture";
        SDL_Texture* continentMapTexture = assetManager->getTexture(continentMapTextureId);
        if (!continentMapTexture) { // Attempt to load if not found (e.g., if we came here directly for debugging)
            if (assetManager->loadTexture(continentMapTextureId, continent.mapImagePath)) {
                continentMapTexture = assetManager->getTexture(continentMapTextureId);
            }
        }

        int screenWidth = 0, screenHeight = 0;
        display.getWindowSize(screenWidth, screenHeight);

        if (continentMapTexture) {
            SDL_Rect dstRect = {0, 0, screenWidth, screenHeight};
            display.drawTexture(continentMapTexture, nullptr, &dstRect);
        } else {
            textRenderer->drawText(display.getRenderer(), "ERROR: CONTINENT MAP MISSING", screenWidth / 2 - 100, screenHeight / 2, 1.0f); // Changed to uppercase
        }
        
        // Commenting out the rendering of the continent name at the top of the node selection view
        /*
        float continentNameScale = 1.5f;
        SDL_Point unscaled_continent_name_size = textRenderer->getTextDimensions(continent.name);
        float scaled_continent_name_width = static_cast<float>(unscaled_continent_name_size.x) * continentNameScale;
        textRenderer->drawText(display.getRenderer(), continent.name, (screenWidth - static_cast<int>(scaled_continent_name_width)) / 2, 20, continentNameScale); 
        */

        if (continent.nodes.empty()) {
            textRenderer->drawText(display.getRenderer(), "NO NODES ON THIS CONTINENT", 10, 100, 1.0f); // Changed to uppercase
            return;
        }

        if (currentNodeIndex_ < 0 || currentNodeIndex_ >= static_cast<int>(continent.nodes.size())) { // Corrected static_cast
            textRenderer->drawText(display.getRenderer(), "ERROR: INVALID NODE INDEX", 10, 120, 1.0f); // Changed to uppercase
            // Potentially reset currentNodeIndex_ to 0 if valid
            if (!continent.nodes.empty()) currentNodeIndex_ = 0;
            else return; // No nodes to select
        }
          // Render all nodes on the map
        // Iterate through all nodes and render their icons
        const int NODE_ICON_SIZE = 24; // Size for node icons - adjust as needed
        const float SELECTED_SCALE = 1.5f; // Scale factor for the selected node
        
        // First, render all nodes
        for (size_t i = 0; i < continent.nodes.size(); i++) {
            const auto& node = continent.nodes[i];
            bool isSelected = (static_cast<int>(i) == currentNodeIndex_);
            
            // Calculate position for the node icon
            int iconX = static_cast<int>(node.mapPositionX) - (NODE_ICON_SIZE / 2);
            int iconY = static_cast<int>(node.mapPositionY) - (NODE_ICON_SIZE / 2);
            
            // Load the node sprite if needed
            std::string nodeTextureId = node.id + "_icon";
            SDL_Texture* nodeTexture = assetManager->getTexture(nodeTextureId);
            if (!nodeTexture) {
                // If we don't have a specific node icon yet, load the placeholder
                if (assetManager->loadTexture(nodeTextureId, node.unlockedSpritePath)) {
                    nodeTexture = assetManager->getTexture(nodeTextureId);
                }
            }
            
            if (nodeTexture) {
                // Draw the node icon
                SDL_Rect dstRect;
                if (isSelected) {
                    // Make the selected node larger for emphasis
                    int selectedSize = static_cast<int>(NODE_ICON_SIZE * SELECTED_SCALE);
                    dstRect = {iconX - (selectedSize - NODE_ICON_SIZE) / 2, 
                               iconY - (selectedSize - NODE_ICON_SIZE) / 2, 
                               selectedSize, selectedSize};
                } else {
                    dstRect = {iconX, iconY, NODE_ICON_SIZE, NODE_ICON_SIZE};
                }
                display.drawTexture(nodeTexture, nullptr, &dstRect);
                
                // If this node is locked, overlay a lock icon or gray it out
                if (!node.isUnlocked) {
                    // Here we could draw a lock icon or apply a gray/semi-transparent effect
                    // For now, we'll just draw a black semi-transparent rectangle over it
                    SDL_SetRenderDrawBlendMode(display.getRenderer(), SDL_BLENDMODE_BLEND);
                    SDL_SetRenderDrawColor(display.getRenderer(), 0, 0, 0, 128); // Semi-transparent black
                    SDL_RenderFillRect(display.getRenderer(), &dstRect);
                    SDL_SetRenderDrawBlendMode(display.getRenderer(), SDL_BLENDMODE_NONE);
                }
            } else {
                // Fallback: draw a colored circle if texture isn't available
                SDL_SetRenderDrawColor(display.getRenderer(), 255, 255, 255, 255);
                int radius = isSelected ? NODE_ICON_SIZE/2 + 2 : NODE_ICON_SIZE/2;
                // Basic circle drawing algorithm
                for (int w = 0; w < radius * 2; w++) {
                    for (int h = 0; h < radius * 2; h++) {
                        int dx = radius - w;
                        int dy = radius - h;
                        if ((dx*dx + dy*dy) <= (radius * radius)) {
                            SDL_RenderDrawPoint(display.getRenderer(), iconX + w, iconY + h);
                        }
                    }
                }
            }
        }
        
        // Display the name of the selected node at the bottom of the screen
        const auto& selectedNode = continent.nodes[currentNodeIndex_];
        std::string selectedNodeText = selectedNode.name; // Node name is already uppercase
        float nodeTextScale = 0.8f; // Increased size for better readability
        SDL_Point unscaled_node_text_size = textRenderer->getTextDimensions(selectedNodeText);
        float scaled_node_text_width = static_cast<float>(unscaled_node_text_size.x) * nodeTextScale;
        float scaled_node_text_height = static_cast<float>(unscaled_node_text_size.y) * nodeTextScale;

        int nodeTextX = (screenWidth - static_cast<int>(scaled_node_text_width)) / 2;
        int nodeTextY = screenHeight - static_cast<int>(scaled_node_text_height) - 20; // Position at bottom with padding
        
        // Add a background behind the text for better readability
        SDL_SetRenderDrawBlendMode(display.getRenderer(), SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(display.getRenderer(), 0, 0, 0, 180); // Semi-transparent black
        SDL_Rect textBgRect = {
            nodeTextX - 10, 
            nodeTextY - 5, 
            static_cast<int>(scaled_node_text_width) + 20, 
            static_cast<int>(scaled_node_text_height) + 10
        };
        SDL_RenderFillRect(display.getRenderer(), &textBgRect);
        SDL_SetRenderDrawBlendMode(display.getRenderer(), SDL_BLENDMODE_NONE);
        
        textRenderer->drawText(display.getRenderer(), selectedNodeText, nodeTextX, nodeTextY, nodeTextScale);
    }

    void MapSystemState::render_node_detail(PCDisplay& display) {
        if (!game_ptr) return;
        AssetManager* assetManager = game_ptr->getAssetManager(); // Added for boss sprites
        TextRenderer* textRenderer = game_ptr->getTextRenderer();

        if (!assetManager) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MapSystemState::render_node_detail - AssetManager is null!");
            if (textRenderer) { // Check if textRenderer is valid before using
                textRenderer->drawText(display.getRenderer(), "ERROR: ASSET MANAGER NULL", 10, 10, 1.0f);
            }
            return;
        }
        if (!textRenderer) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MapSystemState::render_node_detail - TextRenderer is null!");
            return; 
        }

        if (currentContinentIndex_ < 0 || currentContinentIndex_ >= static_cast<int>(continents_.size()) ||
            currentNodeIndex_ < 0 || currentNodeIndex_ >= static_cast<int>(continents_[currentContinentIndex_].nodes.size())) {
            textRenderer->drawText(display.getRenderer(), "NODE DETAIL: INVALID NODE/CONTINENT", 10, 70, 1.0f); // Changed to uppercase
            return;
        }
        
        const auto& node = continents_[currentContinentIndex_].nodes[currentNodeIndex_];
        
        // Render basic info for now
        int screenWidth = 0, screenHeight = 0;
        display.getWindowSize(screenWidth, screenHeight);

        // Display Node Name
        float nodeNameScale_detail = 1.5f;
        SDL_Point unscaled_node_name_size_detail = textRenderer->getTextDimensions(node.name); // Name is already uppercase
        float scaled_node_name_width_detail = static_cast<float>(unscaled_node_name_size_detail.x) * nodeNameScale_detail;
        // float scaled_node_name_height_detail = static_cast<float>(unscaled_node_name_size_detail.y) * nodeNameScale_detail; // Not needed if Y is fixed
        textRenderer->drawText(display.getRenderer(), node.name, (screenWidth - static_cast<int>(scaled_node_name_width_detail)) / 2, 30, nodeNameScale_detail);

        // Display "NODE DETAIL VIEW (WIP)" or more specific details
        // textRenderer->drawText(display.getRenderer(), "NODE DETAIL VIEW (WIP)", 10, 70, 1.0f); // Changed to uppercase

        // Display Step Count
        std::string stepsText = "STEPS: " + std::to_string(node.totalSteps);
        float stepsTextScale = 1.0f;
        SDL_Point unscaled_steps_text_size = textRenderer->getTextDimensions(stepsText);
        float scaled_steps_text_width = static_cast<float>(unscaled_steps_text_size.x) * stepsTextScale;
        // float scaled_steps_text_height = static_cast<float>(unscaled_steps_text_size.y) * stepsTextScale; // Not needed if Y is fixed
        textRenderer->drawText(display.getRenderer(), stepsText, (screenWidth - static_cast<int>(scaled_steps_text_width)) / 2, screenHeight - 70, stepsTextScale);
          // Load and render the environment background as a preview
        if (!node.adventureBackgroundLayers.empty() && node.adventureBackgroundLayers.size() >= 3) {
            // Use the background layer (layer 2) as a preview
            const auto& bgLayer = node.adventureBackgroundLayers[2]; // Background layer
            
            if (!bgLayer.texturePaths.empty()) {
                std::string bgTextureId = node.id + "_bg_preview";
                SDL_Texture* bgTexture = assetManager->getTexture(bgTextureId);
                
                if (!bgTexture) {
                    // Load the texture if it's not already loaded
                    if (assetManager->loadTexture(bgTextureId, bgLayer.texturePaths[0])) {
                        bgTexture = assetManager->getTexture(bgTextureId);
                    }
                }
                
                if (bgTexture) {
                    // Draw the background in the center area of the screen
                    int previewWidth = screenWidth / 2;
                    int previewHeight = screenHeight / 3;
                    SDL_Rect dstRect = {
                        (screenWidth - previewWidth) / 2,
                        80, // Position below the title
                        previewWidth,
                        previewHeight
                    };
                    display.drawTexture(bgTexture, nullptr, &dstRect);
                    
                    // Draw a frame around the preview
                    SDL_SetRenderDrawColor(display.getRenderer(), 255, 255, 255, 255);
                    SDL_RenderDrawRect(display.getRenderer(), &dstRect);
                }
            }
        }
        
        // Load and render the boss sprite
        std::string bossTextureId = node.id + "_boss";
        SDL_Texture* bossTexture = assetManager->getTexture(bossTextureId);
        
        if (!bossTexture) {
            // Load the texture if it's not already loaded
            if (assetManager->loadTexture(bossTextureId, node.bossSpritePath)) {
                bossTexture = assetManager->getTexture(bossTextureId);
            }
        }
        
        if (bossTexture) {
            // Get the texture dimensions
            int texWidth, texHeight;
            SDL_QueryTexture(bossTexture, nullptr, nullptr, &texWidth, &texHeight);
            
            // Calculate a reasonable size for the boss sprite
            int maxHeight = screenHeight / 4;
            float scale = static_cast<float>(maxHeight) / static_cast<float>(texHeight);
            int scaledWidth = static_cast<int>(texWidth * scale);
            int scaledHeight = maxHeight;
            
            // Position to the right of center
            int bossX = (screenWidth / 2) + 50;
            int bossY = screenHeight / 2 - scaledHeight / 2;
            
            SDL_Rect dstRect = {bossX, bossY, scaledWidth, scaledHeight};
            display.drawTexture(bossTexture, nullptr, &dstRect);
        }
        
        // Display additional information
        int infoY = screenHeight / 2 + 20;
        int infoX = screenWidth / 4;
        int lineSpacing = 30;
        
        // Display steps required
        std::string stepsInfo = "STEPS TO COMPLETE: " + std::to_string(node.totalSteps);
        textRenderer->drawText(display.getRenderer(), stepsInfo, infoX, infoY, 0.8f);
        
        // Display location info
        std::string locationInfo = "LOCATION: " + continents_[currentContinentIndex_].name;
        textRenderer->drawText(display.getRenderer(), locationInfo, infoX, infoY + lineSpacing, 0.8f);
        
        // Display "Confirm to Start" prompt with a more prominent style
        std::string promptText = "CONFIRM TO START ADVENTURE";
        float promptTextScale = 1.0f;
        SDL_Point unscaled_prompt_text_size = textRenderer->getTextDimensions(promptText);
        float scaled_prompt_text_width = static_cast<float>(unscaled_prompt_text_size.x) * promptTextScale;
        float scaled_prompt_text_height = static_cast<float>(unscaled_prompt_text_size.y) * promptTextScale;
        
        int promptX = (screenWidth - static_cast<int>(scaled_prompt_text_width)) / 2;
        int promptY = screenHeight - 60;
        
        // Draw a highlight box around the prompt
        SDL_Rect promptRect = {
            promptX - 20,
            promptY - 10,
            static_cast<int>(scaled_prompt_text_width) + 40,
            static_cast<int>(scaled_prompt_text_height) + 20
        };
        
        // Pulse effect for the prompt (using SDL_GetTicks for animation)
        float pulse = (sinf(static_cast<float>(SDL_GetTicks()) / 500.0f) + 1.0f) / 2.0f; // 0 to 1 pulse
        Uint8 alpha = static_cast<Uint8>(128 + 127 * pulse); // 128-255 range for semi-transparency
        
        SDL_SetRenderDrawBlendMode(display.getRenderer(), SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(display.getRenderer(), 0, 0, 128, alpha); // Semi-transparent blue with pulse
        SDL_RenderFillRect(display.getRenderer(), &promptRect);
        SDL_SetRenderDrawColor(display.getRenderer(), 255, 255, 255, 255); // White border
        SDL_RenderDrawRect(display.getRenderer(), &promptRect);
        SDL_SetRenderDrawBlendMode(display.getRenderer(), SDL_BLENDMODE_NONE);
        
        textRenderer->drawText(display.getRenderer(), promptText, promptX, promptY, promptTextScale);
    }

} // namespace Digivice
