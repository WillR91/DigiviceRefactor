#include "states/MapSystemState.h"
#include "Core/Game.h" 
#include "Core/InputManager.h" 
#include "Core/PlayerData.h"   
#include "Core/AssetManager.h" // Added for loading textures
#include "UI/TextRenderer.h"   // Added for rendering text
#include "platform/pc/pc_display.h" // Corrected path for PCDisplay.h
#include "../../include/states/adventurestate.h" // Ensure AdventureState is included
#include "core/BackgroundVariantManager.h" // Added for variant system
#include <algorithm> // Added for std::min and std::max
#include <iostream> // Added for std::cout debug logging
#include <set> // Added for tracking failed texture attempts
#include <cmath> // Added for sin function

namespace Digivice {
    
    // Helper function to create BackgroundLayerData using the new variant system
    BackgroundLayerData createEnvironmentBackground(const std::string& environmentName, float parallaxX, float parallaxY) {
        BackgroundLayerData layerData;
        layerData.parallaxFactorX = parallaxX;
        layerData.parallaxFactorY = parallaxY;
        
        std::cout << "createEnvironmentBackground: Creating background for '" << environmentName << "'" << std::endl;
        
        // Initialize variants using the BackgroundVariantManager
        BackgroundVariantManager::initializeVariantsForNode(layerData, environmentName);
        
        std::cout << "createEnvironmentBackground: Created background for '" << environmentName 
                  << "' with FG:" << layerData.foregroundPaths.size() 
                  << " MG:" << layerData.middlegroundPaths.size() 
                  << " BG:" << layerData.backgroundPaths.size() << " variants" << std::endl;
        
        // Debug: Print first path from each layer to verify generation
        if (!layerData.foregroundPaths.empty()) {
            std::cout << "createEnvironmentBackground: Sample FG path: " << layerData.foregroundPaths[0] << std::endl;
        }
        if (!layerData.middlegroundPaths.empty()) {
            std::cout << "createEnvironmentBackground: Sample MG path: " << layerData.middlegroundPaths[0] << std::endl;
        }
        if (!layerData.backgroundPaths.empty()) {
            std::cout << "createEnvironmentBackground: Sample BG path: " << layerData.backgroundPaths[0] << std::endl;
        }
        
        return layerData;
    }

