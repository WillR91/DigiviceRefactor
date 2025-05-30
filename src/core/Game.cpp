// File: src/core/Game.cpp

#include "core/Game.h"
#include "states/GameState.h"        // Include base class for enter/exit
#include "states/AdventureState.h"   // Include concrete states used
#include "states/TransitionState.h" // Include concrete states used
#include "core/PlayerData.h"
#include "core/InputManager.h"
#include "platform/pc/pc_display.h" // Needed for PCDisplay type
#include "core/AssetManager.h"
#include "ui/TextRenderer.h"
#include "core/AnimationManager.h"
#include "core/FrameRateManager.h" // Required for std::unique_ptr destructor
#include "utils/ConfigManager.h" // Add ConfigManager include
#include "entities/DigimonRegistry.h" // <<< ADDED for Digimon definitions
#include "graphics/SeamlessBackgroundRenderer.h" // Required for std::unique_ptr destructor
#include "core/ErrorManager.h" // Required for std::unique_ptr destructor
#include "core/BackgroundVariantManager.h" // Required for default node initialization

#include <SDL_log.h>
#include <stdexcept>
#include <filesystem> // For CWD logging
#include <vector>
#include <memory>
#include <string>

Game::Game() : 
    display(),                 // Change from window_
    inputManager(),            // Change from input_manager_
    assetManager(),            // Change from asset_manager_
    states_(),
    is_running(true),          // Change from is_running_
    fade_step_(FadeSequenceStep::NONE),  
    targetStateAfterFade_(StateType::None),
    pop_until_target_type_(StateType::None)
{
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Game initializing...");
}

Game::~Game() {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Game destructor called.");
    // Cleanup happens in close() if game loop exited normally,
    // otherwise destructor handles unique_ptr cleanup.
}

