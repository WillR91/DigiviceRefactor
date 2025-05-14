// src/utils/ConfigManager.cpp
// Implementation of the configuration management system
// Created by Claude AI - May 14, 2025

#include "utils/ConfigManager.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <SDL_log.h>
#include <chrono>
#include <sstream>

// Initialize static members
std::unique_ptr<ConfigManager> ConfigManager::instance = nullptr;
std::string ConfigManager::configFilePath = "";
bool ConfigManager::isDirty = false;
std::mutex ConfigManager::configMutex;

bool ConfigManager::initialize(const std::string& configPath) {
    std::lock_guard<std::mutex> lock(configMutex);
    
    if (instance) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ConfigManager is already initialized");
        return true;  // Already initialized
    }
    
    // Create a new instance
    instance = std::make_unique<ConfigManager>();
    configFilePath = configPath;
    
    // Try to load the configuration file
    std::ifstream configFile(configFilePath);
    
    if (configFile.is_open()) {
        try {
            // Parse the JSON data
            configFile >> instance->config;
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ConfigManager: Loaded configuration from %s", configFilePath.c_str());
            return true;
        }
        catch (const nlohmann::json::exception& e) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ConfigManager: Failed to parse JSON: %s", e.what());
            instance.reset();  // Clean up on error
            return false;
        }
    }
    else {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ConfigManager: Configuration file not found at %s", configFilePath.c_str());
        
        // Create an empty configuration
        instance->config = nlohmann::json::object();
        
        // Check if directory exists and create it if needed
        std::filesystem::path filePath(configFilePath);
        std::filesystem::path directory = filePath.parent_path();
        
        if (!directory.empty() && !std::filesystem::exists(directory)) {
            try {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ConfigManager: Creating directory: %s", directory.string().c_str());
                std::filesystem::create_directories(directory);
            }
            catch (const std::filesystem::filesystem_error& e) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ConfigManager: Failed to create directory: %s", e.what());
                instance.reset();
                return false;
            }
        }
        
        // Try to create and write an empty config file
        if (!saveChanges()) {
            instance.reset();
            return false;
        }
        
        return true;
    }
}

void ConfigManager::shutdown() {
    std::lock_guard<std::mutex> lock(configMutex);
    
    if (!instance) {
        return;
    }
    
    // Save changes if there are any
    if (isDirty) {
        saveChanges();
    }
    
    // Release the instance
    instance.reset();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ConfigManager: Shutdown complete");
}

bool ConfigManager::isInitialized() {
    std::lock_guard<std::mutex> lock(configMutex);
    return instance != nullptr;
}

bool ConfigManager::saveChanges() {
    std::lock_guard<std::mutex> lock(configMutex);
    
    if (!instance) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ConfigManager: Cannot save; not initialized");
        return false;
    }
    
    // Open the file for writing
    std::ofstream configFile(configFilePath);
    if (!configFile.is_open()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ConfigManager: Failed to open file for writing: %s", configFilePath.c_str());
        return false;
    }
    
    try {
        // Write the JSON data with pretty formatting (4-space indentation)
        configFile << instance->config.dump(4) << std::endl;
        isDirty = false;
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ConfigManager: Saved configuration to %s", configFilePath.c_str());
        return true;
    }
    catch (const std::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ConfigManager: Failed to write configuration: %s", e.what());
        return false;
    }
}

bool ConfigManager::createBackup(const std::string& backupPath) {
    std::lock_guard<std::mutex> lock(configMutex);
    
    if (!instance) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ConfigManager: Cannot create backup; not initialized");
        return false;
    }
    
    std::string targetPath;
    
    if (backupPath.empty()) {
        // Generate backup filename with timestamp
        auto now = std::chrono::system_clock::now();
        auto timeT = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << configFilePath << ".backup_" << std::put_time(std::localtime(&timeT), "%Y%m%d_%H%M%S");
        targetPath = ss.str();
    }
    else {
        targetPath = backupPath;
    }
    
    try {
        // Write the JSON data to backup file
        std::ofstream backupFile(targetPath);
        if (!backupFile.is_open()) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ConfigManager: Failed to open backup file: %s", targetPath.c_str());
            return false;
        }
        
        backupFile << instance->config.dump(4) << std::endl;
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ConfigManager: Created backup at %s", targetPath.c_str());
        return true;
    }
    catch (const std::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ConfigManager: Failed to create backup: %s", e.what());
        return false;
    }
}

bool ConfigManager::setValue(const std::string& key, const nlohmann::json& value) {
    std::lock_guard<std::mutex> lock(configMutex);
    
    if (!instance || key.empty()) {
        return false;
    }
    
    // Parse dot notation (e.g., "display.width")
    auto keys = parseKey(key);
    
    // Build the path to the destination in the JSON structure
    nlohmann::json* current = &instance->config;
    
    for (size_t i = 0; i < keys.size() - 1; ++i) {
        // Create nested objects if they don't exist
        if (!current->contains(keys[i]) || !(*current)[keys[i]].is_object()) {
            (*current)[keys[i]] = nlohmann::json::object();
        }
        current = &(*current)[keys[i]];
    }
    
    // Set the value at the final key
    const auto& lastKey = keys.back();
    (*current)[lastKey] = value;
    isDirty = true;
    
    return true;
}

bool ConfigManager::hasKey(const std::string& key) {
    std::lock_guard<std::mutex> lock(configMutex);
    
    if (!instance || key.empty()) {
        return false;
    }
    
    // Parse dot notation (e.g., "display.width")
    auto keys = parseKey(key);
    auto* current = &instance->config;
    
    // Traverse the JSON object
    for (size_t i = 0; i < keys.size() - 1; ++i) {
        if (!current->contains(keys[i]) || !(*current)[keys[i]].is_object()) {
            return false;
        }
        current = &(*current)[keys[i]];
    }
    
    // Check if the final key exists
    return current->contains(keys.back());
}

bool ConfigManager::reload() {
    std::lock_guard<std::mutex> lock(configMutex);
    
    if (!instance) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ConfigManager: Cannot reload; not initialized");
        return false;
    }
    
    // Save changes if needed
    if (isDirty) {
        // Create a backup of the current unsaved changes
        createBackup();
    }
    
    // Try to load the configuration file
    std::ifstream configFile(configFilePath);
    
    if (configFile.is_open()) {
        try {
            // Parse the JSON data
            nlohmann::json newConfig;
            configFile >> newConfig;
            
            // Replace the current configuration
            instance->config = newConfig;
            isDirty = false;
            
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ConfigManager: Reloaded configuration from %s", configFilePath.c_str());
            return true;
        }
        catch (const nlohmann::json::exception& e) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ConfigManager: Failed to parse JSON during reload: %s", e.what());
            return false;
        }
    }
    else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ConfigManager: Failed to open configuration file for reload: %s", configFilePath.c_str());
        return false;
    }
}

std::vector<std::string> ConfigManager::parseKey(const std::string& key) {
    std::vector<std::string> keys;
    std::string current;
    
    for (char c : key) {
        if (c == '.') {
            if (!current.empty()) {
                keys.push_back(current);
                current.clear();
            }
        }
        else {
            current += c;
        }
    }
    
    if (!current.empty()) {
        keys.push_back(current);
    }
    
    // If no valid keys were found, use the original key as a single key
    if (keys.empty()) {
        keys.push_back(key);
    }
    
    return keys;
}
