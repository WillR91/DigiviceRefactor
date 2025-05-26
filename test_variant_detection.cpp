#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

// Simple test of the variant detection logic
std::vector<std::string> testGenerateVariantPaths(const std::string& environmentName,
                                                   const std::string& layer,
                                                   int maxVariants) {
    std::vector<std::string> variants;
    const std::string NEW_ASSET_BASE_PATH = "assets/backgrounds/environmentsnew/";
    
    // Map environment names to their directory structure
    std::string continentPath;
    if (environmentName == "tropicaljungle") {
        continentPath = "01_file_island/01_tropicaljungle/";
    } else if (environmentName == "lake") {
        continentPath = "01_file_island/02_lake/";
    } else {
        std::cout << "Unknown environment '" << environmentName << "', using fallback path" << std::endl;
        continentPath = "01_file_island/01_tropicaljungle/"; // Fallback
    }
    
    // Dynamically detect which variants actually exist
    int foundVariants = 0;
    for (int i = 1; i <= maxVariants; ++i) {
        std::string variantPath = NEW_ASSET_BASE_PATH + continentPath + environmentName + "_" + layer + "_v" + std::to_string(i) + ".png";
        
        // Check if the file actually exists
        try {
            if (std::filesystem::exists(variantPath)) {
                variants.push_back(variantPath);
                foundVariants++;
                std::cout << "Found variant " << i << " for " << environmentName << "_" << layer << ": " << variantPath << std::endl;
            } else {
                std::cout << "Variant " << i << " does not exist for " << environmentName << "_" << layer << ": " << variantPath << std::endl;
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cout << "Error checking file existence for " << variantPath << ": " << e.what() << std::endl;
        }
    }
    
    std::cout << "Found " << foundVariants << " variants for " << environmentName << "_" << layer << " (checked up to v" << maxVariants << ")" << std::endl;

    // Fallback: if no variants found, try adding the first variant path anyway for compatibility
    if (variants.empty()) {
        std::string fallbackPath = NEW_ASSET_BASE_PATH + continentPath + environmentName + "_" + layer + "_v1.png";
        variants.push_back(fallbackPath);
        std::cout << "WARNING - No variants found, added fallback path: " << fallbackPath << std::endl;
    }

    return variants;
}

int main() {
    std::cout << "Testing variant detection for tropicaljungle..." << std::endl;
    
    auto fgPaths = testGenerateVariantPaths("tropicaljungle", "fg", 3);
    std::cout << "\nForeground paths generated: " << fgPaths.size() << std::endl;
    for (const auto& path : fgPaths) {
        std::cout << "  " << path << std::endl;
    }
    
    auto mgPaths = testGenerateVariantPaths("tropicaljungle", "mg", 3);
    std::cout << "\nMiddleground paths generated: " << mgPaths.size() << std::endl;
    for (const auto& path : mgPaths) {
        std::cout << "  " << path << std::endl;
    }
    
    auto bgPaths = testGenerateVariantPaths("tropicaljungle", "bg", 10);
    std::cout << "\nBackground paths generated: " << bgPaths.size() << std::endl;
    for (const auto& path : bgPaths) {
        std::cout << "  " << path << std::endl;
    }
    
    return 0;
}