bool Game::init(const std::string& title, int width, int height) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Initializing Game systems...");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL Init Error: %s", SDL_GetError()); return false; }
    if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0")) { SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "Hint Warning: %s", SDL_GetError()); }
    else { SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "SDL_HINT_RENDER_SCALE_QUALITY set to 0 (Nearest Neighbor)."); }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL Initialized.");

    // Get small screen size from config (if not found, use defaults)
    int smallScreenWidth = ConfigManager::getValue<int>("display.smallScreenWidth", DEFAULT_SMALL_SCREEN_WIDTH);
    int smallScreenHeight = ConfigManager::getValue<int>("display.smallScreenHeight", DEFAULT_SMALL_SCREEN_HEIGHT);
    bool vsync = ConfigManager::getValue<bool>("display.vsync", true);
    
    // Store the original received dimensions (from main)
    original_width_ = width;
    original_height_ = height;
    is_small_screen_ = false; // Ensure it starts with normal size
    
    // Pass vsync setting from config
    if (!display.init(title.c_str(), width, height, vsync)) { 
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay Init Error"); 
        SDL_Quit(); 
        return false; 
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay Initialized.");

    if (!assetManager.init(display.getRenderer())) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AssetManager Init Error"); display.close(); SDL_Quit(); return false; }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetManager Initialized.");

    // Initialize Digimon Registry
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Initializing DigimonRegistry...");
    std::string definitionsPath = ConfigManager::getValue<std::string>("paths.digimonDefinitions", "assets/definitions/digimon_definitions");
    if (!Digimon::DigimonRegistry::getInstance().initialize(definitionsPath)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DigimonRegistry Initialization Failed!");
        // Decide if this is a fatal error. For now, we'll log and continue.
        // return false; // Uncomment if this should be a fatal error
    } else {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "DigimonRegistry Initialized Successfully.");
        
        // Register all enemy Digimon for the Enemy Test Screen
        auto& registry = Digimon::DigimonRegistry::getInstance();
        
        // Register all enemy Digimon - first batch (already defined in DigimonDefinition.cpp will be skipped)
        registry.registerEnemyDigimon("kuwagamon", "Kuwagamon", "An Insectoid Digimon with large pincers and red shell.");
        registry.registerEnemyDigimon("andromon", "Andromon", "A Cyborg Digimon with enhanced mechanical parts.");
        registry.registerEnemyDigimon("apocalymon", "Apocalymon", "A monstrous Digimon born from the data of defeated and deleted Digimon.");
        registry.registerEnemyDigimon("bakemon", "Bakemon", "A Ghost Digimon that loves to frighten others.");
        registry.registerEnemyDigimon("blossomon", "Blossomon", "A Plant Digimon with dangerous ivy tendrils.");
        registry.registerEnemyDigimon("centaurmon", "Centaurmon", "A Beast Man Digimon with the upper body of a human and lower body of a horse.");
        registry.registerEnemyDigimon("cherrymon", "Cherrymon", "An ancient Plant Digimon that resembles a large cherry tree.");
        registry.registerEnemyDigimon("cockatrimon", "Cockatrimon", "A Bird Digimon that can petrify enemies with its gaze.");
        registry.registerEnemyDigimon("colosseumgreymon", "Colosseum Greymon", "A gladiator-like variant of Greymon trained for arena combat.");
        registry.registerEnemyDigimon("darktyranomon", "DarkTyranomon", "A dark-colored, virus-type variant of Tyranomon with increased aggression.");
        registry.registerEnemyDigimon("demidevimon", "DemiDevimon", "A small but devious bat-like Digimon that serves evil masters.");
        registry.registerEnemyDigimon("deramon", "Deramon", "A Bird Digimon with a bush growing on its back.");
        registry.registerEnemyDigimon("devidramon", "Devidramon", "A dark Dragon Digimon with four red eyes and sharp claws.");
        registry.registerEnemyDigimon("devimon", "Devimon", "A fallen Angel Digimon with long arms and black wings.");
        registry.registerEnemyDigimon("deviwomon", "Deviwomon", "A female demonic Digimon with powerful dark magic abilities.");
        
        // Register second batch of enemy Digimon
        registry.registerEnemyDigimon("diablomon", "Diablomon", "A virus Digimon that attacks computer networks and multiplies rapidly.");
        registry.registerEnemyDigimon("digitamamon", "Digitamamon", "A mysterious Digimon with a body like an egg and only legs and eyes visible.");
        registry.registerEnemyDigimon("divermon", "Divermon", "An aquatic Digimon expert in underwater combat with a harpoon.");
        registry.registerEnemyDigimon("dokugumon", "Dokugumon", "A poisonous spider Digimon that traps prey in its sticky web.");
        registry.registerEnemyDigimon("drimogemon", "Drimogemon", "A Beast Digimon that excels at digging with its drill nose and claws.");
        registry.registerEnemyDigimon("elecmon", "Elecmon", "A Mammal Digimon that can generate electricity from its tail.");
        registry.registerEnemyDigimon("etemon", "Etemon", "A Monkey Digimon who loves to perform and controls the Dark Network.");
        registry.registerEnemyDigimon("evilmon", "Evilmon", "A small demonic Digimon with a mischievous personality.");
        registry.registerEnemyDigimon("floramon", "Floramon", "A Plant Digimon with flower-like hands that emit a pleasant fragrance.");
        registry.registerEnemyDigimon("flymon", "Flymon", "An Insectoid Digimon with a dangerous poisonous stinger.");
        registry.registerEnemyDigimon("frigimon", "Frigimon", "A Beast Digimon made of snow with ice punching attacks.");
        registry.registerEnemyDigimon("garbagemon", "Garbagemon", "A Digimon that lives in trash heaps and attacks with garbage.");
        registry.registerEnemyDigimon("gazimon", "Gazimon", "A Mammal Digimon with sharp claws and a mischievous nature.");
        registry.registerEnemyDigimon("gekomon", "Gekomon", "An Amphibian Digimon that attacks using sound waves from its horn.");
        registry.registerEnemyDigimon("gesomon", "Gesomon", "A Mollusk Digimon that resembles a squid with powerful tentacles.");
        
        // Register third batch of enemy Digimon
        registry.registerEnemyDigimon("gigadramon", "Gigadramon", "A Cyborg Dragon Digimon with mechanical parts and devastating attacks.");
        registry.registerEnemyDigimon("gizamon", "Gizamon", "An Amphibian Digimon with a spiky back that moves quickly on both land and water.");
        registry.registerEnemyDigimon("gotsumon", "Gotsumon", "A Rock Digimon whose body is entirely made of stone.");
        registry.registerEnemyDigimon("hagurumon", "Hagurumon", "A Machine Digimon shaped like a gear with simple mechanical capabilities.");
        registry.registerEnemyDigimon("infermon", "Infermon", "An evolved form of Keramon that moves quickly through computer networks.");
        registry.registerEnemyDigimon("keramon", "Keramon", "A virus-type Digimon that causes chaos in computer systems.");
        registry.registerEnemyDigimon("kiwimon", "Kiwimon", "A Bird Digimon resembling a kiwi bird that attacks with mini-birds.");
        registry.registerEnemyDigimon("leomon", "Leomon", "A Beast Man Digimon with a strong sense of justice.");
        registry.registerEnemyDigimon("machinedramon", "Machinedramon", "A massive metallic Dragon Digimon with devastating firepower.");
        registry.registerEnemyDigimon("mammothmon", "Mammothmon", "A large Ancient Animal Digimon resembling a wooly mammoth.");
        registry.registerEnemyDigimon("megadramon", "Megadramon", "A Cyborg Dragon Digimon created as a weapon of mass destruction.");
        registry.registerEnemyDigimon("megaseadramon", "MegaSeadramon", "A Sea Animal Digimon with a lightning-shaped horn and enhanced powers.");
        registry.registerEnemyDigimon("mekanorimon", "Mekanorimon", "A robotic Digimon that moves using electromagnetic forces.");
        registry.registerEnemyDigimon("meramon", "Meramon", "A Flame Digimon with a body made entirely of fire.");
        registry.registerEnemyDigimon("metaletemon", "MetalEtemon", "A Digimon with a body coated in Chrome Digizoid, making him nearly invincible.");
        
        // Register fourth batch of enemy Digimon
        registry.registerEnemyDigimon("metalseadramon", "MetalSeadramon", "A Sea Animal Digimon coated in Chrome Digizoid, leader of the Dark Masters.");
        registry.registerEnemyDigimon("mojyamon", "Mojyamon", "A Beast Man Digimon that lives in snowy mountains.");
        registry.registerEnemyDigimon("monochromon", "Monochromon", "A Dinosaur Digimon resembling a triceratops with a hard shell.");
        registry.registerEnemyDigimon("monzaemon", "Monzaemon", "A Puppet Digimon resembling a giant teddy bear.");
        registry.registerEnemyDigimon("mushmon", "Mushmon", "A Plant Digimon resembling a mushroom with spore-based attacks.");
        registry.registerEnemyDigimon("myotismon", "Myotismon", "A Vampire Digimon with powerful dark abilities.");
        registry.registerEnemyDigimon("nanimon", "Nanimon", "A bizarre Digimon with a round body and unpredictable behavior.");
        registry.registerEnemyDigimon("nanomon", "Nanomon", "A small Machine Digimon with exceptional technical knowledge.");
        registry.registerEnemyDigimon("numemon", "Numemon", "A slimy Mollusk Digimon that prefers dark and damp places.");
        registry.registerEnemyDigimon("ogremon", "Ogremon", "A virus-type ogre Digimon that carries a bone club.");
        registry.registerEnemyDigimon("otamamon", "Otamamon", "An Amphibian Digimon resembling a tadpole that lives in lakes and rivers.");
        registry.registerEnemyDigimon("parrotmon", "Parrotmon", "A giant Bird Digimon with the ability to generate electricity from its feathers.");
        registry.registerEnemyDigimon("phantomon", "Phantomon", "A Ghost Digimon resembling a reaper with a large scythe.");
        registry.registerEnemyDigimon("piemon", "Piemon", "The leader of the Dark Masters with deadly sword attacks and reality-altering powers.");
        registry.registerEnemyDigimon("piximon", "Piximon", "A tiny fairy Digimon known for training other Digimon with harsh methods.");
        
        // Register fifth batch of enemy Digimon
        registry.registerEnemyDigimon("pumpmon", "Pumpmon", "A Digimon with a pumpkin head who loves to play pranks.");
        registry.registerEnemyDigimon("puppetmon", "Puppetmon", "A Puppet Digimon and one of the Dark Masters with a malicious personality.");
        registry.registerEnemyDigimon("raremon", "Raremon", "A slimy, decaying Digimon that emits a foul odor and corrosive gases.");
        registry.registerEnemyDigimon("redvegimon", "RedVegimon", "A red variant of Vegimon with enhanced fighting capabilities.");
        registry.registerEnemyDigimon("sabreleomon", "SabreLeomon", "A Beast Man Digimon with saber-like fangs and incredible speed.");
        registry.registerEnemyDigimon("scorpiomon", "Scorpiomon", "An Ancient Crustacean Digimon resembling a scorpion with powerful pincers.");
        registry.registerEnemyDigimon("seadramon", "Seadramon", "A Sea Animal Digimon that resembles a large serpent and lives in deep water.");
        registry.registerEnemyDigimon("shellmon", "Shellmon", "A Mollusk Digimon with a hard shell and water-based attacks.");
        registry.registerEnemyDigimon("shogungekomon", "ShogunGekomon", "A large Amphibian Digimon with massive musical horns and royal status.");
        registry.registerEnemyDigimon("skullmeramon", "SkullMeramon", "A Flame Digimon with a skeletal body and blue fire burning all over.");
        registry.registerEnemyDigimon("snimon", "Snimon", "An Insectoid Digimon with large sickle-like arms and excellent hunting skills.");
        registry.registerEnemyDigimon("sukamon", "Sukamon", "A mutant Digimon made from waste data, resembling a yellow blob.");
        registry.registerEnemyDigimon("tankmon", "Tankmon", "A Machine Digimon equipped with cannons and treads for movement.");
        registry.registerEnemyDigimon("tuskmon", "Tuskmon", "A Dinosaur Digimon with large tusks and powerful physical attacks.");
        registry.registerEnemyDigimon("tyranomon", "Tyranomon", "A Dinosaur Digimon resembling a T-Rex with powerful fire breath.");
        
        // Register final batch of enemy Digimon
        registry.registerEnemyDigimon("unimon", "Unimon", "A Mythical Beast Digimon resembling a winged unicorn.");
        registry.registerEnemyDigimon("vademon", "Vademon", "An alien-like Digimon with a large brain and strange scientific knowledge.");
        registry.registerEnemyDigimon("vegimon", "Vegimon", "A Plant Digimon with extensible vine-like arms.");
        registry.registerEnemyDigimon("venommyotismon", "VenomMyotismon", "A more powerful and monstrous form of Myotismon.");
        registry.registerEnemyDigimon("warumonzaemon", "WaruMonzaemon", "A darker, sinister version of Monzaemon with an evil personality.");
        registry.registerEnemyDigimon("whamon", "Whamon", "A gigantic Aquatic Digimon resembling a whale.");
        
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "All 81 enemy Digimon registered successfully for testing.");
    }

    // Log CWD
    try {
        std::filesystem::path cwd = std::filesystem::current_path();
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Current Working Directory: %s", cwd.string().c_str());
    } catch (const std::filesystem::filesystem_error& e) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error getting CWD using std::filesystem: %s", e.what()); }

    // Load Initial Assets
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Attempting to load initial assets...");
     bool assets_ok = true;
     assets_ok &= assetManager.loadTexture("ui_round_mask", "assets/ui/mask/round_mask.png"); // Use forward slashes for consistency
     ui_mask_texture_ = assetManager.getTexture("ui_round_mask"); // Load and store the mask texture
     if (!ui_mask_texture_) {
         SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Failed to load UI mask texture: assets/ui/mask/round_mask.png");
         // Decide if this is a critical failure or if the game can run without it
     }     assets_ok &= assetManager.loadTexture("round_mask", "assets/ui/mask/round_mask.png");
     
     // Load new player Digimon sprite sheets with their new textureIds
     assets_ok &= assetManager.loadTexture("agumon", "assets/sprites/player_digimon/agumon.png");
     assets_ok &= assetManager.loadTexture("gabumon", "assets/sprites/player_digimon/gabumon.png");
     assets_ok &= assetManager.loadTexture("biyomon", "assets/sprites/player_digimon/biyomon.png");
     assets_ok &= assetManager.loadTexture("gatomon", "assets/sprites/player_digimon/gatomon.png");
     assets_ok &= assetManager.loadTexture("gomamon", "assets/sprites/player_digimon/gomamon.png");
     assets_ok &= assetManager.loadTexture("palmon", "assets/sprites/player_digimon/palmon.png");     assets_ok &= assetManager.loadTexture("tentomon", "assets/sprites/player_digimon/tentomon.png");
     assets_ok &= assetManager.loadTexture("patamon", "assets/sprites/player_digimon/patamon.png");
       // Load unlockable Digimon
     assets_ok &= assetManager.loadTexture("veedramon", "assets/sprites/player_digimon/veedramon.png");
     assets_ok &= assetManager.loadTexture("wizardmon", "assets/sprites/player_digimon/wizardmon.png");
     
     // Note: Legacy castle_bg assets removed - using new environment backgrounds instead
     assets_ok &= assetManager.loadTexture("menu_bg_blue", "assets/ui/backgrounds/menu_base_blue.png");
     assets_ok &= assetManager.loadTexture("transition_borders", "assets/ui/transition/transition_borders.png");
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Loading UI font...");
     assets_ok &= assetManager.loadTexture("ui_font_atlas", "assets/ui/fonts/bluewhitefont.png");
     if (!assets_ok) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "One or more essential assets failed to load! Check paths and file existence."); assetManager.shutdown(); display.close(); SDL_Quit(); return false; }
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Finished loading initial assets attempt.");

    // Initialize Text Renderer
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Initializing TextRenderer...");
    SDL_Texture* fontTexturePtr = assetManager.getTexture("ui_font_atlas");
    if (fontTexturePtr) {
        textRenderer_ = std::make_unique<TextRenderer>(fontTexturePtr);
        const std::string fontJsonPath = "assets/ui/fonts/bluewhitefont.json"; // Use forward slashes
        if (!textRenderer_->loadFontData(fontJsonPath)) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load font data for TextRenderer from '%s'. Text rendering may fail.", fontJsonPath.c_str());
            textRenderer_.reset();
        } else {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer initialized successfully.");
        }
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize TextRenderer: Font texture 'ui_font_atlas' not found in AssetManager.");
    }

    // Initialize Animation Manager & Load Animations
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Initializing AnimationManager...");
    try {
        animationManager_ = std::make_unique<AnimationManager>(&assetManager); // Pass AssetManager pointer        
        bool anims_ok = true;
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager: Loading animation data files...");
          // Load new player Digimon animation data 
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/player_digimon/agumon.json", "agumon");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/player_digimon/gabumon.json", "gabumon");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/player_digimon/biyomon.json", "biyomon");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/player_digimon/gatomon.json", "gatomon");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/player_digimon/gomamon.json", "gomamon");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/player_digimon/palmon.json", "palmon");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/player_digimon/tentomon.json", "tentomon");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/player_digimon/patamon.json", "patamon");
        
        // Load unlockable Digimon animations
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/player_digimon/veedramon.json", "veedramon");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/player_digimon/wizardmon.json", "wizardmon");
        
        // Enemy Digimon animation data is already loaded by loadAllEnemyDigimonAssets()
        
        // Keep loading legacy animation data for backward compatibility
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/agumon_sheet.json", "agumon_sheet");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/gabumon_sheet.json", "gabumon_sheet");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/biyomon_sheet.json", "biyomon_sheet");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/gatomon_sheet.json", "gatomon_sheet");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/gomamon_sheet.json", "gomamon_sheet");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/palmon_sheet.json", "palmon_sheet");        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/tentomon_sheet.json", "tentomon_sheet");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/patamon_sheet.json", "patamon_sheet");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/digimon/kuwagamon/animation.json", "kuwagamon_sheet");

        // Now load all enemy Digimon assets (textures and animations)
        bool enemyAssetsOk = loadAllEnemyDigimonAssets();
        if (!enemyAssetsOk) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Some enemy Digimon assets failed to load. EnemyTestState may not work properly.");
        }

        if (!anims_ok) {
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"One or more animation files failed to load properly. See errors above. Game may continue with missing/default animations.");
        } else {
             SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager finished loading animation data.");
        }

    } catch (const std::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create or initialize AnimationManager: %s", e.what());
        textRenderer_.reset();
        assetManager.shutdown();
        display.close();
        SDL_Quit();
        return false;    }

    // Initialize PlayerData with default node containing proper background data
    try {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Setting up default node data for initial state...");
        
        // Create a default node with the tropical jungle background using the variant system
        Digivice::NodeData defaultNode;
        defaultNode.id = "01_fi_node_01_tropical_jungle";
        defaultNode.name = "TROPICAL JUNGLE";
        defaultNode.continentId = "01_file_island";
        defaultNode.totalSteps = 20;
        defaultNode.isUnlocked = true;
        
        // Create background data using the variant system directly
        Digivice::BackgroundLayerData layerData;
        layerData.parallaxFactorX = 0.5f;
        layerData.parallaxFactorY = 0.0f;
        
        // Initialize variants using the BackgroundVariantManager
        Digivice::BackgroundVariantManager::initializeVariantsForNode(layerData, "tropicaljungle");
        
        defaultNode.adventureBackgroundLayers.push_back(layerData);
        
        // Set this as the current node in PlayerData
        playerData_.setCurrentMapNode(defaultNode);
        
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Default node '%s' initialized with %zu background layers", 
                   defaultNode.name.c_str(), defaultNode.adventureBackgroundLayers.size());
                   
    } catch (const std::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize default node data: %s", e.what());
        // Continue anyway with empty node data (fallback to legacy system)
    }

    // Push Initial State and Call Enter
    try {
        states_.push_back(std::make_unique<AdventureState>(this));
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Initial AdventureState created and added.");
        if (!states_.empty()) {
             states_.back()->enter();
             SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Called enter() on initial state.");
        }
    } catch (const std::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create initial state: %s", e.what());
        animationManager_.reset();
        textRenderer_.reset();
        assetManager.shutdown();
        display.close();
        SDL_Quit();
        return false;
    }

    // Make sure fade step is reset
    fade_step_ = FadeSequenceStep::NONE;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Fade step reset to NONE during initialization.");

    is_running = true;
    last_frame_time = SDL_GetTicks();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Game Initialization Successful.");
    return true;
}

