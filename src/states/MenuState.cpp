// File: src/states/MenuState.cpp

#include "states/MenuState.h"
#include "core/Game.h"              // Needed for game_ptr access, requestPopState, requestPushState
#include "core/AssetManager.h"      // Needed for asset loading
#include "platform/pc/pc_display.h" // Needed for PCDisplay& parameter and methods
#include "core/InputManager.h"      // Needed for InputManager& parameter
#include "core/GameAction.h"        // Needed for GameAction enum
#include "core/PlayerData.h"        // Needed for PlayerData* parameter
#include <SDL_log.h>
#include <SDL.h>                    // Needed for SDL types used in helpers
#include <stdexcept>
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <memory>                   // Needed for std::make_unique
#include "vendor/nlohmann/json.hpp" // Needed for font loading

// Need to potentially include PartnerSelectState.h if pushing it directly
#include "states/PartnerSelectState.h"

MenuState::MenuState(Game* game, const std::vector<std::string>& options) :
    GameState(game), // <<< --- ADDED Base class constructor call --- >>>
    menuOptions_(options),
    currentSelection_(0),
    backgroundTexture_(nullptr),
    fontTexture_(nullptr),
    cursorTexture_(nullptr) // Explicitly initialize cursorTexture_
{
    // game_ptr is initialized by GameState(game) call above
    // this->game_ptr = game; // No longer needed

    if (!game_ptr || !game_ptr->getAssetManager()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MenuState requires valid Game pointer with initialized AssetManager!");
        // Consider throwing an exception or handling this error more robustly
    }
    else {
        AssetManager* assets = game_ptr->getAssetManager();
        backgroundTexture_ = assets->getTexture("menu_bg_blue");
        fontTexture_ = assets->getTexture("ui_font_atlas");
        if (!backgroundTexture_) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "MenuState Warning: Background texture 'menu_bg_blue' not found."); }
        if (!fontTexture_) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "MenuState Warning: Font texture 'ui_font_atlas' not found."); }
        else {
            // Load font map
            if (!loadFontDataFromJson("assets/ui/fonts/bluewhitefont.json")) {
                 SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MenuState Error: Failed to load font map data 'assets/ui/fonts/bluewhitefont.json'.");
            }
        }
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState Created with %zu options.", options.size());
}

MenuState::~MenuState() {
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState Destroyed.");
}

