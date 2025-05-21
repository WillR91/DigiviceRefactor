#pragma once

#include <string>
#include <map>
#include <memory>
#include "entities/DigimonDefinition.h"

namespace Digimon {

class DigimonRegistry {
public:
    // Singleton pattern
    static DigimonRegistry& getInstance();
    
    // Initialize the registry
    bool initialize(const std::string& definitionsPath);
    
    // Get a Digimon definition by ID
    const DigimonDefinition* getDefinitionById(const std::string& id) const;
    
    // Get a Digimon definition by legacy enum value (for transition period)
    const DigimonDefinition* getDefinitionByLegacyType(DigimonType type) const;
    
    // Get all definitions of a specific class
    std::vector<const DigimonDefinition*> getDefinitionsByClass(DigimonClass digimonClass) const;
    
    // Get a random Digimon of a specific class
    const DigimonDefinition* getRandomDigimonByClass(DigimonClass digimonClass) const;
    
    // Register an enemy Digimon with default stats and animations
    void registerEnemyDigimon(const std::string& id, const std::string& displayName, const std::string& description, 
                              int hp = 180, int attack = 18, int defense = 15, int speed = 12);
    
    // Clear all loaded definitions
    void clear();
    
private:
    // Private constructor for singleton
    DigimonRegistry() = default;
    
    // Load a single Digimon definition file
    bool loadDefinitionFromFile(const std::string& filePath);
    
    // Load all definition files from a directory
    bool loadDefinitionsFromDirectory(const std::string& directoryPath);
    
    // Map of Digimon definitions, keyed by ID
    std::map<std::string, DigimonDefinition> definitions_;
    
    // Map from legacy enum to definition ID for backward compatibility
    std::map<DigimonType, std::string> legacyEnumToId_;
    
    // Path to the definitions directory
    std::string definitionsPath_;

    // Delete copy constructor and assignment operator
    DigimonRegistry(const DigimonRegistry&) = delete;
    DigimonRegistry& operator=(const DigimonRegistry&) = delete;
};

} // namespace Digimon
