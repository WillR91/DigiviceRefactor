// File: src/core/Game.cpp

#include "core/Game.h"
#include "states/GameState.h"        // Include base class for enter/exit
#include "states/AdventureState.h"
#include "core/PlayerData.h"
#include "core/InputManager.h"
#include "platform/pc/pc_display.h"
#include "core/AssetManager.h"
#include "ui/TextRenderer.h"
#include "core/AnimationManager.h"
#include <SDL_log.h>
#include <stdexcept>
#include <filesystem> // For CWD logging

// Include standard library headers needed by this file
#include <vector>
#include <memory>
#include <string>

Game::Game() :
    is_running(false),
    last_frame_time(0),
    request_pop_(false),
    request_push_(nullptr),
    pop_until_target_type_(StateType::None),
    textRenderer_(nullptr),
    animationManager_(nullptr)
{
    // Single init log instead of multiple debug logs
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Game initializing...");
}

Game::~Game() {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Game destructor called.");
    // Cleanup happens in close()
}

bool Game::init(const std::string& title, int width, int height) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Initializing Game systems...");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL Init Error: %s", SDL_GetError()); return false; }
    if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0")) { SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "Hint Warning: %s", SDL_GetError()); }
    else { SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "SDL_HINT_RENDER_SCALE_QUALITY set to 0 (Nearest Neighbor)."); }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL Initialized.");

    if (!display.init(title.c_str(), width, height)) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay Init Error"); SDL_Quit(); return false; }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay Initialized.");

    if (!assetManager.init(display.getRenderer())) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AssetManager Init Error"); display.close(); SDL_Quit(); return false; }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetManager Initialized.");

    // InputManager is initialized by its own constructor when Game is created
    // PlayerData is initialized by its own constructor when Game is created

    try {
        std::filesystem::path cwd = std::filesystem::current_path();
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Current Working Directory: %s", cwd.string().c_str());
    } catch (const std::filesystem::filesystem_error& e) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error getting CWD using std::filesystem: %s", e.what()); }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Attempting to load initial assets...");
     bool assets_ok = true;
     assets_ok &= assetManager.loadTexture("agumon_sheet", "assets\\sprites\\agumon_sheet.png");
     assets_ok &= assetManager.loadTexture("gabumon_sheet", "assets\\sprites\\gabumon_sheet.png");
     assets_ok &= assetManager.loadTexture("biyomon_sheet", "assets\\sprites\\biyomon_sheet.png");
     assets_ok &= assetManager.loadTexture("gatomon_sheet", "assets\\sprites\\gatomon_sheet.png");
     assets_ok &= assetManager.loadTexture("gomamon_sheet", "assets\\sprites\\gomamon_sheet.png");
     assets_ok &= assetManager.loadTexture("palmon_sheet", "assets\\sprites\\palmon_sheet.png");
     assets_ok &= assetManager.loadTexture("tentomon_sheet", "assets\\sprites\\tentomon_sheet.png");
     assets_ok &= assetManager.loadTexture("patamon_sheet", "assets\\sprites\\patamon_sheet.png");
     assets_ok &= assetManager.loadTexture("castle_bg_0", "assets\\backgrounds\\castlebackground0.png");
     assets_ok &= assetManager.loadTexture("castle_bg_1", "assets\\backgrounds\\castlebackground1.png");
     assets_ok &= assetManager.loadTexture("castle_bg_2", "assets\\backgrounds\\castlebackground2.png");
     assets_ok &= assetManager.loadTexture("menu_bg_blue", "assets\\ui\\backgrounds\\menu_base_blue.png");
     assets_ok &= assetManager.loadTexture("transition_borders", "assets\\ui\\transition\\transition_borders.png");
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Loading UI font...");
     assets_ok &= assetManager.loadTexture("ui_font_atlas", "assets/ui/fonts/bluewhitefont.png"); // Use the specified font path
     if (!assets_ok) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "One or more essential assets failed to load! Check paths and file existence."); assetManager.shutdown(); display.close(); SDL_Quit(); return false; }
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Finished loading initial assets attempt.");

    // Initialize Text Renderer
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Initializing TextRenderer...");
    SDL_Texture* fontTexturePtr = assetManager.getTexture("ui_font_atlas");
    if (fontTexturePtr) {
        textRenderer_ = std::make_unique<TextRenderer>(fontTexturePtr);
        const std::string fontJsonPath = "assets/ui/fonts/bluewhitefont.json";
        if (!textRenderer_->loadFontData(fontJsonPath)) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load font data for TextRenderer from '%s'. Text rendering may fail.", fontJsonPath.c_str());
            textRenderer_.reset(); // Invalidate renderer if font data fails
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
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/agumon_sheet.json", "agumon_sheet");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/gabumon_sheet.json", "gabumon_sheet");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/biyomon_sheet.json", "biyomon_sheet");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/gatomon_sheet.json", "gatomon_sheet");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/gomamon_sheet.json", "gomamon_sheet");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/palmon_sheet.json", "palmon_sheet");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/tentomon_sheet.json", "tentomon_sheet");
        anims_ok &= animationManager_->loadAnimationDataFromFile("assets/sprites/patamon_sheet.json", "patamon_sheet");

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
        return false;
    }

    // Push Initial State and Call Enter
    try {
        states_.push_back(std::make_unique<AdventureState>(this));
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Initial AdventureState created and added.");
        // <<< Call enter() on the initial state >>>
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
        if (delta_time > 0.1f) delta_time = 0.1f;
        last_frame_time = current_time;

        // Input Processing
        inputManager.prepareNewFrame();
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit_game();
            }
            inputManager.processEvent(event);
        }

        // State Logic
        applyStateChanges();

        if (!states_.empty()) {
            GameState* currentStatePtr = states_.back().get();
            if (currentStatePtr) {
                PlayerData* pd = getPlayerData();
                currentStatePtr->handle_input(inputManager, pd);

                if (is_running && !states_.empty() && states_.back().get() == currentStatePtr) {
                    currentStatePtr->update(delta_time, pd);
                }
            } else {
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "RunLoop Update: Top state pointer is NULL despite non-empty stack!");
                is_running = false;
            }
        } else if (is_running) {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "RunLoop Update: State stack empty after applyStateChanges. Quitting.");
            is_running = false;
        }

        // Rendering
        if (is_running && !states_.empty()) {
            GameState* currentStateForRender = getCurrentState();
            if (currentStateForRender) {
                display.clear(0x0000);
                currentStateForRender->render(display);
                display.present();
            }
        }
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Exited main game loop.");
    close();
}

