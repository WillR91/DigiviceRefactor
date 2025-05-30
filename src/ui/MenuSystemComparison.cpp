#include "ui/MenuSystemComparison.h"
#include "states/MenuState.h"
#include "states/EnhancedMenuState.h"
#include "core/Game.h"
#include <SDL_log.h>

MenuState* MenuSystemComparison::createOldStyleMenu(Game* game, const std::vector<std::string>& options) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Creating menu using OLD manual positioning system");
    return new MenuState(game, options);
}

EnhancedMenuState* MenuSystemComparison::createNewStyleMenu(Game* game, const std::vector<std::string>& options) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Creating menu using NEW structured UI framework");
    return new EnhancedMenuState(game, options);
}

std::vector<std::string> MenuSystemComparison::getMainMenuOptions() {
    return {"MAP", "DIGIMON", "DEBUG"};
}

std::vector<std::string> MenuSystemComparison::getDebugMenuOptions() {
    return {"TEST DIGIMON", "SETTINGS", "BACK"};
}

namespace UIExamples {
    
    void createSimpleMenu() {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, 
                   "Example: Simple Menu Creation\n"
                   "=============================\n"
                   "// Create the main screen\n"
                   "auto screen = std::make_unique<DigiviceScreen>(game, \"menu_bg_blue\");\n"
                   "\n"
                   "// Create menu list\n" 
                   "auto menuList = std::make_shared<MenuList>(x, y, width, height, textRenderer);\n"
                   "menuList->setItems({\"Option 1\", \"Option 2\", \"Option 3\"});\n"
                   "\n"
                   "// Add to screen\n"
                   "screen->addChild(menuList);");
    }
    
    void createStyledMenu() {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                   "Example: Styled Menu Creation\n"
                   "============================\n"
                   "// Create menu with custom styling\n"
                   "auto menuList = std::make_shared<MenuList>(x, y, width, height, textRenderer);\n"
                   "menuList->setItems(options);\n"
                   "\n"
                   "// Configure appearance\n"
                   "menuList->setAlignment(MenuList::Alignment::Center);\n"
                   "menuList->setTextScale(0.9f);\n"
                   "menuList->setTextKerning(-15);\n"
                   "menuList->setItemSpacing(20);\n"
                   "\n"
                   "// Set colors\n"
                   "menuList->setTextColor(255, 255, 255, 255);        // White\n"
                   "menuList->setSelectedTextColor(255, 255, 0, 255);  // Yellow\n"
                   "\n"
                   "// Setup cursor\n"
                   "menuList->setCursorTexture(cursorTexture, 16, 16);\n"
                   "menuList->setCursorOffset(-25, 0);");
    }
    
    void createNestedMenus() {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                   "Example: Nested Menu Creation\n"
                   "=============================\n"
                   "// Main menu\n"
                   "auto mainMenu = std::make_unique<EnhancedMenuState>(game, mainOptions);\n"
                   "\n"
                   "// Handle selection with callback\n"
                   "menuList->setSelectionCallback([game](int index, const std::string& text) {\n"
                   "    if (text == \"DEBUG\") {\n"
                   "        // Create and push submenu\n"
                   "        std::vector<std::string> debugOptions = {\"TEST 1\", \"TEST 2\", \"BACK\"};\n"
                   "        auto debugMenu = std::make_unique<EnhancedMenuState>(game, debugOptions);\n"
                   "        game->requestPushState(std::move(debugMenu));\n"
                   "    }\n"
                   "});");
    }
}
