#include "Editor.h"
#include <GLFW/glfw3.h>
#include <iostream>

namespace BlacksiteEditor {

// Forward declarations for panels
class SceneHierarchyPanel : public EditorPanel {
public:
    SceneHierarchyPanel(Blacksite::Engine* engine, Editor* editor)
        : m_engine(engine), m_editor(editor) {}

    void Render() override {
        if (!ImGui::Begin("Scene Hierarchy", &m_visible)) {
            ImGui::End();
            return;
        }

        // Entity creation buttons
        if (ImGui::Button("Add Cube")) {
            int id = m_engine->SpawnCube(glm::vec3(0, 2, 0), "basic", glm::vec3(1, 0, 0));
            m_editor->SetSelectedEntity(id);
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Sphere")) {
            int id = m_engine->SpawnSphere(glm::vec3(2, 2, 0), "basic", glm::vec3(0, 1, 0));
            m_editor->SetSelectedEntity(id);
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Plane")) {
            int id = m_engine->SpawnPlane(glm::vec3(0, -1, 0), glm::vec3(10, 1, 10), "basic", glm::vec3(0.5f, 0.5f, 0.5f));
            m_editor->SetSelectedEntity(id);
        }

        ImGui::Separator();

        // Real entity list from EntitySystem
        ImGui::Text("Entities (%zu):", m_engine->GetEntitySystem()->GetEntities().size());

        const auto& entities = m_engine->GetEntitySystem()->GetEntities();
        for (size_t i = 0; i < entities.size(); ++i) {
            const auto& entity = entities[i];
            if (!entity.active) continue;

            // Entity icon based on shape
            const char* icon = "?";
            switch (entity.shape) {
                case Blacksite::Entity::CUBE: icon = "📦"; break;
                case Blacksite::Entity::SPHERE: icon = "🔵"; break;
                case Blacksite::Entity::PLANE: icon = "⬜"; break;
            }

            // Entity name with physics indicator
            char label[64];
            snprintf(label, sizeof(label), "%s Entity %zu %s",
                    icon, i, entity.hasPhysics ? "(Dynamic)" : "(Static)");

            bool isSelected = (m_editor->GetSelectedEntity() == (int)i);
            if (ImGui::Selectable(label, isSelected)) {
                m_editor->SetSelectedEntity((int)i);
            }

            // Right-click context menu
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Duplicate")) {
                    m_engine->GetEntitySystem()->DuplicateEntity((int)i);
                }
                if (ImGui::MenuItem("Delete")) {
                    m_engine->GetEntitySystem()->RemoveEntity((int)i);
                    if (m_editor->GetSelectedEntity() == (int)i) {
                        m_editor->SetSelectedEntity(-1);
                    }
                }
                ImGui::EndPopup();
            }
        }

        ImGui::End();
    }

    const char* GetName() const override { return "Scene Hierarchy"; }

private:
    Blacksite::Engine* m_engine;
    Editor* m_editor;
};

class InspectorPanel : public EditorPanel {
public:
    InspectorPanel(Blacksite::Engine* engine, Editor* editor)
        : m_engine(engine), m_editor(editor) {}

    void Render() override {
        if (!ImGui::Begin("Inspector", &m_visible)) {
            ImGui::End();
            return;
        }

        int selectedEntity = m_editor->GetSelectedEntity();
        if (selectedEntity >= 0) {
            Blacksite::Entity* entity = m_engine->GetEntitySystem()->GetEntityPtr(selectedEntity);
            if (!entity) {
                ImGui::Text("Invalid entity selected");
                ImGui::End();
                return;
            }

            ImGui::Text("Entity ID: %d", selectedEntity);
            ImGui::Separator();

            // Transform Component
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                glm::vec3 pos = entity->transform.position;
                glm::vec3 rot = entity->transform.rotation;
                glm::vec3 scale = entity->transform.scale;

                if (ImGui::DragFloat3("Position", &pos.x, 0.1f)) {
                    entity->transform.position = pos;
                    // Update physics body if it exists
                    if (entity->hasPhysics) {
                        m_engine->GetPhysicsSystem()->SetBodyPosition(entity->physicsBody, pos);
                    }
                }

                if (ImGui::DragFloat3("Rotation", &rot.x, 1.0f)) {
                    entity->transform.rotation = rot;
                    if (entity->hasPhysics) {
                        m_engine->GetPhysicsSystem()->SetBodyRotation(entity->physicsBody, rot);
                    }
                }

                if (ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.1f, 10.0f)) {
                    entity->transform.scale = scale;
                    // Note: Jolt doesn't support runtime scale changes easily
                    // You might need to recreate the physics body for scale changes
                }
            }

