// File: src/core/Game.cpp

#include "core/Game.h"
#include "ui/TextRenderer.h"       // Include the new header
// ... other necessary includes ...
#include "states/AdventureState.h" // For initial state push
#include "core/PlayerData.h"
#include "core/InputManager.h"
#include "platform/pc/pc_display.h"
#include "core/AssetManager.h"     // Needed for AssetManager
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
    textRenderer_(nullptr) // Initialize new pointer member
{
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Game constructor called. PlayerData implicitly constructed.");
}

Game::~Game() {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Game destructor called.");
    // Cleanup happens in close()
    // playerData_ is automatically destructed when Game object goes out of scope
    // textRenderer_ unique_ptr is also automatically handled if close() wasn't called,
    // but explicit reset in close() is good practice for order.
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

    // <<< --- Initialize Text Renderer --- >>>
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
    // <<< --- End Text Renderer Init --- >>>

    try {
        states_.push_back(std::make_unique<AdventureState>(this)); // Pass 'this' to constructor
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Initial AdventureState created and added.");
    } catch (const std::exception& e) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create initial state: %s", e.what()); assetManager.shutdown(); display.close(); SDL_Quit(); return false; }

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
    last_frame_time = SDL_GetTicks(); // Ensure timer starts correctly

    while (is_running) {
        // --- Calculate Delta Time ---
        Uint32 current_time = SDL_GetTicks();
        float delta_time = (current_time - last_frame_time) / 1000.0f;
        if (delta_time > 0.1f) delta_time = 0.1f; // Clamp delta time
        last_frame_time = current_time;

        // --- Input Processing ---
        inputManager.prepareNewFrame();
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit_game();
            }
            inputManager.processEvent(event);
        }

        // --- State Logic ---
        SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "Game::run - BEFORE applyStateChanges (Top State Ptr: %p)", (void*)(states_.empty() ? nullptr : states_.back().get()));
        applyStateChanges(); // Apply Pop/Push requested from previous frame
        SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "Game::run - AFTER applyStateChanges (Top State Ptr: %p)", (void*)(states_.empty() ? nullptr : states_.back().get()));

        if (!states_.empty()) {
            GameState* currentStatePtr = states_.back().get();
            if (currentStatePtr) {
                PlayerData* pd = getPlayerData();

                SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "Game::run - Calling handle_input on state %p", (void*)currentStatePtr);
                currentStatePtr->handle_input(inputManager, pd);

                SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "Game::run - Calling update on state %p", (void*)currentStatePtr);
                currentStatePtr->update(delta_time, pd);

            } else {
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "RunLoop Update: Top state pointer is NULL despite non-empty stack!");
                is_running = false;
            }
        } else {
             SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "RunLoop Update: State stack empty after applyStateChanges.");
             is_running = false;
        }

        // --- Rendering ---
        if (is_running && !states_.empty()) {
             GameState* currentStateForRender = getCurrentState();
             if (currentStateForRender) {
                 display.clear(0x0000);
                 // SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "Game::run - Calling render on state %p", (void*)currentStateForRender); // Optional: Can be very spammy
                 currentStateForRender->render(display);
                 display.present();
             }
        }

    } // End while(is_running)

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Exited main game loop.");
    close(); // Perform cleanup after loop ends
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
    if (request_push_) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Overwriting previous push request."); }
    request_push_ = std::move(state);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Push requested for state %p.", (void*)request_push_.get());
}

void Game::requestPopState() {
    if(request_pop_) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Multiple pop requests made before applyStateChanges.");
    request_pop_ = true;
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Pop requested.");
}

// --- Helper to apply queued changes ---
void Game::applyStateChanges() {
    if (request_pop_) {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyStateChanges: Applying Pop request.");
        pop_state();
        request_pop_ = false;
    }
    if (request_push_) {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "ApplyStateChanges: Applying Push request for state %p.", (void*)request_push_.get());
        push_state(std::move(request_push_));
        request_push_.reset(); // Use reset() to clear the unique_ptr after moving
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

// <<< --- ADDED TextRenderer Getter Implementation --- >>>
TextRenderer* Game::getTextRenderer() {
    return textRenderer_.get(); // .get() returns raw ptr from unique_ptr
}
// <<< -------------------------------------------- >>>


// --- close ---
void Game::close() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Shutting down Game systems...");
    states_.clear(); // Destroys GameState objects managed by unique_ptr
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "State stack cleared.");

    // Reset unique_ptr for TextRenderer before AssetManager (which owns the texture)
    textRenderer_.reset(); // <<<--- ADDED Reset for TextRenderer ---<<<
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer reset.");

    assetManager.shutdown();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetManager shutdown.");
    display.close();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay closed.");
    SDL_Quit();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL quit.");
    // Note: playerData_ is automatically destructed here
}

// --- DEBUG_getStack Implementation ---
std::vector<std::unique_ptr<GameState>>& Game::DEBUG_getStack() { return states_; }