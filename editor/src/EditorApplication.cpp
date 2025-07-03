#include "blacksite_editor/EditorApplication.h"
#include "blacksite_editor/core/EditorCore.h"
#include "blacksite_editor/panels/ConsolePanel.h"
#include "blacksite_editor/panels/InspectorPanel.h"
#include "blacksite_editor/panels/PerformancePanel.h"
#include "blacksite_editor/panels/SceneHierarchyPanel.h"
#include "blacksite_editor/panels/ViewportPanel.h"

#include <blacksite/core/Logger.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace BlacksiteEditor {

EditorApplication::EditorApplication() : Application("Blacksite Editor", 1920, 1080) {}

void EditorApplication::OnInitialize() {
    BS_INFO(Blacksite::LogCategory::CORE, "Initializing Blacksite Editor");

    SetupImGui();

    // Initialize editor components
    m_editorCore = std::make_unique<EditorCore>(&GetEngine());
    m_editorCore->Initialize();

    m_hierarchyPanel = std::make_unique<SceneHierarchyPanel>(m_editorCore.get());
    m_inspectorPanel = std::make_unique<InspectorPanel>(m_editorCore.get());
    m_viewportPanel = std::make_unique<ViewportPanel>(m_editorCore.get());
    m_consolePanel = std::make_unique<ConsolePanel>(m_editorCore.get());
    m_performancePanel = std::make_unique<PerformancePanel>(m_editorCore.get());

    // Setup callbacks
    m_hierarchyPanel->SetEntitySelectedCallback([this](int entityId) {
        m_selectedEntityId = entityId;
        m_editorCore->SetSelectedEntity(entityId);
    });

    m_editorCore->NewScene("DefaultScene");
}

void EditorApplication::OnUpdate(float deltaTime) {
    HandleEditorInput();
    UpdateEditor(deltaTime);
}

void EditorApplication::OnRender() {
    // Pure UI rendering - no background scene rendering here
    RenderEditor();
}

void EditorApplication::OnShutdown() {
    ShutdownImGui();
    BS_INFO(Blacksite::LogCategory::CORE, "Blacksite Editor shutdown");
}

