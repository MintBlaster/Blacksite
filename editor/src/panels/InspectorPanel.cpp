#include "blacksite_editor/panels/InspectorPanel.h"
#include "blacksite_editor/core/EditorCore.h"
#include "imgui_internal.h"
#include <blacksite/scene/Scene.h>
#include <blacksite/core/EntitySystem.h>
#include <imgui.h>

namespace BlacksiteEditor {

InspectorPanel::InspectorPanel(EditorCore* editorCore) : m_editorCore(editorCore) {}

void InspectorPanel::Update(float deltaTime) {
    // Update logic if needed
}

void InspectorPanel::Render() {
    if (!m_isOpen) return;

    if (ImGui::Begin("Inspector", &m_isOpen)) {
        if (m_editorCore->HasSelectedEntity()) {
            auto* scene = m_editorCore->GetActiveScene();
            if (scene) {
                auto* entitySystem = scene->GetEntitySystem();
                if (entitySystem) {
                    auto* entity = entitySystem->GetEntityPtr(m_editorCore->GetSelectedEntity());
                    if (entity && entity->active) {
                        RenderEntityInspector(entity);
                    } else {
                        ImGui::Text("Selected entity is invalid or inactive");
                    }
                }
            }
        } else {
            ImGui::Text("No entity selected");
            ImGui::Separator();
            ImGui::Text("Select an entity from the Scene Hierarchy");
            ImGui::Text("to view and edit its properties.");
        }
    }
    ImGui::End();
}

void InspectorPanel::RenderEntityInspector(Blacksite::Entity* entity) {
    // Entity header with ID and active status
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::Text("Entity ID: %d", entity->id);
    ImGui::PopFont();

    ImGui::SameLine();
    ImGui::Checkbox("##Active", &entity->active);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Entity Active");
    }

    // Entity name
    char nameBuffer[256];
    strncpy(nameBuffer, entity->name.c_str(), sizeof(nameBuffer));
    nameBuffer[sizeof(nameBuffer) - 1] = '\0';

    if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
        entity->name = nameBuffer;
    }

    ImGui::Separator();

    // Transform component
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        RenderTransformComponent(entity);
    }

    // Physics component
    if (entity->hasPhysics) {
        if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen)) {
            RenderPhysicsComponent(entity);
        }
    }

    // Render component
    if (ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
        RenderRenderComponent(entity);
    }

    ImGui::Separator();
    RenderEntityActions(entity);

    ImGui::Separator();
    RenderComponentButtons(entity);
}