// --- Run Loop ---
void Game::run() {
    if (!is_running || states_.empty()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Game::run() called in invalid state (not initialized or no initial state).");
        return;
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Entering main game loop.");
    last_frame_time = SDL_GetTicks();

    while (is_running) {
        // Calculate Delta Time
        Uint32 current_time = SDL_GetTicks();
        float delta_time = (current_time - last_frame_time) / 1000.0f;
        if (delta_time > 0.1f) delta_time = 0.1f; // Clamp max delta time
        last_frame_time = current_time;

        // Input Processing
        inputManager.prepareNewFrame();
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit_game();
            }
            inputManager.processEvent(event);
        }        // Handle screen toggle action
        if (inputManager.isActionJustPressed(GameAction::TOGGLE_SCREEN_SIZE)) {
            is_small_screen_ = !is_small_screen_;
            if (is_small_screen_) {
                // Get small screen size from config
                int smallScreenWidth = ConfigManager::getValue<int>("display.smallScreenWidth", DEFAULT_SMALL_SCREEN_WIDTH);
                int smallScreenHeight = ConfigManager::getValue<int>("display.smallScreenHeight", DEFAULT_SMALL_SCREEN_HEIGHT);
                
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Toggling to small screen mode: %dx%d", smallScreenWidth, smallScreenHeight);
                display.setWindowSize(smallScreenWidth, smallScreenHeight);
                display.setLogicalSize(original_width_, original_height_); // Render at original size, scale down
            } else {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Toggling to normal screen mode: %dx%d", original_width_, original_height_);
                display.setWindowSize(original_width_, original_height_);
                display.setLogicalSize(original_width_, original_height_); // Reset logical size
            }
        }

        // State Logic
        if (!states_.empty()) {
            // Apply any pending state changes
            applyStateChanges();

            // Handle input for the current state
            if (!states_.empty()) { // Check again, applyStateChanges might alter the stack
                states_.back()->handle_input(inputManager, &playerData_);
            }            // Update the current state
            if (!states_.empty()) { // Check again
                states_.back()->update(delta_time, &playerData_);
            }

            // Render Logic
            if (!states_.empty()) { // Check again in case update caused a state change
                GameState* currentStateForRender = states_.back().get();
                if (currentStateForRender) {
                    // Clear the screen (e.g., black or a debug color)
                    // display.clear(0, 0, 0, 255); // Example: Clear to black
                    currentStateForRender->render(display); // Pass by reference

                    // Apply the UI mask after the state has rendered its content
                    if (ui_mask_texture_) {
                        display.applyMask(ui_mask_texture_);
                    }
                }
            }
            // Present the frame
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Presenting frame.");
            display.present();
            
        } else {
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Game::run() - State stack is empty, nothing to update or render.");
        }
        
        // Add this debugging line somewhere in your update cycle
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Current fade step: %d", 
                    static_cast<int>(fade_step_));
        // ... rest of the run method ...
    }
}

