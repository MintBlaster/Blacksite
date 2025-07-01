#pragma once

#include "blacksite/ui/UI.h"
#include <string>

namespace Blacksite {
    class Engine;
}

namespace Blacksite::UI {

class SceneHierarchyPanel : public Panel {
public:
    SceneHierarchyPanel();
    ~SceneHierarchyPanel() = default;

    void Render(UISystem& ui) override;

private:
    void RenderEntityNode(UISystem& ui, Engine& engine, int entityId);
    void RenderContextMenu(UISystem& ui, Engine& engine, int entityId);
    
    int m_selectedEntityId = -1;
    bool m_showInactiveEntities = true;
    std::string m_searchFilter;
};

class EntityInspectorPanel : public Panel {
public:
    EntityInspectorPanel();
    ~EntityInspectorPanel() = default;

    void Render(UISystem& ui) override;
    void SetSelectedEntity(int entityId) { m_selectedEntityId = entityId; }

private:
    void RenderTransformSection(UISystem& ui, Engine& engine);
    void RenderPhysicsSection(UISystem& ui, Engine& engine);
    void RenderRenderingSection(UISystem& ui, Engine& engine);
    
    int m_selectedEntityId = -1;
};

class ConsolePanel : public Panel {
public:
    ConsolePanel();
    ~ConsolePanel() = default;

    void Render(UISystem& ui) override;

private:
    void RenderLogHistory(UISystem& ui);
    void RenderCommandInput(UISystem& ui);
    void ExecuteCommand(const std::string& command);
    
    std::string m_commandInput;
    bool m_autoScroll = true;
    bool m_showTimestamps = true;
    
    // Log level filters
    bool m_showTrace = false;
    bool m_showDebug = true;
    bool m_showInfo = true;
    bool m_showWarn = true;
    bool m_showError = true;
    bool m_showFatal = true;
};

class PerformancePanel : public Panel {
public:
    PerformancePanel();
    ~PerformancePanel() = default;

    void Render(UISystem& ui) override;

private:
    void UpdateMetrics();
    void RenderFPSGraph(UISystem& ui);
    void RenderMemoryInfo(UISystem& ui);
    void RenderSystemInfo(UISystem& ui);
    
    float m_frameTime = 0.0f;
    float m_fps = 0.0f;
    std::vector<float> m_frameHistory;
    static constexpr int MAX_FRAME_HISTORY = 100;
};

} // namespace Blacksite::UI