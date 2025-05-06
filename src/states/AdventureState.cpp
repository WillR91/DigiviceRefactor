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
// (Unchanged from your version)
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
// (Unchanged from your version)
const std::vector<int> IDLE_INDICES = {0, 1};
const std::vector<Uint32> IDLE_DURATIONS = {1000, 1000};
const std::vector<int> WALK_INDICES = {2, 3, 2, 3};
const std::vector<Uint32> WALK_DURATIONS = {300, 300, 300, 300};
const std::vector<int> ATTACK_INDICES = {1, 0, 3, 8};
const std::vector<Uint32> ATTACK_DURATIONS = {200, 150, 150, 400};

} // end anonymous namespace


// --- Constructor ---
// (Unchanged from your version)
AdventureState::AdventureState(Game* game) :
    GameState(game), // Call base constructor
    bgTexture0_(nullptr),
    bgTexture1_(nullptr),
    bgTexture2_(nullptr),
    active_anim_(nullptr),
    // current_digimon_ initialized below
    current_state_(STATE_IDLE),
    current_anim_frame_idx_(0),
    current_frame_elapsed_time_(0.0f),
    queued_steps_(0),
    firstWalkUpdate_(true), // Initialize the member variable again
    // Add these initializations:
    bg_scroll_offset_0_(0.0f),
    bg_scroll_offset_1_(0.0f),
    bg_scroll_offset_2_(0.0f)
{
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "+++ AdventureState Constructor ENTER +++"); // Basic Log Added
    if (!game_ptr || !game_ptr->getAssetManager() || !game_ptr->get_display() || !game_ptr->getPlayerData()) {
        throw std::runtime_error("AdventureState requires valid Game pointer with initialized systems (Assets, Display, PlayerData)!");
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState Constructor: Initializing...");

    // --- Set Initial Partner from PlayerData ---
    PlayerData* pd = game_ptr->getPlayerData();
    current_digimon_ = pd->currentPartner;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState: Initial partner set to %d from PlayerData.", static_cast<int>(current_digimon_));
    // ---

    AssetManager* assets = game_ptr->getAssetManager();
    bgTexture0_ = assets->getTexture("castle_bg_0");
    bgTexture1_ = assets->getTexture("castle_bg_1");
    bgTexture2_ = assets->getTexture("castle_bg_2");
    if (!bgTexture0_ || !bgTexture1_ || !bgTexture2_) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"AdventureState: Background texture(s) missing!"); }

    initializeAnimations();

    setActiveAnimation(); // Set initial animation based on initial state

    if (!active_anim_) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"CONSTRUCTOR FAIL: Failed to set initial active animation for partner %d! active_anim_ is NULL.", static_cast<int>(current_digimon_));
    } else {
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,"CONSTRUCTOR OK: Initial active_anim_ set in constructor: %p", (void*)active_anim_);
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState Initialized Successfully.");
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "--- AdventureState Constructor EXIT ---"); // Basic Log Added
}


// --- Destructor ---
// (Unchanged from your version)
AdventureState::~AdventureState() { SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState Destructor called."); }


// --- Initialize Animations ---
// (Unchanged from your version)
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
// (Unchanged from your version)
void AdventureState::setActiveAnimation() {
     Animation* previous_anim_ptr = active_anim_; // Store the pointer to compare

     // Determine the target animation based on the current state
     Animation* target_anim = nullptr;
     if (current_state_ == STATE_IDLE) {
         auto it = idleAnimations_.find(current_digimon_);
         if (it != idleAnimations_.end()) { target_anim = &(it->second); }
     } else { // STATE_WALKING
         auto it = walkAnimations_.find(current_digimon_);
         if (it != walkAnimations_.end()) { target_anim = &(it->second); }
     }

     // --- Change Logic: Only reset frame/timer if the animation OBJECT actually changes ---
     if (target_anim != active_anim_) {
         SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "setActiveAnimation: Animation object changed from %p to %p. Resetting frame index and timer.", (void*)active_anim_, (void*)target_anim);
         active_anim_ = target_anim; // Set the new active animation pointer
         current_anim_frame_idx_ = 0;
         current_frame_elapsed_time_ = 0.0f;
     } else {
         // If the animation object is the same (e.g., looping walk anim), do NOT reset frame/timer here.
         SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "setActiveAnimation: Animation object %p is the same. Frame/timer not reset.", (void*)active_anim_);
     }

     if (!active_anim_) {
         SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "setActiveAnimation: Could not find or set active animation for state %d, digi %d", current_state_, current_digimon_);
     }
}