// --- State Management - Actual Push/Pop ---
void Game::push_state(std::unique_ptr<GameState> new_state) {
    if (!new_state) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "push_state called with null state!"); return; }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Executing push_state for state %p (Type: %d)...",
                (void*)new_state.get(), (int)new_state->getType()); // Log type
    states_.push_back(std::move(new_state));
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "push_state complete. New stack size: %zu", states_.size());
}

void Game::pop_state() {
    if (!states_.empty()) {
         GameState* stateToPop = states_.back().get();
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Executing pop_state for state %p (Type: %d)...", (void*)stateToPop, (int)stateToPop->getType());
         // exit() called before this in applyStateChanges
         states_.pop_back(); // Removes the unique_ptr, deleting the state object
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "pop_state complete. New stack size: %zu", states_.size());
    } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "pop_state called on empty stack!"); }
}

// --- State Management - Requests ---
void Game::requestPushState(std::unique_ptr<GameState> state) {
    if (!state) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Attempted to request push of NULL state!"); return; }
    if (request_push_) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Overwriting previous push request for state %p with new request for %p.", (void*)request_push_.get(), (void*)state.get()); }
    request_push_ = std::move(state);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Push requested for state %p (Type: %d).",
                 (void*)request_push_.get(), (int)request_push_->getType()); // Log type
}

