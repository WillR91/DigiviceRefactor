#include "core/ErrorManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

ErrorManager::ErrorManager(size_t maxHistory, bool enableRecovery)
    : maxHistorySize_(maxHistory)
    , enableRecovery_(enableRecovery)
    , enableLogging_(false)
    , suppressionDuration_(std::chrono::seconds(5))
    , stats_{}
{
    stats_.lastReset = std::chrono::system_clock::now();
}

ErrorManager::~ErrorManager() {
    if (!errorHistory_.empty()) {
        std::cout << "ErrorManager: Shutting down with " << errorHistory_.size() 
                  << " errors in history" << std::endl;
    }
}

void ErrorManager::reportError(ErrorLevel level, ErrorCategory category, 
                              const std::string& message, const std::string& context,
                              const std::string& file, int line) {
    ErrorInfo error(level, category, message, context, file, line);
    
    // Check if this error should be suppressed
    std::string errorKey = generateErrorKey(error);
    if (shouldSuppressError(errorKey)) {
        // Find existing error and increment count
        auto it = std::find_if(errorHistory_.rbegin(), errorHistory_.rend(),
            [&errorKey, this](const ErrorInfo& e) {
                return generateErrorKey(e) == errorKey;
            });
        if (it != errorHistory_.rend()) {
            it->count++;
        }
        return;
    }
    
    // Add to history
    errorHistory_.push_back(error);
    trimHistory();
    
    // Update statistics
    updateStats();
    
    // Log to file if enabled
    if (enableLogging_) {
        writeToLog(error);
    }
    
    // Try category-specific handler first
    bool handled = false;
    auto categoryIt = categoryHandlers_.find(category);
    if (categoryIt != categoryHandlers_.end()) {
        handled = categoryIt->second(error);
    }
    
    // Try level-specific handler if not handled
    if (!handled) {
        auto levelIt = levelHandlers_.find(level);
        if (levelIt != levelHandlers_.end()) {
            handled = levelIt->second(error);
        }
    }
    
    // Default handling if not handled by custom handlers
    if (!handled) {
        std::string levelStr = errorLevelToString(level);
        std::string categoryStr = errorCategoryToString(category);
        
        std::cerr << "[" << levelStr << "] [" << categoryStr << "] " << message;
        if (!context.empty()) {
            std::cerr << " (Context: " << context << ")";
        }
        if (!file.empty() && line > 0) {
            std::cerr << " at " << file << ":" << line;
        }
        std::cerr << std::endl;
        
        // For critical and fatal errors, terminate or attempt recovery
        if (level == ErrorLevel::FATAL) {
            std::cerr << "FATAL ERROR: Application will terminate" << std::endl;
            std::exit(1);
        } else if (level == ErrorLevel::CRITICAL && enableRecovery_) {
            tryRecovery(error);
        }
    }
}

void ErrorManager::registerCategoryHandler(ErrorCategory category, ErrorHandler handler) {
    categoryHandlers_[category] = handler;
}

void ErrorManager::registerLevelHandler(ErrorLevel level, ErrorHandler handler) {
    levelHandlers_[level] = handler;
}

void ErrorManager::registerRecoveryStrategy(const std::string& errorType, RecoveryStrategy strategy) {
    recoveryStrategies_[errorType] = strategy;
}

void ErrorManager::setMaxHistorySize(size_t size) {
    maxHistorySize_ = size;
    trimHistory();
}

void ErrorManager::setLoggingEnabled(bool enabled, const std::string& logFile) {
    enableLogging_ = enabled;
    if (enabled && !logFile.empty()) {
        logFilePath_ = logFile;
    } else if (enabled && logFilePath_.empty()) {
        logFilePath_ = "error_log.txt";
    }
}

std::vector<ErrorManager::ErrorInfo> ErrorManager::getErrorsByLevel(ErrorLevel level) const {
    std::vector<ErrorInfo> result;
    std::copy_if(errorHistory_.begin(), errorHistory_.end(), std::back_inserter(result),
                [level](const ErrorInfo& error) { return error.level == level; });
    return result;
}