// <<< --- MODIFIED handle_input Signature --- >>>
void MenuState::handle_input(InputManager& inputManager, PlayerData* playerData) {
    // InputManager is now passed in directly
    // PlayerData* is passed in but not currently used in this logic
    if (!game_ptr) return; // Still need game_ptr for state changes

    // Use the passed-in inputManager reference
    bool navUp = inputManager.isActionJustPressed(GameAction::NAV_UP);
    bool navDown = inputManager.isActionJustPressed(GameAction::NAV_DOWN);
    bool confirm = inputManager.isActionJustPressed(GameAction::CONFIRM);
    bool cancel = inputManager.isActionJustPressed(GameAction::CANCEL);

    // Logging remains the same
    if (navUp || navDown || confirm || cancel || inputManager.isActionHeld(GameAction::NAV_UP) || inputManager.isActionHeld(GameAction::NAV_DOWN) || inputManager.isActionHeld(GameAction::CONFIRM) || inputManager.isActionHeld(GameAction::CANCEL) ) { SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "MenuState Input Check: JustPressed(Up=%d, Down=%d, Confirm=%d, Cancel=%d)", navUp, navDown, confirm, cancel); }

    // Navigation logic remains the same
    if (navUp) { if (!menuOptions_.empty()) { currentSelection_ = (currentSelection_ == 0) ? menuOptions_.size() - 1 : currentSelection_ - 1; SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "Menu: Selected option %zu - '%s'", currentSelection_, menuOptions_[currentSelection_].c_str()); } }
    else if (navDown) { if (!menuOptions_.empty()) { currentSelection_ = (currentSelection_ + 1) % menuOptions_.size(); SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "Menu: Selected option %zu - '%s'", currentSelection_, menuOptions_[currentSelection_].c_str()); } }
    // Confirmation logic needs update for PartnerSelectState
    else if (confirm) {
        if (!menuOptions_.empty()) {
            const std::string& selectedOption = menuOptions_[currentSelection_];
            SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Menu: Confirmed '%s'", selectedOption.c_str());

            if (selectedOption == "EXIT") {
                game_ptr->requestPopState();
            } else if (selectedOption == "DIGIMON") {
                std::vector<std::string> opts = {"PARTNER", "STATUS", "EVOLVE", "BACK"};
                game_ptr->requestPushState(std::make_unique<MenuState>(game_ptr, opts));
            } else if (selectedOption == "MAP") {
                std::vector<std::string> opts = {"VIEW MAP", "TRAVEL", "BACK"};
                game_ptr->requestPushState(std::make_unique<MenuState>(game_ptr, opts));
            } else if (selectedOption == "ITEMS") {
                std::vector<std::string> opts = {"VIEW", "USE", "DROP", "BACK"};
                game_ptr->requestPushState(std::make_unique<MenuState>(game_ptr, opts));
            } else if (selectedOption == "SAVE") {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Action for SAVE not implemented.");
                // Potentially call game_ptr->saveGame(playerData); here later
            } else if (selectedOption == "BACK") {
                game_ptr->requestPopState();
            } else if (selectedOption == "PARTNER") {
                // <<< --- ACTION: Push PartnerSelectState --- >>>
                SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Pushing PartnerSelectState.");
                game_ptr->requestPushState(std::make_unique<PartnerSelectState>(game_ptr));
            } else if (selectedOption == "STATUS") {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Action for STATUS not implemented.");
                // Push StatusState later
            } else if (selectedOption == "EVOLVE") {
                 SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Action for EVOLVE not implemented.");
                 // Push EvolveState later
            }
            // Add cases for VIEW MAP, TRAVEL, VIEW, USE, DROP etc. later
            else {
                SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "Selected option '%s' has no defined action yet.", selectedOption.c_str());
            }
        }
    }
    // Cancel logic remains the same
    else if (cancel) { SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Cancel action detected in MenuState, requesting popState."); game_ptr->requestPopState(); }
}
// <<< --- END MODIFIED handle_input --- >>>


// <<< --- MODIFIED update Signature --- >>>
void MenuState::update(float delta_time, PlayerData* playerData) {
    // PlayerData* is passed in but not currently used
    // Update logic remains empty for now
    // (Could add logic for animating menu transitions later)
}
// <<< --- END MODIFIED update --- >>>