void Game::requestPopState() {
    if(request_pop_) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Multiple pop requests made before applyStateChanges.");
    request_pop_ = true;
    pop_until_target_type_ = StateType::None; // Cancel any pending single pop
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Single Pop requested.");
}

void Game::requestPopUntil(StateType targetType) {
    if (targetType == StateType::None) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "requestPopUntil called with StateType::None. Performing single pop instead.");
        requestPopState();
        return;
    }
    pop_until_target_type_ = targetType;
    request_pop_ = false; // Cancel any pending single pop
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "PopUntil requested for type: %d", static_cast<int>(targetType));
}

void Game::requestFadeToState(std::unique_ptr<GameState> targetState, float duration, bool popCurrent) {
    if (fade_step_ != FadeSequenceStep::NONE) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "requestFadeToState called while a fade sequence is already in progress. Ignoring.");
        if (targetState) { SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "requestFadeToState: Discarding targetState due to ongoing fade."); }
        return;
    }
    // Allow nullptr targetState ONLY if we intend to pop the current state.
    // If popCurrent is false, a targetState is mandatory.
    if (!targetState && !popCurrent) { 
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "requestFadeToState called with null targetState and popCurrent is false. This is invalid."); 
        return; 
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Requesting Fade-To-State sequence (Target Type: %d, Duration: %.2fs, PopCurrent: %s).",
                (targetState ? (int)targetState->getType() : (int)StateType::None), // Log target type or None
                duration, popCurrent ? "true" : "false");
    pending_state_for_fade_ = std::move(targetState);
    fade_duration_ = duration > 0.001f ? duration : 0.001f;
    pop_current_after_fade_out_ = popCurrent;
    fade_step_ = FadeSequenceStep::FADING_OUT;
    active_fade_type_ = TransitionEffectType::FADE_TO_COLOR; // Set type for this fade

    auto fadeOutState = std::make_unique<TransitionState>(
        this,
        fade_duration_,
        active_fade_type_
    );
    requestPushState(std::move(fadeOutState)); // Use existing requestPushState
}

// Implementation moved to Game_fade.cpp

void Game::update(float delta_time) {
    if (states_.empty()) return;

    // Apply any state changes before update
    applyStateChanges();

    // Update the current state
    if (!states_.empty()) {
        states_.back()->update(delta_time, &playerData_);
    }
    
    /* Comment out the problematic fade detection code for now
    // Check if the current state is a TransitionState
    if (!states_.empty() && states_.back()->getType() == StateType::Transition) {
        TransitionState* transition = static_cast<TransitionState*>(states_.back().get());
        float progress = transition->getProgress();
        
        // Update fade_step_ based on transition progress
        if (transition->getEffectType() == TransitionEffectType::FADE_TO_COLOR) {
            if (progress >= 0.5f && fade_step_ == FadeSequenceStep::FADING_OUT) {
                fade_step_ = FadeSequenceStep::READY_FOR_STATE_SWAP;
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, 
                           "Fade transition halfway point reached");
            }
            else if (progress >= 0.95f && fade_step_ == FadeSequenceStep::READY_FOR_STATE_SWAP) {
                fade_step_ = FadeSequenceStep::FADING_IN;
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, 
                           "Fade transition almost complete, fading in");
            }
            else if (progress >= 1.0f && fade_step_ == FadeSequenceStep::FADING_IN) {
                fade_step_ = FadeSequenceStep::NONE;
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, 
                           "Fade transition complete");
            }
        }
    }
    */
}

