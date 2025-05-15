// include/utils/ConfigManager.h
// A centralized configuration manager for DigiviceSim
// Created by Claude AI - May 14, 2025

#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include "vendor/nlohmann/json.hpp"

class ConfigManager {
public:
    // Initialize configuration from JSON file
    static bool initialize(const std::string& configPath = "config/game_config.json");
    
    // Free resources and save any pending changes
    static void shutdown();
    
    // Check if ConfigManager has been initialized
    static bool isInitialized();
    
    // Save changes to disk
    static bool saveChanges();
    
    // Create a backup of current config file
    static bool createBackup(const std::string& backupPath = "");
    
    // Get a configuration value with type conversion
    template<typename T>
    static T getValue(const std::string& key, const T& defaultValue = T()) {
        std::lock_guard<std::mutex> lock(configMutex);
        
        if (!instance || key.empty()) {
            return defaultValue;
        }
        
        // Parse dot notation (e.g., "display.width")
        auto keys = parseKey(key);
        auto* current = &instance->config;
        
        // Traverse the JSON object following the key path
        for (size_t i = 0; i < keys.size() - 1; ++i) {
            if (!current->contains(keys[i]) || !(*current)[keys[i]].is_object()) {
                return defaultValue; // Key path doesn't exist or isn't an object
            }
            current = &(*current)[keys[i]];
        }
        
        // Check if the final key exists
        const auto& lastKey = keys.back();
        if (!current->contains(lastKey)) {
            return defaultValue;
        }
        
        // Try to convert value to requested type
        try {
            return (*current)[lastKey].get<T>();
        } catch (const nlohmann::json::exception&) {
            // Type conversion failed, return default
            return defaultValue;
        }
    }
    
    // Set a configuration value
    // Returns true if successful
    static bool setValue(const std::string& key, const nlohmann::json& value);
    
    // Check if a configuration key exists
    static bool hasKey(const std::string& key);
    
    // Reload configuration from file
    static bool reload();

private:
    // Private constructor for singleton
    ConfigManager() = default;
    
    // Allow std::make_unique to access private constructor
    friend std::unique_ptr<ConfigManager> std::make_unique<ConfigManager>();
    
    // Split dot-notated key into vector of keys
    static std::vector<std::string> parseKey(const std::string& key);
    
    // File path of the current configuration
    static std::string configFilePath;
    
    // JSON object holding all configuration data
    nlohmann::json config;
    
    // Singleton instance
    static std::unique_ptr<ConfigManager> instance;
    
    // Flag for tracking if the configuration has been modified
    static bool isDirty;
    
    // Mutex for thread safety
    static std::mutex configMutex;
};