            // Renderer Component
            if (ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
                glm::vec3 color = entity->color;
                if (ImGui::ColorEdit3("Color", &color.x)) {
                    entity->color = color;
                    m_engine->GetEntitySystem()->SetEntityColor(selectedEntity, color);
                }

                // Shape display (read-only for now)
                const char* shapeNames[] = {"Cube", "Sphere", "Plane"};
                int currentShape = (int)entity->shape;
                ImGui::Text("Shape: %s", shapeNames[currentShape]);

                // Shader selection
                const char* shaders[] = {"basic", "rainbow", "glow", "holo", "wireframe", "plasma"};
                int currentShader = 0;

                // Find current shader index
                for (int i = 0; i < IM_ARRAYSIZE(shaders); ++i) {
                    if (entity->shader == shaders[i]) {
                        currentShader = i;
                        break;
                    }
                }

                if (ImGui::Combo("Shader", &currentShader, shaders, IM_ARRAYSIZE(shaders))) {
                    entity->shader = shaders[currentShader];
                    m_engine->GetEntitySystem()->SetEntityShader(selectedEntity, shaders[currentShader]);
                }
            }

            // Physics Component - The main feature!
            if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen)) {
                bool hasPhysics = entity->hasPhysics;
                if (ImGui::Checkbox("Enable Physics", &hasPhysics)) {
                    if (hasPhysics && !entity->hasPhysics) {
                        // Create physics body
                        JPH::BodyID bodyID;
                        switch (entity->shape) {
                            case Blacksite::Entity::CUBE:
                                bodyID = m_engine->GetPhysicsSystem()->CreateBoxBody(
                                    entity->transform.position, entity->transform.scale, false);
                                break;
                            case Blacksite::Entity::SPHERE:
                                bodyID = m_engine->GetPhysicsSystem()->CreateSphereBody(
                                    entity->transform.position, entity->transform.scale.x, false);
                                break;
                            case Blacksite::Entity::PLANE:
                                bodyID = m_engine->GetPhysicsSystem()->CreatePlaneBody(
                                    entity->transform.position);
                                break;
                        }
                        entity->physicsBody = bodyID;
                        entity->hasPhysics = true;
                        m_engine->GetPhysicsSystem()->MapEntityToBody(selectedEntity, bodyID);
                    } else if (!hasPhysics && entity->hasPhysics) {
                        // Remove physics (you might need to implement body removal in PhysicsSystem)
                        entity->hasPhysics = false;
                        m_engine->GetPhysicsSystem()->UnmapEntity(selectedEntity);
                    }
                }

                if (entity->hasPhysics) {
                    ImGui::Separator();

                    // Static/Dynamic toggle - Key feature of your physics-first engine!
                    bool isStatic = m_engine->GetPhysicsSystem()->IsBodyStatic(entity->physicsBody);
                    bool wasStatic = isStatic;

                    if (ImGui::Checkbox("Static Body", &isStatic)) {
                        if (isStatic && !wasStatic) {
                            m_engine->GetPhysicsSystem()->MakeBodyStatic(entity->physicsBody);
                        } else if (!isStatic && wasStatic) {
                            m_engine->GetPhysicsSystem()->MakeBodyDynamic(entity->physicsBody);
                        }
                    }

                    // Physics properties for dynamic bodies
                    if (!isStatic) {
                        ImGui::Separator();
                        ImGui::Text("Dynamic Properties:");

                        // Mass (read-only for now, as Jolt handles this internally)
                        float mass = m_engine->GetPhysicsSystem()->GetBodyMass(entity->physicsBody);
                        ImGui::Text("Mass: %.2f kg", mass);

                        // Velocity controls
                        glm::vec3 velocity = m_engine->GetPhysicsSystem()->GetVelocity(entity->physicsBody);
                        if (ImGui::DragFloat3("Velocity", &velocity.x, 0.1f, -50.0f, 50.0f)) {
                            m_engine->GetPhysicsSystem()->SetVelocity(entity->physicsBody, velocity);
                        }

                        glm::vec3 angularVel = m_engine->GetPhysicsSystem()->GetAngularVelocity(entity->physicsBody);
                        if (ImGui::DragFloat3("Angular Velocity", &angularVel.x, 0.1f, -10.0f, 10.0f)) {
                            m_engine->GetPhysicsSystem()->SetAngularVelocity(entity->physicsBody, angularVel);
                        }

                        // Force application
                        ImGui::Separator();
                        ImGui::Text("Apply Forces:");

                        static glm::vec3 forceToApply(0.0f, 0.0f, 0.0f);
                        ImGui::DragFloat3("Force Vector", &forceToApply.x, 0.1f, -100.0f, 100.0f);

                        if (ImGui::Button("Apply Force")) {
                            m_engine->GetPhysicsSystem()->AddForce(entity->physicsBody, forceToApply);
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Apply Impulse")) {
                            m_engine->GetPhysicsSystem()->AddImpulse(entity->physicsBody, forceToApply);
                        }

                        // Quick force buttons
                        if (ImGui::Button("Jump (+Y)")) {
                            m_engine->GetPhysicsSystem()->AddImpulse(entity->physicsBody, glm::vec3(0, 10, 0));
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Push (+X)")) {
                            m_engine->GetPhysicsSystem()->AddImpulse(entity->physicsBody, glm::vec3(5, 0, 0));
                        }
                    }

                    // Body state info
                    ImGui::Separator();
                    ImGui::Text("Physics State:");
                    ImGui::Text("Active: %s", m_engine->GetPhysicsSystem()->IsBodyActive(entity->physicsBody) ? "Yes" : "No");

                    glm::vec3 physicsPos = m_engine->GetPhysicsSystem()->GetBodyPosition(entity->physicsBody);
                    ImGui::Text("Physics Pos: (%.2f, %.2f, %.2f)", physicsPos.x, physicsPos.y, physicsPos.z);
                }
            }

        } else {
            ImGui::Text("No entity selected");
            ImGui::Text("Select an entity from the Scene Hierarchy");
        }

        ImGui::End();
    }

    const char* GetName() const override { return "Inspector"; }

