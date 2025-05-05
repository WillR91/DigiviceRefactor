// File: src/states/AdventureState.cpp

#include "states/AdventureState.h"  // Include own header
#include "core/Game.h"              // To access Game methods/members
#include "core/AssetManager.h"      // To get assets
#include "platform/pc/pc_display.h" // To draw
#include "graphics/Animation.h"     // Uses Animation/SpriteFrame
#include "states/MenuState.h"       // Needed for creating MenuState instance directly
#include "core/InputManager.h"      // For InputManager& parameter
#include "core/GameAction.h"        // For GameAction enum
#include "core/PlayerData.h"        // For PlayerData* parameter
#include <SDL_log.h>                // SDL logging
#include <stdexcept>                // For exceptions
#include <fstream>                  // For reading files
#include "vendor/nlohmann/json.hpp" // Path to JSON library header
#include <cstddef>                  // For size_t
#include <vector>
#include <string>
#include <map>
#include <cmath>                    // For std::fmod
#include <memory>                   // For std::make_unique

// Use the nlohmann::json namespace
using json = nlohmann::json;


// --- Anonymous Namespace for Helpers and Constants ---
namespace {

// --- Helper Function to Create Animation from Indices ---
// (Remains unchanged)
Animation createAnimationFromIndices(
    SDL_Texture* texture,
    const std::vector<SDL_Rect>& allFrameRects,
    const std::vector<int>& indices,
    const std::vector<Uint32>& durations,
    bool loops)
{
    Animation anim;
    anim.loops = loops;
    if (!texture) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot create animation: Null texture provided."); return anim; }
    if (indices.size() != durations.size()) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Animation creation error: Indices count (%zu) does not match durations count (%zu).", indices.size(), durations.size()); return anim; }
    if (allFrameRects.empty()) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot create animation: Provided frame rectangle list is empty."); return anim; }

    for (size_t i = 0; i < indices.size(); ++i) {
        int frameIndex = indices[i];
        Uint32 duration = durations[i];
        if (frameIndex >= 0 && static_cast<size_t>(frameIndex) < allFrameRects.size()) {
            anim.addFrame(SpriteFrame(texture, allFrameRects[frameIndex]), duration);
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Anim Creation: Index %d out of bounds (%zu). Using frame 0.", frameIndex, allFrameRects.size());
            anim.addFrame(SpriteFrame(texture, allFrameRects[0]), duration); // Placeholder
        }
    }
    return anim;
}

// --- Animation Sequence Templates ---
// (Remain unchanged)
const std::vector<int> IDLE_INDICES = {0, 1};
const std::vector<Uint32> IDLE_DURATIONS = {1000, 1000};
const std::vector<int> WALK_INDICES = {2, 3, 2, 3};
const std::vector<Uint32> WALK_DURATIONS = {300, 300, 300, 300};
const std::vector<int> ATTACK_INDICES = {1, 0, 3, 8};
const std::vector<Uint32> ATTACK_DURATIONS = {200, 150, 150, 400};

} // end anonymous namespace


// --- Constructor ---
AdventureState::AdventureState(Game* game) :
    GameState(game), // <<< --- ADDED Base class constructor call --- >>>
    bgTexture0_(nullptr),
    bgTexture1_(nullptr),
    bgTexture2_(nullptr),
    active_anim_(nullptr),
    current_digimon_(DIGI_AGUMON), // <<<--- TODO: Read initial partner from PlayerData?
    current_state_(STATE_IDLE),
    current_anim_frame_idx_(0),
    current_frame_elapsed_time_(0.0f),
    queued_steps_(0)
{
    // game_ptr is initialized by GameState(game) call above
    // this->game_ptr = game; // No longer needed

    if (!game_ptr || !game_ptr->getAssetManager() || !game_ptr->get_display()) {
        throw std::runtime_error("AdventureState requires valid Game pointer with initialized systems!");
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState Constructor: Initializing...");

    AssetManager* assets = game_ptr->getAssetManager();
    bgTexture0_ = assets->getTexture("castle_bg_0");
    bgTexture1_ = assets->getTexture("castle_bg_1");
    bgTexture2_ = assets->getTexture("castle_bg_2");
    if (!bgTexture0_ || !bgTexture1_ || !bgTexture2_) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"AdventureState: Background texture(s) missing!"); }

    initializeAnimations();

    // TODO: Consider getting initial Digimon from PlayerData instead of default
    // PlayerData* pd = game_ptr->getPlayerData();
    // if (pd) { current_digimon_ = pd->currentPartner; }

    setActiveAnimation(); // Call after current_digimon_ is potentially set from PlayerData

    if (!active_anim_) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"CONSTRUCTOR FAIL: Failed to set initial active animation! active_anim_ is NULL.");
    } else {
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,"CONSTRUCTOR OK: Initial active_anim_ set in constructor: %p", active_anim_);
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState Initialized Successfully.");
}