// --- handle_input ---
// (Unchanged from your version)
void AdventureState::handle_input(InputManager& inputManager, PlayerData* playerData) {
    SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "--- AdventureState handle_input CALLED (Start State: %d, QueuedSteps: %d) ---", current_state_, queued_steps_); // Log at start

    // <<< --- ADDED: Force IDLE if no steps queued --- >>>
    if (queued_steps_ == 0 && current_state_ == STATE_WALKING) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "handle_input: Forcing state to IDLE because no steps were queued.");
        current_state_ = STATE_IDLE;
        // setActiveAnimation(); // Considered potentially calling here, but update handles it
    }
    // <<< --------------------------------------------- >>>


    if (!game_ptr || game_ptr->getCurrentState() != this) { return; }
    if (!playerData) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AdventureState::handle_input - PlayerData pointer is null!"); }

    bool stateOrDigiChanged = false;

    // Menu Activation
    if (inputManager.isActionJustPressed(GameAction::CONFIRM)) {
         SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Confirm action in AdventureState, pushing MenuState.");
         std::vector<std::string> mainMenuItems = {"DIGIMON", "MAP", "ITEMS", "SAVE", "EXIT"};
         game_ptr->requestPushState(std::make_unique<MenuState>(game_ptr, mainMenuItems));
         SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "--- AdventureState handle_input EXITING (Pushing Menu) ---");
          return;
    }

    // <<< --- Log state BEFORE step check --- >>>
    SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "handle_input: Before STEP check (State: %d, QueuedSteps: %d)", current_state_, queued_steps_);
    // Step Input
    if (inputManager.isActionJustPressed(GameAction::STEP)) {
        if (queued_steps_ < MAX_QUEUED_STEPS) {
            queued_steps_++; // Increment queued_steps_
            SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "Step action detected. Queued: %d", queued_steps_);
            if (playerData) {
                playerData->stepsTakenThisChapter++;
                playerData->totalSteps++;
                SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "PlayerData updated: ChapterSteps=%d, TotalSteps=%d", playerData->stepsTakenThisChapter, playerData->totalSteps);
            }
        } else {
             SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "Step action detected, but queue is full (%d).", queued_steps_);
        }
    }
    // <<< --- Log state AFTER step check --- >>>
    SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "handle_input: After STEP check (State: %d, QueuedSteps: %d)", current_state_, queued_steps_);


    // Switch Digimon
    DigimonType previousDigimon = current_digimon_;
    DigimonType keySelectedDigimon = previousDigimon;
    if (inputManager.isActionJustPressed(GameAction::SELECT_DIGI_1)) { keySelectedDigimon = DIGI_AGUMON; }
    else if (inputManager.isActionJustPressed(GameAction::SELECT_DIGI_2)) { keySelectedDigimon = DIGI_GABUMON; }
    else if (inputManager.isActionJustPressed(GameAction::SELECT_DIGI_3)) { keySelectedDigimon = DIGI_BIYOMON; }
    else if (inputManager.isActionJustPressed(GameAction::SELECT_DIGI_4)) { keySelectedDigimon = DIGI_GATOMON; }
    else if (inputManager.isActionJustPressed(GameAction::SELECT_DIGI_5)) { keySelectedDigimon = DIGI_GOMAMON; }
    else if (inputManager.isActionJustPressed(GameAction::SELECT_DIGI_6)) { keySelectedDigimon = DIGI_PALMON; }
    else if (inputManager.isActionJustPressed(GameAction::SELECT_DIGI_7)) { keySelectedDigimon = DIGI_TENTOMON; }
    else if (inputManager.isActionJustPressed(GameAction::SELECT_DIGI_8)) { keySelectedDigimon = DIGI_PATAMON; }
    if (keySelectedDigimon != previousDigimon) {
         current_digimon_ = keySelectedDigimon;
         stateOrDigiChanged = true;
         if (playerData) {
             playerData->currentPartner = current_digimon_;
             SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Updated PlayerData partner to %d via debug key.", static_cast<int>(current_digimon_));
         }
    }
    if (stateOrDigiChanged) {
        current_state_ = STATE_IDLE;
        queued_steps_ = 0;
        SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Switched character to %d via debug key.", static_cast<int>(current_digimon_));
        setActiveAnimation();
    }

    SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "--- AdventureState handle_input EXITING (State: %d, QueuedSteps: %d) ---", current_state_, queued_steps_);
}