// <<< --- MODIFIED render Signature --- >>>
void MenuState::render(PCDisplay& display) {
    // PCDisplay object is passed in directly by reference
    // PCDisplay* displayPtr = game_ptr->get_display(); // No longer needed
    SDL_Renderer* renderer = display.getRenderer(); // Get renderer from the display object
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER,"MenuState Render Error: Renderer is null!");
        return; // Cannot render without a renderer
    }

    int windowW = 0; int windowH = 0;
    display.getWindowSize(windowW, windowH); // Get size from the display object
    if (windowW <= 0 || windowH <= 0) {
        SDL_LogWarn(SDL_LOG_CATEGORY_RENDER,"MenuState Render Warning: Invalid window dimensions (%d x %d), defaulting.", windowW, windowH);
        windowW = 466; windowH = 466; // Use default if error
    }

    // 1. Draw Background
    if (backgroundTexture_) {
        // Use the display object's drawTexture method (assuming it exists)
        // or directly use SDL_RenderCopy with the renderer obtained above.
        // Let's assume direct SDL_RenderCopy for clarity here.
        SDL_RenderCopy(renderer, backgroundTexture_, NULL, NULL);
    }
    else {
        SDL_SetRenderDrawColor(renderer, 0, 0, 80, 255);
        SDL_RenderClear(renderer);
    }

    // 2. Draw Currently Selected Menu Option (Logic largely unchanged, uses local renderer)
    SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "--- Menu Render Step 2 START ---");
    if (fontTexture_ && !fontCharMap_.empty() && !menuOptions_.empty()) {
        SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Assets OK (Texture: %p, Map size: %zu, Options size: %zu)", (void*)fontTexture_, fontCharMap_.size(), menuOptions_.size());

        const float textScale = 0.9f; const int kerning = -15;
        const std::string& selectedText = menuOptions_[currentSelection_];
        SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Selected Text: '%s'", selectedText.c_str());

        SDL_Point baseDimensions = getTextDimensions(selectedText, kerning);
        SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Base Dimensions calculated: w=%d, h=%d", baseDimensions.x, baseDimensions.y);

        if (baseDimensions.x > 0 && baseDimensions.y > 0) {
            int scaledTextWidth = static_cast<int>(static_cast<float>(baseDimensions.x) * textScale);
            int scaledTextHeight = static_cast<int>(static_cast<float>(baseDimensions.y) * textScale);
            int startX = (windowW / 2) - (scaledTextWidth / 2);
            int startY = (windowH / 2) - (scaledTextHeight / 2);
            SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Calculated Position & Scale: scale=%.2f, scaledW=%d, scaledH=%d, startX=%d, startY=%d", textScale, scaledTextWidth, scaledTextHeight, startX, startY);

            Uint8 r, g, b, a; SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a); SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Render draw color BEFORE drawText: R=%d G=%d B=%d A=%d", r, g, b, a);
            SDL_BlendMode blendMode; SDL_GetRenderDrawBlendMode(renderer, &blendMode); SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Render draw blend mode BEFORE drawText: %d", (int)blendMode);

            SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "--- Calling drawText ---");
            // Call drawText helper, passing the renderer obtained from display
            drawText(renderer, selectedText, startX, startY, textScale, kerning);
            SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "--- Returned from drawText ---");

        } else {
             SDL_LogError(SDL_LOG_CATEGORY_RENDER, "MenuState::render - SKIPPING DRAW due to invalid dimensions for text: %s", selectedText.c_str());
        }
    } else {
        SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "MenuState::render - SKIPPING DRAW because font assets missing or no options. Font Texture: %p, Map Empty: %d, Options Empty: %d", (void*)fontTexture_, fontCharMap_.empty(), menuOptions_.empty());
    }
    SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "--- Menu Render Step 2 END ---");
}
// <<< --- END MODIFIED render --- >>>


// --- Helper Functions ---
// (loadFontDataFromJson, getTextDimensions, drawText implementations remain the same internally)
// (They already use member variables like fontTexture_, fontCharMap_ and accept SDL_Renderer*)

