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
        fileIsland.id = "file_island";
        fileIsland.name = "FILE ISLAND"; // Changed to uppercase
        fileIsland.mapImagePath = "assets/backgrounds/environments/file_island/file_island_map.png";        // --- Define Node: 01_tropical_jungle ---
        NodeData tropicalJungleNode;
        tropicalJungleNode.id = "file_island_node_01";
        tropicalJungleNode.name = "TROPICAL JUNGLE"; // Changed to uppercase
        tropicalJungleNode.continentId = fileIsland.id;
        tropicalJungleNode.mapPositionX = 120.0f; // Positioned on left side of map
        tropicalJungleNode.mapPositionY = 150.0f; // Positioned at center height
        tropicalJungleNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png"; // Placeholder
        tropicalJungleNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_tj_idle.png"; // Placeholder
        tropicalJungleNode.totalSteps = 400;    // Placeholder
        tropicalJungleNode.isUnlocked = true;// Unlocked for prototype

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
        );        // --- Define Node: 02_lake ---
        NodeData lakeNode;
        lakeNode.id = "file_island_node_02";
        lakeNode.name = "LAKE";
        lakeNode.continentId = fileIsland.id;
        lakeNode.mapPositionX = 200.0f; // Positioned to the right of Tropical Jungle
        lakeNode.mapPositionY = 100.0f; // Positioned above the center line
        lakeNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        lakeNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png"; // Generic placeholder
        lakeNode.totalSteps = 300; // Placeholder steps
        lakeNode.isUnlocked = true; // Set to unlocked        // Layer 0: Foreground (only one variant)
        lakeNode.adventureBackgroundLayers.push_back(BackgroundLayerData(
            {"assets/backgrounds/environments/file_island/02_lake/layer_0.png"}, 
            0.5f, 0.0f
        ));
        
        // Layer 1: Midground (two variants)
        lakeNode.adventureBackgroundLayers.push_back(BackgroundLayerData(
            {
                "assets/backgrounds/environments/file_island/02_lake/layer_1_0.png",
                "assets/backgrounds/environments/file_island/02_lake/layer_1_1.png"
            }, 
            0.25f, 0.0f
        ));
        
        // Layer 2: Background (two variants)
        lakeNode.adventureBackgroundLayers.push_back(BackgroundLayerData(
            {
                "assets/backgrounds/environments/file_island/02_lake/layer_2_0.png",
                "assets/backgrounds/environments/file_island/02_lake/layer_2_1.png"
            }, 
            0.1f, 0.0f
        ));// --- Define Node: 03_gear_savannah ---
        NodeData gearSavannahNode;
        gearSavannahNode.id = "file_island_node_03";
        gearSavannahNode.name = "GEAR SAVANNAH";
        gearSavannahNode.continentId = fileIsland.id;
        gearSavannahNode.mapPositionX = 300.0f; // Positioned to the right of Lake
        gearSavannahNode.mapPositionY = 180.0f; // Positioned below center line
        gearSavannahNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        gearSavannahNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png"; // Generic placeholder
        gearSavannahNode.totalSteps = 350; // Placeholder steps
        gearSavannahNode.isUnlocked = true; // Set to unlocked
        gearSavannahNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/file_island/03_gear_savannah/layer_0.png"}, 0.5f, 0.0f));
        gearSavannahNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/file_island/03_gear_savannah/layer_1.png"}, 0.25f, 0.0f));
        gearSavannahNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/file_island/03_gear_savannah/layer_2.png"}, 0.1f, 0.0f));        // --- Define Node: 04_factorial_town ---
        NodeData factorialTownNode;
        factorialTownNode.id = "file_island_node_04";
        factorialTownNode.name = "FACTORIAL TOWN";
        factorialTownNode.continentId = fileIsland.id;
        factorialTownNode.mapPositionX = 350.0f; // Positioned to the right of Gear Savannah
        factorialTownNode.mapPositionY = 120.0f; // Positioned near top center
        factorialTownNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        factorialTownNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png"; // Generic placeholder
        factorialTownNode.totalSteps = 450; // Placeholder steps
        factorialTownNode.isUnlocked = true; // Set to unlocked
        factorialTownNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/file_island/04_factorial_town/layer_0.png"}, 0.5f, 0.0f));
        factorialTownNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/file_island/04_factorial_town/layer_1.png"}, 0.25f, 0.0f));
        factorialTownNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/file_island/04_factorial_town/layer_2.png"}, 0.1f, 0.0f));        // --- Define Node: 05_toy_town ---
        NodeData toyTownNode;
        toyTownNode.id = "file_island_node_05";
        toyTownNode.name = "TOY TOWN";
        toyTownNode.continentId = fileIsland.id;
        toyTownNode.mapPositionX = 250.0f; // Positioned to the right of center
        toyTownNode.mapPositionY = 220.0f; // Positioned below center
        toyTownNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        toyTownNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png"; // Generic placeholder
        toyTownNode.totalSteps = 320; // Placeholder steps
        toyTownNode.isUnlocked = true; // Set to unlocked
        toyTownNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/file_island/05_toy_town/layer_0.png"}, 0.5f, 0.0f));
        toyTownNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/file_island/05_toy_town/layer_1.png"}, 0.25f, 0.0f));
        toyTownNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/file_island/05_toy_town/layer_2.png"}, 0.1f, 0.0f));        // --- Define Node: 06_infinity_mountain ---
        NodeData infinityMountainNode;
        infinityMountainNode.id = "file_island_node_06";
        infinityMountainNode.name = "INFINITY MOUNTAIN";
        infinityMountainNode.continentId = fileIsland.id;
        infinityMountainNode.mapPositionX = 400.0f; // Positioned at far right
        infinityMountainNode.mapPositionY = 220.0f; // Positioned at bottom right
        infinityMountainNode.unlockedSpritePath = "assets/ui/node_white_placeholder.png";
        infinityMountainNode.bossSpritePath = "assets/sprites/enemies/boss_placeholder_generic_idle.png"; // Generic placeholder
        infinityMountainNode.totalSteps = 600; // Placeholder steps
        infinityMountainNode.isUnlocked = true; // Set to unlocked
        infinityMountainNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/file_island/06_infinity_mountain/layer_0.png"}, 0.5f, 0.0f));
        infinityMountainNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/file_island/06_infinity_mountain/layer_1.png"}, 0.25f, 0.0f));
        infinityMountainNode.adventureBackgroundLayers.push_back(BackgroundLayerData({"assets/backgrounds/environments/file_island/06_infinity_mountain/layer_2.png"}, 0.1f, 0.0f));

        // Add all nodes to File Island
        fileIsland.nodes.push_back(tropicalJungleNode);
        fileIsland.nodes.push_back(lakeNode);
        fileIsland.nodes.push_back(gearSavannahNode);
        fileIsland.nodes.push_back(factorialTownNode);
        fileIsland.nodes.push_back(toyTownNode);
        fileIsland.nodes.push_back(infinityMountainNode);

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
