#pragma once
#include <vector>
#include <string>

namespace BlacksiteEditor {

class EditorCore;

struct LogEntry {
    std::string message;
    int level; // 0=Info, 1=Warning, 2=Error
    float timestamp;
};

class ConsolePanel {
public:
    ConsolePanel(EditorCore* editorCore);
    ~ConsolePanel() = default;

    void Update(float deltaTime);
    void Render();

    void AddLogEntry(const std::string& message, int level = 0);

private:
    EditorCore* m_editorCore;
    bool m_isOpen = true;
    std::vector<LogEntry> m_logEntries;
    bool m_autoScroll = true;

    // Command system
    char m_commandBuffer[256] = "";

    void ProcessCommand(const std::string& command);
    void RenderLogDisplay();
    void RenderCommandInput();
};

}  // namespace BlacksiteEditor