bool MenuState::loadFontDataFromJson(const std::string& jsonPath) {
     // ... (implementation unchanged) ...
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "LOAD FONT JSON: --- Function START --- Path: %s", jsonPath.c_str()); fontCharMap_.clear(); bool success = false;
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "LOAD FONT JSON: Attempting std::ifstream open..."); std::ifstream f(jsonPath);
     if (!f.is_open()) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "LOAD FONT JSON ***ERROR***: std::ifstream failed to open file: %s", jsonPath.c_str()); goto cleanup; }
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "LOAD FONT JSON: File stream is open.");
     if (f.fail() || f.bad()) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "LOAD FONT JSON ***ERROR***: File stream state is bad/failed after opening! Failbit: %d, Badbit: %d", (int)f.fail(), (int)f.bad()); f.close(); goto cleanup; }
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "LOAD FONT JSON: File stream state OK after opening.");
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "LOAD FONT JSON: --- Entering TRY block ---");
     try {
        nlohmann::json data; SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "LOAD FONT JSON: Attempting nlohmann::json::parse(f)..."); data = nlohmann::json::parse(f);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "LOAD FONT JSON: JSON Parsing SUCCESSFUL.");
        if (!data.contains("frames") || !data["frames"].is_object()) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "LOAD FONT JSON ERROR: JSON missing 'frames' object or it's not an object: %s", jsonPath.c_str()); goto cleanup; }
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "LOAD FONT JSON: Found 'frames' object."); const auto& frames = data["frames"]; int loadedCount = 0;
        for (auto& [key, value] : frames.items()) {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "LOAD FONT JSON: Processing key: '%s'", key.c_str());
            if (!value.contains("frame") || !value["frame"].is_object()) { /* Warn/Skip */ continue; }
            const auto& frameData = value["frame"]; if (!(frameData.contains("x") && frameData.contains("y") && frameData.contains("w") && frameData.contains("h"))) { /* Warn/Skip */ continue; }
            char character = '\0'; if (key.length() == 1) { character = key[0]; } else { /* Handle descriptive keys */ if (key == "QUESTION") character = '?'; else if (key == "apostrophe") character = '\''; else if (key == "colon") character = ':'; else if (key == "comma") character = ','; else if (key == "dash") character = '-'; else if (key == "divide") character = '/'; else if (key == "equals") character = '='; else if (key == "exclamation") character = '!'; else if (key == "forwardslash") character = '/'; else if (key == "period") character = '.'; else if (key == "plus") character = '+'; else if (key == "roundbracketleft") character = '('; else if (key == "roundbracketright") character = ')'; else if (key == "speech") character = '"'; else if (key == "times") character = '*'; else if (key == "weirdbracketleft") character = '['; else if (key == "weirdbracketright") character = ']'; else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "LOAD FONT JSON WARN: Skipping unrecognized descriptive key: '%s'", key.c_str()); continue; } }
            if (character != '\0') {
                int x = frameData.value("x", -1); int y = frameData.value("y", -1); int w = frameData.value("w", -1); int h = frameData.value("h", -1);
                if (x < 0 || y < 0 || w <= 0 || h <= 0) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "LOAD FONT JSON WARN: Skipping char '%c' (key '%s'): Invalid rect data (x=%d, y=%d, w=%d, h=%d)", character, key.c_str(), x, y, w, h); continue; }
                fontCharMap_[character] = SDL_Rect{ x, y, w, h }; SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "LOAD FONT JSON: Mapped char '%c' (key '%s') to rect {x=%d, y=%d, w=%d, h=%d}", character, key.c_str(), x, y, w, h); loadedCount++;
            }
        } SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "LOAD FONT JSON: Finished processing frames. Loaded map for %d characters.", loadedCount); success = !fontCharMap_.empty();
     } catch (nlohmann::json::parse_error& e) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "LOAD FONT JSON ***ERROR (JSON PARSE)***: %s - %s (byte %zu)", jsonPath.c_str(), e.what(), e.byte); }
     catch (std::exception& e) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "LOAD FONT JSON ***ERROR (STD EXCEPTION)***: %s - %s", jsonPath.c_str(), e.what()); }
     catch (...) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "LOAD FONT JSON ***ERROR (UNKNOWN)***: An unknown exception occurred during JSON processing."); }
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "LOAD FONT JSON: --- Exited TRY-CATCH block ---");
cleanup: if (f.is_open()) { SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "LOAD FONT JSON: Closing file stream before exit."); f.close(); } SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "LOAD FONT JSON: --- Function END --- Returning: %s", success ? "true" : "false"); return success;
}

SDL_Point MenuState::getTextDimensions(const std::string& text, int kerning) {
    // ... (implementation unchanged) ...
    SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "getTextDimensions START for text: '%s'", text.c_str());
    SDL_Point dimensions = {0, 0}; int currentX = 0; int maxHeight = 0;
    if (fontCharMap_.empty()) { SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "getTextDimensions WARN: fontCharMap_ is empty!"); return dimensions; }
    bool foundAnyChar = false;
    for (char c : text) {
        int charWidth = 0; int charHeight = 0; // Declared inside loop
        if (c == ' ') { /* Need to handle space width based on font */ int spaceWidth = 5; auto it_A = fontCharMap_.find('A'); if(it_A != fontCharMap_.end()){spaceWidth=it_A->second.w;} charWidth=spaceWidth; charHeight=0; foundAnyChar = true; }
        else { auto it = fontCharMap_.find(c); if (it != fontCharMap_.end()) { const SDL_Rect& srcRect = it->second; if (srcRect.w > 0 && srcRect.h > 0) { charWidth = srcRect.w; charHeight = srcRect.h; SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "getTextDimensions: Found char '%c', w=%d, h=%d", c, charWidth, charHeight); foundAnyChar = true; } else { charWidth = 5; charHeight = 8; SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "getTextDimensions WARN: Found char '%c' but rect is invalid (w=%d, h=%d)!", c, srcRect.w, srcRect.h); } } else { charWidth = 5; charHeight = 8; SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "getTextDimensions WARN: Character '%c' not found in fontCharMap_!", c); } }
        currentX += charWidth + kerning; if (charHeight > maxHeight) { maxHeight = charHeight; }
    }
    dimensions.x = (foundAnyChar && !text.empty()) ? (currentX - kerning) : 0; if (dimensions.x < 0) dimensions.x = 0; dimensions.y = foundAnyChar ? maxHeight : 0;
    SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "getTextDimensions END for text: '%s'. Result: w=%d, h=%d", text.c_str(), dimensions.x, dimensions.y); return dimensions;
}