// --- Destructor ---
// (Remains unchanged)
AdventureState::~AdventureState() { SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState Destructor called."); }


// --- Initialize Animations ---
// (Remains unchanged)
void AdventureState::initializeAnimations() {
    AssetManager* assets = game_ptr->getAssetManager();
    if (!assets) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot init anims: AssetManager null"); return; }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Initializing animations using JSON data...");
    std::map<DigimonType, std::pair<std::string, std::string>> digimonAssets = {
        {DIGI_AGUMON, {"agumon_sheet", "assets/sprites/agumon_sheet.json"}},
        {DIGI_GABUMON, {"gabumon_sheet", "assets/sprites/gabumon_sheet.json"}},
        {DIGI_BIYOMON, {"biyomon_sheet", "assets/sprites/biyomon_sheet.json"}},
        {DIGI_GATOMON, {"gatomon_sheet", "assets/sprites/gatomon_sheet.json"}},
        {DIGI_GOMAMON, {"gomamon_sheet", "assets/sprites/gomamon_sheet.json"}},
        {DIGI_PALMON, {"palmon_sheet", "assets/sprites/palmon_sheet.json"}},
        {DIGI_TENTOMON, {"tentomon_sheet", "assets/sprites/tentomon_sheet.json"}},
        {DIGI_PATAMON, {"patamon_sheet", "assets/sprites/patamon_sheet.json"}}
    };

    for (const auto& pair : digimonAssets) {
        DigimonType type = pair.first;
        const std::string& textureId = pair.second.first;
        const std::string& jsonPath = pair.second.second;
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Processing animations for %s...", textureId.c_str());
        SDL_Texture* texture = assets->getTexture(textureId);
        if (!texture) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Tex '%s' not found for type %d.", textureId.c_str(), type); continue; }
        std::vector<SDL_Rect> frameRects;
        try {
            std::ifstream jsonFile(jsonPath);
             if (!jsonFile.is_open()) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open JSON: %s", jsonPath.c_str()); continue; }
             json data = json::parse(jsonFile);
             jsonFile.close();
             if (data.contains("frames")) {
                 const auto& framesNode = data["frames"];
                 if (framesNode.is_array()) {
                      for (const auto& frameData : framesNode) {
                          if (frameData.contains("frame")) {
                             const auto& rectData = frameData["frame"];
                             if (rectData.contains("x") && rectData.contains("y") && rectData.contains("w") && rectData.contains("h")) {
                                frameRects.push_back({ rectData["x"].get<int>(), rectData["y"].get<int>(), rectData["w"].get<int>(), rectData["h"].get<int>() });
                             } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "JSON frame missing x,y,w, or h in %s (array item)", jsonPath.c_str()); }
                          } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "JSON frame missing 'frame' object in %s (array item)", jsonPath.c_str()); }
                      }
                 } else if (framesNode.is_object()) {
                      for (auto& [key, frameData] : framesNode.items()) {
                           if (frameData.contains("frame")) {
                                const auto& rectData = frameData["frame"];
                                if (rectData.contains("x") && rectData.contains("y") && rectData.contains("w") && rectData.contains("h")) {
                                    frameRects.push_back({ rectData["x"].get<int>(), rectData["y"].get<int>(), rectData["w"].get<int>(), rectData["h"].get<int>() });
                                } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "JSON frame missing x,y,w, or h in %s (object key: %s)", jsonPath.c_str(), key.c_str()); }
                           } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "JSON frame missing 'frame' object in %s (object key: %s)", jsonPath.c_str(), key.c_str()); }
                      }
                 } else { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "'frames' not array/object in %s", jsonPath.c_str()); continue; }
             } else { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Missing 'frames' in %s", jsonPath.c_str()); continue; }
        } catch (json::parse_error& e) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to parse JSON file '%s': %s (at byte %zu)", jsonPath.c_str(), e.what(), e.byte);
            continue;
        } catch (std::exception& e) {
             SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error reading/processing JSON file '%s': %s", jsonPath.c_str(), e.what());
             continue;
        }
        if (frameRects.empty()) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "No frames loaded from %s.", jsonPath.c_str()); continue; }
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Loaded %zu frame rectangles for %s.", frameRects.size(), textureId.c_str());
        idleAnimations_[type] = createAnimationFromIndices(texture, frameRects, IDLE_INDICES, IDLE_DURATIONS, true);
        walkAnimations_[type] = createAnimationFromIndices(texture, frameRects, WALK_INDICES, WALK_DURATIONS, false);
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Created animations for type %d.", type);
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Finished initializing animations from JSON.");
}


