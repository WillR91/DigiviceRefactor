#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <chrono>

// Simplified version of the variant manager for testing
class TestBackgroundVariantManager {
public:
    static const std::string NEW_ASSET_BASE_PATH;
    
    static std::vector<std::string> generateVariantPaths(const std::string& environmentName,
                                                         const std::string& layer,
                                                         int maxVariants) {
        std::vector<std::string> variants;
        
        // Map environment names to their directory structure
        std::string continentPath;
        if (environmentName == "tropicaljungle") {
            continentPath = "01_file_island/01_tropicaljungle/";
        } else {
            continentPath = "01_file_island/01_tropicaljungle/"; // Fallback
        }
        
        // Generate paths based on the new naming convention
        for (int i = 1; i <= maxVariants; ++i) {
            std::string variantPath = NEW_ASSET_BASE_PATH + continentPath + environmentName + "_" + layer + "_v" + std::to_string(i) + ".png";
            variants.push_back(variantPath);
        }
        
        return variants;
    }
};

const std::string TestBackgroundVariantManager::NEW_ASSET_BASE_PATH = "assets/backgrounds/environmentsnew/";

int main() {
    std::cout << "Testing variant path generation for tropicaljungle:" << std::endl;
    
    // Test foreground paths
    auto fgPaths = TestBackgroundVariantManager::generateVariantPaths("tropicaljungle", "fg", 3);
    std::cout << "\nForeground paths:" << std::endl;
    for (const auto& path : fgPaths) {
        std::cout << "  " << path << std::endl;
    }
    
    // Test middleground paths  
    auto mgPaths = TestBackgroundVariantManager::generateVariantPaths("tropicaljungle", "mg", 3);
    std::cout << "\nMiddleground paths:" << std::endl;
    for (const auto& path : mgPaths) {
        std::cout << "  " << path << std::endl;
    }
    
    // Test background paths
    auto bgPaths = TestBackgroundVariantManager::generateVariantPaths("tropicaljungle", "bg", 10);
    std::cout << "\nBackground paths:" << std::endl;
    for (const auto& path : bgPaths) {
        std::cout << "  " << path << std::endl;
    }
    
    return 0;
}
