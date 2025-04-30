// File: src/states/AdventureState.cpp

#include "states/AdventureState.h"  // Include own header
#include "core/Game.h"              // To access Game methods/members
#include "core/AssetManager.h"      // To get assets
#include "platform/pc/pc_display.h" // To draw
#include "graphics/Animation.h"     // Uses Animation/SpriteFrame
#include <SDL_log.h>                // SDL logging
#include <stdexcept>                // For exceptions
#include <fstream>                  // For reading files
#include "vendor/nlohmann/json.hpp" // Path to JSON library header
#include <cstddef>                  // Needed for size_t if not implicitly included elsewhere

// Use the nlohmann::json namespace
using json = nlohmann::json;


// --- Anonymous Namespace for Helpers and Constants ---
namespace {

// --- Helper Function to Create Animation from Indices ---
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
            anim.addFrame({texture, allFrameRects[frameIndex]}, duration);
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Anim Creation: Index %d out of bounds (%zu). Using frame 0.", frameIndex, allFrameRects.size());
             anim.addFrame({texture, allFrameRects[0]}, duration); // Placeholder on error
        }
    }
    return anim;
}

// --- Animation Sequence Templates ---
const std::vector<int> IDLE_INDICES = {0, 1};
const std::vector<Uint32> IDLE_DURATIONS = {1000, 1000};
const std::vector<int> WALK_INDICES = {2, 3, 2, 3};
const std::vector<Uint32> WALK_DURATIONS = {300, 300, 300, 300};
const std::vector<int> ATTACK_INDICES = {1, 0, 3, 8};
const std::vector<Uint32> ATTACK_DURATIONS = {200, 150, 150, 400};

} // end anonymous namespace


// --- Constructor ---
AdventureState::AdventureState(Game* game) :
    bgTexture0_(nullptr), bgTexture1_(nullptr), bgTexture2_(nullptr),
    active_anim_(nullptr),
    current_digimon_(DIGI_AGUMON), current_state_(STATE_IDLE),
    current_anim_frame_idx_(0), // Initialized as size_t
    current_frame_elapsed_time_(0.0f),
    queued_steps_(0)
{
    this->game_ptr = game;
    if (!game_ptr || !game_ptr->getAssetManager() || !game_ptr->get_display()) {
        throw std::runtime_error("AdventureState requires valid Game pointer with initialized systems!");
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState Constructor: Initializing...");

    AssetManager* assets = game_ptr->getAssetManager();
    bgTexture0_ = assets->getTexture("castle_bg_0");
    bgTexture1_ = assets->getTexture("castle_bg_1");
    bgTexture2_ = assets->getTexture("castle_bg_2");
    if (!bgTexture0_ || !bgTexture1_ || !bgTexture2_) { /* Log warning */ }
    initializeAnimations();
    setActiveAnimation();
    if (!active_anim_) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Failed to set initial active animation!"); }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState Initialized Successfully.");
}

// --- Destructor ---
AdventureState::~AdventureState() {
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AdventureState Destructor called.");
}