// --- Set Active Animation ---
// (Remains unchanged)
void AdventureState::setActiveAnimation() {
     Animation* previous_anim = active_anim_;
     if (current_state_ == STATE_IDLE) {
         auto it = idleAnimations_.find(current_digimon_);
         active_anim_ = (it != idleAnimations_.end()) ? &(it->second) : nullptr;
     } else { // STATE_WALKING
         auto it = walkAnimations_.find(current_digimon_);
         active_anim_ = (it != walkAnimations_.end()) ? &(it->second) : nullptr;
     }
     if (active_anim_ != previous_anim || (active_anim_ && current_anim_frame_idx_ != 0)) {
        current_anim_frame_idx_ = 0;
        current_frame_elapsed_time_ = 0.0f;
     }
     if (!active_anim_) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "setActiveAnimation: Could not find animation for state %d, digi %d", current_state_, current_digimon_); }
}


// <<< --- MODIFIED handle_input Signature --- >>>
void AdventureState::handle_input(InputManager& inputManager, PlayerData* playerData) {
    // Only handle input if this state is the current active one
    if (!game_ptr || game_ptr->getCurrentState() != this) {
        return; // Should not happen if called correctly by Game loop
    }

    // InputManager is now passed in, no need to get it from game_ptr
    // InputManager* inputManagerPtr = game_ptr->getInputManager(); // No longer needed
    // if (!inputManagerPtr) { /* ... */ } // No longer needed

    // We receive playerData, but don't use it in this specific input logic yet
    // (We will use it later for checking conditions, inventory etc.)
    if (!playerData) {
         SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AdventureState::handle_input - PlayerData pointer is null!");
         // Decide how to handle this - maybe return, maybe proceed cautiously?
         // For now, let's proceed as input doesn't depend on it yet.
    }


    bool stateOrDigiChanged = false; // Flag to check if we need to update animation

    // --- Menu Activation ---
    // Using CONFIRM action
    if (inputManager.isActionJustPressed(GameAction::CONFIRM)) {
         SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Confirm action in AdventureState, pushing MenuState.");
         std::vector<std::string> mainMenuItems = {"DIGIMON", "MAP", "ITEMS", "SAVE", "EXIT"};
         // <<< TODO: Need to create PartnerSelectState from DIGIMON option later >>>
         game_ptr->requestPushState(std::make_unique<MenuState>(game_ptr, mainMenuItems));
         return; // Exit input handling immediately after pushing menu
    }

    // --- Step Input ---
    // Using STEP action
    if (inputManager.isActionJustPressed(GameAction::STEP)) {
        if (queued_steps_ < MAX_QUEUED_STEPS) {
            queued_steps_++;
            SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "Step action detected. Queued: %d", queued_steps_);

            // <<< --- TODO: Update PlayerData with step --- >>>
            // if (playerData) {
            //     playerData->stepsTakenThisChapter++;
            //     playerData->totalSteps++;
            //     // Maybe add D-Power logic here or in update based on steps
            // }

        } else {
             SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "Step action detected, but queue is full (%d).", queued_steps_);
        }
    }

    // --- Switch Digimon (Temporary Debug/Testing Input) ---
    DigimonType previousDigimon = current_digimon_;
    if (inputManager.isActionJustPressed(GameAction::SELECT_DIGI_1)) {
        current_digimon_ = DIGI_AGUMON;
    } else if (inputManager.isActionJustPressed(GameAction::SELECT_DIGI_2)) {
        current_digimon_ = DIGI_GABUMON;
    } else if (inputManager.isActionJustPressed(GameAction::SELECT_DIGI_3)) {
        current_digimon_ = DIGI_BIYOMON;
    } else if (inputManager.isActionJustPressed(GameAction::SELECT_DIGI_4)) {
        current_digimon_ = DIGI_GATOMON;
    } else if (inputManager.isActionJustPressed(GameAction::SELECT_DIGI_5)) {
        current_digimon_ = DIGI_GOMAMON;
    } else if (inputManager.isActionJustPressed(GameAction::SELECT_DIGI_6)) {
        current_digimon_ = DIGI_PALMON;
    } else if (inputManager.isActionJustPressed(GameAction::SELECT_DIGI_7)) {
        current_digimon_ = DIGI_TENTOMON;
    } else if (inputManager.isActionJustPressed(GameAction::SELECT_DIGI_8)) {
        current_digimon_ = DIGI_PATAMON;
    }

    // If Digimon was changed by the above keys
    if (current_digimon_ != previousDigimon) {
         stateOrDigiChanged = true;
        // <<< --- TODO: Update PlayerData with new partner --- >>>
        // if (playerData) {
        //      playerData->currentPartner = current_digimon_;
        // }
    }


    // If Digimon was changed, reset state and clear steps
    if (stateOrDigiChanged) {
        current_state_ = STATE_IDLE; // Reset to idle animation
        queued_steps_ = 0;           // Clear any pending steps
        SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Switched character to %d via debug key.", current_digimon_);
        setActiveAnimation(); // Update the displayed animation immediately
    }
}
// <<< --- END MODIFIED handle_input --- >>>


