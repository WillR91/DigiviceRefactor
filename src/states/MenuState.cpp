// File: src/states/MenuState.cpp

#include "states/MenuState.h"
#include "core/Game.h"              // Needed for game_ptr access, requestPopState, requestPushState
#include "core/AssetManager.h"      // Needed for asset loading
#include "platform/pc/pc_display.h" // Needed for display pointer and getWindowSize
// #include "states/TransitionState.h" // No longer needed here
#include <SDL_log.h>
#include <SDL.h>                    // Needed for SDL_*, including SDL_Point
#include <stdexcept>                // For std::runtime_error
#include <vector>                   // Needed for menuOptions_, std::vector
#include <string>                   // Needed for menuOptions_, std::string
#include <map>                      // Needed for std::map (fontCharMap_)
#include <fstream>                  // Needed for std::ifstream (JSON loading)
#include <memory>                   // <<< ADDED for std::make_unique >>>
#include "vendor/nlohmann/json.hpp" // Needed for JSON parsing

MenuState::MenuState(Game* game, const std::vector<std::string>& options) :
    menuOptions_(options),
    currentSelection_(0),
    backgroundTexture_(nullptr),
    fontTexture_(nullptr),
    cursorTexture_(nullptr) // Ensure this is nullptr as we aren't loading/using it
{
    this->game_ptr = game;
    if (!game_ptr || !game_ptr->getAssetManager()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MenuState Critical Error: Game or AssetManager pointer is null!");
    } else {
        AssetManager* assets = game_ptr->getAssetManager();
        backgroundTexture_ = assets->getTexture("menu_bg_blue");
        fontTexture_ = assets->getTexture("ui_font_atlas");

        if (!backgroundTexture_) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "MenuState: Background texture 'menu_bg_blue' not found!"); }
        if (!fontTexture_) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "MenuState: Font texture 'ui_font_atlas' not found!");
        } else {
            if (!loadFontDataFromJson("assets/ui/fonts/bluewhitefont.json")) { // Use correct path
                 SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load font map data.");
            }
        }
        // Cursor texture getting removed previously
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState Created with %zu options.", options.size());
}

MenuState::~MenuState() {
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "MenuState Destroyed.");
}

// <<< *** UPDATED handle_input FOR NESTED MENUS *** >>>
void MenuState::handle_input() {
    // If this state is not the top state, don't handle input.
    if (game_ptr && game_ptr->getCurrentState() != this) {
        return;
    }

    const Uint8* keys = SDL_GetKeyboardState(NULL);
    static bool esc_pressed_last_frame = false;
    static bool up_pressed_last_frame = false;
    static bool down_pressed_last_frame = false;
    static bool select_pressed_last_frame = false;

    // Exit Menu (Escape or Backspace) - Pops the CURRENT MenuState
    if (keys[SDL_SCANCODE_ESCAPE] || keys[SDL_SCANCODE_BACKSPACE]) {
        if (!esc_pressed_last_frame) {
            SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Exit/Back key pressed in MenuState, requesting popState.");
            if(game_ptr) {
                game_ptr->requestPopState(); // This will pop the current menu (main or sub)
            }
            esc_pressed_last_frame = true;
            return; // Exit input handling for this frame
        }
    } else {
        esc_pressed_last_frame = false;
    }

    // Navigate (Using Up/Down for now, will change for carousel later)
    if (keys[SDL_SCANCODE_UP]) { // Keep Up/Down for vertical list testing
        if (!up_pressed_last_frame && !menuOptions_.empty()) {
             currentSelection_ = (currentSelection_ == 0) ? menuOptions_.size() - 1 : currentSelection_ - 1;
             SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "Menu: Selected option %zu - '%s'", currentSelection_, menuOptions_[currentSelection_].c_str());
             up_pressed_last_frame = true;
        }
    } else {
        up_pressed_last_frame = false;
    }

    if (keys[SDL_SCANCODE_DOWN]) { // Keep Up/Down for vertical list testing
        if (!down_pressed_last_frame && !menuOptions_.empty()) {
            currentSelection_ = (currentSelection_ + 1) % menuOptions_.size();
            SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "Menu: Selected option %zu - '%s'", currentSelection_, menuOptions_[currentSelection_].c_str());
            down_pressed_last_frame = true;
        }
    } else {
        down_pressed_last_frame = false;
    }

    // Handle Selection (Enter key)
    if (keys[SDL_SCANCODE_RETURN]) {
        if (!select_pressed_last_frame && !menuOptions_.empty() && game_ptr) { // Ensure game_ptr is valid
            const std::string& selectedOption = menuOptions_[currentSelection_];
            SDL_LogInfo(SDL_LOG_CATEGORY_INPUT, "Menu: Selected '%s'", selectedOption.c_str());

            // --- NESTED MENU LOGIC ---
            if (selectedOption == "EXIT") {
                 game_ptr->requestPopState(); // Pop current menu

            } else if (selectedOption == "DIGIMON") {
                 // Define options for the Digimon sub-menu
                 std::vector<std::string> digimonSubMenuOptions = {"PARTNER", "STATUS", "EVOLVE", "BACK"}; // Added BACK
                 // Push a new MenuState with these options
                 game_ptr->requestPushState(std::make_unique<MenuState>(game_ptr, digimonSubMenuOptions));
                 SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Pushing Digimon Sub-Menu.");

            } else if (selectedOption == "MAP") {
                 // Define options for the Map sub-menu
                 std::vector<std::string> mapSubMenuOptions = {"VIEW MAP", "TRAVEL", "BACK"}; // Added BACK
                 // Push a new MenuState with these options
                 game_ptr->requestPushState(std::make_unique<MenuState>(game_ptr, mapSubMenuOptions));
                 SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Pushing Map Sub-Menu.");

            } else if (selectedOption == "ITEMS") {
                 // Example: Items sub-menu
                 std::vector<std::string> itemsSubMenuOptions = {"VIEW", "USE", "DROP", "BACK"};
                 game_ptr->requestPushState(std::make_unique<MenuState>(game_ptr, itemsSubMenuOptions));
                 SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Pushing Items Sub-Menu.");

            } else if (selectedOption == "SAVE") {
                 // Example: Save probably performs an action directly
                 SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Action for SAVE not implemented.");
                 // Later: game_ptr->saveGame(); // Or similar

            }
            // Handle options specific to sub-menus IF they don't just pop back
            // The "BACK" option added to the sub-menu vectors above will work if selected,
            // because the logic below will hit the 'else' and do nothing, allowing
            // Escape/Backspace to handle the popping as intended.
            // Or, you could explicitly handle "BACK":
            else if (selectedOption == "BACK") {
                 game_ptr->requestPopState(); // Explicitly pop on selecting "BACK"
                 SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Popping Sub-Menu via BACK selection.");
            }
            // --- Handle actions within sub-menus ---
            else if (selectedOption == "PARTNER") {
                 SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Action for PARTNER not implemented.");
                 // Could push another state: e.g., Partner Select state
            } else if (selectedOption == "STATUS") {
                 SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Action for STATUS not implemented.");
                 // Could push another state: e.g., Status Display state
            }
            // Add handlers for other sub-menu options like "EVOLVE", "VIEW MAP", "TRAVEL", "VIEW", "USE", "DROP" etc.
            else {
                // Option not explicitly handled above (could be from a sub-menu we don't have an action for yet)
                SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "Selected option '%s' has no defined action yet.", selectedOption.c_str());
            }
            // --- END NESTED MENU LOGIC ---

            select_pressed_last_frame = true; // Prevent immediate re-trigger
        }
    } else {
        select_pressed_last_frame = false;
    }
}
// <<< *** END UPDATED handle_input *** >>>


