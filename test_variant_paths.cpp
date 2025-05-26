#include <iostream>
#include <string>
#include <vector>

// Simple test to verify path generation logic
int main() {
    std::string environmentName = "tropicaljungle";
    std::string layer = "bg";
    int maxVariants = 3;
    
    // Map environment names to their directory structure based on actual asset layout
    std::string continentPath;
    if (environmentName == "tropicaljungle") {
        continentPath = "01_file_island/01_tropicaljungle/";
    } else {
        continentPath = "01_file_island/01_tropicaljungle/"; // Fallback
    }
    
    const std::string NEW_ASSET_BASE_PATH = "assets/backgrounds/environmentsnew/";
    
    // Generate paths based on the new naming convention
    std::vector<std::string> variants;
    for (int i = 1; i <= maxVariants; ++i) {
        std::string variantPath = NEW_ASSET_BASE_PATH + continentPath + environmentName + "_" + layer + "_v" + std::to_string(i) + ".png";
        variants.push_back(variantPath);
        std::cout << "Generated path: " << variantPath << std::endl;
    }
    
    return 0;
}