private:
    Blacksite::Engine* m_engine;
    Editor* m_editor;
};

class ViewportPanel : public EditorPanel {
public:
    ViewportPanel(Blacksite::Engine* engine) : m_engine(engine) {}

    void Render() override {
        if (!ImGui::Begin("Viewport", &m_visible)) {
            ImGui::End();
            return;
        }

        ImVec2 viewportSize = ImGui::GetContentRegionAvail();

        // Camera controls
        if (ImGui::CollapsingHeader("Camera Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
            glm::vec3 cameraPos = m_engine->GetCameraPosition();
            float pos[3] = {cameraPos.x, cameraPos.y, cameraPos.z};
            if (ImGui::DragFloat3("Position", pos, 0.1f)) {
                m_engine->SetCameraPosition(glm::vec3(pos[0], pos[1], pos[2]));
            }

            glm::vec3 cameraTarget = m_engine->GetCameraTarget();
            float target[3] = {cameraTarget.x, cameraTarget.y, cameraTarget.z};
            if (ImGui::DragFloat3("Target", target, 0.1f)) {
                m_engine->SetCameraTarget(glm::vec3(target[0], target[1], target[2]));
            }

            // Quick camera presets
            if (ImGui::Button("Top View")) {
                m_engine->SetCameraPosition(glm::vec3(0, 10, 0));
                m_engine->SetCameraTarget(glm::vec3(0, 0, 0));
            }
            ImGui::SameLine();
            if (ImGui::Button("Side View")) {
                m_engine->SetCameraPosition(glm::vec3(10, 2, 0));
                m_engine->SetCameraTarget(glm::vec3(0, 0, 0));
            }
            ImGui::SameLine();
            if (ImGui::Button("Reset")) {
                m_engine->SetCameraPosition(glm::vec3(0, 5, 10));
                m_engine->SetCameraTarget(glm::vec3(0, 0, 0));
            }
        }

        ImGui::Separator();
        ImGui::Text("Viewport Size: %.0fx%.0f", viewportSize.x, viewportSize.y);

        // Physics simulation controls
        if (ImGui::CollapsingHeader("Physics Controls")) {
            if (ImGui::Button("Reset All Physics")) {
                // Reset all dynamic bodies to their original positions
                const auto& entities = m_engine->GetEntitySystem()->GetEntities();
                for (size_t i = 0; i < entities.size(); ++i) {
                    const auto& entity = entities[i];
                    if (entity.hasPhysics && !m_engine->GetPhysicsSystem()->IsBodyStatic(entity.physicsBody)) {
                        m_engine->GetPhysicsSystem()->SetBodyPosition(entity.physicsBody, entity.transform.position);
                        m_engine->GetPhysicsSystem()->SetVelocity(entity.physicsBody, glm::vec3(0));
                        m_engine->GetPhysicsSystem()->SetAngularVelocity(entity.physicsBody, glm::vec3(0));
                    }
                }
            }
        }

        // TODO: Render actual 3D viewport here
        // For now, just show a placeholder
        ImGui::Button("3D Viewport (Scene renders behind this)", viewportSize);

        ImGui::End();
    }

    const char* GetName() const override { return "Viewport"; }

private:
    Blacksite::Engine* m_engine;
};

class ConsolePanel : public EditorPanel {
public:
    ConsolePanel(Blacksite::Engine* engine) : m_engine(engine) {
        m_logs.push_back("Blacksite Physics Editor initialized");
        m_logs.push_back("Physics-first engine ready!");
        m_logs.push_back("Type 'help' for available commands");
    }