std::vector<ErrorManager::ErrorInfo> ErrorManager::getErrorsByCategory(ErrorCategory category) const {
    std::vector<ErrorInfo> result;
    std::copy_if(errorHistory_.begin(), errorHistory_.end(), std::back_inserter(result),
                [category](const ErrorInfo& error) { return error.category == category; });
    return result;
}

bool ErrorManager::hasRecentErrors(ErrorLevel minLevel, std::chrono::seconds timeWindow) const {
    auto cutoff = std::chrono::system_clock::now() - timeWindow;
    
    return std::any_of(errorHistory_.begin(), errorHistory_.end(),
                      [minLevel, cutoff](const ErrorInfo& error) {
                          return error.level >= minLevel && error.timestamp >= cutoff;
                      });
}

void ErrorManager::clearHistory() {
    errorHistory_.clear();
    suppressionMap_.clear();
}

void ErrorManager::resetStats() {
    stats_ = {};
    stats_.lastReset = std::chrono::system_clock::now();
}

std::string ErrorManager::formatErrorInfo(const ErrorInfo& error) const {
    std::ostringstream oss;
    
    // Format timestamp
    auto time_t = std::chrono::system_clock::to_time_t(error.timestamp);
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    
    oss << " [" << errorLevelToString(error.level) << "]";
    oss << " [" << errorCategoryToString(error.category) << "]";
    oss << " " << error.message;
    
    if (!error.context.empty()) {
        oss << " (Context: " << error.context << ")";
    }
    
    if (!error.file.empty() && error.line > 0) {
        oss << " at " << error.file << ":" << error.line;
    }
    
    if (error.count > 1) {
        oss << " (occurred " << error.count << " times)";
    }
    
    return oss.str();
}

void ErrorManager::dumpErrorReport(const std::string& filename) const {
    std::string reportFile = filename.empty() ? "error_report.txt" : filename;
    std::ofstream file(reportFile);
    
    if (!file.is_open()) {
        std::cerr << "ErrorManager: Failed to open " << reportFile << " for writing" << std::endl;
        return;
    }    file << "=== ERROR REPORT ===" << std::endl;
    auto now = std::chrono::system_clock::now();
    std::time_t time_t = std::chrono::system_clock::to_time_t(now);
    file << "Generated: " << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << std::endl;
    file << "Total Errors: " << stats_.totalErrors << std::endl;
    file << "Error Rate: " << stats_.errorRate << " errors/second" << std::endl;
    file << std::endl;
    
    file << "=== ERROR BREAKDOWN ===" << std::endl;
    file << "By Level:" << std::endl;
    for (int i = 0; i < 5; ++i) {
        ErrorLevel level = static_cast<ErrorLevel>(i);
        file << "  " << errorLevelToString(level) << ": " << stats_.errorsByLevel[i] << std::endl;
    }
    
    file << "By Category:" << std::endl;
    for (int i = 0; i < 10; ++i) {
        ErrorCategory category = static_cast<ErrorCategory>(i);
        file << "  " << errorCategoryToString(category) << ": " << stats_.errorsByCategory[i] << std::endl;
    }
    
    file << std::endl << "=== ERROR HISTORY ===" << std::endl;
    for (const auto& error : errorHistory_) {
        file << formatErrorInfo(error) << std::endl;
    }
    
    file.close();
    std::cout << "ErrorManager: Error report written to " << reportFile << std::endl;
}

