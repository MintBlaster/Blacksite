#pragma once

namespace BlacksiteEditor {

class EditorCore;

class PerformancePanel {
public:
    PerformancePanel(EditorCore* editorCore);
    ~PerformancePanel() = default;

    void Update(float deltaTime);
    void Render();

private:
    EditorCore* m_editorCore;
    bool m_isOpen = true;

    // Performance tracking
    float m_frameTimeHistory[120] = {0};
    int m_frameTimeIndex = 0;
    float m_currentFPS = 0.0f;

    void UpdatePerformanceStats(float deltaTime);
    void RenderFrameRateInfo();
    void RenderMemoryInfo();
    void RenderSystemInfo();
    void RenderPerformanceControls();
};

}  // namespace BlacksiteEditor