    void Render() override {
        if (!ImGui::Begin("Console", &m_visible)) {
            ImGui::End();
            return;
        }

        // Log display
        ImGui::BeginChild("LogRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);
        for (const auto& log : m_logs) {
            ImGui::TextUnformatted(log.c_str());
        }
        if (m_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();

        // Command input
        static char inputBuffer[256] = "";
        if (ImGui::InputText("Command", inputBuffer, sizeof(inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (strlen(inputBuffer) > 0) {
                m_logs.push_back(std::string("> ") + inputBuffer);
                ProcessCommand(inputBuffer);
                inputBuffer[0] = '\0';
            }
        }

        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &m_autoScroll);

        ImGui::End();
    }

    const char* GetName() const override { return "Console"; }

private:
    void ProcessCommand(const std::string& command) {
        if (command == "clear") {
            m_logs.clear();
        } else if (command == "help") {
            m_logs.push_back("Available commands:");
            m_logs.push_back("  clear - Clear console");
            m_logs.push_back("  help - Show this help");
            m_logs.push_back("  spawn cube/sphere/plane - Spawn entities");
            m_logs.push_back("  physics info - Show physics stats");
            m_logs.push_back("  reset physics - Reset all physics bodies");
        } else if (command == "spawn cube") {
            int id = m_engine->SpawnCube(glm::vec3(0, 5, 0), "basic", glm::vec3(1, 0, 0));
            m_logs.push_back("Spawned cube with ID: " + std::to_string(id));
        } else if (command == "spawn sphere") {
            int id = m_engine->SpawnSphere(glm::vec3(0, 5, 0), "basic", glm::vec3(0, 1, 0));
            m_logs.push_back("Spawned sphere with ID: " + std::to_string(id));
        } else if (command == "spawn plane") {
            int id = m_engine->SpawnPlane(glm::vec3(0, -2, 0), glm::vec3(10, 1, 10), "basic", glm::vec3(0.5f));
            m_logs.push_back("Spawned plane with ID: " + std::to_string(id));
        } else if (command == "physics info") {
            const auto& entities = m_engine->GetEntitySystem()->GetEntities();
            int physicsCount = 0;
            int staticCount = 0;
            int dynamicCount = 0;

            for (const auto& entity : entities) {
                if (entity.hasPhysics) {
                    physicsCount++;
                    if (m_engine->GetPhysicsSystem()->IsBodyStatic(entity.physicsBody)) {
                        staticCount++;
                    } else {
                        dynamicCount++;
                    }
                }
            }

            m_logs.push_back("Physics Statistics:");
            m_logs.push_back("  Total entities: " + std::to_string(entities.size()));
            m_logs.push_back("  Physics bodies: " + std::to_string(physicsCount));
            m_logs.push_back("  Static bodies: " + std::to_string(staticCount));
            m_logs.push_back("  Dynamic bodies: " + std::to_string(dynamicCount));
        } else if (command == "reset physics") {
            // Reset all physics bodies
            const auto& entities = m_engine->GetEntitySystem()->GetEntities();
            for (size_t i = 0; i < entities.size(); ++i) {
                const auto& entity = entities[i];
                if (entity.hasPhysics) {
                    m_engine->GetPhysicsSystem()->SetBodyPosition(entity.physicsBody, entity.transform.position);
                    m_engine->GetPhysicsSystem()->SetVelocity(entity.physicsBody, glm::vec3(0));
                    m_engine->GetPhysicsSystem()->SetAngularVelocity(entity.physicsBody, glm::vec3(0));
                }
            }
            m_logs.push_back("Reset all physics bodies");
        } else {
            m_logs.push_back("Unknown command: " + command + " (type 'help' for commands)");
        }
    }

    std::vector<std::string> m_logs;
    bool m_autoScroll = true;
    Blacksite::Engine* m_engine;
};

class PerformancePanel : public EditorPanel {
public:
    PerformancePanel(Blacksite::Engine* engine) : m_engine(engine) {}

    void Render() override {
        if (!ImGui::Begin("Performance", &m_visible)) {
            ImGui::End();
            return;
        }

        ImGuiIO& io = ImGui::GetIO();

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                   1000.0f / io.Framerate, io.Framerate);

        // FPS graph
        static float values[90] = {};
        static int values_offset = 0;
        values[values_offset] = io.Framerate;
        values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);

        ImGui::PlotLines("FPS", values, IM_ARRAYSIZE(values), values_offset,
                        nullptr, 0.0f, 120.0f, ImVec2(0, 80));

        ImGui::Separator();

        // Entity and physics statistics
        const auto& entities = m_engine->GetEntitySystem()->GetEntities();
        int physicsCount = 0;
        int staticCount = 0;
        int dynamicCount = 0;

        for (const auto& entity : entities) {
            if (entity.hasPhysics) {
                physicsCount++;
                if (m_engine->GetPhysicsSystem()->IsBodyStatic(entity.physicsBody)) {
                    staticCount++;
                } else {
                    dynamicCount++;
                }
            }
        }

        ImGui::Text("Entities: %zu", entities.size());
        ImGui::Text("Physics Bodies: %d", physicsCount);
        ImGui::Text("  Static: %d", staticCount);
        ImGui::Text("  Dynamic: %d", dynamicCount);

        ImGui::Separator();
        ImGui::Text("Memory Usage: ~%.1f MB", 50.0f); // Placeholder - you could implement actual memory tracking

        ImGui::End();
    }

    const char* GetName() const override { return "Performance"; }

private:
    Blacksite::Engine* m_engine;
};

// Editor implementation
Editor::Editor() = default;
Editor::~Editor() = default;

bool Editor::Initialize(Blacksite::Engine* engine, GLFWwindow* window) {
    m_engine = engine;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Enable docking and viewports
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Setup platform/renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Set theme
    SetupTheme();

    // Initialize panels with proper engine integration
    AddPanel(std::make_unique<SceneHierarchyPanel>(engine, this));
    AddPanel(std::make_unique<InspectorPanel>(engine, this));
    AddPanel(std::make_unique<ViewportPanel>(engine));
    AddPanel(std::make_unique<ConsolePanel>(engine));
    AddPanel(std::make_unique<PerformancePanel>(engine));

    std::cout << "Blacksite Physics Editor initialized successfully" << std::endl;
    return true;
}

void Editor::Update(float deltaTime) {
    if (!m_enabled) return;

    HandleShortcuts();

    // Update selected entity's transform from physics if it has physics
    if (m_selectedEntity >= 0) {
        Blacksite::Entity* entity = m_engine->GetEntitySystem()->GetEntityPtr(m_selectedEntity);
        if (entity && entity->hasPhysics) {
            // Sync transform from physics system
            entity->transform.position = m_engine->GetPhysicsSystem()->GetBodyPosition(entity->physicsBody);
            entity->transform.rotation = m_engine->GetPhysicsSystem()->GetBodyRotation(entity->physicsBody);
        }
    }
}

void Editor::Render() {
    if (!m_enabled) return;

    // Save current OpenGL state
    GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLboolean blendEnabled = glIsEnabled(GL_BLEND);
    GLint blendSrc, blendDst;
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrc);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &blendDst);

    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    RenderDockSpace();
    RenderMainMenuBar();

