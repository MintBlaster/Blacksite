#pragma once
#include <blacksite/app/Application.h>
#include <blacksite/core/Logger.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <memory>

#include "blacksite_editor/core/EditorCore.h"
#include "blacksite_editor/panels/ConsolePanel.h"
#include "blacksite_editor/panels/InspectorPanel.h"
#include "blacksite_editor/panels/PerformancePanel.h"
#include "blacksite_editor/panels/SceneHierarchyPanel.h"
#include "blacksite_editor/panels/ViewportPanel.h"

namespace BlacksiteEditor {

class EditorCore;
class SceneHierarchyPanel;
class InspectorPanel;
class ViewportPanel;
class ConsolePanel;
class PerformancePanel;

class EditorApplication : public Blacksite::Application {
  public:
    EditorApplication();
    ~EditorApplication() = default;

  protected:
    void OnInitialize() override;
    void OnUpdate(float deltaTime) override;
    void OnRender() override;
    void OnShutdown() override;

  private:
    // Editor core
    std::unique_ptr<EditorCore> m_editorCore;

    // Panels
    std::unique_ptr<SceneHierarchyPanel> m_hierarchyPanel;
    std::unique_ptr<InspectorPanel> m_inspectorPanel;
    std::unique_ptr<ViewportPanel> m_viewportPanel;
    std::unique_ptr<ConsolePanel> m_consolePanel;
    std::unique_ptr<PerformancePanel> m_performancePanel;

    // Editor state
    bool m_showHierarchy = true;
    bool m_showInspector = true;
    bool m_showConsole = true;
    bool m_showPerformance = true;
    bool m_showDemoWindow = false;
    bool m_showMetrics = false;
    int m_selectedEntityId = -1;

    void SetupImGui();
    void UpdateEditor(float deltaTime);
    void RenderEditor();
    void RenderMenuBar();
    void RenderMainDockSpace();
    void HandleEditorInput();
    void ShutdownImGui();
    void SetupTheme();
    void LoadEditorFonts(ImGuiIO& io);
};

}  // namespace BlacksiteEditor
