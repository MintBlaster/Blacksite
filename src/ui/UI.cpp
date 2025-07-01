#include "blacksite/ui/UI.h"
#include "blacksite/core/Logger.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <algorithm>

namespace Blacksite::UI {

// UISystem Implementation
UISystem::UISystem() = default;

UISystem::~UISystem() {
    if (m_initialized) {
        Shutdown();
    }
}

bool UISystem::Initialize(GLFWwindow* window) {
    if (m_initialized) {
        BS_ERROR(LogCategory::EDITOR, "UISystem already initialized!");
        return false;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    
    // Enable keyboard navigation
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Enable docking
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    // Enable multi-viewport (optional - can be disabled for simpler setup)
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Setup Dear ImGui style
    SetupStyle();

    // When viewports are enabled we tweak WindowRounding/WindowBg 
    // so platform windows can look identical to regular ones
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    const char* glsl_version = "#version 330";
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        BS_ERROR(LogCategory::EDITOR, "Failed to initialize ImGui GLFW backend!");
        return false;
    }

    if (!ImGui_ImplOpenGL3_Init(glsl_version)) {
        BS_ERROR(LogCategory::EDITOR, "Failed to initialize ImGui OpenGL3 backend!");
        ImGui_ImplGlfw_Shutdown();
        return false;
    }

    m_initialized = true;
    BS_INFO(LogCategory::EDITOR, "UI System initialized successfully");
    return true;
}

void UISystem::Shutdown() {
    if (!m_initialized) {
        return;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    m_initialized = false;
    BS_INFO(LogCategory::EDITOR, "UI System shutdown complete");
}

void UISystem::BeginFrame() {
    if (!m_initialized) return;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UISystem::EndFrame() {
    if (!m_initialized) return;

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void UISystem::BeginWindow(const std::string& title, bool* open) {
    ImGui::Begin(title.c_str(), open);
}

void UISystem::EndWindow() {
    ImGui::End();
}

void UISystem::BeginDockSpace() {
    static bool dockspaceOpen = true;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Blacksite Editor", &dockspaceOpen, window_flags);
    ImGui::PopStyleVar();
    ImGui::PopStyleVar(2);

    // DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("BlacksiteDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        
        // Set up default layout on first run
        static bool first_time = true;
        if (first_time) {
            first_time = false;
            SetupDefaultDocking();
        }
    }
}

void UISystem::EndDockSpace() {
    ImGui::End();
}

void UISystem::SetupDefaultDocking() {
    // This will be called once to set up the default docking layout
    BS_DEBUG(LogCategory::EDITOR, "Setting up default docking layout");
}

// Widget implementations
bool UISystem::Button(const std::string& label) {
    return ImGui::Button(label.c_str());
}

bool UISystem::Button(const std::string& label, const glm::vec2& size) {
    return ImGui::Button(label.c_str(), ImVec2(size.x, size.y));
}

void UISystem::Text(const std::string& text) {
    ImGui::Text("%s", text.c_str());
}

void UISystem::TextColored(const glm::vec3& color, const std::string& text) {
    ImGui::TextColored(ImVec4(color.r, color.g, color.b, 1.0f), "%s", text.c_str());
}

bool UISystem::InputText(const std::string& label, std::string& text) {
    char buffer[256];
    strncpy(buffer, text.c_str(), sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';
    
    if (ImGui::InputText(label.c_str(), buffer, sizeof(buffer))) {
        text = buffer;
        return true;
    }
    return false;
}

bool UISystem::Checkbox(const std::string& label, bool& value) {
    return ImGui::Checkbox(label.c_str(), &value);
}

bool UISystem::SliderFloat(const std::string& label, float& value, float min, float max) {
    return ImGui::SliderFloat(label.c_str(), &value, min, max);
}

bool UISystem::SliderInt(const std::string& label, int& value, int min, int max) {
    return ImGui::SliderInt(label.c_str(), &value, min, max);
}

// Layout functions
void UISystem::SameLine() {
    ImGui::SameLine();
}

void UISystem::Separator() {
    ImGui::Separator();
}

void UISystem::Spacing() {
    ImGui::Spacing();
}

void UISystem::NewLine() {
    ImGui::NewLine();
}

// Menu system
bool UISystem::BeginMainMenuBar() {
    return ImGui::BeginMainMenuBar();
}

void UISystem::EndMainMenuBar() {
    ImGui::EndMainMenuBar();
}

bool UISystem::BeginMenu(const std::string& label) {
    return ImGui::BeginMenu(label.c_str());
}

void UISystem::EndMenu() {
    ImGui::EndMenu();
}

bool UISystem::MenuItem(const std::string& label, bool* selected) {
    return ImGui::MenuItem(label.c_str(), nullptr, selected);
}

// Utilities
bool UISystem::IsWindowHovered() {
    return ImGui::IsWindowHovered();
}

bool UISystem::IsWindowFocused() {
    return ImGui::IsWindowFocused();
}

glm::vec2 UISystem::GetWindowSize() {
    ImVec2 size = ImGui::GetWindowSize();
    return glm::vec2(size.x, size.y);
}

glm::vec2 UISystem::GetWindowPos() {
    ImVec2 pos = ImGui::GetWindowPos();
    return glm::vec2(pos.x, pos.y);
}

// Style
void UISystem::PushStyleColor(int idx, const glm::vec4& color) {
    ImGui::PushStyleColor(idx, ImVec4(color.r, color.g, color.b, color.a));
}

void UISystem::PopStyleColor(int count) {
    ImGui::PopStyleColor(count);
}

void UISystem::SetupStyle() {
    // Apply dark theme by default
    ImGui::StyleColorsDark();
    
    // Customize colors for Blacksite
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Blacksite color scheme
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
    
    // Rounding
    style.WindowRounding = 2.0f;
    style.FrameRounding = 2.0f;
    style.GrabRounding = 2.0f;
    
    // Padding
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.FramePadding = ImVec2(4.0f, 3.0f);
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
}

// Panel Implementation
Panel::Panel(const std::string& id, const std::string& title) 
    : m_id(id), m_title(title) {
}

// PanelManager Implementation
PanelManager::PanelManager() = default;

PanelManager::~PanelManager() = default;

void PanelManager::RegisterPanel(std::unique_ptr<Panel> panel) {
    if (panel) {
        BS_DEBUG_F(LogCategory::EDITOR, "Registered panel: %s", panel->GetTitle().c_str());
        m_panels.push_back(std::move(panel));
    }
}

void PanelManager::RemovePanel(const std::string& id) {
    auto it = std::remove_if(m_panels.begin(), m_panels.end(),
        [&id](const std::unique_ptr<Panel>& panel) {
            return panel->GetID() == id;
        });
    
    if (it != m_panels.end()) {
        BS_DEBUG_F(LogCategory::EDITOR, "Removed panel: %s", id.c_str());
        m_panels.erase(it, m_panels.end());
    }
}

Panel* PanelManager::GetPanel(const std::string& id) {
    for (auto& panel : m_panels) {
        if (panel->GetID() == id) {
            return panel.get();
        }
    }
    return nullptr;
}

void PanelManager::RenderAllPanels(UISystem& ui) {
    for (auto& panel : m_panels) {
        if (panel->IsVisible()) {
            panel->Render(ui);
        }
    }
}

void PanelManager::RenderPanelMenuItems(UISystem& ui) {
    for (auto& panel : m_panels) {
        bool visible = panel->IsVisible();
        if (ui.MenuItem(panel->GetTitle(), &visible)) {
            panel->SetVisible(visible);
        }
    }
}

void PanelManager::ShowPanel(const std::string& id) {
    if (Panel* panel = GetPanel(id)) {
        panel->SetVisible(true);
    }
}

void PanelManager::HidePanel(const std::string& id) {
    if (Panel* panel = GetPanel(id)) {
        panel->SetVisible(false);
    }
}

void PanelManager::TogglePanel(const std::string& id) {
    if (Panel* panel = GetPanel(id)) {
        panel->SetVisible(!panel->IsVisible());
    }
}

// ThemeManager Implementation
void ThemeManager::ApplyTheme(Theme theme) {
    switch (theme) {
        case Theme::Dark:
            ImGui::StyleColorsDark();
            break;
        case Theme::Light:
            ImGui::StyleColorsLight();
            break;
        case Theme::Blacksite:
            SetupBlacksiteTheme();
            break;
    }
}

void ThemeManager::SetupBlacksiteTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Blacksite custom theme - dark with orange accents
    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(1.0f, 0.4f, 0.0f, 0.4f); // Orange
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(1.0f, 0.4f, 0.0f, 0.6f);  // Orange
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.4f, 0.0f, 1.0f);  // Orange
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 0.4f, 0.0f, 1.0f);  // Orange
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 0.3f, 0.0f, 1.0f);   // Darker orange
    style.Colors[ImGuiCol_Header] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 0.4f, 0.0f, 0.8f);  // Orange
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 0.4f, 0.0f, 1.0f);   // Orange
    
    // Rounding and spacing
    style.WindowRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.FramePadding = ImVec2(5.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
}

} // namespace Blacksite::UI