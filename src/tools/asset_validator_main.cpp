#include "core/AssetManager.h"
#include "tools/AssetValidator.h"
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <string>

/**
 * @brief Asset validation utility for DigiviceRefactor
 * 
 * This utility can be used during development to validate all game assets,
 * find missing files, and suggest alternatives. It can be run standalone
 * or integrated into the build process.
 * 
 * Usage:
 *   asset_validator [--assets-path <path>] [--config <config.json>] [--fix] [--report <output.txt>]
 */

void printUsage(const char* programName) {
    std::cout << "Asset Validation Utility for DigiviceRefactor\n\n";
    std::cout << "Usage: " << programName << " [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --assets-path <path>  Path to assets directory (default: ./assets)\n";
    std::cout << "  --config <file>       Path to game config file (default: ./assets/config/game_config.json)\n";
    std::cout << "  --fix                 Attempt to auto-fix missing asset paths\n";
    std::cout << "  --report <file>       Save validation report to file\n";
    std::cout << "  --category <name>     Validate only specific category (digimon, ui, backgrounds, etc.)\n";
    std::cout << "  --help                Show this help message\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << programName << " --assets-path ./assets --report validation.txt\n";
    std::cout << "  " << programName << " --category digimon --fix\n";
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    std::string assetsPath = "./assets";
    std::string configPath = "./assets/config/game_config.json";
    std::string reportPath = "";
    std::string category = "";
    bool autoFix = false;
    bool showHelp = false;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--help") {
            showHelp = true;
        } else if (arg == "--assets-path" && i + 1 < argc) {
            assetsPath = argv[++i];
        } else if (arg == "--config" && i + 1 < argc) {
            configPath = argv[++i];
        } else if (arg == "--report" && i + 1 < argc) {
            reportPath = argv[++i];
        } else if (arg == "--category" && i + 1 < argc) {
            category = argv[++i];
        } else if (arg == "--fix") {
            autoFix = true;
        } else {
            std::cout << "Unknown option: " << arg << std::endl;
            showHelp = true;
        }
    }
    
    if (showHelp) {
        printUsage(argv[0]);
        return 0;
    }
    
    std::cout << "DigiviceRefactor Asset Validator\n";
    std::cout << "===============================\n\n";
    
    // Initialize SDL (minimal setup for texture operations)
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    // Create a dummy window and renderer for texture operations
    SDL_Window* window = SDL_CreateWindow("Asset Validator", 
                                        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                        1, 1, SDL_WINDOW_HIDDEN);
    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Initialize asset manager
    AssetManager assetManager;
    if (!assetManager.init(renderer)) {
        std::cerr << "Failed to initialize AssetManager" << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Enable fallback textures for validation
    assetManager.enableFallbackTextures(true);
    
    // Initialize asset validator
    AssetValidator validator;
    validator.setAssetManager(&assetManager);
    
    // Load configuration if available
    std::cout << "Loading configuration from: " << configPath << std::endl;
    if (!validator.loadAssetConfiguration(configPath)) {
        std::cout << "Warning: Could not load configuration file. Proceeding with auto-discovery.\n" << std::endl;
    }
    
    // Auto-discover assets
    std::cout << "Discovering assets in: " << assetsPath << std::endl;
    validator.autoDiscoverAssets(assetsPath);
    
    // Perform validation
    AssetValidator::ValidationReport report;
    
    if (!category.empty()) {
        std::cout << "Validating category: " << category << std::endl;
        report = validator.validateCategory(category);
    } else {
        std::cout << "Performing full validation..." << std::endl;
        report = validator.performFullValidation();
    }
    
    // Auto-fix if requested
    if (autoFix) {
        std::cout << "\nAttempting to auto-fix asset paths..." << std::endl;
        int fixedCount = validator.autoFixAssetPaths(false);
        
        if (fixedCount > 0) {
            std::cout << "Fixed " << fixedCount << " asset paths. Re-running validation..." << std::endl;
            
            // Re-run validation after fixes
            if (!category.empty()) {
                report = validator.validateCategory(category);
            } else {
                report = validator.performFullValidation();
            }
        }
    }
    
    // Print report
    report.printReport();
    
    // Save report to file if requested
    if (!reportPath.empty()) {
        std::cout << "\nSaving report to: " << reportPath << std::endl;
        report.saveToFile(reportPath);
    }
    
    // Cleanup
    assetManager.shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    // Return appropriate exit code
    int exitCode = report.overallResult.hasIssues() ? 1 : 0;
    
    if (exitCode == 0) {
        std::cout << "\n✅ All assets validated successfully!" << std::endl;
    } else {
        std::cout << "\n❌ Asset validation found issues. Check the report above." << std::endl;
    }
    
    return exitCode;
}