void MenuState::update(float delta_time) {
    // Nothing state-specific to update here yet
}


// (render function remains unchanged from previous version)
void MenuState::render() {
    if (!game_ptr) return;
    PCDisplay* display = game_ptr->get_display();
    if (!display) return;
    SDL_Renderer* renderer = display->getRenderer();
    if (!renderer) return;
    int windowW = 0; int windowH = 0;
    display->getWindowSize(windowW, windowH);
    if (windowW <= 0 || windowH <= 0) { windowW = 466; windowH = 466; } // Fallback

    // 1. Draw Background
    if (backgroundTexture_) { SDL_RenderCopy(renderer, backgroundTexture_, NULL, NULL); }
    else { SDL_SetRenderDrawColor(renderer, 0, 0, 80, 255); SDL_RenderClear(renderer); }

    // 2. Draw Currently Selected Menu Option (Centered and Scaled)
    if (fontTexture_ && !fontCharMap_.empty() && !menuOptions_.empty()) {
        const float textScale = 0.9f; // Your adjusted scale
        const int kerning = -10;      // Your adjusted kerning
        const std::string& selectedText = menuOptions_[currentSelection_];
        SDL_Point baseDimensions = getTextDimensions(selectedText, kerning);
        if (baseDimensions.x > 0 && baseDimensions.y > 0) {
            int scaledTextWidth = static_cast<int>(static_cast<float>(baseDimensions.x) * textScale);
            int scaledTextHeight = static_cast<int>(static_cast<float>(baseDimensions.y) * textScale);
            int startX = (windowW / 2) - (scaledTextWidth / 2);
            int startY = (windowH / 2) - (scaledTextHeight / 2);
            drawText(renderer, selectedText, startX, startY, textScale, kerning);
        } else { SDL_LogError(SDL_LOG_CATEGORY_RENDER, "MenuState::render - Failed to get valid dimensions for text: %s", selectedText.c_str()); }
    } else { /* Log warnings */ }
}