// <<< --- MODIFIED update Signature --- >>>
void AdventureState::update(float delta_time, PlayerData* playerData) {

    // We receive playerData, but don't use it *yet* in this update logic
    // (Will be used for checking steps for encounters, goals etc.)
    if (!playerData) {
         SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AdventureState::update - PlayerData pointer is null!");
    }
    // <<< --- TODO: Read current partner from PlayerData --- >>>
    // This ensures changes from PartnerSelectState are reflected visually
    // if (playerData && current_digimon_ != playerData->currentPartner) {
    //     current_digimon_ = playerData->currentPartner;
    //     current_state_ = STATE_IDLE; // Reset animation state on partner change
    //     queued_steps_ = 0;
    //     setActiveAnimation();
    //     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState::update detected partner change from PlayerData to %d.", current_digimon_);
    // }


    // --- Normal Adventure Update Logic ---
    bool stateNeedsAnimUpdate = false;
    // Scroll Background
    if (current_state_ == STATE_WALKING) {
        float scrollAmount0 = SCROLL_SPEED_0 * delta_time; float scrollAmount1 = SCROLL_SPEED_1 * delta_time; float scrollAmount2 = SCROLL_SPEED_2 * delta_time;
        int effW0=0, effW1=0, effW2=0;
        if(bgTexture0_) {int w; SDL_QueryTexture(bgTexture0_,0,0,&w,0); effW0=w*2/3; if(effW0<=0)effW0=w;}
        if(bgTexture1_) {int w; SDL_QueryTexture(bgTexture1_,0,0,&w,0); effW1=w*2/3; if(effW1<=0)effW1=w;}
        if(bgTexture2_) {int w; SDL_QueryTexture(bgTexture2_,0,0,&w,0); effW2=w*2/3; if(effW2<=0)effW2=w;}
        if(effW0 > 0) { bg_scroll_offset_0_ -= scrollAmount0; bg_scroll_offset_0_ = std::fmod(bg_scroll_offset_0_ + effW0, (float)effW0); }
        if(effW1 > 0) { bg_scroll_offset_1_ -= scrollAmount1; bg_scroll_offset_1_ = std::fmod(bg_scroll_offset_1_ + effW1, (float)effW1); }
        if(effW2 > 0) { bg_scroll_offset_2_ -= scrollAmount2; bg_scroll_offset_2_ = std::fmod(bg_scroll_offset_2_ + effW2, (float)effW2); }
     }
    // State Change: Idle -> Walking
    if (current_state_ == STATE_IDLE && queued_steps_ > 0) { current_state_ = STATE_WALKING; stateNeedsAnimUpdate = true; /* SDL_LogDebug(...) */ }
    // Advance Animation Frame
    bool animation_cycle_finished = false;
    if (active_anim_ && active_anim_->getFrameCount() > 0) {
         size_t frameCount = active_anim_->getFrameCount();
         if (current_anim_frame_idx_ >= frameCount) current_anim_frame_idx_ = 0;
         if (current_anim_frame_idx_ < active_anim_->frame_durations_ms.size()) {
            float duration_sec = active_anim_->frame_durations_ms[current_anim_frame_idx_] / 1000.0f;
            current_frame_elapsed_time_ += delta_time;
            while (current_frame_elapsed_time_ >= duration_sec && frameCount > 0) {
                current_frame_elapsed_time_ -= duration_sec; current_anim_frame_idx_++;
                if (current_anim_frame_idx_ >= frameCount) {
                    animation_cycle_finished = true;
                    current_anim_frame_idx_ = active_anim_->loops ? 0 : frameCount - 1;
                    if (!active_anim_->loops) { current_frame_elapsed_time_ = 0.0f; break; }
                }
                 if (current_anim_frame_idx_ < active_anim_->frame_durations_ms.size()) {
                    duration_sec = active_anim_->frame_durations_ms[current_anim_frame_idx_] / 1000.0f;
                 } else { current_anim_frame_idx_ = 0; current_frame_elapsed_time_ = 0.0f; break; }
                 if (duration_sec <= 0.0f) {
                      current_anim_frame_idx_++;
                       if (current_anim_frame_idx_ >= frameCount) {
                            animation_cycle_finished = true; current_anim_frame_idx_ = active_anim_->loops ? 0 : frameCount - 1;
                            if (!active_anim_->loops) current_frame_elapsed_time_ = 0.0f;
                       }
                       if (!active_anim_->loops && animation_cycle_finished) break;
                       if(current_anim_frame_idx_ == 0 && active_anim_->loops) break;
                 }
            }
         } else { current_anim_frame_idx_ = 0; }
    }
    // State Change: Walking -> Idle
    if (current_state_ == STATE_WALKING && animation_cycle_finished && active_anim_ && !active_anim_->loops) {
         queued_steps_--; /* SDL_LogDebug */

         // <<<--- TODO: Check step goals / encounters using playerData --- >>>
         // if (playerData) {
         //    if (playerData->stepsTakenThisChapter >= GOAL_FOR_CHAPTER) {
         //        // Trigger end-of-chapter event/state change
         //    } else {
         //        // Check for random encounter based on playerData->totalSteps or chapter steps
         //    }
         // }

         if (queued_steps_ > 0) { current_anim_frame_idx_ = 0; current_frame_elapsed_time_ = 0.0f; /* Start next walk cycle */ }
         else { current_state_ = STATE_IDLE; stateNeedsAnimUpdate = true; /* Return to idle */ }
    }
    if (stateNeedsAnimUpdate) setActiveAnimation();
    // --- END Normal Logic ---
}
// <<< --- END MODIFIED update --- >>>


