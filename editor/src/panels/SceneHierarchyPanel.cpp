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

                ImGui::Text("Entities: %zu", entities.size());
                ImGui::Separator();

                for (const auto& entity : entities) {
                    if (entity.active) {
                        RenderEntityNode(entity);
                    }
                }
            }
        } else {
            ImGui::Text("No active scene");
        }

        // Context menu for empty space
        if (ImGui::BeginPopupContextWindow()) {
            if (ImGui::MenuItem("Create Cube")) {
                m_editorCore->CreateEntity("Cube");
            }
            if (ImGui::MenuItem("Create Sphere")) {
                m_editorCore->CreateEntity("Sphere");
            }
            if (ImGui::MenuItem("Create Plane")) {
                m_editorCore->CreateEntity("Plane");
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

    // Add icon based on entity type
    const char* icon = "🔲"; // Default cube
    switch (entity.shape) {
        case Blacksite::Entity::SPHERE: icon = "🔴"; break;
        case Blacksite::Entity::PLANE: icon = "⬜"; break;
        default: break;
    }

    std::string displayName = std::string(icon) + " " + label;

    bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)entity.id, flags, "%s", displayName.c_str());

    if (ImGui::IsItemClicked()) {
        HandleEntitySelection(entity.id);
    }

    // Context menu for entity
    if (ImGui::BeginPopupContextItem()) {
        m_contextMenuEntityId = entity.id;
        if (ImGui::MenuItem("Focus Camera")) {
            // Focus camera on this entity
            auto* scene = m_editorCore->GetActiveScene();
            if (scene) {
                glm::vec3 entityPos = entity.transform.position;
                scene->SetCameraTarget(entityPos);
                scene->SetCameraPosition(entityPos + glm::vec3(5, 5, 5));
            }
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Duplicate")) {
            m_editorCore->DuplicateEntity(entity.id);
        }
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
    if (m_entitySelectedCallback) {
        m_entitySelectedCallback(entityId);
    }
}

}  // namespace BlacksiteEditor