void MenuState::drawText(SDL_Renderer* renderer, const std::string& text, int startX, int startY, float scale, int kerning) {
    // ... (implementation unchanged) ...
     SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "drawText START for text '%s' at (%d, %d), scale=%.2f", text.c_str(), startX, startY, scale);
     if (!fontTexture_ || fontCharMap_.empty() || !renderer || scale <= 0.0f) {
        SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "drawText returning early. Texture=%p, MapEmpty=%d, Renderer=%p, Scale=%.2f", (void*)fontTexture_, fontCharMap_.empty(), (void*)renderer, scale);
        return;
     }
     int currentX = startX;
     float scaledKerning = static_cast<float>(kerning) * scale;

     for (size_t i = 0; i < text.length(); ++i) { // Use index loop for logging
         char c = text[i];
         SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "drawText: Processing char #%zu: '%c'", i, c);
         int charWidth = 0; int charHeight = 0;
         const SDL_Rect* pSrcRect = nullptr;

         if (c == ' ') {
             auto it_space = fontCharMap_.find('A'); // Use width of 'A' or fallback
             charWidth = (it_space != fontCharMap_.end() && it_space->second.w > 0) ? it_space->second.w : 5;
             int advance = static_cast<int>(static_cast<float>(charWidth) * scale + scaledKerning);
             SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "drawText: Space char, advancing by %d", advance);
             currentX += advance;
             continue;
         } else {
             auto it = fontCharMap_.find(c);
             if (it != fontCharMap_.end()) {
                 const SDL_Rect& srcRect = it->second;
                 SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "drawText: Found char '%c' in map. Retrieved Rect: {x=%d,y=%d,w=%d,h=%d}", c, srcRect.x, srcRect.y, srcRect.w, srcRect.h);

                 if(srcRect.w > 0 && srcRect.h > 0) {
                     pSrcRect = &srcRect;
                     charWidth = pSrcRect->w; charHeight = pSrcRect->h;
                     SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "drawText: Rect is valid.");
                 } else {
                    charWidth = 5; charHeight = 8; SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "drawText: Char '%c' found but rect invalid.", c);
                 }
             } else { charWidth = 5; charHeight = 8; SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "drawText: Char '%c' NOT found in map.", c); }
         }

         int scaledW = static_cast<int>(static_cast<float>(charWidth) * scale);
         int scaledH = static_cast<int>(static_cast<float>(charHeight) * scale);

         if (pSrcRect != nullptr && scaledW > 0 && scaledH > 0) {
              SDL_Rect destRect = { currentX, startY, scaledW, scaledH };
              SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "drawText: Attempting SDL_RenderCopy for '%c'. SrcRect={%d,%d,%d,%d}, DestRect={%d,%d,%d,%d}",
                           c, pSrcRect->x, pSrcRect->y, pSrcRect->w, pSrcRect->h,
                           destRect.x, destRect.y, destRect.w, destRect.h);
              SDL_SetTextureBlendMode(fontTexture_, SDL_BLENDMODE_BLEND);
              SDL_SetTextureAlphaMod(fontTexture_, 255);
              int result = SDL_RenderCopy(renderer, fontTexture_, pSrcRect, &destRect);
              if (result != 0) {
                  SDL_LogError(SDL_LOG_CATEGORY_RENDER, "drawText: SDL_RenderCopy failed for char '%c'! SDL_Error: %s", c, SDL_GetError());
              } else {
                  SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "drawText: SDL_RenderCopy succeeded for char '%c'.", c);
              }
         } else {
              SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "drawText: Skipping SDL_RenderCopy for char '%c' (pSrcRect=%p, scaledW=%d, scaledH=%d)", c, (void*)pSrcRect, scaledW, scaledH);
         }
         int advance = scaledW + static_cast<int>(scaledKerning);
         SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "drawText: Advancing X by %d (scaledW=%d + scaledKerning=%d)", advance, scaledW, static_cast<int>(scaledKerning));
         currentX += advance;
     }
     SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "drawText END for text '%s'", text.c_str());
}