// <<< --- MODIFIED render Signature --- >>>
void AdventureState::render(PCDisplay& display) {
    // Display is now passed in by reference
    // PCDisplay* displayPtr = game_ptr->get_display(); // No longer needed
    // if (!displayPtr) { /* ... */ } // No longer needed

    // --- Get Window Dimensions ---
    // TODO: Get dimensions from the passed-in display object if needed,
    // or continue using constants for now if the display wrapper doesn't expose size easily.
    // int windowW = 0, windowH = 0;
    // display.getWindowSize(windowW, windowH); // Example if method exists
    const int windowW = WINDOW_WIDTH;  // Using constants for now
    const int windowH = WINDOW_HEIGHT; // Using constants for now

    // --- Draw Backgrounds ---
    // This lambda captures 'display' by reference automatically
    auto drawTiledBg = [&](SDL_Texture* tex, float offset, int texW, int texH, int effectiveWidth, const char* layerName) {
        if (!tex || texW <= 0 || effectiveWidth <= 0) { return; }
        int drawX1 = -static_cast<int>(std::fmod(offset, (float)effectiveWidth));
        if (drawX1 > 0) drawX1 -= effectiveWidth;
        SDL_Rect dst1 = { drawX1, 0, texW, texH };
        display.drawTexture(tex, NULL, &dst1); // Use display reference directly
        int drawX2 = drawX1 + effectiveWidth; SDL_Rect dst2 = { drawX2, 0, texW, texH };
        display.drawTexture(tex, NULL, &dst2); // Use display reference directly
        if (drawX2 + texW < windowW) { int drawX3 = drawX2 + effectiveWidth; SDL_Rect dst3 = { drawX3, 0, texW, texH };
            display.drawTexture(tex, NULL, &dst3); // Use display reference directly
        }
    };
    int bgW0=0,bgH0=0,effW0=0, bgW1=0,bgH1=0,effW1=0, bgW2=0,bgH2=0,effW2=0;
    if(bgTexture0_) { SDL_QueryTexture(bgTexture0_,0,0,&bgW0,&bgH0); effW0=bgW0*2/3; if(effW0<=0)effW0=bgW0;}
    if(bgTexture1_) { SDL_QueryTexture(bgTexture1_,0,0,&bgW1,&bgH1); effW1=bgW1*2/3; if(effW1<=0)effW1=bgW1;}
    if(bgTexture2_) { SDL_QueryTexture(bgTexture2_,0,0,&bgW2,&bgH2); effW2=bgW2*2/3; if(effW2<=0)effW2=bgW2;}

    drawTiledBg(bgTexture2_, bg_scroll_offset_2_, bgW2, bgH2, effW2, "Layer 2");
    drawTiledBg(bgTexture1_, bg_scroll_offset_1_, bgW1, bgH1, effW1, "Layer 1");


    // --- Draw Character ---
    if (active_anim_) {
        const SpriteFrame* currentFrame = active_anim_->getFrame(current_anim_frame_idx_);
        if (currentFrame && currentFrame->texturePtr && currentFrame->sourceRect.w > 0 && currentFrame->sourceRect.h > 0) {
            int drawX = (windowW / 2) - (currentFrame->sourceRect.w / 2); // Keep centered horizontally
            int verticalOffset = 7; // Pixels to shift upwards
            int drawY = (windowH / 2) - (currentFrame->sourceRect.h / 2) - verticalOffset; // Subtract offset
            SDL_Rect dstRect = { drawX, drawY, currentFrame->sourceRect.w, currentFrame->sourceRect.h };

            display.drawTexture(currentFrame->texturePtr, &currentFrame->sourceRect, &dstRect); // Use display reference directly

        } else { /* Log */ }
    } else { SDL_LogWarn(SDL_LOG_CATEGORY_RENDER,"AS Render: active_anim_ is null!"); }
    // --- End Character ---


    // --- Draw Foreground ---
    drawTiledBg(bgTexture0_, bg_scroll_offset_0_, bgW0, bgH0, effW0, "Layer 0");
    // --- End Foreground ---

} // End of AdventureState::render() function
// <<< --- END MODIFIED render --- >>>