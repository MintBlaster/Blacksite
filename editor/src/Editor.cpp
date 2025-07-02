#include "Editor.h"
#include <GLFW/glfw3.h>
#include <iostream>

namespace BlacksiteEditor {

// Forward declarations for panels
class SceneHierarchyPanel : public EditorPanel {
  public:
    SceneHierarchyPanel(Blacksite::Engine* engine, Editor* editor) : m_engine(engine), m_editor(editor) {}

    void Render() override {
        if (!ImGui::Begin("Scene Hierarchy", &m_visible)) {
            ImGui::End();
            return;
        }

        // Scene management section
        if (ImGui::CollapsingHeader("Scene Management", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto* sceneSystem = m_engine->GetSceneSystem();
            auto* activeScene = sceneSystem->GetActiveScene();

            // Current scene info
            if (activeScene) {
                ImGui::Text("Active Scene: %s", activeScene->GetName().c_str());
            } else {
                ImGui::Text("No active scene");
            }

            // Scene list
            auto sceneNames = sceneSystem->GetSceneNames();
            if (ImGui::BeginCombo("Switch Scene", activeScene ? activeScene->GetName().c_str() : "None")) {
                for (const auto& name : sceneNames) {
                    bool isSelected = (activeScene && activeScene->GetName() == name);
                    if (ImGui::Selectable(name.c_str(), isSelected)) {
                        sceneSystem->SwitchToScene(name);
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // Create new scene
            static char newSceneName[64] = "NewScene";
            ImGui::InputText("Scene Name", newSceneName, sizeof(newSceneName));
            ImGui::SameLine();
            if (ImGui::Button("Create Scene")) {
                auto newScene = sceneSystem->CreateScene(newSceneName);
                if (newScene) {
                    sceneSystem->SwitchToScene(newSceneName);
                    // Clear the input
                    strcpy(newSceneName, "NewScene");
                }
            }
        }

        ImGui::Separator();

        // Entity creation buttons (only if we have an active scene)
        auto* activeScene = m_engine->GetSceneSystem()->GetActiveScene();
        if (!activeScene) {
            ImGui::Text("Create or switch to a scene to add entities");
            ImGui::End();
            return;
        }

        if (ImGui::Button("Add Cube")) {
            int id = activeScene->SpawnCube(glm::vec3(0, 2, 0), "basic", glm::vec3(1, 0, 0));
            m_editor->SetSelectedEntity(id);
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Sphere")) {
            int id = activeScene->SpawnSphere(glm::vec3(2, 2, 0), "basic", glm::vec3(0, 1, 0));
            m_editor->SetSelectedEntity(id);
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Plane")) {
            int id = activeScene->SpawnPlane(glm::vec3(0, -1, 0), glm::vec3(10, 1, 10), "basic",
                                             glm::vec3(0.5f, 0.5f, 0.5f));
            m_editor->SetSelectedEntity(id);
        }

        ImGui::Separator();

        // Real entity list from active scene's EntitySystem
        auto* entitySystem = activeScene->GetEntitySystem();
        ImGui::Text("Entities (%zu):", entitySystem->GetEntities().size());

        const auto& entities = entitySystem->GetEntities();
        for (size_t i = 0; i < entities.size(); ++i) {
            const auto& entity = entities[i];
            if (!entity.active)
                continue;

            // Entity icon based on shape
            const char* icon = "?";
            switch (entity.shape) {
                case Blacksite::Entity::CUBE:
                    icon = "📦";
                    break;
                case Blacksite::Entity::SPHERE:
                    icon = "🔵";
                    break;
                case Blacksite::Entity::PLANE:
                    icon = "⬜";
                    break;
            }

            // Entity name with physics indicator
            char label[64];
            snprintf(label, sizeof(label), "%s Entity %zu %s", icon, i, entity.hasPhysics ? "(Dynamic)" : "(Static)");

            bool isSelected = (m_editor->GetSelectedEntity() == (int)i);
            if (ImGui::Selectable(label, isSelected)) {
                m_editor->SetSelectedEntity((int)i);
            }

            // Right-click context menu
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Duplicate")) {
                    entitySystem->DuplicateEntity((int)i);
                }
                if (ImGui::MenuItem("Delete")) {
                    entitySystem->RemoveEntity((int)i);
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
    InspectorPanel(Blacksite::Engine* engine, Editor* editor) : m_engine(engine), m_editor(editor) {}

    void Render() override {
        if (!ImGui::Begin("Inspector", &m_visible)) {
            ImGui::End();
            return;
        }

        auto* activeScene = m_engine->GetSceneSystem()->GetActiveScene();
        if (!activeScene) {
            ImGui::Text("No active scene");
            ImGui::End();
            return;
        }

        int selectedEntity = m_editor->GetSelectedEntity();
        if (selectedEntity >= 0) {
            auto* entitySystem = activeScene->GetEntitySystem();
            Blacksite::Entity* entity = entitySystem->GetEntityPtr(selectedEntity);
            if (!entity) {
                ImGui::Text("Invalid entity selected");
                ImGui::End();
                return;
            }

            ImGui::Text("Entity ID: %d", selectedEntity);
            ImGui::Text("Scene: %s", activeScene->GetName().c_str());
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
                    entitySystem->SetEntityColor(selectedEntity, color);
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
                    entitySystem->SetEntityShader(selectedEntity, shaders[currentShader]);
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
                                bodyID = m_engine->GetPhysicsSystem()->CreateBoxBody(entity->transform.position,
                                                                                     entity->transform.scale, false);
                                break;
                            case Blacksite::Entity::SPHERE:
                                bodyID = m_engine->GetPhysicsSystem()->CreateSphereBody(
                                    entity->transform.position, entity->transform.scale.x, false);
                                break;
                            case Blacksite::Entity::PLANE:
                                bodyID = m_engine->GetPhysicsSystem()->CreatePlaneBody(entity->transform.position);
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
                    ImGui::Text("Active: %s",
                                m_engine->GetPhysicsSystem()->IsBodyActive(entity->physicsBody) ? "Yes" : "No");

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

        auto* activeScene = m_engine->GetSceneSystem()->GetActiveScene();
        if (!activeScene) {
            ImGui::Text("No active scene");
            ImGui::End();
            return;
        }

        ImVec2 viewportSize = ImGui::GetContentRegionAvail();

        // Camera controls
        if (ImGui::CollapsingHeader("Camera Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
            glm::vec3 cameraPos = activeScene->GetCameraPosition();
            float pos[3] = {cameraPos.x, cameraPos.y, cameraPos.z};
            if (ImGui::DragFloat3("Position", pos, 0.1f)) {
                activeScene->SetCameraPosition(glm::vec3(pos[0], pos[1], pos[2]));
            }

            glm::vec3 cameraTarget = activeScene->GetCameraTarget();
            float target[3] = {cameraTarget.x, cameraTarget.y, cameraTarget.z};
            if (ImGui::DragFloat3("Target", target, 0.1f)) {
                activeScene->SetCameraTarget(glm::vec3(target[0], target[1], target[2]));
            }

            // Quick camera presets
            if (ImGui::Button("Top View")) {
                activeScene->SetCameraPosition(glm::vec3(0, 10, 0));
                activeScene->SetCameraTarget(glm::vec3(0, 0, 0));
            }
            ImGui::SameLine();
            if (ImGui::Button("Side View")) {
                activeScene->SetCameraPosition(glm::vec3(10, 2, 0));
                activeScene->SetCameraTarget(glm::vec3(0, 0, 0));
            }
            ImGui::SameLine();
            if (ImGui::Button("Reset")) {
                activeScene->SetCameraPosition(glm::vec3(0, 5, 10));
                activeScene->SetCameraTarget(glm::vec3(0, 0, 0));
            }
        }

        ImGui::Separator();
        ImGui::Text("Viewport Size: %.0fx%.0f", viewportSize.x, viewportSize.y);
        ImGui::Text("Scene: %s", activeScene->GetName().c_str());

        // Physics simulation controls
        if (ImGui::CollapsingHeader("Physics Controls")) {
            if (ImGui::Button("Reset All Physics")) {
                // Reset all dynamic bodies to their original positions
                const auto& entities = activeScene->GetEntitySystem()->GetEntities();
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
        m_logs.push_back("Physics-first engine with Scene System ready!");
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
        auto* activeScene = m_engine->GetSceneSystem()->GetActiveScene();

        if (command == "clear") {
            m_logs.clear();
        } else if (command == "help") {
            m_logs.push_back("Available commands:");
            m_logs.push_back("  clear - Clear console");
            m_logs.push_back("  help - Show this help");
            m_logs.push_back("  spawn cube/sphere/plane - Spawn entities");
            m_logs.push_back("  physics info - Show physics stats");
            m_logs.push_back("  reset physics - Reset all physics bodies");
            m_logs.push_back("  scene info - Show scene information");
            m_logs.push_back("  scene list - List all scenes");
            m_logs.push_back("  scene switch <name> - Switch to scene");
        } else if (command == "spawn cube" && activeScene) {
            int id = activeScene->SpawnCube(glm::vec3(0, 5, 0), "basic", glm::vec3(1, 0, 0));
            m_logs.push_back("Spawned cube with ID: " + std::to_string(id) + " in scene: " + activeScene->GetName());
        } else if (command == "spawn sphere" && activeScene) {
            int id = activeScene->SpawnSphere(glm::vec3(0, 5, 0), "basic", glm::vec3(0, 1, 0));
            m_logs.push_back("Spawned sphere with ID: " + std::to_string(id) + " in scene: " + activeScene->GetName());
        } else if (command == "spawn plane" && activeScene) {
            int id = activeScene->SpawnPlane(glm::vec3(0, -2, 0), glm::vec3(10, 1, 10), "basic", glm::vec3(0.5f));
            m_logs.push_back("Spawned plane with ID: " + std::to_string(id) + " in scene: " + activeScene->GetName());
        } else if (command == "physics info" && activeScene) {
            const auto& entities = activeScene->GetEntitySystem()->GetEntities();
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

            m_logs.push_back("Physics Statistics for scene '" + activeScene->GetName() + "':");
            m_logs.push_back("  Total entities: " + std::to_string(entities.size()));
            m_logs.push_back("  Physics bodies: " + std::to_string(physicsCount));
            m_logs.push_back("  Static bodies: " + std::to_string(staticCount));
            m_logs.push_back("  Dynamic bodies: " + std::to_string(dynamicCount));
        } else if (command == "reset physics" && activeScene) {
            // Reset all physics bodies in active scene
            const auto& entities = activeScene->GetEntitySystem()->GetEntities();
            for (size_t i = 0; i < entities.size(); ++i) {
                const auto& entity = entities[i];
                if (entity.hasPhysics) {
                    m_engine->GetPhysicsSystem()->SetBodyPosition(entity.physicsBody, entity.transform.position);
                    m_engine->GetPhysicsSystem()->SetVelocity(entity.physicsBody, glm::vec3(0));
                    m_engine->GetPhysicsSystem()->SetAngularVelocity(entity.physicsBody, glm::vec3(0));
                }
            }
            m_logs.push_back("Reset all physics bodies in scene: " + activeScene->GetName());
        } else if (command == "scene info") {
            auto* sceneSystem = m_engine->GetSceneSystem();
            if (activeScene) {
                m_logs.push_back("Active Scene: " + activeScene->GetName());
                m_logs.push_back("  Entities: " + std::to_string(activeScene->GetEntitySystem()->GetEntities().size()));
            } else {
                m_logs.push_back("No active scene");
            }
            m_logs.push_back("Total Scenes: " + std::to_string(sceneSystem->GetSceneCount()));
        } else if (command == "scene list") {
            auto sceneNames = m_engine->GetSceneSystem()->GetSceneNames();
            m_logs.push_back("Available Scenes:");
            for (const auto& name : sceneNames) {
                std::string prefix = (activeScene && activeScene->GetName() == name) ? "* " : "  ";
                m_logs.push_back(prefix + name);
            }
        } else if (command.substr(0, 12) == "scene switch") {
            if (command.length() > 13) {
                std::string sceneName = command.substr(13);
                if (m_engine->GetSceneSystem()->SwitchToScene(sceneName)) {
                    m_logs.push_back("Switched to scene: " + sceneName);
                } else {
                    m_logs.push_back("Failed to switch to scene: " + sceneName);
                }
            } else {
                m_logs.push_back("Usage: scene switch <scene_name>");
            }
        } else if (!activeScene) {
            m_logs.push_back("No active scene! Create or switch to a scene first.");
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

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

        // FPS graph
        static float values[90] = {};
        static int values_offset = 0;
        values[values_offset] = io.Framerate;
        values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);

        ImGui::PlotLines("FPS", values, IM_ARRAYSIZE(values), values_offset, nullptr, 0.0f, 120.0f, ImVec2(0, 80));

        ImGui::Separator();

        // Scene and entity statistics
        auto* sceneSystem = m_engine->GetSceneSystem();
        auto* activeScene = sceneSystem->GetActiveScene();

        ImGui::Text("Scenes: %zu", sceneSystem->GetSceneCount());
        if (activeScene) {
            ImGui::Text("Active Scene: %s", activeScene->GetName().c_str());

            const auto& entities = activeScene->GetEntitySystem()->GetEntities();
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
        } else {
            ImGui::Text("No active scene");
        }

        ImGui::Separator();
        ImGui::Text("Memory Usage: ~%.1f MB", 50.0f);  // Placeholder - you could implement actual memory tracking

        ImGui::End();
    }

    const char* GetName() const override { return "Performance"; }

  private:
    Blacksite::Engine* m_engine;
};

class PostProcessPanel : public EditorPanel {
  public:
    PostProcessPanel(Blacksite::Engine* engine) : m_engine(engine) {}

    void Render() override {
        if (!ImGui::Begin("Post-Processing", &m_visible)) {
            ImGui::End();
            return;
        }

        auto* renderer = m_engine->GetRenderer();
        if (!renderer || !renderer->GetPostProcessManager()) {
            ImGui::Text("Post-processing not available");
            ImGui::End();
            return;
        }

        auto* postProcess = renderer->GetPostProcessManager();
        auto& settings = postProcess->GetSettings();

        // Master toggle
        bool postProcessEnabled = renderer->IsPostProcessingEnabled();
        if (ImGui::Checkbox("Enable Post-Processing", &postProcessEnabled)) {
            renderer->EnablePostProcessing(postProcessEnabled);
        }

        if (!postProcessEnabled) {
            ImGui::Text("Post-processing is disabled");
            ImGui::End();
            return;
        }

        ImGui::Separator();

        // Tone Mapping Section
        if (ImGui::CollapsingHeader("Tone Mapping", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Enable Tone Mapping", &settings.enableToneMapping);

            if (settings.enableToneMapping) {
                ImGui::SliderFloat("Exposure", &settings.exposure, 0.1f, 3.0f, "%.2f");
                ImGui::SliderFloat("Gamma", &settings.gamma, 1.0f, 3.0f, "%.2f");

                // Quick presets
                if (ImGui::Button("Dark")) {
                    settings.exposure = 0.8f;
                    settings.gamma = 2.2f;
                }
                ImGui::SameLine();
                if (ImGui::Button("Normal")) {
                    settings.exposure = 1.2f;
                    settings.gamma = 2.2f;
                }
                ImGui::SameLine();
                if (ImGui::Button("Bright")) {
                    settings.exposure = 1.8f;
                    settings.gamma = 2.0f;
                }
            }
        }

        // Bloom Section
        if (ImGui::CollapsingHeader("Bloom", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Enable Bloom", &settings.enableBloom);

            if (settings.enableBloom) {
                ImGui::SliderFloat("Bloom Threshold", &settings.bloomThreshold, 0.1f, 2.0f, "%.2f");
                ImGui::SliderFloat("Bloom Strength", &settings.bloomStrength, 0.0f, 2.0f, "%.2f");
                ImGui::SliderInt("Blur Passes", &settings.bloomBlurPasses, 1, 10);

                ImGui::Separator();
                ImGui::Checkbox("Show Bloom Texture (Debug)", &settings.showBloomTexture);

                // Quick presets
                if (ImGui::Button("Subtle")) {
                    settings.bloomThreshold = 1.2f;
                    settings.bloomStrength = 0.3f;
                    settings.bloomBlurPasses = 3;
                }
                ImGui::SameLine();
                if (ImGui::Button("Normal")) {
                    settings.bloomThreshold = 0.8f;
                    settings.bloomStrength = 0.8f;
                    settings.bloomBlurPasses = 5;
                }
                ImGui::SameLine();
                if (ImGui::Button("Intense")) {
                    settings.bloomThreshold = 0.5f;
                    settings.bloomStrength = 1.5f;
                    settings.bloomBlurPasses = 7;
                }
            }
        }

        // FXAA Section
        if (ImGui::CollapsingHeader("Anti-Aliasing")) {
            ImGui::Checkbox("Enable FXAA", &settings.enableFXAA);

            if (settings.enableFXAA) {
                ImGui::Text("FXAA helps smooth jagged edges");
                ImGui::Text("Performance impact: Low");
            }
        }

        // Performance Info
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Performance Info")) {
            ImGuiIO& io = ImGui::GetIO();
            ImGui::Text("Frame Time: %.3f ms", 1000.0f / io.Framerate);
            ImGui::Text("FPS: %.1f", io.Framerate);

            // Estimate performance impact
            int activeEffects = 0;
            if (settings.enableToneMapping)
                activeEffects++;
            if (settings.enableBloom)
                activeEffects += 2;  // Bloom is more expensive
            if (settings.enableFXAA)
                activeEffects++;

            const char* perfLevel[] = {"Minimal", "Low", "Medium", "High", "Very High"};
            int perfIndex = std::min(activeEffects, 4);
            ImGui::Text("Performance Impact: %s", perfLevel[perfIndex]);
        }

        ImGui::End();
    }

    const char* GetName() const override { return "Post-Processing"; }

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

    // Set hellish theme
    SetupTheme();

    // Initialize panels with proper engine integration
    AddPanel(std::make_unique<SceneHierarchyPanel>(engine, this));
    AddPanel(std::make_unique<InspectorPanel>(engine, this));
    AddPanel(std::make_unique<ViewportPanel>(engine));
    AddPanel(std::make_unique<ConsolePanel>(engine));
    AddPanel(std::make_unique<PerformancePanel>(engine));
    AddPanel(std::make_unique<PostProcessPanel>(engine));

    std::cout << "Blacksite Physics Editor with Scene System initialized successfully" << std::endl;
    return true;
}

void Editor::Update(float deltaTime) {
    if (!m_enabled)
        return;

    HandleShortcuts();

    // Update selected entity's transform from physics if it has physics
    auto* activeScene = m_engine->GetSceneSystem()->GetActiveScene();
    if (activeScene && m_selectedEntity >= 0) {
        Blacksite::Entity* entity = activeScene->GetEntitySystem()->GetEntityPtr(m_selectedEntity);
        if (entity && entity->hasPhysics) {
            // Sync transform from physics system
            entity->transform.position = m_engine->GetPhysicsSystem()->GetBodyPosition(entity->physicsBody);
            entity->transform.rotation = m_engine->GetPhysicsSystem()->GetBodyRotation(entity->physicsBody);
        }
    }
}

void Editor::Render() {
    if (!m_enabled)
        return;

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

    if (m_showDemo)
        ImGui::ShowDemoWindow(&m_showDemo);
    if (m_showMetrics)
        ImGui::ShowMetricsWindow(&m_showMetrics);

    // Prepare for ImGui rendering
    ImGui::Render();

    // Set up OpenGL state for UI rendering
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Render ImGui
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Restore previous OpenGL state
    if (depthTestEnabled)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);

    if (blendEnabled)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);

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
                // Create a new scene
                static int sceneCounter = 1;
                std::string sceneName = "Scene" + std::to_string(sceneCounter++);
                auto newScene = m_engine->GetSceneSystem()->CreateScene(sceneName);
                if (newScene) {
                    m_engine->GetSceneSystem()->SwitchToScene(sceneName);
                    SetSelectedEntity(-1);
                    std::cout << "New Scene created: " << sceneName << std::endl;
                }
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

        if (ImGui::BeginMenu("Scene")) {
            auto* sceneSystem = m_engine->GetSceneSystem();
            auto sceneNames = sceneSystem->GetSceneNames();
            auto* activeScene = sceneSystem->GetActiveScene();

            for (const auto& name : sceneNames) {
                bool isActive = (activeScene && activeScene->GetName() == name);
                if (ImGui::MenuItem(name.c_str(), nullptr, isActive)) {
                    if (!isActive) {
                        sceneSystem->SwitchToScene(name);
                        SetSelectedEntity(-1);
                    }
                }
            }

            if (!sceneNames.empty()) {
                ImGui::Separator();
            }

            if (ImGui::MenuItem("Create New Scene")) {
                static int sceneCounter = 1;
                std::string sceneName = "Scene" + std::to_string(sceneCounter++);
                auto newScene = sceneSystem->CreateScene(sceneName);
                if (newScene) {
                    sceneSystem->SwitchToScene(sceneName);
                    SetSelectedEntity(-1);
                }
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
            auto* activeScene = m_engine->GetSceneSystem()->GetActiveScene();
            if (activeScene) {
                if (ImGui::MenuItem("Spawn Dynamic Cube")) {
                    int id = activeScene->SpawnCube(glm::vec3(0, 5, 0), "basic", glm::vec3(1, 0, 0));
                    SetSelectedEntity(id);
                }
                if (ImGui::MenuItem("Spawn Static Plane")) {
                    int id = activeScene->SpawnPlane(glm::vec3(0, -2, 0), glm::vec3(15, 1, 15), "basic",
                                                     glm::vec3(0.3f, 0.3f, 0.3f));
                    // Make it static
                    Blacksite::Entity* entity = activeScene->GetEntitySystem()->GetEntityPtr(id);
                    if (entity && entity->hasPhysics) {
                        m_engine->GetPhysicsSystem()->MakeBodyStatic(entity->physicsBody);
                    }
                    SetSelectedEntity(id);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Reset All Physics")) {
                    const auto& entities = activeScene->GetEntitySystem()->GetEntities();
                    for (size_t i = 0; i < entities.size(); ++i) {
                        const auto& entity = entities[i];
                        if (entity.hasPhysics) {
                            m_engine->GetPhysicsSystem()->SetBodyPosition(entity.physicsBody,
                                                                          entity.transform.position);
                            m_engine->GetPhysicsSystem()->SetVelocity(entity.physicsBody, glm::vec3(0));
                            m_engine->GetPhysicsSystem()->SetAngularVelocity(entity.physicsBody, glm::vec3(0));
                        }
                    }
                }
            } else {
                ImGui::MenuItem("No active scene", nullptr, false, false);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools")) {
            auto* activeScene = m_engine->GetSceneSystem()->GetActiveScene();
            if (activeScene) {
                if (ImGui::MenuItem("Reset Camera")) {
                    activeScene->SetCameraPosition(glm::vec3(0, 5, 10));
                    activeScene->SetCameraTarget(glm::vec3(0, 0, 0));
                }
            } else {
                ImGui::MenuItem("No active scene", nullptr, false, false);
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
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_NoMove;
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
        ImGuiID dockspace_id = ImGui::GetID("BlacksitePhysicsSceneDockSpace");
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
    auto* activeScene = m_engine->GetSceneSystem()->GetActiveScene();
    if (activeScene && ImGui::IsKeyPressed(ImGuiKey_Delete) && m_selectedEntity >= 0) {
        activeScene->GetEntitySystem()->RemoveEntity(m_selectedEntity);
        SetSelectedEntity(-1);
    }

    // Duplicate selected entity with Ctrl+D
    if (activeScene && io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_D) && m_selectedEntity >= 0) {
        activeScene->GetEntitySystem()->DuplicateEntity(m_selectedEntity);
    }
}

void Editor::SetupTheme() {
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();

    // Smokey Black + Orangish-Red Accent theme
    style.WindowRounding = 3.0f;
    style.FrameRounding = 2.0f;
    style.GrabRounding = 2.0f;
    style.ScrollbarRounding = 2.0f;
    style.TabRounding = 2.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;

    // Clean rendering
    style.AntiAliasedLines = true;
    style.AntiAliasedLinesUseTex = true;
    style.AntiAliasedFill = true;

    // Professional spacing
    style.WindowPadding = ImVec2(8, 8);
    style.FramePadding = ImVec2(6, 4);
    style.ItemSpacing = ImVec2(8, 5);
    style.ItemInnerSpacing = ImVec2(5, 4);
    style.IndentSpacing = 20.0f;
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 12.0f;

    // Smokey Black + Orangish-Red Palette
    ImVec4* colors = style.Colors;

    // Base: Smokey blacks and grays (no color)
    colors[ImGuiCol_WindowBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.95f);        // Pure black
    colors[ImGuiCol_ChildBg] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);         // Smokey black
    colors[ImGuiCol_PopupBg] = ImVec4(0.01f, 0.01f, 0.01f, 0.98f);         // Deeper black
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);       // Charcoal

    // Text: Pure white on black
    colors[ImGuiCol_Text] = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);            // Pure white
    colors[ImGuiCol_TextDisabled] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);    // Gray

    // Input fields: Smokey grays (no color)
    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);         // Dark gray
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);  // Medium gray
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);   // Light gray

    // Scrollbars: Gray base
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);

    // Tables: Clean grays
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.02f);

    // ORANGISH-RED ACCENTS (different intensity levels)

    // Level 1: Subtle (30% intensity) - Borders and separators
    colors[ImGuiCol_Border] = ImVec4(0.30f, 0.12f, 0.06f, 0.70f);          // Subtle orangish-red
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
    colors[ImGuiCol_Separator] = ImVec4(0.35f, 0.14f, 0.07f, 0.60f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.50f, 0.20f, 0.10f, 0.80f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.65f, 0.26f, 0.13f, 1.00f);

    // Level 2: Medium (50% intensity) - Headers and tabs
    colors[ImGuiCol_Header] = ImVec4(0.50f, 0.20f, 0.10f, 0.70f);          // Medium orangish-red
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.60f, 0.24f, 0.12f, 0.85f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.70f, 0.28f, 0.14f, 1.00f);

    colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);             // Gray base
    colors[ImGuiCol_TabHovered] = ImVec4(0.45f, 0.18f, 0.09f, 1.00f);      // Medium accent
    colors[ImGuiCol_TabActive] = ImVec4(0.55f, 0.22f, 0.11f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.35f, 0.14f, 0.07f, 1.00f);

    colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);         // Gray base
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.45f, 0.18f, 0.09f, 1.00f);   // Medium accent
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.05f, 0.05f, 0.05f, 0.80f);

    // Level 3: Strong (70% intensity) - Buttons and interactive elements
    colors[ImGuiCol_Button] = ImVec4(0.70f, 0.28f, 0.14f, 0.80f);          // Strong orangish-red
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.80f, 0.32f, 0.16f, 0.90f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.90f, 0.36f, 0.18f, 1.00f);

    colors[ImGuiCol_SliderGrab] = ImVec4(0.75f, 0.30f, 0.15f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.85f, 0.34f, 0.17f, 1.00f);

    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.70f, 0.28f, 0.14f, 1.00f);

    colors[ImGuiCol_ResizeGrip] = ImVec4(0.65f, 0.26f, 0.13f, 0.60f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.75f, 0.30f, 0.15f, 0.80f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.85f, 0.34f, 0.17f, 1.00f);

    // Level 4: Bright (90% intensity) - Check marks and highlights
    colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.36f, 0.18f, 1.00f);       // Bright orangish-red
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.85f, 0.34f, 0.17f, 0.35f);

    colors[ImGuiCol_DragDropTarget] = ImVec4(0.95f, 0.38f, 0.19f, 0.80f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.90f, 0.36f, 0.18f, 1.00f);

    // Docking and plots
    colors[ImGuiCol_DockingPreview] = ImVec4(0.80f, 0.32f, 0.16f, 0.50f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);

    colors[ImGuiCol_PlotLines] = ImVec4(0.85f, 0.34f, 0.17f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.95f, 0.38f, 0.19f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.75f, 0.30f, 0.15f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.85f, 0.34f, 0.17f, 1.00f);

    // Modal and navigation
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.85f);
}


}  // namespace BlacksiteEditor