// --- update ---
// <<< --- INTEGRATED LATEST LOGGING CHANGES --- >>>
void AdventureState::update(float delta_time, PlayerData* playerData) {
    // Log entry state
    SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "--- AdventureState update CALLED (Start State: %d, QueuedSteps: %d, DT: %.4f) ---", current_state_, queued_steps_, delta_time);

    // Partner check logic...
    if (playerData && current_digimon_ != playerData->currentPartner) {
        // ... update partner, state, steps, setActiveAnimation ...
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState::update detected partner change in PlayerData (from %d to %d).", static_cast<int>(current_digimon_), static_cast<int>(playerData->currentPartner));
         current_digimon_ = playerData->currentPartner;
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Partner changed, resetting state to IDLE.");
         current_state_ = STATE_IDLE; // Ensure state is IDLE after partner change
         queued_steps_ = 0;
         setActiveAnimation();
         // No need to set stateNeedsAnimUpdate = true here, setActiveAnimation was called directly
    }

    // Reset first walk flag if not walking
    if (current_state_ != STATE_WALKING) {
        firstWalkUpdate_ = true;
    }

    bool stateNeedsAnimUpdate = false;
    PlayerState stateBeforeChanges = current_state_;

    // --- State Change: Idle -> Walking ---
    // <<< --- LOG IMMEDIATELY BEFORE IF --- >>>
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, ">>> PRE-CHECK: State=%d, QueuedSteps=%d <<<", current_state_, queued_steps_);
    if (current_state_ == STATE_IDLE && queued_steps_ > 0) {
        // If we get here, the condition was TRUE
        current_state_ = STATE_WALKING;
        stateNeedsAnimUpdate = true;
        firstWalkUpdate_ = true;
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "*** CONDITION MET: Changing state IDLE -> WALKING ***"); // Log moved inside
    } else {
        // <<< --- ADD ELSE LOG --- >>>
        // Only log failure if it *would* have changed (i.e., was IDLE but no steps, or wasn't IDLE)
        // This avoids spamming logs when simply staying in WALKING state.
        if (stateBeforeChanges == STATE_IDLE && queued_steps_ <= 0) {
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "*** IDLE->WALK CONDITION FAILED: State=%d, QueuedSteps=%d ***", current_state_, queued_steps_);
        }
    }


    // Advance Animation Frame logic...
    bool animation_cycle_finished = false;
    if (active_anim_ && active_anim_->getFrameCount() > 0) {
         // --- Animation Advancement Logic (Exact block from PREVIOUS working code) ---
         size_t frameCount = active_anim_->getFrameCount();
         if (current_anim_frame_idx_ >= frameCount) current_anim_frame_idx_ = 0; // Handle invalid index
         if (current_anim_frame_idx_ < active_anim_->frame_durations_ms.size()) { // Check duration exists
             float duration_sec = active_anim_->frame_durations_ms[current_anim_frame_idx_] / 1000.0f;
             current_frame_elapsed_time_ += delta_time;

             while (current_frame_elapsed_time_ >= duration_sec && frameCount > 0) {
                 current_frame_elapsed_time_ -= duration_sec;
                 current_anim_frame_idx_++;

                 if (current_anim_frame_idx_ >= frameCount) { // Reached end of animation
                     animation_cycle_finished = true;
                     if (active_anim_->loops) {
                         current_anim_frame_idx_ = 0; // Loop back to start
                     } else {
                         current_anim_frame_idx_ = frameCount - 1; // Stay on last frame
                         current_frame_elapsed_time_ = 0.0f; // Stop accumulating time on last frame
                         break; // Exit loop if not looping and finished
                     }
                 }

                 // Get duration for the *next* frame (or start frame if looped)
                 if (current_anim_frame_idx_ < active_anim_->frame_durations_ms.size()) {
                      duration_sec = active_anim_->frame_durations_ms[current_anim_frame_idx_] / 1000.0f;
                      // Handle potential zero duration frames (skip them)
                      if (duration_sec <= 0.0f) {
                          SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Anim frame %zu has zero duration, skipping.", current_anim_frame_idx_);
                          continue; // Skip to next frame check in the while loop
                      }
                 } else {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Anim logical error: index %zu out of bounds after loop/advance.", current_anim_frame_idx_);
                    current_anim_frame_idx_ = 0; // Reset to be safe
                    current_frame_elapsed_time_ = 0.0f;
                    break; // Exit loop
                 }
             } // End while loop
         } else {
              SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Anim logical error: index %zu out of bounds for durations.", current_anim_frame_idx_);
              current_anim_frame_idx_ = 0; // Reset to be safe
              current_frame_elapsed_time_ = 0.0f;
         }
         // --- End Animation Advancement Logic ---
    }


    // State Change: Walking -> Idle logic...
    if (current_state_ == STATE_WALKING && animation_cycle_finished && active_anim_ && !active_anim_->loops) {
         // ... (Decrement steps, potentially change state back to IDLE) ...
         queued_steps_--;
         if (queued_steps_ <= 0) {
             current_state_ = STATE_IDLE;
             stateNeedsAnimUpdate = true;
             queued_steps_ = 0;
         } else {
             current_anim_frame_idx_ = 0;
             current_frame_elapsed_time_ = 0.0f;
              SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Walk cycle finished, starting next. Steps remaining: %d", queued_steps_);
         }
    }

    // Log state AFTER potential changes
    if(current_state_ != stateBeforeChanges) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, ">>> State Changed This Frame: %d -> %d <<<", stateBeforeChanges, current_state_);
    }

    // Scroll Background logic...
    if (current_state_ == STATE_WALKING) {
        // ... (scroll logic) ...
        float scrollAmount0 = SCROLL_SPEED_0 * delta_time; float scrollAmount1 = SCROLL_SPEED_1 * delta_time; float scrollAmount2 = SCROLL_SPEED_2 * delta_time;
        int effW0=0, effW1=0, effW2=0;
        if(bgTexture0_) {int w; SDL_QueryTexture(bgTexture0_,0,0,&w,0); effW0=w*2/3; if(effW0<=0)effW0=w;}
        if(bgTexture1_) {int w; SDL_QueryTexture(bgTexture1_,0,0,&w,0); effW1=w*2/3; if(effW1<=0)effW1=w;}
        if(bgTexture2_) {int w; SDL_QueryTexture(bgTexture2_,0,0,&w,0); effW2=w*2/3; if(effW2<=0)effW2=w;}
        if(effW0 > 0) { bg_scroll_offset_0_ -= scrollAmount0; bg_scroll_offset_0_ = std::fmod(bg_scroll_offset_0_ + effW0, (float)effW0); }
        if(effW1 > 0) { bg_scroll_offset_1_ -= scrollAmount1; bg_scroll_offset_1_ = std::fmod(bg_scroll_offset_1_ + effW1, (float)effW1); }
        if(effW2 > 0) { bg_scroll_offset_2_ -= scrollAmount2; bg_scroll_offset_2_ = std::fmod(bg_scroll_offset_2_ + effW2, (float)effW2); }
       }

    // Set active animation logic...
    if (stateNeedsAnimUpdate) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, ">>> Calling setActiveAnimation (State is now %d) <<<", current_state_);
        setActiveAnimation();
    }

    // Clear first walk flag logic...
    if (current_state_ == STATE_WALKING && firstWalkUpdate_) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, ">>> First Walk Update Frame Processing Completed <<<");
        firstWalkUpdate_ = false;
    }

    SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "Update End: State=%d, QueuedSteps=%d", current_state_, queued_steps_); // Log steps too
}