// Static utility functions
std::string ErrorManager::errorLevelToString(ErrorLevel level) {
    switch (level) {
        case ErrorLevel::INFO: return "INFO";
        case ErrorLevel::WARNING: return "WARNING";
        case ErrorLevel::ERROR: return "ERROR";
        case ErrorLevel::CRITICAL: return "CRITICAL";
        case ErrorLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

std::string ErrorManager::errorCategoryToString(ErrorCategory category) {
    switch (category) {
        case ErrorCategory::GENERAL: return "GENERAL";
        case ErrorCategory::GRAPHICS: return "GRAPHICS";
        case ErrorCategory::AUDIO: return "AUDIO";
        case ErrorCategory::INPUT: return "INPUT";
        case ErrorCategory::FILESYSTEM: return "FILESYSTEM";
        case ErrorCategory::NETWORK: return "NETWORK";
        case ErrorCategory::ANIMATION: return "ANIMATION";
        case ErrorCategory::ASSET_LOADING: return "ASSET_LOADING";
        case ErrorCategory::STATE_MANAGEMENT: return "STATE_MANAGEMENT";
        case ErrorCategory::PERFORMANCE: return "PERFORMANCE";
        default: return "UNKNOWN";
    }
}

ErrorManager::ErrorLevel ErrorManager::stringToErrorLevel(const std::string& str) {
    if (str == "INFO") return ErrorLevel::INFO;
    if (str == "WARNING") return ErrorLevel::WARNING;
    if (str == "ERROR") return ErrorLevel::ERROR;
    if (str == "CRITICAL") return ErrorLevel::CRITICAL;
    if (str == "FATAL") return ErrorLevel::FATAL;
    return ErrorLevel::ERROR; // Default
}

ErrorManager::ErrorCategory ErrorManager::stringToErrorCategory(const std::string& str) {
    if (str == "GENERAL") return ErrorCategory::GENERAL;
    if (str == "GRAPHICS") return ErrorCategory::GRAPHICS;
    if (str == "AUDIO") return ErrorCategory::AUDIO;
    if (str == "INPUT") return ErrorCategory::INPUT;
    if (str == "FILESYSTEM") return ErrorCategory::FILESYSTEM;
    if (str == "NETWORK") return ErrorCategory::NETWORK;
    if (str == "ANIMATION") return ErrorCategory::ANIMATION;
    if (str == "ASSET_LOADING") return ErrorCategory::ASSET_LOADING;
    if (str == "STATE_MANAGEMENT") return ErrorCategory::STATE_MANAGEMENT;
    if (str == "PERFORMANCE") return ErrorCategory::PERFORMANCE;
    return ErrorCategory::GENERAL; // Default
}

// Private methods
void ErrorManager::updateStats() {
    stats_.totalErrors = static_cast<int>(errorHistory_.size());
    
    // Reset level and category counts
    std::fill(std::begin(stats_.errorsByLevel), std::end(stats_.errorsByLevel), 0);
    std::fill(std::begin(stats_.errorsByCategory), std::end(stats_.errorsByCategory), 0);
    
    // Count by level and category
    for (const auto& error : errorHistory_) {
        stats_.errorsByLevel[static_cast<int>(error.level)]++;
        stats_.errorsByCategory[static_cast<int>(error.category)]++;
    }
    
    // Calculate error rate
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - stats_.lastReset);
    if (duration.count() > 0) {
        stats_.errorRate = static_cast<float>(stats_.totalErrors) / duration.count();
    }
}

bool ErrorManager::shouldSuppressError(const std::string& errorKey) {
    auto now = std::chrono::system_clock::now();
    auto it = suppressionMap_.find(errorKey);
    
    if (it != suppressionMap_.end()) {
        if (now - it->second < suppressionDuration_) {
            return true; // Still within suppression window
        } else {
            suppressionMap_.erase(it); // Suppression expired
        }
    }
    
    suppressionMap_[errorKey] = now;
    return false;
}

void ErrorManager::writeToLog(const ErrorInfo& error) {
    if (logFilePath_.empty()) return;
    
    std::ofstream logFile(logFilePath_, std::ios::app);
    if (logFile.is_open()) {
        logFile << formatErrorInfo(error) << std::endl;
    }
}

void ErrorManager::tryRecovery(const ErrorInfo& error) {
    std::string errorType = errorCategoryToString(error.category);
    auto it = recoveryStrategies_.find(errorType);
    
    if (it != recoveryStrategies_.end()) {
        std::cout << "ErrorManager: Attempting recovery for " << errorType << " error" << std::endl;
        bool success = it->second();
        if (success) {
            std::cout << "ErrorManager: Recovery successful" << std::endl;
        } else {
            std::cout << "ErrorManager: Recovery failed" << std::endl;
        }
    }
}

std::string ErrorManager::generateErrorKey(const ErrorInfo& error) const {
    return errorCategoryToString(error.category) + ":" + error.message;
}

void ErrorManager::trimHistory() {
    while (errorHistory_.size() > maxHistorySize_) {
        errorHistory_.erase(errorHistory_.begin());
    }
}
