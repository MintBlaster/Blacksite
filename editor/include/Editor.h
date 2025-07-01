#pragma once
#include "blacksite/core/Engine.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <memory>
#include <vector>
#include <string>

namespace BlacksiteEditor {

class EditorPanel {
public:
    virtual ~EditorPanel() = default;
    virtual void Render() = 0;
    virtual const char* GetName() const = 0;
    virtual bool IsVisible() const { return m_visible; }
    virtual void SetVisible(bool visible) { m_visible = visible; }

protected:
    bool m_visible = true;
};

class Editor {
public:
    Editor();
    ~Editor();

    bool Initialize(Blacksite::Engine* engine, GLFWwindow* window);
    void Update(float deltaTime);
    void Render();
    void Shutdown();

    // Panel management
    void AddPanel(std::unique_ptr<EditorPanel> panel);
    EditorPanel* GetPanel(const std::string& name);

    // Editor state
    bool IsEnabled() const { return m_enabled; }
    void SetEnabled(bool enabled) { m_enabled = enabled; }
    void ToggleEnabled() { m_enabled = !m_enabled; }

    // Selection
    void SetSelectedEntity(int entityId) { m_selectedEntity = entityId; }
    int GetSelectedEntity() const { return m_selectedEntity; }

private:
    void RenderMainMenuBar();
    void RenderDockSpace();
    void HandleShortcuts();
    void SetupTheme();

    Blacksite::Engine* m_engine = nullptr;
    bool m_enabled = true;
    bool m_showDemo = false;
    bool m_showMetrics = false;
    int m_selectedEntity = -1;

    std::vector<std::unique_ptr<EditorPanel>> m_panels;
};

} // namespace BlacksiteEditor
