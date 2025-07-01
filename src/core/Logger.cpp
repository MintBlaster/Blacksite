#include "blacksite/core/Logger.h"
#include <iostream>

namespace Blacksite {

void Logger::Initialize() {
    std::lock_guard<std::mutex> lock(m_mutex);

    // Initialize without filesystem operations to avoid memory corruption
    std::cout << "[INIT] Blacksite Logger initialized (console only)" << std::endl;

    // Disable file output for now to avoid the hanging issue
    m_fileOutput = false;
}

void Logger::Shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_logFile.is_open()) {
        Info(LogCategory::CORE, "Logger shutting down");
        m_logFile.close();
    }
}

void Logger::Log(LogLevel level, LogCategory category, const std::string& message,
                 const std::string& file, int line) {
    // Early return if level is filtered out
    if (level < m_level) {
        return;
    }

    // Early return if category is filtered out
    if (!m_categoryFilter[(int)category]) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    // Create log entry
    LogEntry entry(level, category, message, file, line);

    // Output to various destinations
    if (m_consoleOutput) {
        WriteToConsole(entry);
    }

    if (m_fileOutput && m_logFile.is_open()) {
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

    try {
        m_logFile.open(filename, std::ios::out | std::ios::app);
        if (!m_logFile.is_open()) {
            std::cerr << "Failed to open log file: " << filename << std::endl;
            m_fileOutput = false;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception opening log file: " << e.what() << std::endl;
        m_fileOutput = false;
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
        default: return "UNKNW";
    }
}

std::string Logger::CategoryToString(LogCategory category) const {
    switch (category) {
        case LogCategory::CORE:     return "CORE    ";
        case LogCategory::RENDERER: return "RENDERER";
        case LogCategory::PHYSICS:  return "PHYSICS ";
        case LogCategory::AUDIO:    return "AUDIO   ";
        case LogCategory::INPUT:    return "INPUT   ";
        case LogCategory::EDITOR:   return "EDITOR  ";
        case LogCategory::GAME:     return "GAME    ";
        case LogCategory::NETWORK:  return "NETWORK ";
        default: return "UNKNOWN ";
    }
}



void Logger::WriteToConsole(const LogEntry& entry) {
    // Clean professional console output
    std::ostream& stream = (entry.level >= LogLevel::WARN) ? std::cerr : std::cout;

    stream << entry.timestamp << " │ "
           << LevelToString(entry.level) << " │ "
           << CategoryToString(entry.category) << " │ "
           << entry.message;

    // Add file/line info for debug builds
    #ifdef _DEBUG
    if (!entry.file.empty() && entry.line > 0) {
        // Extract just the filename from the full path
        std::string filename = entry.file;
        size_t lastSlash = filename.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            filename = filename.substr(lastSlash + 1);
        }
        stream << " (" << filename << ":" << entry.line << ")";
    }
    #endif

    stream << std::endl;
}

void Logger::WriteToFile(const LogEntry& entry) {
    if (!m_logFile.is_open()) {
        return; // Skip if file is not open
    }

    try {
        // Simple file format - just use the timestamp from entry
        m_logFile << "2025-07-01 " << entry.timestamp << " [" << LevelToString(entry.level) << "] "
                  << "[" << CategoryToString(entry.category) << "] "
                  << entry.message;

        // Always include file/line info in file logs
        if (!entry.file.empty() && entry.line > 0) {
            std::string filename = entry.file;
            size_t lastSlash = filename.find_last_of("/\\");
            if (lastSlash != std::string::npos) {
                filename = filename.substr(lastSlash + 1);
            }
            m_logFile << " (" << filename << ":" << entry.line << ")";
        }

        m_logFile << std::endl;
        m_logFile.flush(); // Ensure immediate write for crashes
    } catch (const std::exception& e) {
        std::cerr << "Exception writing to log file: " << e.what() << std::endl;
    }
}

void Logger::AddToHistory(const LogEntry& entry) {
    // Keep only last 1000 entries to prevent memory bloat
    if (m_logHistory.size() >= 1000) {
        m_logHistory.erase(m_logHistory.begin(), m_logHistory.begin() + 100);
    }

    m_logHistory.push_back(entry);
}

} // namespace Blacksite
