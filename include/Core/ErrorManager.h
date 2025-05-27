#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <chrono>

/**
 * ErrorManager - Centralized error handling and graceful degradation system
 * 
 * Features:
 * - Hierarchical error categories and severity levels
 * - Automatic error recovery strategies
 * - Performance impact monitoring
 * - Detailed error reporting and logging
 */
class ErrorManager {
public:
    enum class ErrorLevel {
        INFO = 0,       // Informational messages
        WARNING = 1,    // Warnings that don't affect functionality
        ERROR = 2,      // Errors that may affect functionality
        CRITICAL = 3,   // Critical errors that require immediate attention
        FATAL = 4       // Fatal errors that should terminate the application
    };
    
    enum class ErrorCategory {
        GENERAL = 0,
        GRAPHICS = 1,
        AUDIO = 2,
        INPUT = 3,
        FILESYSTEM = 4,
        NETWORK = 5,
        ANIMATION = 6,
        ASSET_LOADING = 7,
        STATE_MANAGEMENT = 8,
        PERFORMANCE = 9
    };
    
    struct ErrorInfo {
        ErrorLevel level;
        ErrorCategory category;
        std::string message;
        std::string context;
        std::string file;
        int line;
        std::chrono::system_clock::time_point timestamp;
        int count; // Number of times this error occurred
        
        ErrorInfo(ErrorLevel lvl, ErrorCategory cat, const std::string& msg, 
                 const std::string& ctx = "", const std::string& f = "", int l = 0)
            : level(lvl), category(cat), message(msg), context(ctx), file(f), line(l)
            , timestamp(std::chrono::system_clock::now()), count(1) {}
    };
    
    using ErrorHandler = std::function<bool(const ErrorInfo&)>; // Return true if handled
    using RecoveryStrategy = std::function<bool()>; // Return true if recovery successful

private:
    struct ErrorStats {
        int totalErrors;
        int errorsByLevel[5]; // Index by ErrorLevel
        int errorsByCategory[10]; // Index by ErrorCategory
        float errorRate; // Errors per second
        std::chrono::system_clock::time_point lastReset;
    };
    
    std::vector<ErrorInfo> errorHistory_;
    std::unordered_map<ErrorCategory, ErrorHandler> categoryHandlers_;
    std::unordered_map<ErrorLevel, ErrorHandler> levelHandlers_;
    std::unordered_map<std::string, RecoveryStrategy> recoveryStrategies_;
    
    ErrorStats stats_;
    size_t maxHistorySize_;
    bool enableRecovery_;
    bool enableLogging_;
    std::string logFilePath_;
    
    // Error suppression (to prevent spam)
    std::unordered_map<std::string, std::chrono::system_clock::time_point> suppressionMap_;
    std::chrono::seconds suppressionDuration_;

public:
    ErrorManager(size_t maxHistory = 1000, bool enableRecovery = true);
    ~ErrorManager();
    
    // Error reporting
    void reportError(ErrorLevel level, ErrorCategory category, 
                    const std::string& message, const std::string& context = "",
                    const std::string& file = "", int line = 0);
    
    // Convenience macros for error reporting
    #define REPORT_ERROR(level, category, message) \
        reportError(level, category, message, "", __FILE__, __LINE__)
    
    #define REPORT_ERROR_CTX(level, category, message, context) \
        reportError(level, category, message, context, __FILE__, __LINE__)
    
    // Handler registration
    void registerCategoryHandler(ErrorCategory category, ErrorHandler handler);
    void registerLevelHandler(ErrorLevel level, ErrorHandler handler);
    void registerRecoveryStrategy(const std::string& errorType, RecoveryStrategy strategy);
    
    // Configuration
    void setMaxHistorySize(size_t size);
    void setRecoveryEnabled(bool enabled) { enableRecovery_ = enabled; }
    void setLoggingEnabled(bool enabled, const std::string& logFile = "");
    void setSuppressionDuration(std::chrono::seconds duration) { suppressionDuration_ = duration; }
    
    // Information and diagnostics
    const std::vector<ErrorInfo>& getErrorHistory() const { return errorHistory_; }
    const ErrorStats& getErrorStats() const { return stats_; }
    std::vector<ErrorInfo> getErrorsByLevel(ErrorLevel level) const;
    std::vector<ErrorInfo> getErrorsByCategory(ErrorCategory category) const;
    bool hasRecentErrors(ErrorLevel minLevel = ErrorLevel::ERROR, 
                        std::chrono::seconds timeWindow = std::chrono::seconds(60)) const;
    
    // Utility
    void clearHistory();
    void resetStats();
    std::string formatErrorInfo(const ErrorInfo& error) const;
    void dumpErrorReport(const std::string& filename = "") const;
    
    // Static utility functions
    static std::string errorLevelToString(ErrorLevel level);
    static std::string errorCategoryToString(ErrorCategory category);
    static ErrorLevel stringToErrorLevel(const std::string& str);
    static ErrorCategory stringToErrorCategory(const std::string& str);

private:
    void updateStats();
    bool shouldSuppressError(const std::string& errorKey);
    void writeToLog(const ErrorInfo& error);
    void tryRecovery(const ErrorInfo& error);
    std::string generateErrorKey(const ErrorInfo& error) const;
    void trimHistory();
};
