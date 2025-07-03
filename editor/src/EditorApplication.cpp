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
    // Render 3D scene first (full screen background)
    GetEngine().RenderFrame();

    // Then render editor UI on top with transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    RenderEditor();

    glDisable(GL_BLEND);
}


void EditorApplication::OnShutdown() {
    ShutdownImGui();
    BS_INFO(Blacksite::LogCategory::CORE, "Blacksite Editor shutdown");
}

// void EditorApplication::SetupImGui() {
//     IMGUI_CHECKVERSION();
//     ImGui::CreateContext();
//     ImGuiIO& io = ImGui::GetIO();
//     io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
//     io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
//     io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

//     // Setup style
//     ImGui::StyleColorsDark();

//     // When viewports are enabled we tweak WindowRounding/WindowBg
//     ImGuiStyle& style = ImGui::GetStyle();
//     if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
//         style.WindowRounding = 0.0f;
//         style.Colors[ImGuiCol_WindowBg].w = 1.0f;
//     }

//     // Setup Platform/Renderer backends
//     ImGui_ImplGlfw_InitForOpenGL(GetEngine().GetGLFWWindow(), true);
//     ImGui_ImplOpenGL3_Init("#version 330");

//     BS_INFO(Blacksite::LogCategory::CORE, "ImGui initialized");
// }
//

void EditorApplication::SetupImGui() {
    BS_INFO(Blacksite::LogCategory::CORE, "Starting ImGui setup...");

    IMGUI_CHECKVERSION();
    ImGuiContext* context = ImGui::CreateContext();

    if (!context) {
        BS_ERROR(Blacksite::LogCategory::CORE, "Failed to create ImGui context!");
        return;
    }
    BS_INFO(Blacksite::LogCategory::CORE, "ImGui context created successfully");

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Setup style
    ImGui::StyleColorsDark();

    // Verify GLFW window
    GLFWwindow* window = GetEngine().GetGLFWWindow();
    if (!window) {
        BS_ERROR(Blacksite::LogCategory::CORE, "GLFW window is null!");
        return;
    }
    BS_INFO(Blacksite::LogCategory::CORE, "GLFW window verified");

    // Setup Platform/Renderer backends with error checking
    bool glfwInit = ImGui_ImplGlfw_InitForOpenGL(window, true);
    if (!glfwInit) {
        BS_ERROR(Blacksite::LogCategory::CORE, "Failed to initialize ImGui GLFW backend!");
        return;
    }
    BS_INFO(Blacksite::LogCategory::CORE, "ImGui GLFW backend initialized");

    bool openglInit = ImGui_ImplOpenGL3_Init("#version 330");
    if (!openglInit) {
        BS_ERROR(Blacksite::LogCategory::CORE, "Failed to initialize ImGui OpenGL backend!");
        return;
    }
    BS_INFO(Blacksite::LogCategory::CORE, "ImGui OpenGL backend initialized");

    BS_INFO(Blacksite::LogCategory::CORE, "ImGui setup completed successfully");
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

    // Render menu bar
    RenderMenuBar();

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
    static bool opt_fullscreen_persistant = true;
    bool opt_fullscreen = opt_fullscreen_persistant;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen) {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove; window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;
    }

    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
    ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    ImGui::End();
}



void EditorApplication::RenderMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                m_editorCore->NewScene("NewScene");
            }
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                m_editorCore->SaveScene("scene.bscn");
            }
            if (ImGui::MenuItem("Load Scene", "Ctrl+O")) {
                m_editorCore->LoadScene("scene.bscn");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                glfwSetWindowShouldClose(GetEngine().GetGLFWWindow(), GLFW_TRUE);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
                // TODO: Implement undo
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {
                // TODO: Implement redo
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Delete Entity", "Delete", false, m_editorCore->HasSelectedEntity())) {
                m_editorCore->DeleteEntity(m_editorCore->GetSelectedEntity());
            }
            if (ImGui::MenuItem("Duplicate Entity", "Ctrl+D", false, m_editorCore->HasSelectedEntity())) {
                m_editorCore->DuplicateEntity(m_editorCore->GetSelectedEntity());
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Grid", nullptr, m_editorCore->IsGridVisible());
            ImGui::MenuItem("Gizmos", nullptr, m_editorCore->IsGizmosVisible());
            ImGui::Separator();
            ImGui::MenuItem("Demo Window", nullptr, &m_showDemoWindow);
            ImGui::MenuItem("Metrics", nullptr, &m_showMetrics);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Create")) {
            if (ImGui::MenuItem("Cube")) {
                m_editorCore->CreateEntity("Cube");
            }
            if (ImGui::MenuItem("Sphere")) {
                m_editorCore->CreateEntity("Sphere");
            }
            if (ImGui::MenuItem("Plane")) {
                m_editorCore->CreateEntity("Plane");
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
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

}  // namespace BlacksiteEditor