    // Render all panels
    for (auto& panel : m_panels) {
        if (panel->IsVisible()) {
            panel->Render();
        }
    }

    if (m_showDemo) ImGui::ShowDemoWindow(&m_showDemo);
    if (m_showMetrics) ImGui::ShowMetricsWindow(&m_showMetrics);

    // Prepare for ImGui rendering
    ImGui::Render();

    // Set up OpenGL state for UI rendering
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Render ImGui
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Restore previous OpenGL state
    if (depthTestEnabled) glEnable(GL_DEPTH_TEST);
    else glDisable(GL_DEPTH_TEST);

    if (blendEnabled) glEnable(GL_BLEND);
    else glDisable(GL_BLEND);

    // Handle viewports
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void Editor::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    std::cout << "Blacksite Physics Editor shutdown complete" << std::endl;
}

void Editor::AddPanel(std::unique_ptr<EditorPanel> panel) {
    m_panels.push_back(std::move(panel));
}

EditorPanel* Editor::GetPanel(const std::string& name) {
    for (auto& panel : m_panels) {
        if (panel->GetName() == name) {
            return panel.get();
        }
    }
    return nullptr;
}

void Editor::RenderMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                // Clear all entities
                auto& entities = m_engine->GetEntitySystem()->GetEntities();
                for (size_t i = entities.size(); i > 0; --i) {
                    m_engine->GetEntitySystem()->RemoveEntity(i - 1);
                }
                SetSelectedEntity(-1);
                std::cout << "New Scene created" << std::endl;
            }
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                std::cout << "Save Scene requested (not implemented)" << std::endl;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                std::cout << "Exit requested" << std::endl;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
                std::cout << "Undo requested (not implemented)" << std::endl;
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {
                std::cout << "Redo requested (not implemented)" << std::endl;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            for (auto& panel : m_panels) {
                bool visible = panel->IsVisible();
                if (ImGui::MenuItem(panel->GetName(), nullptr, &visible)) {
                    panel->SetVisible(visible);
                }
            }
            ImGui::Separator();
            ImGui::MenuItem("Demo Window", nullptr, &m_showDemo);
            ImGui::MenuItem("Metrics", nullptr, &m_showMetrics);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Physics")) {
            if (ImGui::MenuItem("Spawn Dynamic Cube")) {
                int id = m_engine->SpawnCube(glm::vec3(0, 5, 0), "basic", glm::vec3(1, 0, 0));
                SetSelectedEntity(id);
            }
            if (ImGui::MenuItem("Spawn Static Plane")) {
                int id = m_engine->SpawnPlane(glm::vec3(0, -2, 0), glm::vec3(15, 1, 15), "basic", glm::vec3(0.3f, 0.3f, 0.3f));
                // Make it static
                Blacksite::Entity* entity = m_engine->GetEntitySystem()->GetEntityPtr(id);
                if (entity && entity->hasPhysics) {
                    m_engine->GetPhysicsSystem()->MakeBodyStatic(entity->physicsBody);
                }
                SetSelectedEntity(id);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Reset All Physics")) {
                const auto& entities = m_engine->GetEntitySystem()->GetEntities();
                for (size_t i = 0; i < entities.size(); ++i) {
                    const auto& entity = entities[i];
                    if (entity.hasPhysics) {
                        m_engine->GetPhysicsSystem()->SetBodyPosition(entity.physicsBody, entity.transform.position);
                        m_engine->GetPhysicsSystem()->SetVelocity(entity.physicsBody, glm::vec3(0));
                        m_engine->GetPhysicsSystem()->SetAngularVelocity(entity.physicsBody, glm::vec3(0));
                    }
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Reset Camera")) {
                m_engine->SetCameraPosition(glm::vec3(0, 5, 10));
                m_engine->SetCameraTarget(glm::vec3(0, 0, 0));
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void Editor::RenderDockSpace() {
    static bool opt_fullscreen = true;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen) {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("BlacksitePhysicsDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    ImGui::End();
}

void Editor::HandleShortcuts() {
    ImGuiIO& io = ImGui::GetIO();

    // Toggle editor with F1
    if (ImGui::IsKeyPressed(ImGuiKey_F1)) {
        ToggleEnabled();
    }

    // Delete selected entity with Delete key
    if (ImGui::IsKeyPressed(ImGuiKey_Delete) && m_selectedEntity >= 0) {
        m_engine->GetEntitySystem()->RemoveEntity(m_selectedEntity);
        SetSelectedEntity(-1);
    }

    // Duplicate selected entity with Ctrl+D
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_D) && m_selectedEntity >= 0) {
        m_engine->GetEntitySystem()->DuplicateEntity(m_selectedEntity);
    }
}

void Editor::SetupTheme() {
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();

    // Physics-themed dark style
    style.WindowRounding = 5.0f;
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.ScrollbarRounding = 3.0f;
    style.TabRounding = 3.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.40f, 0.80f, 0.31f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.40f, 0.80f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.40f, 0.80f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.40f, 0.80f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.20f, 0.40f, 0.80f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.35f, 0.75f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.30f, 0.70f, 1.00f, 1.00f);
}

} // namespace BlacksiteEditor
