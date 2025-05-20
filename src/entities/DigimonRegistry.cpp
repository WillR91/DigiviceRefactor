#include "entities/DigimonRegistry.h"
#include <SDL_log.h>
#include <fstream>
#include <filesystem>
#include <random>
#include "vendor/nlohmann/json.hpp"
#include "utils/AnimationUtils.h"

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace Digimon {

DigimonRegistry& DigimonRegistry::getInstance() {
    static DigimonRegistry instance;
    return instance;
}

bool DigimonRegistry::initialize(const std::string& definitionsPath) {
    clear();
    definitionsPath_ = definitionsPath;
    
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Initializing DigimonRegistry from path: %s", definitionsPath_.c_str());
    
    // First, try loading from a directory
    if (loadDefinitionsFromDirectory(definitionsPath_)) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Successfully loaded Digimon definitions from directory.");
        return true;
    }
    
    // If directory loading fails, try loading a single JSON file with all definitions
    if (loadDefinitionFromFile(definitionsPath_)) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Successfully loaded Digimon definitions from file.");
        return true;
    }
    
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load Digimon definitions from %s", definitionsPath_.c_str());
    return false;
}

const DigimonDefinition* DigimonRegistry::getDefinitionById(const std::string& id) const {
    auto it = definitions_.find(id);
    if (it != definitions_.end()) {
        return &it->second;
    }
    
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "DigimonRegistry: No definition found for ID: %s", id.c_str());
    return nullptr;
}

const DigimonDefinition* DigimonRegistry::getDefinitionByLegacyType(DigimonType type) const {
    auto it = legacyEnumToId_.find(type);
    if (it != legacyEnumToId_.end()) {
        return getDefinitionById(it->second);
    }
    
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "DigimonRegistry: No definition found for legacy type: %d", static_cast<int>(type));
    return nullptr;
}

std::vector<const DigimonDefinition*> DigimonRegistry::getDefinitionsByClass(DigimonClass digimonClass) const {
    std::vector<const DigimonDefinition*> result;
    
    for (const auto& [id, definition] : definitions_) {
        if (definition.digimonClass == digimonClass) {
            result.push_back(&definition);
        }
    }
    
    return result;
}

const DigimonDefinition* DigimonRegistry::getRandomDigimonByClass(DigimonClass digimonClass) const {
    auto digimonList = getDefinitionsByClass(digimonClass);
    
    if (digimonList.empty()) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "DigimonRegistry: No Digimon found for class: %s", 
                   digimonClassToString(digimonClass).c_str());
        return nullptr;
    }
    
    // Use random device for better randomness
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, digimonList.size() - 1);
    
    return digimonList[distrib(gen)];
}

void DigimonRegistry::clear() {
    definitions_.clear();
    legacyEnumToId_.clear();
}