// --- State Management - Actual Push/Pop ---
void Game::push_state(std::unique_ptr<GameState> new_state) {
    if (!new_state) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "push_state called with null state!"); return; }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Executing push_state for state %p...", (void*)new_state.get());
    states_.push_back(std::move(new_state));
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "push_state complete. New stack size: %zu", states_.size());
}

void Game::pop_state() {
    if (!states_.empty()) {
         GameState* stateToPop = states_.back().get();
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Executing pop_state for state %p...", (void*)stateToPop);
         states_.pop_back();
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "pop_state complete. New stack size: %zu", states_.size());
    } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "pop_state called on empty stack!"); }
}

// --- State Management - Requests ---
void Game::requestPushState(std::unique_ptr<GameState> state) {
    if (!state) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Attempted to request push of NULL state!"); return; }
    if (request_push_) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Overwriting previous push request for state %p with new request for %p.", (void*)request_push_.get(), (void*)state.get()); }
    request_push_ = std::move(state);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Push requested for state %p.", (void*)request_push_.get());
}

void Game::requestPopState() {
    if(request_pop_) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Multiple pop requests made before applyStateChanges.");
    request_pop_ = true; // Flag for single pop
    pop_until_target_type_ = StateType::None; // Ensure pop_until is not active if single pop is called
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Single Pop requested.");
}

void Game::requestPopUntil(StateType targetType) {
    if (targetType == StateType::None) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "requestPopUntil called with StateType::None. Performing single pop instead.");
        requestPopState(); // Fallback to single pop if target is None
        return;
    }
    pop_until_target_type_ = targetType;
    request_pop_ = false; // Ensure single pop is not active
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "PopUntil requested for type: %d", static_cast<int>(targetType));
}

// --- Helper to apply queued changes ---
void Game::applyStateChanges() {
    if (pop_until_target_type_ != StateType::None) {
        while (!states_.empty()) {
            GameState* topState = states_.back().get();
            if (topState->getType() == pop_until_target_type_) {
                topState->enter();
                break;
            }
            topState->exit();
            states_.pop_back();
        }
        pop_until_target_type_ = StateType::None;
        request_pop_ = false;
        request_push_.reset();
        return;
    }

    if (request_pop_ && !states_.empty()) {
        states_.back()->exit();
        pop_state();
        if (!states_.empty()) states_.back()->enter();
        request_pop_ = false;
    }

    if (request_push_) {
        if (!states_.empty()) states_.back()->exit();
        push_state(std::move(request_push_));
        if(!states_.empty()) states_.back()->enter();
        request_push_.reset();
    }
}

// --- getCurrentState ---
GameState* Game::getCurrentState() {
    return states_.empty() ? nullptr : states_.back().get();
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

// --- close ---
void Game::close() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Shutting down Game systems...");

    // <<< Call exit() on the final state before clearing >>>
    if (!states_.empty()) {
         states_.back()->exit();
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Called exit() on final state during shutdown.");
    }
    states_.clear();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "State stack cleared.");

    animationManager_.reset();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AnimationManager reset.");
    textRenderer_.reset();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer reset.");

    assetManager.shutdown();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetManager shutdown.");
    display.close();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay closed.");
    SDL_Quit();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL quit.");
}

// --- DEBUG_getStack Implementation ---
std::vector<std::unique_ptr<GameState>>& Game::DEBUG_getStack() { return states_; }