    MapSystemState::MapSystemState(Game* game) // Uncommented Game* game parameter
        : GameState(game), // Uncommented game, passing it to GameState constructor
          currentView_(MapView::CONTINENT_SELECTION),
          currentContinentIndex_(0),
          currentNodeIndex_(0),
          isFading_(false),
          fadingOut_(false),
          fadeAlpha_(0.0f),
          fadeDuration_(0.25f), // A quarter of a second for a quick fade
          fadeTimer_(0.0f),
          targetContinentIndex_(-1) { // Initialize to an invalid index
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
        if (isFading_ && currentView_ == MapView::CONTINENT_SELECTION) {
            fadeTimer_ += dt;
            if (fadingOut_) {
                // Fading out (to black)
                fadeAlpha_ = std::min(255.0f, (fadeTimer_ / fadeDuration_) * 255.0f);
                if (fadeTimer_ >= fadeDuration_) {
                    fadeAlpha_ = 255.0f; // Ensure fully opaque
                    currentContinentIndex_ = targetContinentIndex_; // Change continent image now
                    fadingOut_ = false;    // Switch to fading in
                    fadeTimer_ = 0.0f;     // Reset timer for fade-in phase
                    // The new continent's assets will be loaded/rendered in the render_continent_selection call
                }
            } else {
                // Fading in (from black)
                fadeAlpha_ = std::max(0.0f, 255.0f - (fadeTimer_ / fadeDuration_) * 255.0f);
                if (fadeTimer_ >= fadeDuration_) {
                    fadeAlpha_ = 0.0f; // Ensure fully transparent
                    isFading_ = false;   // End the fade sequence
                    fadeTimer_ = 0.0f;
                }
            }
            // While fading in continent selection, we don't want other view-specific updates or input processing for this view.
            return; 
        }

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
        continents_.clear();        // --- Define File Island ---
        ContinentData fileIsland;
        fileIsland.id = "01_file_island"; // Changed to match folder name
        fileIsland.name = "FILE ISLAND"; 
        fileIsland.mapImagePath = "assets/ui/maps/01_file_island_map.png"; // Updated to new path structure
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
        tropicalJungleNode.isUnlocked = true;        tropicalJungleNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("tropicaljungle"));
        fileIsland.nodes.push_back(tropicalJungleNode);
        // --- Define Node: 02_lake ---
        NodeData lakeNode;
        lakeNode.id = "01_fi_node_02_lake"; // Adjusted ID prefix
        lakeNode.name = "LAKE";
        lakeNode.continentId = fileIsland.id;        lakeNode.mapPositionX = 200.0f; 
        lakeNode.mapPositionY = 100.0f; 
        lakeNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        lakeNode.bossSpritePath = "assets/sprites/enemy_digimon/seadramon.png";
        lakeNode.totalSteps = 20; 
        lakeNode.isUnlocked = true;        lakeNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("lake"));
        fileIsland.nodes.push_back(lakeNode);
        // --- Define Node: 03_gear_savannah ---
        NodeData gearSavannahNode;
        gearSavannahNode.id = "01_fi_node_03_gear_savannah"; // Adjusted ID prefix
        gearSavannahNode.name = "GEAR SAVANNAH";
        gearSavannahNode.continentId = fileIsland.id;
        gearSavannahNode.mapPositionX = 300.0f;        gearSavannahNode.mapPositionY = 180.0f; 
        gearSavannahNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        gearSavannahNode.bossSpritePath = "assets/sprites/enemy_digimon/hagurumon.png";
        gearSavannahNode.totalSteps = 20; 
        gearSavannahNode.isUnlocked = true;        gearSavannahNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("gearsavannah"));
        fileIsland.nodes.push_back(gearSavannahNode);
        // --- Define Node: 04_factorial_town ---
        NodeData factorialTownNode;
        factorialTownNode.id = "01_fi_node_04_factorial_town"; // Adjusted ID prefix
        factorialTownNode.name = "FACTORIAL TOWN";
        factorialTownNode.continentId = fileIsland.id;        factorialTownNode.mapPositionX = 350.0f; 
        factorialTownNode.mapPositionY = 120.0f; 
        factorialTownNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        factorialTownNode.bossSpritePath = "assets/sprites/enemy_digimon/mekanorimon.png";
        factorialTownNode.totalSteps = 20; 
        factorialTownNode.isUnlocked = true;        factorialTownNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("factorialtown"));
        fileIsland.nodes.push_back(factorialTownNode);
        // --- Define Node: 05_toy_town ---
        NodeData toyTownNode;
        toyTownNode.id = "01_fi_node_05_toy_town"; // Adjusted ID prefix
        toyTownNode.name = "TOY TOWN";
        toyTownNode.continentId = fileIsland.id;        toyTownNode.mapPositionX = 250.0f; 
        toyTownNode.mapPositionY = 220.0f; 
        toyTownNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        toyTownNode.bossSpritePath = "assets/sprites/enemy_digimon/monzaemon.png";
        toyTownNode.totalSteps = 20;        toyTownNode.isUnlocked = true; 
        toyTownNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("toytown"));
        fileIsland.nodes.push_back(toyTownNode);
        // --- Define Node: 06_infinity_mountain ---
        NodeData infinityMountainNode;
        infinityMountainNode.id = "01_fi_node_06_infinity_mountain"; // Adjusted ID prefix
        infinityMountainNode.name = "INFINITY MOUNTAIN";
        infinityMountainNode.continentId = fileIsland.id;        infinityMountainNode.mapPositionX = 180.0f; 
        infinityMountainNode.mapPositionY = 280.0f; 
        infinityMountainNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        infinityMountainNode.bossSpritePath = "assets/sprites/enemy_digimon/mammothmon.png";
        infinityMountainNode.totalSteps = 20;        infinityMountainNode.isUnlocked = true; 
        infinityMountainNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("infinitymountain"));
        fileIsland.nodes.push_back(infinityMountainNode);
        continents_.push_back(fileIsland);        // --- Define File Island Broken ---
        ContinentData fileIslandBroken;
        fileIslandBroken.id = "02_file_island_broken";
        fileIslandBroken.name = "FILE ISLAND BROKEN";
        fileIslandBroken.mapImagePath = "assets/ui/maps/02_file_island_broken_map.png";

        // Node: 01_freezeland
        NodeData freezelandNode;
        freezelandNode.id = "02_fib_node_01_freezeland"; 
        freezelandNode.name = "FREEZELAND";
        freezelandNode.continentId = fileIslandBroken.id;
        freezelandNode.mapPositionX = 100.0f; 
        freezelandNode.mapPositionY = 100.0f;
        freezelandNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        freezelandNode.bossSpritePath = "assets/sprites/enemy_digimon/frigimon.png";
        freezelandNode.totalSteps = 20;        freezelandNode.isUnlocked = true;
        freezelandNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("freezeland"));
        fileIslandBroken.nodes.push_back(freezelandNode);

        // Node: 02_ancientdinoregion
        NodeData ancientDinoRegionNode;
        ancientDinoRegionNode.id = "02_fib_node_02_ancientdinoregion";
        ancientDinoRegionNode.name = "ANCIENT DINO REGION";
        ancientDinoRegionNode.continentId = fileIslandBroken.id;
        ancientDinoRegionNode.mapPositionX = 200.0f; 
        ancientDinoRegionNode.mapPositionY = 100.0f;
        ancientDinoRegionNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        ancientDinoRegionNode.bossSpritePath = "assets/sprites/enemy_digimon/tyranomon.png";
        ancientDinoRegionNode.totalSteps = 20;        ancientDinoRegionNode.isUnlocked = true;
        ancientDinoRegionNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("ancientdinoregion"));
        fileIslandBroken.nodes.push_back(ancientDinoRegionNode);
        
        // Node: 03_overdellcemetary
        NodeData overdellCemetaryNode;
        overdellCemetaryNode.id = "02_fib_node_03_overdellcemetary";
        overdellCemetaryNode.name = "OVERDELL CEMETARY";
        overdellCemetaryNode.continentId = fileIslandBroken.id;
        overdellCemetaryNode.mapPositionX = 300.0f;
        overdellCemetaryNode.mapPositionY = 100.0f;
        overdellCemetaryNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        overdellCemetaryNode.bossSpritePath = "assets/sprites/enemy_digimon/phantomon.png";
        overdellCemetaryNode.totalSteps = 20;        overdellCemetaryNode.isUnlocked = true;
        overdellCemetaryNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("overdellcemetary"));
        fileIslandBroken.nodes.push_back(overdellCemetaryNode);

        // Node: 04_townofbeginnings
        NodeData townOfBeginningsNode;
        townOfBeginningsNode.id = "02_fib_node_04_townofbeginnings";
        townOfBeginningsNode.name = "TOWN OF BEGINNINGS";
        townOfBeginningsNode.continentId = fileIslandBroken.id;
        townOfBeginningsNode.mapPositionX = 100.0f;
        townOfBeginningsNode.mapPositionY = 200.0f;
        townOfBeginningsNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        townOfBeginningsNode.bossSpritePath = "assets/sprites/enemy_digimon/elecmon.png";
        townOfBeginningsNode.totalSteps = 20;        townOfBeginningsNode.isUnlocked = true;
        townOfBeginningsNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("townofbeginnings"));
        fileIslandBroken.nodes.push_back(townOfBeginningsNode);
        
        // Node: 05_infinitymountain2
        NodeData infinityMountain2Node;
        infinityMountain2Node.id = "02_fib_node_05_infinitymountain2";
        infinityMountain2Node.name = "INFINITY MOUNTAIN 2";
        infinityMountain2Node.continentId = fileIslandBroken.id;
        infinityMountain2Node.mapPositionX = 200.0f;
        infinityMountain2Node.mapPositionY = 200.0f;
        infinityMountain2Node.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        infinityMountain2Node.bossSpritePath = "assets/sprites/enemy_digimon/unimon.png";
        infinityMountain2Node.totalSteps = 20;        infinityMountain2Node.isUnlocked = true;
        infinityMountain2Node.adventureBackgroundLayers.push_back(createEnvironmentBackground("infinitymountain2"));
        fileIslandBroken.nodes.push_back(infinityMountain2Node);

        // Node: 06_opensea
        NodeData openSeaNode;
        openSeaNode.id = "02_fib_node_06_opensea";
        openSeaNode.name = "OPEN SEA";
        openSeaNode.continentId = fileIslandBroken.id;
        openSeaNode.mapPositionX = 300.0f;
        openSeaNode.mapPositionY = 200.0f;
        openSeaNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        openSeaNode.bossSpritePath = "assets/sprites/enemy_digimon/whamon.png";
        openSeaNode.totalSteps = 20;        openSeaNode.isUnlocked = true;
        openSeaNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("opensea"));
        fileIslandBroken.nodes.push_back(openSeaNode);

        continents_.push_back(fileIslandBroken);        // --- Define Server Continent ---
        ContinentData serverContinent;
        serverContinent.id = "03_server_continent";
        serverContinent.name = "SERVER CONTINENT";
        serverContinent.mapImagePath = "assets/ui/maps/03_server_continent_map.png";
        
        NodeData koromonsVillageNode;
        koromonsVillageNode.id = "03_sc_node_01_koromonsvillage";
        koromonsVillageNode.name = "KOROMONS VILLAGE";
        koromonsVillageNode.continentId = serverContinent.id;
        koromonsVillageNode.mapPositionX = 100.0f; 
        koromonsVillageNode.mapPositionY = 100.0f;
        koromonsVillageNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        koromonsVillageNode.bossSpritePath = "assets/sprites/enemy_digimon/koromon.png";
        koromonsVillageNode.totalSteps = 20;        koromonsVillageNode.isUnlocked = true;
        koromonsVillageNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("koromonsvillage"));
        serverContinent.nodes.push_back(koromonsVillageNode);

        NodeData scColosseumNode; 
        scColosseumNode.id = "03_sc_node_02_colosseum";
        scColosseumNode.name = "COLOSSEUM";
        scColosseumNode.continentId = serverContinent.id;
        scColosseumNode.mapPositionX = 150.0f; 
        scColosseumNode.mapPositionY = 100.0f;
        scColosseumNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        scColosseumNode.bossSpritePath = "assets/sprites/enemy_digimon/leomon.png";
        scColosseumNode.totalSteps = 20;        scColosseumNode.isUnlocked = true;
        scColosseumNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("colosseum"));
        serverContinent.nodes.push_back(scColosseumNode);

        NodeData desertNode;
        desertNode.id = "03_sc_node_03_desert";
        desertNode.name = "DESERT";
        desertNode.continentId = serverContinent.id;
        desertNode.mapPositionX = 200.0f; 
        desertNode.mapPositionY = 100.0f;
        desertNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        desertNode.bossSpritePath = "assets/sprites/enemy_digimon/monochromon.png";
        desertNode.totalSteps = 20;        desertNode.isUnlocked = true;
        desertNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("desert"));
        serverContinent.nodes.push_back(desertNode);
        
        NodeData piccolomonsForestNode;
        piccolomonsForestNode.id = "03_sc_node_04_piccolomonsforest";
        piccolomonsForestNode.name = "PICCOLOMONS FOREST";
        piccolomonsForestNode.continentId = serverContinent.id;
        piccolomonsForestNode.mapPositionX = 250.0f; 
        piccolomonsForestNode.mapPositionY = 100.0f;
        piccolomonsForestNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        piccolomonsForestNode.bossSpritePath = "assets/sprites/enemy_digimon/cherrymon.png";
        piccolomonsForestNode.totalSteps = 20;        piccolomonsForestNode.isUnlocked = true;
        piccolomonsForestNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("piccolomonsforest"));
        serverContinent.nodes.push_back(piccolomonsForestNode);

        NodeData reversePyramidNode;
        reversePyramidNode.id = "03_sc_node_05_reversepyramid";
        reversePyramidNode.name = "REVERSE PYRAMID";
        reversePyramidNode.continentId = serverContinent.id;
        reversePyramidNode.mapPositionX = 300.0f; 
        reversePyramidNode.mapPositionY = 100.0f;
        reversePyramidNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        reversePyramidNode.bossSpritePath = "assets/sprites/enemy_digimon/sandyanmamon.png";
        reversePyramidNode.totalSteps = 20;        reversePyramidNode.isUnlocked = true;
        reversePyramidNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("reversepyramid"));
        serverContinent.nodes.push_back(reversePyramidNode);

        NodeData amusementParkNode;
        amusementParkNode.id = "03_sc_node_06_amusementpark";
        amusementParkNode.name = "AMUSEMENT PARK";
        amusementParkNode.continentId = serverContinent.id;
        amusementParkNode.mapPositionX = 100.0f; 
        amusementParkNode.mapPositionY = 200.0f;
        amusementParkNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        amusementParkNode.bossSpritePath = "assets/sprites/enemy_digimon/etemon.png";
        amusementParkNode.totalSteps = 20;        amusementParkNode.isUnlocked = true;
        amusementParkNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("amusementpark"));
        serverContinent.nodes.push_back(amusementParkNode);
        
        NodeData restaurantNode;
        restaurantNode.id = "03_sc_node_07_restaurant";
        restaurantNode.name = "RESTAURANT";
        restaurantNode.continentId = serverContinent.id;
        restaurantNode.mapPositionX = 150.0f; 
        restaurantNode.mapPositionY = 200.0f;
        restaurantNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        restaurantNode.bossSpritePath = "assets/sprites/enemy_digimon/digitamamon.png";
        restaurantNode.totalSteps = 20;        restaurantNode.isUnlocked = true;
        restaurantNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("restaurant"));
        serverContinent.nodes.push_back(restaurantNode);

        NodeData differentSpaceNode;
        differentSpaceNode.id = "03_sc_node_08_differentspace";
        differentSpaceNode.name = "DIFFERENT SPACE";
        differentSpaceNode.continentId = serverContinent.id;
        differentSpaceNode.mapPositionX = 200.0f; 
        differentSpaceNode.mapPositionY = 200.0f;
        differentSpaceNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        differentSpaceNode.bossSpritePath = "assets/sprites/enemy_digimon/vademon.png";
        differentSpaceNode.totalSteps = 20;        differentSpaceNode.isUnlocked = true;
        differentSpaceNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("differentspace"));
        serverContinent.nodes.push_back(differentSpaceNode);
        
        NodeData gekomonsCastleNode;
        gekomonsCastleNode.id = "03_sc_node_09_gekomonscastle";
        gekomonsCastleNode.name = "GEKOMONS CASTLE";
        gekomonsCastleNode.continentId = serverContinent.id;
        gekomonsCastleNode.mapPositionX = 250.0f; 
        gekomonsCastleNode.mapPositionY = 200.0f;
        gekomonsCastleNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        gekomonsCastleNode.bossSpritePath = "assets/sprites/enemy_digimon/gekomon.png";
        gekomonsCastleNode.totalSteps = 20;        gekomonsCastleNode.isUnlocked = true;
        gekomonsCastleNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("gekomonscastle"));
        serverContinent.nodes.push_back(gekomonsCastleNode);

        NodeData myotismonsCastleNode;
        myotismonsCastleNode.id = "03_sc_node_10_myotismonscastle";
        myotismonsCastleNode.name = "MYOTISMONS CASTLE";
        myotismonsCastleNode.continentId = serverContinent.id;
        myotismonsCastleNode.mapPositionX = 300.0f; 
        myotismonsCastleNode.mapPositionY = 200.0f;
        myotismonsCastleNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        myotismonsCastleNode.bossSpritePath = "assets/sprites/enemy_digimon/myotismon.png";
        myotismonsCastleNode.totalSteps = 20;        myotismonsCastleNode.isUnlocked = true;
        myotismonsCastleNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("myotismonscastle"));
        serverContinent.nodes.push_back(myotismonsCastleNode);

        continents_.push_back(serverContinent);

        // --- Define Tokyo ---
        ContinentData tokyo;
        tokyo.id = "04_tokyo";
        tokyo.name = "TOKYO";
        tokyo.mapImagePath = "assets/ui/maps/04_tokyo_map.png";

        NodeData hikarigoakaNode;
        hikarigoakaNode.id = "04_tk_node_01_hikarigoaka";
        hikarigoakaNode.name = "HIKARIGOAKA";
        hikarigoakaNode.continentId = tokyo.id;
        hikarigoakaNode.mapPositionX = 100.0f; 
        hikarigoakaNode.mapPositionY = 100.0f;
        hikarigoakaNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        hikarigoakaNode.bossSpritePath = "assets/sprites/enemy_digimon/raremon.png";
        hikarigoakaNode.totalSteps = 20;        hikarigoakaNode.isUnlocked = true;
        hikarigoakaNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("hikarigoaka"));
        tokyo.nodes.push_back(hikarigoakaNode);
        
        NodeData harumiNode;
        harumiNode.id = "04_tk_node_02_harumi";
        harumiNode.name = "HARUMI";
        harumiNode.continentId = tokyo.id;
        harumiNode.mapPositionX = 200.0f; 
        harumiNode.mapPositionY = 100.0f;
        harumiNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        harumiNode.bossSpritePath = "assets/sprites/enemy_digimon/gesomon.png";
        harumiNode.totalSteps = 20;        harumiNode.isUnlocked = true;
        harumiNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("harumi"));
        tokyo.nodes.push_back(harumiNode);

        NodeData pierNode;
        pierNode.id = "04_tk_node_03_pier";
        pierNode.name = "PIER";
        pierNode.continentId = tokyo.id;
        pierNode.mapPositionX = 300.0f; 
        pierNode.mapPositionY = 100.0f;
        pierNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        pierNode.bossSpritePath = "assets/sprites/enemy_digimon/shellmon.png";
        pierNode.totalSteps = 20;        pierNode.isUnlocked = true;
        pierNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("pier"));
        tokyo.nodes.push_back(pierNode);

        NodeData towerNode;
        towerNode.id = "04_tk_node_04_tower";
        towerNode.name = "TOWER";
        towerNode.continentId = tokyo.id;
        towerNode.mapPositionX = 100.0f; 
        towerNode.mapPositionY = 200.0f;
        towerNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        towerNode.bossSpritePath = "assets/sprites/enemy_digimon/machinedramon.png";
        towerNode.totalSteps = 20;        towerNode.isUnlocked = true;
        towerNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("tower"));
        tokyo.nodes.push_back(towerNode);
        
        NodeData shibuyaNode;
        shibuyaNode.id = "04_tk_node_05_shibuya";
        shibuyaNode.name = "SHIBUYA";
        shibuyaNode.continentId = tokyo.id;
        shibuyaNode.mapPositionX = 200.0f; 
        shibuyaNode.mapPositionY = 200.0f;
        shibuyaNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        shibuyaNode.bossSpritePath = "assets/sprites/enemy_digimon/ogremon.png";
        shibuyaNode.totalSteps = 20;        shibuyaNode.isUnlocked = true;
        shibuyaNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("shibuya"));
        tokyo.nodes.push_back(shibuyaNode);

        NodeData exhibitionCentreNode;
        exhibitionCentreNode.id = "04_tk_node_06_exhibitioncentre";
        exhibitionCentreNode.name = "EXHIBITION CENTRE";
        exhibitionCentreNode.continentId = tokyo.id;
        exhibitionCentreNode.mapPositionX = 300.0f; 
        exhibitionCentreNode.mapPositionY = 200.0f;
        exhibitionCentreNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        exhibitionCentreNode.bossSpritePath = "assets/sprites/enemy_digimon/andromon.png";
        exhibitionCentreNode.totalSteps = 20;
        exhibitionCentreNode.isUnlocked = true;        exhibitionCentreNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("exhibitioncentre"));
        tokyo.nodes.push_back(exhibitionCentreNode);

        continents_.push_back(tokyo);

        // --- Define Spiral Mountain ---
        ContinentData spiralMountain;
        spiralMountain.id = "05_spiral_mountain";
        spiralMountain.name = "SPIRAL MOUNTAIN";
        spiralMountain.mapImagePath = "assets/ui/maps/05_spiral_mountain_map.png";

        NodeData smJungleNode; 
        smJungleNode.id = "05_sm_node_01_jungle";
        smJungleNode.name = "JUNGLE";
        smJungleNode.continentId = spiralMountain.id;
        smJungleNode.mapPositionX = 100.0f; 
        smJungleNode.mapPositionY = 100.0f;
        smJungleNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        smJungleNode.bossSpritePath = "assets/sprites/enemy_digimon/kiwimon.png";
        smJungleNode.totalSteps = 20;
        smJungleNode.isUnlocked = true;        smJungleNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("jungle"));
        spiralMountain.nodes.push_back(smJungleNode);
        
        NodeData smColosseum2Node;
        smColosseum2Node.id = "05_sm_node_02_colosseum2";
        smColosseum2Node.name = "COLOSSEUM 2";
        smColosseum2Node.continentId = spiralMountain.id;
        smColosseum2Node.mapPositionX = 200.0f; 
        smColosseum2Node.mapPositionY = 100.0f;
        smColosseum2Node.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        smColosseum2Node.bossSpritePath = "assets/sprites/enemy_digimon/colosseumgreymon.png";
        smColosseum2Node.totalSteps = 20;
        smColosseum2Node.isUnlocked = true;        smColosseum2Node.adventureBackgroundLayers.push_back(createEnvironmentBackground("colosseum2"));
        spiralMountain.nodes.push_back(smColosseum2Node);

        NodeData digitalSeaNode;
        digitalSeaNode.id = "05_sm_node_03_digitalsea";
        digitalSeaNode.name = "DIGITAL SEA";
        digitalSeaNode.continentId = spiralMountain.id;
        digitalSeaNode.mapPositionX = 300.0f; 
        digitalSeaNode.mapPositionY = 100.0f;
        digitalSeaNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        digitalSeaNode.bossSpritePath = "assets/sprites/enemy_digimon/megaseadramon.png";
        digitalSeaNode.totalSteps = 20;
        digitalSeaNode.isUnlocked = true;        digitalSeaNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("digitalsea"));
        spiralMountain.nodes.push_back(digitalSeaNode);

        NodeData digitalForestNode;
        digitalForestNode.id = "05_sm_node_04_digitalforest";
        digitalForestNode.name = "DIGITAL FOREST";
        digitalForestNode.continentId = spiralMountain.id;
        digitalForestNode.mapPositionX = 100.0f; 
        digitalForestNode.mapPositionY = 200.0f;
        digitalForestNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        digitalForestNode.bossSpritePath = "assets/sprites/enemy_digimon/floramon.png";
        digitalForestNode.totalSteps = 20;
        digitalForestNode.isUnlocked = true;        digitalForestNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("digitalforest"));
        spiralMountain.nodes.push_back(digitalForestNode);
        
        NodeData digitalCityNode;
        digitalCityNode.id = "05_sm_node_05_digitalcity";
        digitalCityNode.name = "DIGITAL CITY";
        digitalCityNode.continentId = spiralMountain.id;
        digitalCityNode.mapPositionX = 200.0f; 
        digitalCityNode.mapPositionY = 200.0f;
        digitalCityNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        digitalCityNode.bossSpritePath = "assets/sprites/enemy_digimon/guardromon.png";
        digitalCityNode.totalSteps = 20;
        digitalCityNode.isUnlocked = true;        digitalCityNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("digitalcity"));
        spiralMountain.nodes.push_back(digitalCityNode);

        NodeData wastelandNode;
        wastelandNode.id = "05_sm_node_06_wasteland";
        wastelandNode.name = "WASTELAND";
        wastelandNode.continentId = spiralMountain.id;
        wastelandNode.mapPositionX = 300.0f; 
        wastelandNode.mapPositionY = 200.0f;
        wastelandNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        wastelandNode.bossSpritePath = "assets/sprites/enemy_digimon/bakemon.png";
        wastelandNode.totalSteps = 20;
        wastelandNode.isUnlocked = true;        wastelandNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("wasteland"));
        spiralMountain.nodes.push_back(wastelandNode);
        
        continents_.push_back(spiralMountain);

        // --- Define Subspace ---
        ContinentData subspace;
        subspace.id = "06_subspace";
        subspace.name = "SUBSPACE";
        subspace.mapImagePath = "assets/ui/maps/06_subspace_map.png";

        NodeData subspaceNode;
        subspaceNode.id = "06_ss_node_01_subspace";
        subspaceNode.name = "SUBSPACE";
        subspaceNode.continentId = subspace.id;
        subspaceNode.mapPositionX = 150.0f; 
        subspaceNode.mapPositionY = 150.0f; 
        subspaceNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        subspaceNode.bossSpritePath = "assets/sprites/enemy_digimon/piedmon.png";
        subspaceNode.totalSteps = 20;
        subspaceNode.isUnlocked = true;        subspaceNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("subspace"));
        subspace.nodes.push_back(subspaceNode);
        continents_.push_back(subspace);        // --- Define Network ---
        ContinentData network;
        network.id = "07_network";
        network.name = "NETWORK";
        network.mapImagePath = "assets/ui/maps/07_network_map.png";

        NodeData networkNode;
        networkNode.id = "07_nw_node_01_network";
        networkNode.name = "NETWORK";
        networkNode.continentId = network.id;
        networkNode.mapPositionX = 150.0f; 
        networkNode.mapPositionY = 150.0f; 
        networkNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        networkNode.bossSpritePath = "assets/sprites/enemy_digimon/keramon.png";
        networkNode.totalSteps = 20;
        networkNode.isUnlocked = true;        networkNode.adventureBackgroundLayers.push_back(createEnvironmentBackground("network"));
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
        if (continents_.empty() || (isFading_ && currentView_ == MapView::CONTINENT_SELECTION)) return; // Prevent input during fade for this view

        if (inputManager.isActionJustPressed(GameAction::NAV_UP)) {
            int newTargetIndex = (currentContinentIndex_ == 0) ? static_cast<int>(continents_.size()) - 1 : currentContinentIndex_ - 1;
            if (newTargetIndex != currentContinentIndex_) {
                targetContinentIndex_ = newTargetIndex;
                isFading_ = true;
                fadingOut_ = true;     // Start by fading out
                fadeTimer_ = 0.0f;
                // fadeAlpha_ is already 0 or will be set by the fade-out logic, starting from current screen then to black
                SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Map: Continent NAV_UP. Start fade to Index: %d (%s)", targetContinentIndex_, continents_[targetContinentIndex_].name.c_str());
            }
        } else if (inputManager.isActionJustPressed(GameAction::NAV_DOWN)) {
            int newTargetIndex = (currentContinentIndex_ + 1) % static_cast<int>(continents_.size());
            if (newTargetIndex != currentContinentIndex_) {
                targetContinentIndex_ = newTargetIndex;
                isFading_ = true;
                fadingOut_ = true;     // Start by fading out
                fadeTimer_ = 0.0f;
                SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Map: Continent NAV_DOWN. Start fade to Index: %d (%s)", targetContinentIndex_, continents_[targetContinentIndex_].name.c_str());
            }
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

        const auto& continent = continents_[currentContinentIndex_];        // Create a unique ID for the texture, e.g., by appending "_map" to the continent ID.
        std::string continentMapTextureId = continent.id + "_map_texture"; 

        // Use requestTexture for lazy loading with automatic fallback
        SDL_Texture* continentMapTexture = assetManager->requestTexture(continentMapTextureId, continent.mapImagePath);
        if (!continentMapTexture) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load texture: %s for ID: %s", continent.mapImagePath.c_str(), continentMapTextureId.c_str());
        }
        
        int screenWidth = 0, screenHeight = 0;
        display.getWindowSize(screenWidth, screenHeight);

        if (continentMapTexture) { 
            SDL_Rect dstRect = {0, 0, screenWidth, screenHeight}; // Define destination rectangle to fill the screen
            // Corrected: drawTexture arguments. PCDisplay::drawTexture takes (texture, srcRect, dstRect, flip)
            // Assuming we want to draw the whole texture, srcRect can be nullptr.
            display.drawTexture(continentMapTexture, nullptr, &dstRect); 
        } else {            if (textRenderer) {
                // Corrected: renderText to drawText
                std::string errMsg = "ERROR: MAP MISSING FOR " + continent.name; // Changed to uppercase
                // Corrected: getTextWidth to getTextDimensions().x
                SDL_Point errTextSize = textRenderer->getTextDimensions(errMsg); // getTextDimensions returns SDL_Point {w, h}
                // Account for global text scale in positioning
                float globalTextScale = textRenderer->getGlobalTextScale();
                float finalScale = 1.0f * globalTextScale;
                int scaledTextW = static_cast<int>(static_cast<float>(errTextSize.x) * finalScale);
                int textX = (screenWidth / 2) - (scaledTextW / 2);
                textRenderer->drawText(display.getRenderer(), errMsg, textX, screenHeight / 2, 1.0f);
            } else {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MapSystemState::render_continent_selection - TextRenderer is null, cannot display 'ERROR: MAP MISSING'.");
            }
        }        if (textRenderer) {
            float scale = 0.65f; // Define the scale factor
            SDL_Point unscaled_text_size = textRenderer->getTextDimensions(continent.name);
            // Account for both local scale and global text scale for proper centering
            float globalTextScale = textRenderer->getGlobalTextScale();
            float finalScale = scale * globalTextScale;
            float scaled_text_width = static_cast<float>(unscaled_text_size.x) * finalScale;
            float scaled_text_height = static_cast<float>(unscaled_text_size.y) * finalScale;

            int textX = (screenWidth - static_cast<int>(scaled_text_width)) / 2;
            int textY = static_cast<int>(static_cast<float>(screenHeight) * 0.75f - scaled_text_height / 2.0f);
            textRenderer->drawText(display.getRenderer(), continent.name, textX, textY, scale);
        }

        // Render fade overlay if fading in the continent selection view
        if (isFading_ && currentView_ == MapView::CONTINENT_SELECTION) {
            SDL_SetRenderDrawBlendMode(display.getRenderer(), SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(display.getRenderer(), 0, 0, 0, static_cast<Uint8>(fadeAlpha_));
            SDL_Rect fadeRect = {0, 0, screenWidth, screenHeight};
            SDL_RenderFillRect(display.getRenderer(), &fadeRect);
            SDL_SetRenderDrawBlendMode(display.getRenderer(), SDL_BLENDMODE_NONE); // Reset blend mode to opaque for other rendering
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
        SDL_Texture* continentMapTexture = assetManager->requestTexture(continentMapTextureId, continent.mapImagePath);

        int screenWidth = 0, screenHeight = 0;
        display.getWindowSize(screenWidth, screenHeight);

        if (continentMapTexture) {
            SDL_Rect dstRect = {0, 0, screenWidth, screenHeight};
            display.drawTexture(continentMapTexture, nullptr, &dstRect);        } else {
            // Fix text centering for error message
            std::string errMsg = "ERROR: CONTINENT MAP MISSING";
            SDL_Point errDimensions = textRenderer->getTextDimensions(errMsg);
            float globalTextScale = textRenderer->getGlobalTextScale();
            float finalScale = 1.0f * globalTextScale;
            int scaledW = static_cast<int>(static_cast<float>(errDimensions.x) * finalScale);
            int errX = (screenWidth / 2) - (scaledW / 2);
            textRenderer->drawText(display.getRenderer(), errMsg, errX, screenHeight / 2, 1.0f);
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
            int iconY = static_cast<int>(node.mapPositionY) - (NODE_ICON_SIZE / 2);            // Load the node sprite with lazy loading and fallback handling
            std::string nodeTextureId = node.id + "_icon";
            SDL_Texture* nodeTexture = assetManager->requestTexture(nodeTextureId, node.unlockedSpritePath);
            
            // If the specific node texture failed, try fallback
            if (!nodeTexture) {
                nodeTexture = assetManager->requestTexture("generic_node_icon");
                if (!nodeTexture) {
                    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "MapSystemState: Failed to load node texture for '%s' and fallback", nodeTextureId.c_str());
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
        // Account for both local scale and global text scale for proper centering
        float globalTextScale = textRenderer->getGlobalTextScale();
        float finalScale = nodeTextScale * globalTextScale;
        float scaled_node_text_width = static_cast<float>(unscaled_node_text_size.x) * finalScale;
        float scaled_node_text_height = static_cast<float>(unscaled_node_text_size.y) * finalScale;

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
    }    void MapSystemState::render_node_detail(PCDisplay& display) {
        if (!game_ptr) return;
        AssetManager* assetManager = game_ptr->getAssetManager();
        TextRenderer* textRenderer = game_ptr->getTextRenderer();

        if (!assetManager) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MapSystemState::render_node_detail - AssetManager is null!");
            if (textRenderer) {
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
            textRenderer->drawText(display.getRenderer(), "NODE DETAIL: INVALID NODE/CONTINENT", 10, 70, 1.0f);
            return;
        }
        
        const auto& node = continents_[currentContinentIndex_].nodes[currentNodeIndex_];
        
        int screenWidth = 0, screenHeight = 0;
        display.getWindowSize(screenWidth, screenHeight);

        // Render fullscreen environment background layers behind UI
        if (!node.adventureBackgroundLayers.empty()) {
            const auto& layerData = node.adventureBackgroundLayers[0]; // Use the first (and usually only) layer
            
            // Define fullscreen destination rect
            SDL_Rect fullscreenRect = { 0, 0, screenWidth, screenHeight };
              // Render Background Layer (BG) - furthest back
            if (!layerData.backgroundPaths.empty()) {
                std::string bgPath = BackgroundVariantManager::getSelectedPath(
                    layerData.backgroundPaths, layerData.selectedBackgroundVariant);
                std::string bgTextureId = node.id + "_nodedetail_bg";
                
                SDL_Texture* bgTexture = assetManager->requestTexture(bgTextureId, bgPath);
                if (bgTexture) {
                    display.drawTexture(bgTexture, nullptr, &fullscreenRect);
                }
            }
              // Render Middleground Layer (MG) - middle depth
            if (!layerData.middlegroundPaths.empty()) {
                std::string mgPath = BackgroundVariantManager::getSelectedPath(
                    layerData.middlegroundPaths, layerData.selectedMiddlegroundVariant);
                std::string mgTextureId = node.id + "_nodedetail_mg";
                
                SDL_Texture* mgTexture = assetManager->requestTexture(mgTextureId, mgPath);
                if (mgTexture) {
                    display.drawTexture(mgTexture, nullptr, &fullscreenRect);
                }
            }
              // Render Foreground Layer (FG) - closest to camera
            if (!layerData.foregroundPaths.empty()) {
                std::string fgPath = BackgroundVariantManager::getSelectedPath(
                    layerData.foregroundPaths, layerData.selectedForegroundVariant);
                std::string fgTextureId = node.id + "_nodedetail_fg";
                
                SDL_Texture* fgTexture = assetManager->requestTexture(fgTextureId, fgPath);
                if (fgTexture) {
                    display.drawTexture(fgTexture, nullptr, &fullscreenRect);
                }
            }
        }
        
        // Render semi-transparent UI overlay panel
        SDL_SetRenderDrawBlendMode(display.getRenderer(), SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(display.getRenderer(), 0, 20, 60, 180); // Semi-transparent dark blue
        SDL_RenderFillRect(display.getRenderer(), nullptr); // Fill entire screen
        SDL_SetRenderDrawBlendMode(display.getRenderer(), SDL_BLENDMODE_NONE);        // Display Node Name on top of overlay
        float nodeNameScale_detail = 1.5f;
        SDL_Point unscaled_node_name_size_detail = textRenderer->getTextDimensions(node.name);
        // Account for both local scale and global text scale for proper centering
        float globalTextScale = textRenderer->getGlobalTextScale();
        float finalNodeNameScale = nodeNameScale_detail * globalTextScale;
        float scaled_node_name_width_detail = static_cast<float>(unscaled_node_name_size_detail.x) * finalNodeNameScale;
        int nodeNameX = (screenWidth - static_cast<int>(scaled_node_name_width_detail)) / 2;
        textRenderer->drawText(display.getRenderer(), node.name, nodeNameX, 30, nodeNameScale_detail);

        // Display Step Count
        std::string stepsText = "STEPS: " + std::to_string(node.totalSteps);
        float stepsTextScale = 1.0f;
        SDL_Point unscaled_steps_text_size = textRenderer->getTextDimensions(stepsText);
        float finalStepsScale = stepsTextScale * globalTextScale;
        float scaled_steps_text_width = static_cast<float>(unscaled_steps_text_size.x) * finalStepsScale;
        int stepsX = (screenWidth - static_cast<int>(scaled_steps_text_width)) / 2;
        textRenderer->drawText(display.getRenderer(), stepsText, stepsX, screenHeight - 70, stepsTextScale);// Load and render the boss sprite using lazy loading
        std::string bossTextureId = node.id + "_boss";
        SDL_Texture* bossTexture = assetManager->requestTexture(bossTextureId, node.bossSpritePath);
        
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
        float promptTextScale = 1.0f;        SDL_Point unscaled_prompt_text_size = textRenderer->getTextDimensions(promptText);
        // Account for both local scale and global text scale for proper centering (reuse existing globalTextScale)
        float finalPromptScale = promptTextScale * globalTextScale;
        float scaled_prompt_text_width = static_cast<float>(unscaled_prompt_text_size.x) * finalPromptScale;
        float scaled_prompt_text_height = static_cast<float>(unscaled_prompt_text_size.y) * finalPromptScale;
        
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