bool DigimonRegistry::loadDefinitionFromFile(const std::string& filePath) {
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DigimonRegistry: Failed to open file: %s", filePath.c_str());
            return false;
        }
        
        json jsonData = json::parse(file);
        file.close();
        
        // Check if this is a single Digimon definition or an array
        if (jsonData.contains("id") && jsonData.contains("displayName")) {
            // Single definition
            DigimonDefinition definition;
            definition.id = jsonData["id"].get<std::string>();
            definition.displayName = jsonData["displayName"].get<std::string>();
            
            // Parse class
            if (jsonData.contains("class")) {
                definition.digimonClass = stringToDigimonClass(jsonData["class"].get<std::string>());
            }
            
            // Parse stats
            if (jsonData.contains("stats")) {
                const auto& stats = jsonData["stats"];
                definition.stats.hp = stats.value("hp", 100);
                definition.stats.attack = stats.value("attack", 10);
                definition.stats.defense = stats.value("defense", 5);
                definition.stats.speed = stats.value("speed", 5);
            }
            
            // Parse sprite base ID
            if (jsonData.contains("spriteBaseId")) {
                definition.spriteBaseId = jsonData["spriteBaseId"].get<std::string>();
            }
            
            // Parse description
            if (jsonData.contains("description")) {
                definition.description = jsonData["description"].get<std::string>();
            }
            
            // Parse animations
            if (jsonData.contains("animations") && jsonData["animations"].is_array()) {
                for (const auto& anim : jsonData["animations"]) {
                    definition.animations.push_back(anim.get<std::string>());
                }
            }
            
            // Parse additional attributes
            if (jsonData.contains("attributes") && jsonData["attributes"].is_object()) {
                for (auto it = jsonData["attributes"].begin(); it != jsonData["attributes"].end(); ++it) {
                    definition.attributes[it.key()] = it.value().get<std::string>();
                }
            }
              // Parse legacy enum mapping
            if (jsonData.contains("legacyEnum")) {
                int enumValue = jsonData["legacyEnum"].get<int>();
                DigimonType legacyType = static_cast<DigimonType>(enumValue);
                legacyEnumToId_[legacyType] = definition.id;
                definition.legacyEnum = legacyType;
            }
            
            // Add the definition to the map
            definitions_[definition.id] = definition;
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "DigimonRegistry: Loaded definition for '%s'", definition.displayName.c_str());
            
            return true;
        } 
        else if (jsonData.is_array()) {
            // Array of definitions
            bool success = true;
            for (const auto& digimonJson : jsonData) {
                if (!digimonJson.contains("id") || !digimonJson.contains("displayName")) {
                    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "DigimonRegistry: Invalid Digimon definition in array");
                    success = false;
                    continue;
                }
                
                DigimonDefinition definition;
                definition.id = digimonJson["id"].get<std::string>();
                definition.displayName = digimonJson["displayName"].get<std::string>();
                
                // Parse class
                if (digimonJson.contains("class")) {
                    definition.digimonClass = stringToDigimonClass(digimonJson["class"].get<std::string>());
                }
                
                // Parse stats
                if (digimonJson.contains("stats")) {
                    const auto& stats = digimonJson["stats"];
                    definition.stats.hp = stats.value("hp", 100);
                    definition.stats.attack = stats.value("attack", 10);
                    definition.stats.defense = stats.value("defense", 5);
                    definition.stats.speed = stats.value("speed", 5);
                }
                
                // Parse sprite base ID
                if (digimonJson.contains("spriteBaseId")) {
                    definition.spriteBaseId = digimonJson["spriteBaseId"].get<std::string>();
                }
                
                // Parse description
                if (digimonJson.contains("description")) {
                    definition.description = digimonJson["description"].get<std::string>();
                }
                
                // Parse animations
                if (digimonJson.contains("animations") && digimonJson["animations"].is_array()) {
                    for (const auto& anim : digimonJson["animations"]) {
                        definition.animations.push_back(anim.get<std::string>());
                    }
                }
                
                // Parse additional attributes
                if (digimonJson.contains("attributes") && digimonJson["attributes"].is_object()) {
                    for (auto it = digimonJson["attributes"].begin(); it != digimonJson["attributes"].end(); ++it) {
                        definition.attributes[it.key()] = it.value().get<std::string>();
                    }
                }
                
                // Parse legacy enum mapping
                if (digimonJson.contains("legacyEnum")) {
                    int enumValue = digimonJson["legacyEnum"].get<int>();
                    DigimonType legacyType = static_cast<DigimonType>(enumValue);
                    legacyEnumToId_[legacyType] = definition.id;
                }
                
                // Add the definition to the map
                definitions_[definition.id] = definition;
                SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "DigimonRegistry: Loaded definition for '%s'", definition.displayName.c_str());
            }
            
            return success && !definitions_.empty();
        }
        else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DigimonRegistry: Invalid JSON format in file: %s", filePath.c_str());
            return false;
        }
    }
    catch (const std::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DigimonRegistry: Exception while parsing file %s: %s", filePath.c_str(), e.what());
        return false;
    }
}

bool DigimonRegistry::loadDefinitionsFromDirectory(const std::string& directoryPath) {
    try {
        if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DigimonRegistry: Directory not found or not a directory: %s", directoryPath.c_str());
            return false;
        }
        
        bool success = true;
        int loadedCount = 0;
        
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                if (loadDefinitionFromFile(entry.path().string())) {
                    loadedCount++;
                } else {
                    success = false;
                }
            }
        }
        
        if (loadedCount == 0) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "DigimonRegistry: No Digimon definition files found in: %s", directoryPath.c_str());
            return false;
        }
        
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "DigimonRegistry: Loaded %d Digimon definitions", loadedCount);
        return success && loadedCount > 0;
    }
    catch (const std::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DigimonRegistry: Exception while loading directory %s: %s", directoryPath.c_str(), e.what());
        return false;
    }
}

// String conversion utilities implementation
std::string digimonClassToString(DigimonClass digimonClass) {
    switch (digimonClass) {
        case DigimonClass::PlayerRookie:  return "PlayerRookie";
        case DigimonClass::PlayerChampion: return "PlayerChampion";
        case DigimonClass::StandardEnemy: return "StandardEnemy";
        case DigimonClass::Boss:          return "Boss";
        case DigimonClass::NPC:           return "NPC";
        default:                          return "Unknown";
    }
}

DigimonClass stringToDigimonClass(const std::string& classStr) {
    if (classStr == "PlayerRookie")   return DigimonClass::PlayerRookie;
    if (classStr == "PlayerChampion") return DigimonClass::PlayerChampion;
    if (classStr == "StandardEnemy")  return DigimonClass::StandardEnemy;
    if (classStr == "Boss")           return DigimonClass::Boss;
    if (classStr == "NPC")            return DigimonClass::NPC;
    
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "DigimonRegistry: Unknown DigimonClass string: %s", classStr.c_str());
    return DigimonClass::StandardEnemy; // Default
}

} // namespace Digimon
