#pragma once

#include <string>
#include <functional>
#include <memory>
#include <vector>
#include <glm/glm.hpp>

struct GLFWwindow;

namespace Blacksite::UI {

// Forward declarations
class Window;
class Widget;
class Panel;

// UI System - Main interface
class UISystem {
public:
    UISystem();
    ~UISystem();

    // Lifecycle
    bool Initialize(GLFWwindow* window);
    void Shutdown();

    // Frame management
    void BeginFrame();
    void EndFrame();

    // Window management
    void BeginWindow(const std::string& title, bool* open = nullptr);
    void EndWindow();

    // Docking
    void BeginDockSpace();
    void EndDockSpace();
    void SetupDefaultDocking();

    // Basic widgets
    bool Button(const std::string& label);
    bool Button(const std::string& label, const glm::vec2& size);
    void Text(const std::string& text);
    void TextColored(const glm::vec3& color, const std::string& text);
    bool InputText(const std::string& label, std::string& text);
    bool Checkbox(const std::string& label, bool& value);
    bool SliderFloat(const std::string& label, float& value, float min, float max);
    bool SliderInt(const std::string& label, int& value, int min, int max);

    // Layout
    void SameLine();
    void Separator();
    void Spacing();
    void NewLine();

    // Menu system
    bool BeginMainMenuBar();
    void EndMainMenuBar();
    bool BeginMenu(const std::string& label);
    void EndMenu();
    bool MenuItem(const std::string& label, bool* selected = nullptr);

    // Utilities
    bool IsWindowHovered();
    bool IsWindowFocused();
    glm::vec2 GetWindowSize();
    glm::vec2 GetWindowPos();

    // Style
    void PushStyleColor(int idx, const glm::vec4& color);
    void PopStyleColor(int count = 1);

private:
    bool m_initialized = false;
    bool m_dockingEnabled = true;
    
    void SetupStyle();
    void SetupDocking();
};

// Panel base class for editor panels
class Panel {
public:
    Panel(const std::string& id, const std::string& title);
    virtual ~Panel() = default;

    virtual void Render(UISystem& ui) = 0;

    // Properties
    const std::string& GetID() const { return m_id; }
    const std::string& GetTitle() const { return m_title; }
    bool IsVisible() const { return m_visible; }
    void SetVisible(bool visible) { m_visible = visible; }

protected:
    std::string m_id;
    std::string m_title;
    bool m_visible = true;
};

// Panel manager for organizing editor panels
class PanelManager {
public:
    PanelManager();
    ~PanelManager();

    // Panel management
    void RegisterPanel(std::unique_ptr<Panel> panel);
    void RemovePanel(const std::string& id);
    Panel* GetPanel(const std::string& id);

    // Rendering
    void RenderAllPanels(UISystem& ui);
    void RenderPanelMenuItems(UISystem& ui);

    // Visibility
    void ShowPanel(const std::string& id);
    void HidePanel(const std::string& id);
    void TogglePanel(const std::string& id);

private:
    std::vector<std::unique_ptr<Panel>> m_panels;
};

// Simple theme system
enum class Theme {
    Dark,
    Light,
    Blacksite  // Custom theme
};

class ThemeManager {
public:
    static void ApplyTheme(Theme theme);
    static void SetupBlacksiteTheme();
};

} // namespace Blacksite::UI