// --- render ---
// (Unchanged from your version)
void AdventureState::render(PCDisplay& display) {
    // Simplified render logic for brevity (assume original logic is here)
    const int windowW = WINDOW_WIDTH; const int windowH = WINDOW_HEIGHT; auto drawTiledBg = [&](SDL_Texture* tex, float offset, int texW, int texH, int effectiveWidth, const char* layerName) { if (!tex || texW <= 0 || effectiveWidth <= 0) { return; } int drawX1 = -static_cast<int>(std::fmod(offset, (float)effectiveWidth)); if (drawX1 > 0) drawX1 -= effectiveWidth; SDL_Rect dst1 = { drawX1, 0, texW, texH }; display.drawTexture(tex, NULL, &dst1); int drawX2 = drawX1 + effectiveWidth; SDL_Rect dst2 = { drawX2, 0, texW, texH }; display.drawTexture(tex, NULL, &dst2); if (drawX2 + texW < windowW) { int drawX3 = drawX2 + effectiveWidth; SDL_Rect dst3 = { drawX3, 0, texW, texH }; display.drawTexture(tex, NULL, &dst3); } }; int bgW0=0,bgH0=0,effW0=0, bgW1=0,bgH1=0,effW1=0, bgW2=0,bgH2=0,effW2=0; if(bgTexture0_) { SDL_QueryTexture(bgTexture0_,0,0,&bgW0,&bgH0); effW0=bgW0*2/3; if(effW0<=0)effW0=bgW0;} if(bgTexture1_) { SDL_QueryTexture(bgTexture1_,0,0,&bgW1,&bgH1); effW1=bgW1*2/3; if(effW1<=0)effW1=bgW1;} if(bgTexture2_) { SDL_QueryTexture(bgTexture2_,0,0,&bgW2,&bgH2); effW2=bgW2*2/3; if(effW2<=0)effW2=bgW2;} drawTiledBg(bgTexture2_, bg_scroll_offset_2_, bgW2, bgH2, effW2, "Layer 2"); drawTiledBg(bgTexture1_, bg_scroll_offset_1_, bgW1, bgH1, effW1, "Layer 1"); if (active_anim_) { const SpriteFrame* currentFrame = active_anim_->getFrame(current_anim_frame_idx_); if (currentFrame && currentFrame->texturePtr && currentFrame->sourceRect.w > 0 && currentFrame->sourceRect.h > 0) { int drawX = (windowW / 2) - (currentFrame->sourceRect.w / 2); int verticalOffset = 7; int drawY = (windowH / 2) - (currentFrame->sourceRect.h / 2) - verticalOffset; SDL_Rect dstRect = { drawX, drawY, currentFrame->sourceRect.w, currentFrame->sourceRect.h }; display.drawTexture(currentFrame->texturePtr, &currentFrame->sourceRect, &dstRect); } else { SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "AS Render: Current anim frame %zu invalid (Null Tex or 0 Dim) for partner %d", current_anim_frame_idx_, static_cast<int>(current_digimon_)); } } else { SDL_LogWarn(SDL_LOG_CATEGORY_RENDER,"AS Render: active_anim_ is null for partner %d!", static_cast<int>(current_digimon_)); } drawTiledBg(bgTexture0_, bg_scroll_offset_0_, bgW0, bgH0, effW0, "Layer 0");
}