// --- Initialize Animations ---
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
                             // Basic check for required keys before accessing
                             if (rectData.contains("x") && rectData.contains("y") && rectData.contains("w") && rectData.contains("h")) {
                                frameRects.push_back({ rectData["x"].get<int>(), rectData["y"].get<int>(), rectData["w"].get<int>(), rectData["h"].get<int>() });
                             } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "JSON frame missing x,y,w, or h in %s (array item)", jsonPath.c_str()); }
                          } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "JSON frame missing 'frame' object in %s (array item)", jsonPath.c_str()); }
                      }
                 } else if (framesNode.is_object()) {
                      for (auto& [key, frameData] : framesNode.items()) {
                           if (frameData.contains("frame")) {
                                const auto& rectData = frameData["frame"];
                                // Basic check for required keys before accessing
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

// --- Handle Input ---
void AdventureState::handle_input() {
    const Uint8* keystates = SDL_GetKeyboardState(NULL);
    bool stateOrDigiChanged = false;
    static bool space_pressed = false;
    if(keystates[SDL_SCANCODE_SPACE] && !space_pressed) { if(queued_steps_ < MAX_QUEUED_STEPS) queued_steps_++; space_pressed = true; SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "Step added. Queued: %d", queued_steps_);}
    if(!keystates[SDL_SCANCODE_SPACE]) space_pressed = false;
    static bool num_pressed[DIGI_COUNT] = {false};
    for(int i=0; i<DIGI_COUNT; ++i) {
         SDL_Scancode scancode = (SDL_Scancode)(SDL_SCANCODE_1 + i);
         if(keystates[scancode] && !num_pressed[i]) {
             DigimonType selected = static_cast<DigimonType>(i);
             if (selected != current_digimon_) {
                 current_digimon_ = selected; current_state_ = STATE_IDLE; queued_steps_ = 0; stateOrDigiChanged = true;
                 SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Switched character to %d", current_digimon_);
             }
             num_pressed[i] = true;
         }
         if(!keystates[scancode]) num_pressed[i] = false;
    }
    if(keystates[SDL_SCANCODE_ESCAPE] && game_ptr) game_ptr->quit_game();
    if(stateOrDigiChanged) setActiveAnimation();
}

// --- Update ---
void AdventureState::update(float delta_time) {
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
    if (current_state_ == STATE_IDLE && queued_steps_ > 0) { current_state_ = STATE_WALKING; stateNeedsAnimUpdate = true; SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "State changed to WALKING"); }
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
         queued_steps_--; SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Walk cycle finished. Steps remaining: %d", queued_steps_);
         if (queued_steps_ > 0) { current_anim_frame_idx_ = 0; current_frame_elapsed_time_ = 0.0f; SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Starting next queued walk cycle."); }
         else { current_state_ = STATE_IDLE; stateNeedsAnimUpdate = true; SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Finished walking, switching to IDLE state."); }
    }
    if (stateNeedsAnimUpdate) setActiveAnimation();
}


// --- Render ---
// Draws backgrounds and the current character animation frame, WITH ADDED LOGGING and CORRECTED drawTexture call
void AdventureState::render() {
    if (!game_ptr) return;
    PCDisplay* display = game_ptr->get_display();
    if (!display) return;
    SDL_Renderer* renderer = display->getRenderer();
    if (!renderer) return;

    // Draw Backgrounds
    auto drawTiledBg = [&](SDL_Texture* tex, float offset, int texW, int texH, int effectiveWidth) {
        if (!tex || texW <= 0 || effectiveWidth <= 0) return;
        // Use fmod on the offset directly for correct wrapping calculation
        int drawX1 = -static_cast<int>(std::fmod(offset, (float)effectiveWidth));
        // Ensure the result is negative or zero if offset was positive
        if (drawX1 > 0) drawX1 -= effectiveWidth;

        SDL_Rect dst1 = { drawX1, 0, texW, texH };
        display->drawTexture(tex, NULL, &dst1); // Draw first tile

        int drawX2 = drawX1 + effectiveWidth; // Position of the second tile
        SDL_Rect dst2 = { drawX2, 0, texW, texH };
        display->drawTexture(tex, NULL, &dst2); // Draw second tile

        // Optional: Draw a third tile ONLY if the second tile doesn't fully cover the remaining screen width
         if (drawX2 + texW < WINDOW_WIDTH) { // Check if the *end* of the second tile is before window edge
             int drawX3 = drawX2 + effectiveWidth;
             SDL_Rect dst3 = { drawX3, 0, texW, texH };
             display->drawTexture(tex, NULL, &dst3);
         }
    };
    // Get dimensions and draw backgrounds
    int bgW0=0,bgH0=0,effW0=0, bgW1=0,bgH1=0,effW1=0, bgW2=0,bgH2=0,effW2=0;
    if(bgTexture0_) { SDL_QueryTexture(bgTexture0_,0,0,&bgW0,&bgH0); effW0=bgW0*2/3; if(effW0<=0)effW0=bgW0;}
    if(bgTexture1_) { SDL_QueryTexture(bgTexture1_,0,0,&bgW1,&bgH1); effW1=bgW1*2/3; if(effW1<=0)effW1=bgW1;}
    if(bgTexture2_) { SDL_QueryTexture(bgTexture2_,0,0,&bgW2,&bgH2); effW2=bgW2*2/3; if(effW2<=0)effW2=bgW2;}
    drawTiledBg(bgTexture2_, bg_scroll_offset_2_, bgW2, bgH2, effW2);
    drawTiledBg(bgTexture1_, bg_scroll_offset_1_, bgW1, bgH1, effW1);

    // Logging block
    SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Render: Current Digi=%d, State=%d, FrameIdx=%zu", current_digimon_, current_state_, current_anim_frame_idx_);
    SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Render: Active Anim Ptr = %p", active_anim_);

    // *** Draw Character Sprite using Texture ***
    if (active_anim_) {
        const SpriteFrame* currentFrame = active_anim_->getFrame(current_anim_frame_idx_);
        SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Render: Current Frame Ptr = %p", currentFrame);

        if (currentFrame && currentFrame->texturePtr && currentFrame->sourceRect.w > 0 && currentFrame->sourceRect.h > 0) {
            SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Render: Frame Tex=%p, SrcRect={%d,%d,%d,%d}", currentFrame->texturePtr, currentFrame->sourceRect.x, currentFrame->sourceRect.y, currentFrame->sourceRect.w, currentFrame->sourceRect.h);
            int drawX = (WINDOW_WIDTH / 2) - (currentFrame->sourceRect.w / 2);
            int drawY = (WINDOW_HEIGHT / 2) - (currentFrame->sourceRect.h / 2);
                        SDL_Rect dstRect = { drawX, drawY, currentFrame->sourceRect.w, currentFrame->sourceRect.h };
            SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Render: DstRect={%d,%d,%d,%d}", dstRect.x, dstRect.y, dstRect.w, dstRect.h);

            // <<< --- THIS IS THE CORRECTED LINE --- >>>
            display->drawTexture(currentFrame->texturePtr,
                                 &currentFrame->sourceRect, // Address of source rect
                                 &dstRect);                 // Address of destination rect
            // <<< --- END CORRECTION --- >>>

        } else { // Log failure details
             if (!currentFrame) SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "Render FAIL: Current animation frame %zu is null for anim %p", current_anim_frame_idx_, active_anim_);
             else if (!currentFrame->texturePtr) SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "Render FAIL: Current frame %zu has null texture for anim %p", current_anim_frame_idx_, active_anim_);
             else SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "Render FAIL: Current frame %zu has zero width/height (%d, %d) for anim %p", current_anim_frame_idx_, currentFrame->sourceRect.w, currentFrame->sourceRect.h, active_anim_);
        }
    } else {
         SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "Render: No active animation set!");
    }

    // Draw Foreground
    drawTiledBg(bgTexture0_, bg_scroll_offset_0_, bgW0, bgH0, effW0);
} // <-- Make sure this closing brace is the end of the function