void EditorApplication::SetupImGui() {
    BS_INFO(Blacksite::LogCategory::CORE, "Initializing modern editor interface...");

    IMGUI_CHECKVERSION();
    ImGuiContext* context = ImGui::CreateContext();

    if (!context) {
        BS_ERROR(Blacksite::LogCategory::CORE, "Failed to create ImGui context!");
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Load crisp, modern fonts
    LoadEditorFonts(io);

    // Apply sleek modern theme
    SetupTheme();

    // Verify GLFW window
    GLFWwindow* window = GetEngine().GetGLFWWindow();
    if (!window) {
        BS_ERROR(Blacksite::LogCategory::CORE, "GLFW window is null!");
        return;
    }

    // Setup Platform/Renderer backends
    bool glfwInit = ImGui_ImplGlfw_InitForOpenGL(window, true);
    bool openglInit = ImGui_ImplOpenGL3_Init("#version 330");

    if (!glfwInit || !openglInit) {
        BS_ERROR(Blacksite::LogCategory::CORE, "Failed to initialize ImGui backends!");
        return;
    }

    BS_INFO(Blacksite::LogCategory::CORE, "Modern editor interface initialized successfully!");
}

void EditorApplication::UpdateEditor(float deltaTime) {
    // Update editor core
    m_editorCore->Update(deltaTime);

    // Update panels
    m_hierarchyPanel->Update(deltaTime);
    m_inspectorPanel->Update(deltaTime);
    m_viewportPanel->Update(deltaTime);
    m_consolePanel->Update(deltaTime);
    m_performancePanel->Update(deltaTime);
}

void EditorApplication::RenderEditor() {
    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Render main dockspace
    RenderMainDockSpace();

    // Render panels
    m_hierarchyPanel->Render();
    m_inspectorPanel->Render();
    m_viewportPanel->Render();
    m_consolePanel->Render();
    m_performancePanel->Render();

    // Demo windows
    if (m_showDemoWindow) {
        ImGui::ShowDemoWindow(&m_showDemoWindow);
    }

    if (m_showMetrics) {
        ImGui::ShowMetricsWindow(&m_showMetrics);
    }

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and render additional Platform Windows
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void EditorApplication::RenderMainDockSpace() {
    static bool dockspaceOpen = true;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // Use the main viewport for the dockspace
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    // Remove window padding for seamless docking
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("BlacksiteEditorDockSpace", &dockspaceOpen, window_flags);
    ImGui::PopStyleVar(3);

    // Create the dockspace
    ImGuiID dockspace_id = ImGui::GetID("BlacksiteEditorDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

    // Render the menu bar
    RenderMenuBar();

    ImGui::End();
}

void EditorApplication::RenderMenuBar() {
    if (ImGui::BeginMenuBar()) {
        // Apply modern menu styling
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12.0f, 8.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 6.0f));

        // File menu
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                m_editorCore->NewScene("NewScene");
            }
            if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {
                m_editorCore->LoadScene("scene.bscn");
            }
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                m_editorCore->SaveScene("scene.bscn");
            }
            if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {
                // TODO: Implement save as dialog
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Recent Files")) {
                // TODO: Implement recent files
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                glfwSetWindowShouldClose(GetEngine().GetGLFWWindow(), GLFW_TRUE);
            }
            ImGui::EndMenu();
        }

        // Edit menu
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
                // TODO: Implement undo
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {
                // TODO: Implement redo
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "Ctrl+X", false, m_editorCore->HasSelectedEntity())) {
                // TODO: Implement cut
            }
            if (ImGui::MenuItem("Copy", "Ctrl+C", false, m_editorCore->HasSelectedEntity())) {
                // TODO: Implement copy
            }
            if (ImGui::MenuItem("Paste", "Ctrl+V")) {
                // TODO: Implement paste
            }
            if (ImGui::MenuItem("Duplicate", "Ctrl+D", false, m_editorCore->HasSelectedEntity())) {
                m_editorCore->DuplicateEntity(m_editorCore->GetSelectedEntity());
            }
            if (ImGui::MenuItem("Delete", "Delete", false, m_editorCore->HasSelectedEntity())) {
                m_editorCore->DeleteEntity(m_editorCore->GetSelectedEntity());
            }
            ImGui::EndMenu();
        }

        // View menu
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Show Grid", "G", m_editorCore->IsGridVisible());
            ImGui::MenuItem("Show Gizmos", nullptr, m_editorCore->IsGizmosVisible());
            ImGui::Separator();
            ImGui::MenuItem("Scene Hierarchy", nullptr, &m_showHierarchy);
            ImGui::MenuItem("Inspector", nullptr, &m_showInspector);
            ImGui::MenuItem("Console", nullptr, &m_showConsole);
            ImGui::MenuItem("Performance", nullptr, &m_showPerformance);
            ImGui::Separator();
            ImGui::MenuItem("Demo Window", nullptr, &m_showDemoWindow);
            ImGui::MenuItem("Metrics", nullptr, &m_showMetrics);
            ImGui::EndMenu();
        }

        // Create menu
        if (ImGui::BeginMenu("Create")) {
            if (ImGui::BeginMenu("3D Objects")) {
                if (ImGui::MenuItem("Cube")) {
                    m_editorCore->CreateEntity("Cube");
                }
                if (ImGui::MenuItem("Sphere")) {
                    m_editorCore->CreateEntity("Sphere");
                }
                if (ImGui::MenuItem("Cylinder")) {
                    m_editorCore->CreateEntity("Cylinder");
                }
                if (ImGui::MenuItem("Plane")) {
                    m_editorCore->CreateEntity("Plane");
                }
                if (ImGui::MenuItem("Capsule")) {
                    m_editorCore->CreateEntity("Capsule");
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Lights")) {
                if (ImGui::MenuItem("Directional Light")) {
                    m_editorCore->CreateEntity("DirectionalLight");
                }
                if (ImGui::MenuItem("Point Light")) {
                    m_editorCore->CreateEntity("PointLight");
                }
                if (ImGui::MenuItem("Spot Light")) {
                    m_editorCore->CreateEntity("SpotLight");
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Audio")) {
                if (ImGui::MenuItem("Audio Source")) {
                    m_editorCore->CreateEntity("AudioSource");
                }
                if (ImGui::MenuItem("Audio Listener")) {
                    m_editorCore->CreateEntity("AudioListener");
                }
                ImGui::EndMenu();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Empty Entity")) {
                m_editorCore->CreateEntity("Entity");
            }
            ImGui::EndMenu();
        }

        // Tools menu
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Asset Browser")) {
                // TODO: Open asset browser
            }
            if (ImGui::MenuItem("Material Editor")) {
                // TODO: Open material editor
            }
            if (ImGui::MenuItem("Animation Editor")) {
                // TODO: Open animation editor
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Build Settings")) {
                // TODO: Open build settings
            }
            if (ImGui::MenuItem("Preferences")) {
                // TODO: Open preferences
            }
            ImGui::EndMenu();
        }

        // Help menu
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Documentation")) {
                // TODO: Open documentation
            }
            if (ImGui::MenuItem("Tutorials")) {
                // TODO: Open tutorials
            }
            if (ImGui::MenuItem("Community")) {
                // TODO: Open community links
            }
            ImGui::Separator();
            if (ImGui::MenuItem("About Blacksite")) {
                // TODO: Show about dialog
            }
            ImGui::EndMenu();
        }

        ImGui::PopStyleVar(2);
        ImGui::EndMenuBar();
    }
}