// --- Helper to apply queued changes ---
// <<< --- INTEGRATED NEW VERSION --- >>>
void Game::applyStateChanges() {

    bool state_popped_this_frame = false;
    TransitionEffectType popped_transition_type = TransitionEffectType::BORDER_WIPE; 
    StateType original_popped_state_type_if_transition = StateType::None; // Store type of state that _caused_ transition pop

    // --- 1. Handle Pops ---
    if (pop_until_target_type_ != StateType::None) {
        // --- Pop Until Logic ---
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Processing PopUntil target type %d.", (int)pop_until_target_type_);
        while (!states_.empty()) {
            GameState* topState = states_.back().get();
            if (topState->getType() == pop_until_target_type_) {
                SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges (PopUntil): Found target state %p (type %d). Calling enter().",
                             (void*)topState, (int)topState->getType());
                topState->enter(); 
                state_popped_this_frame = true; 
                break; 
            }
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges (PopUntil): Exiting and popping state %p (type %d).",
                         (void*)topState, (int)topState->getType());
            topState->exit();
            pop_state();
            state_popped_this_frame = true;
        }
        if (states_.empty() && pop_until_target_type_ != StateType::None) {
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: PopUntil target type %d not found in stack.", (int)pop_until_target_type_);
        }
        pop_until_target_type_ = StateType::None;
        request_pop_ = false; 
        request_push_.reset(); 

        if (fade_step_ != FadeSequenceStep::NONE) {
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: pop_until executed during fade sequence. Resetting fade.");
             fade_step_ = FadeSequenceStep::NONE;
             pending_state_for_fade_.reset();
             targetStateAfterFade_ = StateType::None;
             pop_current_after_fade_out_ = false;
        }
    }
    else if (request_pop_) {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Processing request_pop_ = true.");
        if (!states_.empty()) {
            GameState* topStateBeforePop = states_.back().get();
            StateType popped_state_type = topStateBeforePop->getType();
            original_popped_state_type_if_transition = popped_state_type; // Capture type

            if (popped_state_type == StateType::Transition) {
               TransitionState* ts = static_cast<TransitionState*>(topStateBeforePop);
               popped_transition_type = ts->getEffectType();
               SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Popped state was Transition, effect type: %d", (int)popped_transition_type);
            } else {
                popped_transition_type = TransitionEffectType::BORDER_WIPE; // Not a transition
            }

            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Calling exit() on state %p (type %d).", (void*)topStateBeforePop, (int)popped_state_type);
            topStateBeforePop->exit();
            pop_state(); 
            state_popped_this_frame = true;
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: State popped. Stack size AFTER pop: %zu", states_.size());

            // --- Handle Fade Sequence Logic After a TransitionState Pops ---
            if (fade_step_ == FadeSequenceStep::FADING_OUT &&
                original_popped_state_type_if_transition == StateType::Transition &&
                popped_transition_type == active_fade_type_ && // active_fade_type_ was FADE_TO_COLOR
                active_fade_type_ == TransitionEffectType::FADE_TO_COLOR)
            {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: FADE_TO_COLOR TransitionState sequence part finished.");

                if (pop_current_after_fade_out_ && !states_.empty()) {
                    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Popping original state %p (type %d) after its FADE_TO_COLOR sequence as requested by pop_current_after_fade_out_.",
                        (void*)states_.back().get(), (int)states_.back()->getType());
                    states_.back()->exit();
                    pop_state(); 
                }
                pop_current_after_fade_out_ = false; // Reset flag

                if (targetStateAfterFade_ != StateType::None) {
                    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Target state after fade is %d. Setting up target.", (int)targetStateAfterFade_);
                    fade_step_ = FadeSequenceStep::SETUP_TARGET_STATE;
                } else if (pending_state_for_fade_) {
                    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Pending new state %p (type %d) for fade. Swapping.",
                        (void*)pending_state_for_fade_.get(), (int)pending_state_for_fade_->getType());
                    fade_step_ = FadeSequenceStep::READY_FOR_STATE_SWAP;
                } else { // Fading back to an underlying state (e.g., "BACK" from a menu)
                    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: FADE_TO_COLOR finished. Fading in underlying state.");
                    if (!states_.empty()) {
                        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Calling enter() on underlying state %p (type %d) before fade-in.",
                            (void*)states_.back().get(), (int)states_.back()->getType());
                        states_.back()->enter();
                    } else {
                        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Stack empty, cannot enter underlying state for fade-in.");
                    }
                    auto fadeInState = std::make_unique<TransitionState>(this, fade_duration_, TransitionEffectType::FADE_FROM_COLOR, SDL_Color{0,0,0,255});
                    active_fade_type_ = TransitionEffectType::FADE_FROM_COLOR; // Update active fade type
                    requestPushState(std::move(fadeInState));
                    fade_step_ = FadeSequenceStep::FADING_IN;
                }
            }
            else if (fade_step_ == FadeSequenceStep::FADING_IN &&
                     original_popped_state_type_if_transition == StateType::Transition &&
                     popped_transition_type == active_fade_type_ && // active_fade_type_ was FADE_FROM_COLOR
                     active_fade_type_ == TransitionEffectType::FADE_FROM_COLOR)
            {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: FADE_FROM_COLOR TransitionState sequence part finished. Fade sequence complete.");
                fade_step_ = FadeSequenceStep::NONE;
                active_fade_type_ = TransitionEffectType::BORDER_WIPE; // Reset
                // The target state (e.g. AdventureState or new MenuState) should have already had enter() called
                // either in SETUP_TARGET_STATE or READY_FOR_STATE_SWAP.
                // If not, and it's the top state, call enter now.
                if (!states_.empty()) {
                     SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Re-confirming/calling enter() on final state %p (type %d) after FADE_FROM_COLOR.",
                                 (void*)states_.back().get(), (int)states_.back()->getType());
                    states_.back()->enter(); // Ensure the final state is properly entered/re-entered.
                }
            }
            else { // Not a fade-related transition pop, or an unexpected pop during a fade
                if (fade_step_ != FadeSequenceStep::NONE && original_popped_state_type_if_transition == StateType::Transition) {
                     SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Transition (type %d, effect %d) popped during fade step %d but was not the expected active_fade_type_ (%d). Resetting fade.",
                                 (int)popped_state_type, (int)popped_transition_type, (int)fade_step_, (int)active_fade_type_);
                     fade_step_ = FadeSequenceStep::NONE;
                     pending_state_for_fade_.reset();
                     targetStateAfterFade_ = StateType::None;
                     pop_current_after_fade_out_ = false;
                }
                SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Handling as normal pop (not part of expected fade sequence step).");
                if (!states_.empty()) {
                    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Calling enter() on new top state %p (type %d).",
                                 (void*)states_.back().get(), (int)states_.back()->getType());
                    states_.back()->enter();
                } else {
                    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: State stack is empty after pop!");
                }
            }
        } else {
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: request_pop_ was true, but state stack was already empty!");
        }
        request_pop_ = false; 
    }

    // --- 2. Handle State Setup/Swaps for Fades (after pops are processed) ---
    if (fade_step_ == FadeSequenceStep::SETUP_TARGET_STATE) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Processing SETUP_TARGET_STATE for type %d.", (int)targetStateAfterFade_);
        // Pop until targetStateAfterFade_ is on top.
        // Note: The original state that was faded from (e.g. PartnerSelect) should have already been popped if pop_current_after_fade_out_ was true.
        pop_until_target_type_ = targetStateAfterFade_; // Set the target for processPopUntil
        processPopUntil(); // This ALREADY calls enter() on the target state if found.

        if (!states_.empty() && states_.back()->getType() == targetStateAfterFade_) {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges (SETUP_TARGET_STATE): Target state %p (type %d) is now at top (enter() called by processPopUntil). Proceeding to fade-in.",
                (void*)states_.back().get(), (int)states_.back()->getType());
            // states_.back()->enter(); // REDUNDANT: processPopUntil already called enter().
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges (SETUP_TARGET_STATE): Failed to bring target %d to top or stack empty. Aborting fade.", (int)targetStateAfterFade_);
            fade_step_ = FadeSequenceStep::NONE; 
            targetStateAfterFade_ = StateType::None; // Ensure reset if aborted before fade-in setup
            // No pending_state_for_fade_ to worry about here, as SETUP_TARGET_STATE uses targetStateAfterFade_
        }

        if (fade_step_ == FadeSequenceStep::SETUP_TARGET_STATE) { // Check if not aborted
            auto fadeInState = std::make_unique<TransitionState>(this, fade_duration_, TransitionEffectType::FADE_FROM_COLOR, SDL_Color{0,0,0,255});
            active_fade_type_ = TransitionEffectType::FADE_FROM_COLOR;
            requestPushState(std::move(fadeInState)); // This will be handled in the push section below or next frame
            fade_step_ = FadeSequenceStep::FADING_IN;
        }
        targetStateAfterFade_ = StateType::None; // Reset
    }
    else if (fade_step_ == FadeSequenceStep::READY_FOR_STATE_SWAP) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Processing READY_FOR_STATE_SWAP.");
        // This step implies a FADE_TO_COLOR just finished, and pop_current_after_fade_out_ might have popped the original state.
        // Now, we push the pending_state_for_fade_ (if any) and then the FADE_FROM_COLOR transition.

        if (pending_state_for_fade_) {
             std::unique_ptr<GameState> targetStateToPush = std::move(pending_state_for_fade_);
             pending_state_for_fade_.reset(); 

             SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges (READY_FOR_STATE_SWAP): Pushing target state %p (type %d).",
                          (void*)targetStateToPush.get(), (int)targetStateToPush->getType());
             // Directly push, don't use requestPushState, as we need to call enter before fade-in.
             push_state(std::move(targetStateToPush)); 
             if (!states_.empty()) {
                SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges (READY_FOR_STATE_SWAP): Calling enter() on new state %p before fade-in.", (void*)states_.back().get());
                states_.back()->enter(); // Call enter on the new state
             }
        } else {
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges (READY_FOR_STATE_SWAP): No pending_state_for_fade_ to push. Underlying state should have been entered.");
             // If no pending state, the state that was under the popped one (if pop_current_after_fade_out_ was true)
             // should have already had enter() called in the FADING_OUT completion block.
        }
        
        auto fadeInState = std::make_unique<TransitionState>(this, fade_duration_, TransitionEffectType::FADE_FROM_COLOR, SDL_Color{0,0,0,255});
        active_fade_type_ = TransitionEffectType::FADE_FROM_COLOR;
        requestPushState(std::move(fadeInState)); // Request push for the fade-in
        fade_step_ = FadeSequenceStep::FADING_IN;
    }

    // --- 3. Handle Pushes ---
    if (request_push_) {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Processing request_push_ for state %p (type %d).",
                     (void*)request_push_.get(), (int)request_push_->getType());
        // If there's a current state, and it's not a transition state that's about to cover it, call its exit().
        // However, for fades, exit() of the underlying state is typically handled before the fade-out starts,
        // or not at all if it's meant to resume.
        // For now, let's assume exit() is handled by the states themselves or by pop logic.
        // if (!states_.empty() && states_.back()->getType() != StateType::Transition) {
        //     states_.back()->exit(); // Exit previous state if not a transition
        // }
        
        std::unique_ptr<GameState> newState = std::move(request_push_);
        request_push_.reset(); 

        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyChanges: Calling enter() on pushed state %p.", (void*)newState.get());
        newState->enter(); // Call enter on the new state being pushed
        push_state(std::move(newState)); // Actual push
    }
}

