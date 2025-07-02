#include "blacksite/core/Logger.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

namespace Blacksite {

void Logger::Initialize() {
    SetLogFile("blacksite.log");

    // Auto-detect if we should use colors (only if outputting to a terminal)
#ifdef _WIN32
    // Enable ANSI color codes on Windows 10+
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);

    m_colorOutput = isatty(fileno(stdout));
#else
    m_colorOutput = isatty(fileno(stdout));
#endif
}

void Logger::Shutdown() {
    if (m_logFile.is_open()) {
        m_logFile.close();
    }
}

void Logger::Log(LogLevel level, LogCategory category, const std::string& message,
                const std::string& file, int line) {
    // Check level filtering
    if (level < m_level) return;

    // Check category filtering
    if (!m_categoryFilter[(int)category]) return;

    // Create log entry
    LogEntry entry(level, category, message, file, line);

    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_consoleOutput) {
        WriteToConsole(entry);
    }

    if (m_fileOutput) {
        WriteToFile(entry);
    }

    if (m_editorOutput) {
        AddToHistory(entry);
    }
}

void Logger::SetLogFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_logFile.is_open()) {
        m_logFile.close();
    }

    m_logFile.open(filename, std::ios::app);
    if (!m_logFile.is_open()) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
    }
}

std::string Logger::LevelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

std::string Logger::CategoryToString(LogCategory category) const {
    switch (category) {
        case LogCategory::CORE:     return "CORE";
        case LogCategory::RENDERER: return "RENDERER";
        case LogCategory::PHYSICS:  return "PHYSICS";
        case LogCategory::AUDIO:    return "AUDIO";
        case LogCategory::INPUT:    return "INPUT";
        case LogCategory::EDITOR:   return "EDITOR";
        case LogCategory::GAME:     return "GAME";
        case LogCategory::NETWORK:  return "NETWORK";
        default: return "UNKNOWN";
    }
}

LogColor Logger::GetLevelColor(LogLevel level) const {
    switch (level) {
        case LogLevel::TRACE: return LogColor::BRIGHT_BLACK;  // Gray
        case LogLevel::DEBUG: return LogColor::CYAN;          // Cyan
        case LogLevel::INFO:  return LogColor::GREEN;         // Green
        case LogLevel::WARN:  return LogColor::YELLOW;        // Yellow
        case LogLevel::ERROR: return LogColor::RED;           // Red
        case LogLevel::FATAL: return LogColor::BRIGHT_RED;    // Bright Red
        default: return LogColor::WHITE;
    }
}

std::string Logger::ColorToAnsi(LogColor color) const {
    if (!m_colorOutput) return "";

    return "\033[" + std::to_string((int)color) + "m";
}

void Logger::WriteToConsole(const LogEntry& entry) {
    std::string colorCode = ColorToAnsi(GetLevelColor(entry.level));
    std::string resetCode = ColorToAnsi(LogColor::RESET);

    std::ostringstream oss;
    oss << colorCode
        << "[" << entry.timestamp << "] "
        << "[" << LevelToString(entry.level) << "] "
        << "[" << CategoryToString(entry.category) << "] "
        << resetCode
        << entry.message;

    // Add file/line info for ERROR and FATAL
    if (entry.level >= LogLevel::ERROR && !entry.file.empty()) {
        // Extract just the filename from the full path
        std::string filename = entry.file;
        size_t lastSlash = filename.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            filename = filename.substr(lastSlash + 1);
        }

        oss << colorCode << " (" << filename << ":" << entry.line << ")" << resetCode;
    }

    oss << std::endl;

    // Output to stderr for warnings and errors, stdout for everything else
    if (entry.level >= LogLevel::WARN) {
        std::cerr << oss.str();
        std::cerr.flush();
    } else {
        std::cout << oss.str();
        std::cout.flush();
    }
}

void Logger::WriteToFile(const LogEntry& entry) {
    if (!m_logFile.is_open()) return;

    m_logFile << "[" << entry.timestamp << "] "
              << "[" << LevelToString(entry.level) << "] "
              << "[" << CategoryToString(entry.category) << "] "
              << entry.message;

    // Add file/line info for ERROR and FATAL
    if (entry.level >= LogLevel::ERROR && !entry.file.empty()) {
        std::string filename = entry.file;
        size_t lastSlash = filename.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            filename = filename.substr(lastSlash + 1);
        }
        m_logFile << " (" << filename << ":" << entry.line << ")";
    }

    m_logFile << std::endl;
    m_logFile.flush();
}

void Logger::AddToHistory(const LogEntry& entry) {
    m_logHistory.push_back(entry);

    // Keep history size reasonable (last 1000 entries)
    if (m_logHistory.size() > 1000) {
        m_logHistory.erase(m_logHistory.begin());
    }
}

} // namespace Blacksite