void EditorApplication::HandleEditorInput() {
    auto* input = GetEngine().GetInputSystem();

    // Editor-specific shortcuts
    if (input->KeyDown(Blacksite::Key::Ctrl)) {
        if (input->KeyPressed('n')) {
            m_editorCore->NewScene("NewScene");
        }
        if (input->KeyPressed('s')) {
            m_editorCore->SaveScene("scene.bscn");
        }
        if (input->KeyPressed('o')) {
            m_editorCore->LoadScene("scene.bscn");
        }
        if (input->KeyPressed('d') && m_editorCore->HasSelectedEntity()) {
            m_editorCore->DuplicateEntity(m_editorCore->GetSelectedEntity());
        }
    }

    if (input->KeyPressed(Blacksite::Key::Delete) && m_editorCore->HasSelectedEntity()) {
        m_editorCore->DeleteEntity(m_editorCore->GetSelectedEntity());
    }

    // Toggle grid
    if (input->KeyPressed('g')) {
        m_editorCore->SetGridVisible(!m_editorCore->IsGridVisible());
    }
}

void EditorApplication::ShutdownImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void EditorApplication::SetupTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // === DARK GAME ENGINE THEME - INSPIRED BY UNREAL/UNITY/GODOT ===

    // Core background colors - Professional dark grays
    colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);           // Main dark background
    colors[ImGuiCol_ChildBg] = ImVec4(0.16f, 0.17f, 0.18f, 1.00f);            // Panel backgrounds
    colors[ImGuiCol_PopupBg] = ImVec4(0.11f, 0.12f, 0.13f, 0.98f);            // Popup backgrounds
    colors[ImGuiCol_Border] = ImVec4(0.25f, 0.27f, 0.29f, 1.00f);             // Subtle borders
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);       // Subtle shadows
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);            // Input field backgrounds
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.27f, 0.29f, 1.00f);     // Hovered inputs
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.32f, 0.34f, 1.00f);      // Active inputs

    // Title bar colors - Professional dark headers
    colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.10f, 0.11f, 1.00f);            // Inactive title
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.11f, 0.12f, 0.13f, 1.00f);      // Active title
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.07f, 0.08f, 0.09f, 1.00f);   // Collapsed title

    // Menu bar - Dark top bar
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.11f, 0.12f, 1.00f);          // Menu bar background

    // Scrollbar - Dark and minimal
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);        // Scrollbar background
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.35f, 0.37f, 0.39f, 1.00f);      // Scrollbar grab
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.45f, 0.47f, 0.49f, 1.00f); // Hovered scrollbar
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.55f, 0.57f, 0.59f, 1.00f); // Active scrollbar

    // Professional accent colors - Orange/Blue like game engines
    ImVec4 accent_primary = ImVec4(1.00f, 0.60f, 0.20f, 1.00f);   // Orange accent
    ImVec4 accent_secondary = ImVec4(0.30f, 0.70f, 1.00f, 1.00f); // Blue accent

    colors[ImGuiCol_CheckMark] = accent_primary;                               // Checkmarks
    colors[ImGuiCol_SliderGrab] = accent_primary;                             // Slider grab
    colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.70f, 0.30f, 1.00f);  // Active slider

    // Button colors - Professional dark buttons
    colors[ImGuiCol_Button] = ImVec4(0.22f, 0.24f, 0.26f, 1.00f);            // Button
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.30f, 0.32f, 1.00f);     // Hovered button
    colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.37f, 0.39f, 1.00f);      // Active button

    // Header colors - Professional hierarchy
    colors[ImGuiCol_Header] = ImVec4(0.18f, 0.20f, 0.22f, 1.00f);            // Header
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.27f, 0.29f, 1.00f);     // Hovered header
    colors[ImGuiCol_HeaderActive] = ImVec4(0.32f, 0.34f, 0.36f, 1.00f);      // Active header

    // Separator colors - Subtle dark lines
    colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.30f, 0.32f, 1.00f);         // Separators
    colors[ImGuiCol_SeparatorHovered] = accent_primary;                       // Hovered separators
    colors[ImGuiCol_SeparatorActive] = ImVec4(1.00f, 0.70f, 0.30f, 1.00f);   // Active separators

    // Resize grip colors
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.35f, 0.37f, 0.39f, 0.50f);        // Resize grip
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.45f, 0.47f, 0.49f, 0.70f); // Hovered resize grip
    colors[ImGuiCol_ResizeGripActive] = accent_primary;                       // Active resize grip

    // Tab colors - Professional tab system
    colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.16f, 0.17f, 1.00f);               // Tab
    colors[ImGuiCol_TabHovered] = ImVec4(0.22f, 0.24f, 0.26f, 1.00f);        // Hovered tab
    colors[ImGuiCol_TabActive] = ImVec4(0.25f, 0.27f, 0.29f, 1.00f);         // Active tab
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.12f, 0.13f, 0.14f, 1.00f);      // Unfocused tab
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.18f, 0.19f, 0.20f, 1.00f); // Unfocused active tab

    // Docking colors - Professional docking system
    colors[ImGuiCol_DockingPreview] = ImVec4(1.00f, 0.60f, 0.20f, 0.40f);    // Docking preview
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.11f, 0.12f, 0.13f, 1.00f);    // Empty docking area

    // Table colors - Professional data presentation
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.18f, 0.20f, 0.22f, 1.00f);     // Table header
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.35f, 0.37f, 0.39f, 1.00f); // Table border strong
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.25f, 0.27f, 0.29f, 1.00f);  // Table border light
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);        // Table row background
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.18f, 0.20f, 0.22f, 0.30f);     // Alternate table row

    // Text colors - High contrast for dark theme
    colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);              // Main text (light)
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);      // Disabled text
    colors[ImGuiCol_TextSelectedBg] = ImVec4(1.00f, 0.60f, 0.20f, 0.35f);    // Selected text

    // === PROFESSIONAL GAME ENGINE STYLING ===
    style.WindowPadding = ImVec2(8.0f, 8.0f);       // Tighter padding for efficiency
    style.FramePadding = ImVec2(6.0f, 4.0f);        // Compact frame padding
    style.CellPadding = ImVec2(6.0f, 4.0f);         // Cell padding
    style.ItemSpacing = ImVec2(6.0f, 4.0f);         // Tighter item spacing
    style.ItemInnerSpacing = ImVec2(4.0f, 3.0f);    // Inner item spacing
    style.TouchExtraPadding = ImVec2(0.0f, 0.0f);   // No touch padding
    style.IndentSpacing = 18.0f;                    // Indent spacing
    style.ScrollbarSize = 14.0f;                    // Thinner scrollbar
    style.GrabMinSize = 10.0f;                      // Minimum grab size

    // Sharp, professional borders
    style.WindowBorderSize = 1.0f;   // Window border
    style.ChildBorderSize = 1.0f;    // Child border
    style.PopupBorderSize = 1.0f;    // Popup border
    style.FrameBorderSize = 1.0f;    // Frame border (enabled for definition)
    style.TabBorderSize = 0.0f;      // Tab border

    // Minimal rounding for sharp, professional look
    style.WindowRounding = 3.0f;      // Minimal window rounding
    style.ChildRounding = 2.0f;       // Child rounding
    style.FrameRounding = 2.0f;       // Frame rounding
    style.PopupRounding = 3.0f;       // Popup rounding
    style.ScrollbarRounding = 6.0f;   // Scrollbar rounding
    style.GrabRounding = 2.0f;        // Grab rounding
    style.LogSliderDeadzone = 4.0f;   // Log slider deadzone
    style.TabRounding = 2.0f;         // Tab rounding

    // Multi-viewport support
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
}


void EditorApplication::LoadEditorFonts(ImGuiIO& io) {
    // Clear existing fonts
    io.Fonts->Clear();


    // General UI font (replace with your own .ttf for best results)
    io.Fonts->AddFontFromFileTTF("assets/fonts/Geist.ttf", 16.0f);
}

}  // namespace BlacksiteEditor