// --- quit_game ---
void Game::quit_game() {
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Quit requested.");
     is_running = false;
}

// --- Getters ---
PCDisplay* Game::get_display() { return &display; }
AssetManager* Game::getAssetManager() { return &assetManager; }
InputManager* Game::getInputManager() { return &inputManager; }
PlayerData* Game::getPlayerData() { return &playerData_; }
TextRenderer* Game::getTextRenderer() { return textRenderer_.get(); }
AnimationManager* Game::getAnimationManager() { return animationManager_.get(); }

Digimon::DigimonRegistry* Game::getDigimonRegistry() {
    return &Digimon::DigimonRegistry::getInstance();
}

GameState* Game::getCurrentState() {
    return states_.empty() ? nullptr : states_.back().get();
}

GameState* Game::getUnderlyingState(const GameState* currentState) {
    if (states_.size() < 2) { return nullptr; }
    // Find the index of currentState
    size_t currentIndex = states_.size(); // Invalid index initially
    for (size_t i = 0; i < states_.size(); ++i) {
        if (states_[i].get() == currentState) {
            currentIndex = i;
            break;
        }
    }
    // If found and not the bottom-most state, return the one below it
    if (currentIndex > 0 && currentIndex < states_.size()) {
        return states_[currentIndex - 1].get();
    }
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "getUnderlyingState: currentState (%p) not found on stack or is the bottom-most state.", (void*)currentState);
    return nullptr;
}

// Update game settings from configuration
void Game::updateFromConfig() {
    if (textRenderer_) {
        textRenderer_->updateGlobalTextScaleFromConfig();
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Text renderer updated from config.");
    }
    
    // Other configuration updates can be added here in the future
}

// Reload configuration and update settings
bool Game::reloadConfig() {
    bool success = ConfigManager::reload();
    if (success) {
        updateFromConfig();
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Configuration reloaded and settings updated.");
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to reload configuration.");
    }
    return success;
}