// --- JSON Font Map Loading Function ---
// (remains unchanged)
bool MenuState::loadFontDataFromJson(const std::string& jsonPath) {
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Attempting to load font data from: %s", jsonPath.c_str());
     fontCharMap_.clear();
     std::ifstream f(jsonPath);
     if (!f.is_open()) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open font JSON file: %s", jsonPath.c_str()); return false; }
     try {
         nlohmann::json data = nlohmann::json::parse(f);
         f.close();
         if (!data.contains("frames") || !data["frames"].is_object()) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Font JSON missing 'frames' object or it's not an object: %s", jsonPath.c_str()); return false; }
         const auto& frames = data["frames"];
         for (auto& [key, value] : frames.items()) {
             if (!value.contains("frame") || !value["frame"].is_object()) { continue; }
             const auto& frameData = value["frame"];
             if (!(frameData.contains("x") && frameData.contains("y") && frameData.contains("w") && frameData.contains("h"))) { continue; }
             char character = '\0';
             if (key.length() == 1) { character = key[0]; }
             else { // Handle descriptive keys
                 if (key == "QUESTION") character = '?'; else if (key == "apostrophe") character = '\'';
                 else if (key == "colon") character = ':'; else if (key == "comma") character = ',';
                 else if (key == "dash") character = '-'; else if (key == "divide") character = '/';
                 else if (key == "equals") character = '='; else if (key == "exclamation") character = '!';
                 else if (key == "forwardslash") character = '/'; else if (key == "period") character = '.';
                 else if (key == "plus") character = '+'; else if (key == "roundbracketleft") character = '(';
                 else if (key == "roundbracketright") character = ')'; else if (key == "speech") character = '"';
                 else if (key == "times") character = '*'; else if (key == "weirdbracketleft") character = '[';
                 else if (key == "weirdbracketright") character = ']';
                 else { continue; } // Skip unrecognized
             }
             if (character != '\0') { fontCharMap_[character] = SDL_Rect{ frameData.value("x", 0), frameData.value("y", 0), frameData.value("w", 0), frameData.value("h", 0) }; }
         }
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Loaded font map for %zu characters from %s", fontCharMap_.size(), jsonPath.c_str());
         return !fontCharMap_.empty();
     } catch (nlohmann::json::parse_error& e) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to parse font JSON: %s - %s (byte %zu)", jsonPath.c_str(), e.what(), e.byte); if(f.is_open()) f.close(); return false; }
     catch (std::exception& e) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error processing font JSON: %s - %s", jsonPath.c_str(), e.what()); if(f.is_open()) f.close(); return false; }
}

// --- Helper to Calculate Text Dimensions ---
// (remains unchanged)
SDL_Point MenuState::getTextDimensions(const std::string& text, int kerning) {
    SDL_Point dimensions = {0, 0}; int currentX = 0; int maxHeight = 0;
    if (fontCharMap_.empty()) { return dimensions; }
    for (char c : text) {
        int charWidth = 0; int charHeight = 0;
        if (c == ' ') { auto it_space = fontCharMap_.find('A'); charWidth = (it_space != fontCharMap_.end() && it_space->second.w > 0) ? it_space->second.w : 5; charHeight = (it_space != fontCharMap_.end() && it_space->second.h > 0) ? it_space->second.h : 8; }
        else { auto it = fontCharMap_.find(c); if (it != fontCharMap_.end()) { const SDL_Rect& srcRect = it->second; if (srcRect.w > 0 && srcRect.h > 0) { charWidth = srcRect.w; charHeight = srcRect.h; } else { charWidth = 5; charHeight = 8; } } else { charWidth = 5; charHeight = 8; } }
        currentX += charWidth + kerning; if (charHeight > maxHeight) { maxHeight = charHeight; }
    }
    dimensions.x = !text.empty() ? (currentX - kerning) : 0; if (dimensions.x < 0) dimensions.x = 0; dimensions.y = maxHeight; return dimensions;
}


// --- Modified text drawing function with Scaling ---
// (remains unchanged)
void MenuState::drawText(SDL_Renderer* renderer, const std::string& text, int startX, int startY, float scale, int kerning) {
     if (!fontTexture_ || fontCharMap_.empty() || !renderer || scale <= 0.0f) { return; }
     int currentX = startX; float scaledKerning = static_cast<float>(kerning) * scale;
     for (char c : text) {
         int charWidth = 0; int charHeight = 0; const SDL_Rect* pSrcRect = nullptr;
         if (c == ' ') { auto it_space = fontCharMap_.find('A'); charWidth = (it_space != fontCharMap_.end() && it_space->second.w > 0) ? it_space->second.w : 5; currentX += static_cast<int>(static_cast<float>(charWidth) * scale + scaledKerning); continue; }
         else { auto it = fontCharMap_.find(c); if (it != fontCharMap_.end()) { if(it->second.w > 0 && it->second.h > 0) { pSrcRect = &it->second; charWidth = pSrcRect->w; charHeight = pSrcRect->h; } else { charWidth = 5; charHeight = 8; } } else { charWidth = 5; charHeight = 8; } }
         int scaledW = static_cast<int>(static_cast<float>(charWidth) * scale); int scaledH = static_cast<int>(static_cast<float>(charHeight) * scale);
         if (pSrcRect != nullptr && scaledW > 0 && scaledH > 0) { SDL_Rect destRect = { currentX, startY, scaledW, scaledH }; SDL_RenderCopy(renderer, fontTexture_, pSrcRect, &destRect); }
         currentX += scaledW + static_cast<int>(scaledKerning);
     } // End for loop
}