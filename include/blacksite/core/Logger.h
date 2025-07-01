#pragma once

#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>

namespace Blacksite {

enum class LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
};

enum class LogCategory {
    CORE,
    RENDERER,
    PHYSICS,
    AUDIO,
    INPUT,
    EDITOR,
    GAME,
    NETWORK
};

struct LogEntry {
    LogLevel level;
    LogCategory category;
    std::string message;
    std::string timestamp;
    std::string file;
    int line;
    
    LogEntry(LogLevel lvl, LogCategory cat, const std::string& msg, 
             const std::string& f = "", int l = 0)
        : level(lvl), category(cat), message(msg), file(f), line(l) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        timestamp = ss.str();
    }
};

class Logger {
public:
    static Logger& Instance() {
        static Logger instance;
        return instance;
    }

    // Core logging functions
    void Log(LogLevel level, LogCategory category, const std::string& message,
             const std::string& file = "", int line = 0);
    
    // Convenience functions
    void Trace(LogCategory category, const std::string& message) {
        Log(LogLevel::TRACE, category, message);
    }
    
    void Debug(LogCategory category, const std::string& message) {
        Log(LogLevel::DEBUG, category, message);
    }
    
    void Info(LogCategory category, const std::string& message) {
        Log(LogLevel::INFO, category, message);
    }
    
    void Warn(LogCategory category, const std::string& message) {
        Log(LogLevel::WARN, category, message);
    }
    
    void Error(LogCategory category, const std::string& message) {
        Log(LogLevel::ERROR, category, message);
    }
    
    void Fatal(LogCategory category, const std::string& message) {
        Log(LogLevel::FATAL, category, message);
    }

    // Configuration
    void SetLevel(LogLevel level) { m_level = level; }
    void SetConsoleOutput(bool enabled) { m_consoleOutput = enabled; }
    void SetFileOutput(bool enabled) { m_fileOutput = enabled; }
    void SetEditorOutput(bool enabled) { m_editorOutput = enabled; }
    void SetLogFile(const std::string& filename);
    
    // Filter by category
    void EnableCategory(LogCategory category) { m_categoryFilter[(int)category] = true; }
    void DisableCategory(LogCategory category) { m_categoryFilter[(int)category] = false; }
    
    // Editor integration
    const std::vector<LogEntry>& GetLogHistory() const { return m_logHistory; }
    void ClearHistory() { 
        std::lock_guard<std::mutex> lock(m_mutex);
        m_logHistory.clear(); 
    }
    
    // Initialize with sensible defaults
    void Initialize();
    void Shutdown();

private:
    Logger() = default;
    ~Logger() { Shutdown(); }
    
    // Prevent copying
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    LogLevel m_level = LogLevel::DEBUG;
    bool m_consoleOutput = true;
    bool m_fileOutput = true;
    bool m_editorOutput = true;
    
    std::ofstream m_logFile;
    std::vector<LogEntry> m_logHistory;
    std::mutex m_mutex;
    
    // Category filtering (all enabled by default)
    bool m_categoryFilter[8] = {true, true, true, true, true, true, true, true};
    
    std::string LevelToString(LogLevel level) const;
    std::string CategoryToString(LogCategory category) const;
    
    void WriteToConsole(const LogEntry& entry);
    void WriteToFile(const LogEntry& entry);
    void AddToHistory(const LogEntry& entry);
};

// Convenience macros for easy logging with file/line info
#define BS_TRACE(category, message) \
    Blacksite::Logger::Instance().Log(Blacksite::LogLevel::TRACE, category, message, __FILE__, __LINE__)

#define BS_DEBUG(category, message) \
    Blacksite::Logger::Instance().Log(Blacksite::LogLevel::DEBUG, category, message, __FILE__, __LINE__)

#define BS_INFO(category, message) \
    Blacksite::Logger::Instance().Log(Blacksite::LogLevel::INFO, category, message, __FILE__, __LINE__)

#define BS_WARN(category, message) \
    Blacksite::Logger::Instance().Log(Blacksite::LogLevel::WARN, category, message, __FILE__, __LINE__)

#define BS_ERROR(category, message) \
    Blacksite::Logger::Instance().Log(Blacksite::LogLevel::ERROR, category, message, __FILE__, __LINE__)

#define BS_FATAL(category, message) \
    Blacksite::Logger::Instance().Log(Blacksite::LogLevel::FATAL, category, message, __FILE__, __LINE__)

// Formatted logging macros
#define BS_INFO_F(category, format, ...) \
    do { \
        char buffer[512]; \
        snprintf(buffer, sizeof(buffer), format, __VA_ARGS__); \
        BS_INFO(category, std::string(buffer)); \
    } while(0)

#define BS_DEBUG_F(category, format, ...) \
    do { \
        char buffer[512]; \
        snprintf(buffer, sizeof(buffer), format, __VA_ARGS__); \
        BS_DEBUG(category, std::string(buffer)); \
    } while(0)

#define BS_WARN_F(category, format, ...) \
    do { \
        char buffer[512]; \
        snprintf(buffer, sizeof(buffer), format, __VA_ARGS__); \
        BS_WARN(category, std::string(buffer)); \
    } while(0)

#define BS_ERROR_F(category, format, ...) \
    do { \
        char buffer[512]; \
        snprintf(buffer, sizeof(buffer), format, __VA_ARGS__); \
        BS_ERROR(category, std::string(buffer)); \
    } while(0)

// Quick category shortcuts for common use
namespace Log {
    inline void Core(LogLevel level, const std::string& message) {
        Logger::Instance().Log(level, LogCategory::CORE, message);
    }
    
    inline void Renderer(LogLevel level, const std::string& message) {
        Logger::Instance().Log(level, LogCategory::RENDERER, message);
    }
    
    inline void Physics(LogLevel level, const std::string& message) {
        Logger::Instance().Log(level, LogCategory::PHYSICS, message);
    }
    
    inline void Editor(LogLevel level, const std::string& message) {
        Logger::Instance().Log(level, LogCategory::EDITOR, message);
    }
}

} // namespace Blacksite