// Implementation of the missing loadAllEnemyDigimonAssets function
bool Game::loadAllEnemyDigimonAssets() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Loading all enemy Digimon assets...");
    bool assets_ok = true;
    bool anims_ok = true;
    
    // This directory contains enemy Digimon assets
    const std::string enemyBaseDir = "assets/sprites/enemy_digimon";
    
    // Define all enemy Digimon with their correct file names
    std::vector<std::pair<std::string, std::string>> enemyDigimon = {
        // First batch
        {"kuwagamon", "kuwagamon"}, {"andromon", "andromon"}, {"apocalymon", "apocalymon"},
        {"bakemon", "bakemon"}, {"blossomon", "blossomon"}, {"centaurmon", "centaurmon"},
        {"cherrymon", "cherrymon"}, {"cockatrimon", "cockatrimon"}, {"colosseumgreymon", "colosseumgreymon"},
        {"darktyranomon", "darktyranomon"}, {"demidevimon", "demidevimon"}, {"deramon", "deramon"},
        {"devidramon", "devidramon"}, {"devimon", "devimon"}, {"deviwomon", "deviwomon"},
        
        // Second batch
        {"diablomon", "diablomon"}, {"digitamamon", "digitamamon"}, {"divermon", "divermon"},
        {"dokugumon", "dokugumon"}, {"drimogemon", "drimogemon"}, {"elecmon", "elecmon"},
        {"etemon", "etemon"}, {"evilmon", "evilmon"}, {"floramon", "floramon"},
        {"flymon", "flymon"}, {"frigimon", "frigimon"}, {"garbagemon", "garbagemon"},
        {"gazimon", "gazimon"}, {"gekomon", "gekomon"}, {"gesomon", "gesomon"},
        
        // Third batch
        {"gigadramon", "gigadramon"}, {"gizamon", "gizamon"}, {"gotsumon", "gotsumon"},
        {"hagurumon", "hagurumon"}, {"infermon", "infermon"}, {"keramon", "keramon"},
        {"kiwimon", "kiwimon"}, {"leomon", "leomon"}, {"machinedramon", "machinedramon"},
        {"mammothmon", "mammothmon"}, {"megadramon", "megadramon"}, {"megaseadramon", "megaseadramon"},
        {"mekanorimon", "mekanorimon"}, {"meramon", "meramon"}, {"metaletemon", "metaletemon"},
        
        // Fourth batch
        {"metalseadramon", "metalseadramon"}, {"mojyamon", "mojyamon"}, {"monochromon", "monochromon"},
        {"monzaemon", "monzaemon"}, {"mushmon", "mushmon"}, {"myotismon", "myotismon"},
        {"nanimon", "nanimon"}, {"nanomon", "nanomon"}, {"numemon", "numemon"},
        {"ogremon", "ogremon"}, {"otamamon", "otamamon"}, {"parrotmon", "parrotmon"},
        {"phantomon", "phantomon"}, {"piemon", "piedmon"}, {"piximon", "piximon"}, // Note: piemon -> piedmon file mapping
        
        // Fifth batch
        {"pumpmon", "pumpmon"}, {"puppetmon", "puppetmon"}, {"raremon", "raremon"},
        {"redvegimon", "redvegimon"}, {"sabreleomon", "sabreleomon"}, {"scorpiomon", "scorpiomon"},
        {"seadramon", "seadramon"}, {"shellmon", "shellmon"}, {"shogungekomon", "shogungekomon"},
        {"skullmeramon", "skullmeramon"}, {"snimon", "snimon"}, {"sukamon", "sukamon"},
        {"tankmon", "tankmon"}, {"tuskmon", "tuskmon"}, {"tyranomon", "tyranomon"}, // Note: corrected spelling
        
        // Final batch
        {"unimon", "unimon"}, {"vademon", "vademon"}, {"vegimon", "vegimon"},
        {"venommyotismon", "venommyotismon"}, {"warumonzaemon", "warumonzaemon"}, {"whamon", "whamon"}
    };
    
    // Load all enemy Digimon textures
    for (const auto& pair : enemyDigimon) {
        const std::string& textureId = pair.first;
        const std::string& fileName = pair.second;
        std::string texturePath = enemyBaseDir + "/" + fileName + ".png";
        
        if (!assetManager.loadTexture(textureId, texturePath)) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Failed to load enemy Digimon texture: %s from %s", 
                       textureId.c_str(), texturePath.c_str());
            assets_ok = false;
        }
    }
    
    // Load all enemy Digimon animations
    for (const auto& pair : enemyDigimon) {
        const std::string& animationId = pair.first;
        const std::string& fileName = pair.second;
        std::string animationPath = enemyBaseDir + "/" + fileName + ".json";
        
        if (!animationManager_->loadAnimationDataFromFile(animationPath, animationId)) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Failed to load enemy Digimon animation: %s from %s", 
                       animationId.c_str(), animationPath.c_str());
            anims_ok = false;
        }
    }
    
    // Load unlockable player Digimon (keeping existing functionality)
    assets_ok &= assetManager.loadTexture("veedramon", "assets/sprites/player_digimon/veedramon.png");
    assets_ok &= assetManager.loadTexture("wizardmon", "assets/sprites/player_digimon/wizardmon.png");
    anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/player_digimon/veedramon.json", "veedramon");
    anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/player_digimon/wizardmon.json", "wizardmon");
    
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Enemy Digimon asset loading completed. Textures: %s, Animations: %s", 
               assets_ok ? "successful" : "failed", anims_ok ? "successful" : "failed");
    return assets_ok && anims_ok;
}

// Implementation of the missing processPopUntil function
void Game::processPopUntil() {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ProcessPopUntil: Target type=%d", (int)pop_until_target_type_);

    if (pop_until_target_type_ == StateType::None || states_.empty()) {
        // Nothing to do, no target or no states to pop
        pop_until_target_type_ = StateType::None;
        return;
    }

    // Check if the target state is in the stack
    bool found = false;
    for (auto it = states_.rbegin(); it != states_.rend(); ++it) {
        if ((*it)->getType() == pop_until_target_type_) {
            found = true;
            break;
        }
    }

    if (!found) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ProcessPopUntil: Target state type %d not found in stack", 
                   (int)pop_until_target_type_);
        pop_until_target_type_ = StateType::None;
        return;
    }

    // Pop states until we reach the target
    while (!states_.empty() && states_.back()->getType() != pop_until_target_type_) {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "PopUntil: Popping state %p (type %d)", 
                     (void*)states_.back().get(), (int)states_.back()->getType());
        
        states_.back()->exit();
        states_.pop_back();
    }

    // Call enter() on the target state, which is now at the top of the stack
    if (!states_.empty()) {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ProcessPopUntil: Target state %p (type %d) now at top, calling enter()", 
                     (void*)states_.back().get(), (int)states_.back()->getType());
        states_.back()->enter();
    }

    // Reset the pop_until_target_type_ as we've processed it
    pop_until_target_type_ = StateType::None;
}

