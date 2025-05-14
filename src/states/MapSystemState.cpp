#include "States/MapSystemState.h"
#include "Core/Game.h" 
#include "Core/InputManager.h" 
#include "Core/PlayerData.h"   
#include "Core/AssetManager.h" // Added for loading textures
#include "UI/TextRenderer.h"   // Added for rendering text
#include "platform/pc/pc_display.h" // Corrected path for PCDisplay.h

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
        fileIsland.mapImagePath = "assets/backgrounds/environments/file_island/file_island_map.png";

        // --- Define Node: 01_tropical_jungle ---
        NodeData tropicalJungleNode;
        tropicalJungleNode.id = "file_island_node_01";
        tropicalJungleNode.name = "TROPICAL JUNGLE"; // Changed to uppercase
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
        if (currentContinentIndex_ < 0 || currentContinentIndex_ >= static_cast<int>(continents_.size()) || 
            currentNodeIndex_ < 0 || currentNodeIndex_ >= static_cast<int>(continents_[currentContinentIndex_].nodes.size())) return;

        const auto& selectedNode = continents_[currentContinentIndex_].nodes[currentNodeIndex_];

        if (inputManager.isActionJustPressed(GameAction::CONFIRM)) {
            SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Map: Node '%s' Detail CONFIRMED. Initiating adventure...", selectedNode.name.c_str());
            currentView_ = MapView::NODE_SELECTION; 
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Map: AdventureState transition not yet implemented. Returning to Node Selection.");
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
            // Corrected: getTextWidth to getTextDimensions().x
            SDL_Point textSize = textRenderer->getTextDimensions(continent.name); // getTextDimensions returns SDL_Point {w, h}
            int textX = (screenWidth - textSize.x) / 2;
            int textY = static_cast<int>(static_cast<float>(screenHeight) * 0.75f - static_cast<float>(textSize.y) / 2.0f); // Centered around 75% mark
            // Corrected: renderText to drawText
            textRenderer->drawText(display.getRenderer(), continent.name, textX, textY, 0.5f); // Adjusted Y position
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
        
        // Render continent name (already uppercase from load_map_data)
        SDL_Point continentNameSize = textRenderer->getTextDimensions(continent.name);
        textRenderer->drawText(display.getRenderer(), continent.name, (screenWidth - continentNameSize.x) / 2, 20, 1.5f); // Kept at top


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
        
        // Render "NODE SELECTION VIEW (WIP)" for now, or actual node names later
        // textRenderer->drawText(display.getRenderer(), "NODE SELECTION VIEW (WIP)", 10, 50, 1.0f); // Changed to uppercase

        // Iterate and render nodes (actual implementation for PENDING item 3)
        // For now, just display the name of the currently selected node as an example
        const auto& selectedNode = continent.nodes[currentNodeIndex_];
        std::string selectedNodeText = "SELECTED: " + selectedNode.name; // Node name is already uppercase
        SDL_Point nodeTextSize = textRenderer->getTextDimensions(selectedNodeText);
        int nodeTextX = (screenWidth - nodeTextSize.x) / 2;
        int nodeTextY = static_cast<int>(static_cast<float>(screenHeight) * 0.75f - static_cast<float>(nodeTextSize.y) / 2.0f); // Centered around 75% mark
        textRenderer->drawText(display.getRenderer(), selectedNodeText, nodeTextX, nodeTextY, 1.0f); // Adjusted Y position

        // TODO: Render node sprites on the map
        // TODO: Implement selection highlighting
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
        SDL_Point nodeNameSize = textRenderer->getTextDimensions(node.name); // Name is already uppercase
        textRenderer->drawText(display.getRenderer(), node.name, (screenWidth - nodeNameSize.x) / 2, 30, 1.5f);

        // Display "NODE DETAIL VIEW (WIP)" or more specific details
        // textRenderer->drawText(display.getRenderer(), "NODE DETAIL VIEW (WIP)", 10, 70, 1.0f); // Changed to uppercase

        // Display Step Count
        std::string stepsText = "STEPS: " + std::to_string(node.totalSteps);
        SDL_Point stepsTextSize = textRenderer->getTextDimensions(stepsText);
        textRenderer->drawText(display.getRenderer(), stepsText, (screenWidth - stepsTextSize.x) / 2, screenHeight - 70, 1.0f);
        
        // Display "Confirm to Start" or similar prompt
        std::string promptText = "CONFIRM TO START ADVENTURE";
        SDL_Point promptTextSize = textRenderer->getTextDimensions(promptText);
        textRenderer->drawText(display.getRenderer(), promptText, (screenWidth - promptTextSize.x) / 2, screenHeight - 40, 1.0f);


        // TODO: Render boss sprite
        // TODO: Render other details (event list, etc.)
    }

} // namespace Digivice