void InspectorPanel::RenderTransformComponent(Blacksite::Entity* entity) {
    auto& transform = entity->transform;

    // Store original values to detect changes
    glm::vec3 originalPos = transform.position;
    glm::vec3 originalRot = transform.rotation;
    glm::vec3 originalScale = transform.scale;

    bool transformChanged = false;

    transformChanged |= DrawVec3Control("Position", transform.position);
    transformChanged |= DrawVec3Control("Rotation", transform.rotation);
    transformChanged |= DrawVec3Control("Scale", transform.scale, 1.0f);

    if (transformChanged) {
        // Update physics body if entity has physics
        if (entity->hasPhysics) {
            auto* scene = m_editorCore->GetActiveScene();
            if (scene) {
                auto handle = scene->GetEntity(entity->id);

                // Update position if changed
                if (originalPos != transform.position) {
                    handle.At(transform.position);
                }

                // Update rotation if changed
                if (originalRot != transform.rotation) {
                    handle.Rotate(transform.rotation);
                }

                // Update scale if changed
                if (originalScale != transform.scale) {
                    handle.Scale(transform.scale);
                    // For Jolt physics, scaling might require body recreation
                    handle.RecreatePhysicsBodyWithScale(*entity, transform.scale);
                }
            }
        }
    }

    // Quick transform buttons with proper physics sync
    ImGui::Spacing();
    if (ImGui::Button("Reset Position")) {
        transform.position = {0, 0, 0};
        if (entity->hasPhysics) {
            auto* scene = m_editorCore->GetActiveScene();
            if (scene) {
                scene->GetEntity(entity->id).At(transform.position);
            }
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset Rotation")) {
        transform.rotation = {0, 0, 0};
        if (entity->hasPhysics) {
            auto* scene = m_editorCore->GetActiveScene();
            if (scene) {
                scene->GetEntity(entity->id).Rotate(transform.rotation);
            }
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset Scale")) {
        transform.scale = {1, 1, 1};
        if (entity->hasPhysics) {
            auto* scene = m_editorCore->GetActiveScene();
            if (scene) {
                auto handle = scene->GetEntity(entity->id);
                handle.Scale(transform.scale);
                handle.RecreatePhysicsBodyWithScale(*entity, transform.scale);
            }
        }
    }
}

void InspectorPanel::RenderPhysicsComponent(Blacksite::Entity* entity) {
    ImGui::Text("Physics Properties");

    bool wasDynamic = entity->isDynamic;
    ImGui::Checkbox("Dynamic", &entity->isDynamic);

    if (wasDynamic != entity->isDynamic) {
        // Update physics body type
        auto* scene = m_editorCore->GetActiveScene();
        if (scene) {
            auto handle = scene->GetEntity(entity->id);
            try {
                if (entity->isDynamic) {
                    handle.MakeDynamic();
                } else {
                    handle.MakeStatic();
                }
            } catch (const std::exception& e) {
                // Handle physics system errors
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Physics Error: %s", e.what());
                entity->isDynamic = wasDynamic; // Revert on error
            }
        }
    }

    // Show current physics state
    ImGui::Text("Physics Body ID: %u", entity->physicsBody.GetIndexAndSequenceNumber());
    ImGui::Text("Is Dynamic: %s", entity->isDynamic ? "Yes" : "No");

    ImGui::Spacing();

    // Physics controls with error handling
    if (ImGui::Button("Make Static")) {
        auto* scene = m_editorCore->GetActiveScene();
        if (scene) {
            try {
                entity->isDynamic = false;
                scene->GetEntity(entity->id).MakeStatic();
            } catch (const std::exception& e) {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: %s", e.what());
            }
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Make Dynamic")) {
        auto* scene = m_editorCore->GetActiveScene();
        if (scene) {
            try {
                entity->isDynamic = true;
                scene->GetEntity(entity->id).MakeDynamic();
            } catch (const std::exception& e) {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: %s", e.what());
            }
        }
    }

    // Physics actions (only for dynamic bodies)
    if (entity->isDynamic) {
        ImGui::Spacing();
        ImGui::Text("Physics Actions:");

        if (ImGui::Button("Apply Upward Force")) {
            auto* scene = m_editorCore->GetActiveScene();
            if (scene) {
                scene->GetEntity(entity->id).Push({0, 10, 0});
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Apply Random Force")) {
            auto* scene = m_editorCore->GetActiveScene();
            if (scene) {
                glm::vec3 randomForce = {
                    (rand() % 20) - 10.0f,
                    (float)(rand() % 10),
                    (rand() % 20) - 10.0f
                };
                scene->GetEntity(entity->id).Push(randomForce);
            }
        }

        if (ImGui::Button("Stop Movement")) {
            auto* scene = m_editorCore->GetActiveScene();
            if (scene) {
                scene->GetEntity(entity->id).SetVelocity({0, 0, 0});
                scene->GetEntity(entity->id).SetAngularVelocity({0, 0, 0});
            }
        }
    }
}

void InspectorPanel::RenderRenderComponent(Blacksite::Entity* entity) {
    ImGui::Text("Rendering Properties");

    // Shape selection
    const char* shapes[] = { "Cube", "Sphere", "Plane" };
    int currentShape = static_cast<int>(entity->shape);
    if (ImGui::Combo("Shape", &currentShape, shapes, IM_ARRAYSIZE(shapes))) {
        entity->shape = static_cast<Blacksite::Entity::VisualShape>(currentShape);
        // TODO: Update mesh/rendering and physics collider if needed
    }

    // Shader selection
    char shaderBuffer[256];
    strncpy(shaderBuffer, entity->shader.c_str(), sizeof(shaderBuffer));
    shaderBuffer[sizeof(shaderBuffer) - 1] = '\0';

    if (ImGui::InputText("Shader", shaderBuffer, sizeof(shaderBuffer))) {
        entity->shader = shaderBuffer;
    }

    // Common shader presets
    if (ImGui::Button("Basic")) entity->shader = "basic";
    ImGui::SameLine();
    if (ImGui::Button("Unlit")) entity->shader = "unlit";
    ImGui::SameLine();
    if (ImGui::Button("Wireframe")) entity->shader = "wireframe";

    // Color picker
    bool colorChanged = DrawColorControl("Color", entity->color);
    if (colorChanged) {
        // Update entity color in scene
        auto* scene = m_editorCore->GetActiveScene();
        if (scene) {
            scene->GetEntity(entity->id).Color(entity->color);
        }
    }
}

void InspectorPanel::RenderEntityActions(Blacksite::Entity* entity) {
    ImGui::Text("Entity Actions");

    // Duplicate button
    if (ImGui::Button("Duplicate Entity", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - 5, 0))) {
        m_editorCore->DuplicateEntity(entity->id);
    }

    ImGui::SameLine();

    // Delete button with confirmation
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.2f, 0.2f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.3f, 0.3f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.7f, 0.1f, 0.1f, 1.0f});

    if (ImGui::Button("Delete Entity", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
        ImGui::OpenPopup("Delete Entity?");
    }

    ImGui::PopStyleColor(3);

    // Confirmation popup
    if (ImGui::BeginPopupModal("Delete Entity?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to delete '%s'?", entity->name.c_str());
        ImGui::Text("This action cannot be undone.");
        ImGui::Separator();

        if (ImGui::Button("Delete", ImVec2(120, 0))) {
            m_editorCore->DeleteEntity(entity->id);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void InspectorPanel::RenderComponentButtons(Blacksite::Entity* entity) {
    ImGui::Text("Components");

    if (!entity->hasPhysics) {
        if (ImGui::Button("Add Physics Component", ImVec2(-1, 0))) {
            entity->hasPhysics = true;
            // Create physics body
            auto* scene = m_editorCore->GetActiveScene();
            if (scene) {
                auto handle = scene->GetEntity(entity->id);
                try {
                    if (entity->isDynamic) {
                        handle.MakeDynamic();
                    } else {
                        handle.MakeStatic();
                    }
                } catch (const std::exception& e) {
                    ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error adding physics: %s", e.what());
                    entity->hasPhysics = false; // Revert on error
                }
            }
        }
    } else {
        if (ImGui::Button("Remove Physics Component", ImVec2(-1, 0))) {
            entity->hasPhysics = false;
            // Remove physics body
            auto* scene = m_editorCore->GetActiveScene();
            if (scene) {
                // You'll need to implement RemovePhysicsBody in EntityHandle
                // auto handle = scene->GetEntity(entity->id);
                // handle.RemovePhysicsBody();
            }
        }
    }
}

bool InspectorPanel::DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue, float columnWidth) {
    ImGuiIO& io = ImGui::GetIO();
    auto boldFont = io.Fonts->Fonts[0];

    ImGui::PushID(label.c_str());

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, columnWidth);
    ImGui::Text("%s", label.c_str());
    ImGui::NextColumn();

    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

    float lineHeight = GImGui->Style.FramePadding.y * 2.0f + ImGui::GetFontSize();
    ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

    bool modified = false;

    // X component
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
    if (ImGui::Button("X", buttonSize)) {
        values.x = resetValue;
        modified = true;
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    if (ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f"))
        modified = true;
    ImGui::PopItemWidth();
    ImGui::SameLine();

    // Y component
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
    if (ImGui::Button("Y", buttonSize)) {
        values.y = resetValue;
        modified = true;
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    if (ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f"))
        modified = true;
    ImGui::PopItemWidth();
    ImGui::SameLine();

    // Z component
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
    if (ImGui::Button("Z", buttonSize)) {
        values.z = resetValue;
        modified = true;
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    if (ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f"))
        modified = true;
    ImGui::PopItemWidth();

    ImGui::PopStyleVar();
    ImGui::Columns(1);
    ImGui::PopID();

    return modified;
}

bool InspectorPanel::DrawColorControl(const std::string& label, glm::vec3& color) {
    ImGui::PushID(label.c_str());

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, 100.0f);
    ImGui::Text("%s", label.c_str());
    ImGui::NextColumn();

    float col[3] = {color.r, color.g, color.b};
    bool modified = ImGui::ColorEdit3("##Color", col);
    if (modified) {
        color.r = col[0];
        color.g = col[1];
        color.b = col[2];
    }

    ImGui::Columns(1);
    ImGui::PopID();

    return modified;
}

}  // namespace BlacksiteEditor
