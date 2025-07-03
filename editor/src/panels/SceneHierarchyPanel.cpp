#include "blacksite_editor/panels/SceneHierarchyPanel.h"
#include "blacksite_editor/core/EditorCore.h"
#include <blacksite/scene/Scene.h>
#include <blacksite/core/EntitySystem.h>
#include <imgui.h>

namespace BlacksiteEditor {

SceneHierarchyPanel::SceneHierarchyPanel(EditorCore* editorCore) : m_editorCore(editorCore) {}

void SceneHierarchyPanel::Update(float deltaTime) {
    // Update logic if needed
}

void SceneHierarchyPanel::Render() {
    if (!m_isOpen) return;

    if (ImGui::Begin("Scene Hierarchy", &m_isOpen)) {
        auto* scene = m_editorCore->GetActiveScene();
        if (scene) {
            auto* entitySystem = scene->GetEntitySystem();
            if (entitySystem) {
                const auto& entities = entitySystem->GetEntities();

                // Header with entity count and controls
                ImGui::Text("Entities: %zu", entities.size());
                ImGui::SameLine();

                // Quick create buttons
                if (ImGui::SmallButton("+ Cube")) {
                    m_editorCore->CreateEntity("Cube");
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("+ Sphere")) {
                    m_editorCore->CreateEntity("Sphere");
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("+ Plane")) {
                    m_editorCore->CreateEntity("Plane");
                }

                ImGui::Separator();

                // Search/Filter bar
                static char searchBuffer[256] = "";
                ImGui::InputTextWithHint("##Search", "Search entities...", searchBuffer, sizeof(searchBuffer));
                bool hasSearch = strlen(searchBuffer) > 0;

                ImGui::Separator();

                // Entity list
                for (const auto& entity : entities) {
                    if (entity.active) {
                        // Apply search filter
                        if (hasSearch) {
                            std::string entityName = entity.name.empty() ?
                                "Entity_" + std::to_string(entity.id) : entity.name;
                            if (entityName.find(searchBuffer) == std::string::npos) {
                                continue;
                            }
                        }

                        RenderEntityNode(entity);
                    }
                }
            }
        } else {
            ImGui::Text("No active scene");
        }

        // Context menu for empty space
        if (ImGui::BeginPopupContextWindow("SceneContextMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
            ImGui::Text("Create Entity");
            ImGui::Separator();

            if (ImGui::MenuItem("Cube", "Ctrl+1")) {
                m_editorCore->CreateEntity("Cube");
            }
            if (ImGui::MenuItem("Sphere", "Ctrl+2")) {
                m_editorCore->CreateEntity("Sphere");
            }
            if (ImGui::MenuItem("Plane", "Ctrl+3")) {
                m_editorCore->CreateEntity("Plane");
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Clear Scene")) {
                ImGui::OpenPopup("Clear Scene?");
            }

            ImGui::EndPopup();
        }

        // Clear scene confirmation
        if (ImGui::BeginPopupModal("Clear Scene?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Are you sure you want to clear the entire scene?");
            ImGui::Text("This will delete all entities and cannot be undone.");
            ImGui::Separator();

            if (ImGui::Button("Clear Scene", ImVec2(120, 0))) {
                m_editorCore->ClearScene();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        RenderContextMenu();
    }
    ImGui::End();
}

void SceneHierarchyPanel::RenderEntityNode(const Blacksite::Entity& entity) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

    bool isSelected = (m_editorCore->GetSelectedEntity() == entity.id);
    if (isSelected) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    // Use leaf flag since we don't have hierarchy yet
    flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    std::string label = entity.name.empty() ? "Entity_" + std::to_string(entity.id) : entity.name;

    // Add icon based on entity type (removed emoji icons for professionalism and compatibility)
    const char* icon = ""; // Default cube (no icon)
    switch (entity.shape) {
        case Blacksite::Entity::SPHERE: icon = ""; break;
        case Blacksite::Entity::PLANE: icon = ""; break;
        default: break;
    }

    // Add physics indicator
    std::string physicsIndicator = entity.hasPhysics ?
        (entity.isDynamic ? " [D]" : " [S]") : "";

    std::string displayName = std::string(icon) + " " + label + physicsIndicator;

    bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)entity.id, flags, "%s", displayName.c_str());

    if (ImGui::IsItemClicked()) {
        HandleEntitySelection(entity.id);
    }

    // Double-click to focus camera
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
        auto* scene = m_editorCore->GetActiveScene();
        if (scene) {
            glm::vec3 entityPos = entity.transform.position;
            scene->SetCameraTarget(entityPos);
            scene->SetCameraPosition(entityPos + glm::vec3(5, 5, 5));
        }
    }

    // Context menu for entity
    if (ImGui::BeginPopupContextItem()) {
        m_contextMenuEntityId = entity.id;

        ImGui::Text("Entity: %s", label.c_str());
        ImGui::Separator();

        if (ImGui::MenuItem("Focus Camera", "F")) {
            auto* scene = m_editorCore->GetActiveScene();
            if (scene) {
                glm::vec3 entityPos = entity.transform.position;
                scene->SetCameraTarget(entityPos);
                scene->SetCameraPosition(entityPos + glm::vec3(5, 5, 5));
            }
        }

        if (ImGui::MenuItem("Rename", "F2")) {
            // TODO: Implement inline renaming
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Duplicate", "Ctrl+D")) {
            int duplicatedId = m_editorCore->DuplicateEntity(entity.id);
            if (duplicatedId != -1) {
                m_editorCore->SelectEntity(duplicatedId);
            }
        }

        if (ImGui::MenuItem("Copy", "Ctrl+C")) {
            // TODO: Implement copy to clipboard
        }

        ImGui::Separator();

        // Physics quick actions
        if (entity.hasPhysics) {
            if (entity.isDynamic) {
                if (ImGui::MenuItem("Make Static")) {
                    auto* scene = m_editorCore->GetActiveScene();
                    if (scene) {
                        scene->GetEntity(entity.id).MakeStatic();
                    }
                }
            } else {
                if (ImGui::MenuItem("Make Dynamic")) {
                    auto* scene = m_editorCore->GetActiveScene();
                    if (scene) {
                        scene->GetEntity(entity.id).MakeDynamic();
                    }
                }
            }
        } else {
            if (ImGui::MenuItem("Add Physics")) {
                auto* scene = m_editorCore->GetActiveScene();
                if (scene) {
                    auto* entitySystem = scene->GetEntitySystem();
                    if (entitySystem) {
                        auto* entityPtr = entitySystem->GetEntityPtr(entity.id);
                        if (entityPtr) {
                            entityPtr->hasPhysics = true;
                            scene->GetEntity(entity.id).MakeDynamic();
                        }
                    }
                }
            }
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Delete", "Delete")) {
            m_editorCore->DeleteEntity(entity.id);
        }

        ImGui::EndPopup();
    }
}

void SceneHierarchyPanel::RenderContextMenu() {
    // Additional context menu logic if needed
}

void SceneHierarchyPanel::HandleEntitySelection(int entityId) {
    m_editorCore->SelectEntity(entityId);

    if (m_entitySelectedCallback) {
        m_entitySelectedCallback(entityId);
    }
}

}  // namespace BlacksiteEditor
