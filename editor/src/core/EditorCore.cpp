#include "blacksite_editor/core/EditorCore.h"
#include <blacksite/core/Logger.h>
#include <blacksite/scene/Scene.h>
#include <blacksite/core/EntitySystem.h>

namespace BlacksiteEditor {

EditorCore::EditorCore(Blacksite::Engine* engine) : m_engine(engine) {}

void EditorCore::Initialize() {
    BS_INFO(Blacksite::LogCategory::CORE, "EditorCore initialized");

    // Initialize performance tracking
    m_frameTime = 0.0f;
    m_frameCount = 0;
    m_fps = 0.0f;
}

void EditorCore::Update(float deltaTime) {
    UpdatePerformanceStats(deltaTime);

    // Update any editor-specific logic here
    // For example, auto-save functionality, background tasks, etc.
}

void EditorCore::Shutdown() {
    BS_INFO(Blacksite::LogCategory::CORE, "EditorCore shutdown");
}

Blacksite::Scene* EditorCore::GetActiveScene() {
    return m_engine ? m_engine->GetActiveScene() : nullptr;
}

void EditorCore::NewScene(const std::string& name) {
    if (!m_engine) return;

    auto scene = m_engine->CreateScene(name);
    if (scene) {
        m_engine->SwitchToScene(name);
        m_selectedEntityId = -1; // Clear selection
        BS_INFO_F(Blacksite::LogCategory::CORE, "Created new scene: %s", name.c_str());
    } else {
        BS_ERROR_F(Blacksite::LogCategory::CORE, "Failed to create scene: %s", name.c_str());
    }
}

void EditorCore::SaveScene(const std::string& filename) {
    auto* scene = GetActiveScene();
    if (!scene) {
        BS_WARN(Blacksite::LogCategory::CORE, "No active scene to save");
        return;
    }

    // TODO: Implement scene serialization
    // For now, just log the action
    BS_INFO_F(Blacksite::LogCategory::CORE, "Saving scene '%s' to: %s",
              scene->GetName().c_str(), filename.c_str());

    // Future implementation would serialize the scene to JSON/binary format
    // scene->Serialize(filename);
}

void EditorCore::LoadScene(const std::string& filename) {
    if (!m_engine) return;

    // TODO: Implement scene deserialization
    // For now, just log the action
    BS_INFO_F(Blacksite::LogCategory::CORE, "Loading scene from: %s", filename.c_str());

    // Future implementation would deserialize from file
    // auto scene = Scene::Deserialize(filename);
    // if (scene) {
    //     m_engine->AddScene(scene);
    //     m_engine->SwitchToScene(scene->GetName());
    // }
}

int EditorCore::CreateEntity(const std::string& type) {
    auto* scene = GetActiveScene();
    if (!scene) {
        BS_WARN(Blacksite::LogCategory::CORE, "No active scene to create entity in");
        return -1;
    }

    int entityId = -1;
    glm::vec3 spawnPosition = {0, 0, 0};

    // Spawn at camera position if possible
    glm::vec3 cameraPos = scene->GetCameraPosition();
    glm::vec3 cameraTarget = scene->GetCameraTarget();
    glm::vec3 forward = normalize(cameraTarget - cameraPos);
    spawnPosition = cameraPos + forward * 5.0f; // Spawn 5 units in front of camera

    if (type == "Cube" || type == "cube") {
        entityId = scene->SpawnCube(spawnPosition, "basic", {0.8f, 0.2f, 0.2f});
    } else if (type == "Sphere" || type == "sphere") {
        entityId = scene->SpawnSphere(spawnPosition, "basic", {0.2f, 0.8f, 0.2f});
    } else if (type == "Plane" || type == "plane") {
        entityId = scene->SpawnPlane(spawnPosition, {2.0f, 0.1f, 2.0f}, "basic", {0.6f, 0.6f, 0.6f});
        // Make planes static by default
        if (entityId >= 0) {
            scene->GetEntity(entityId).MakeStatic();
        }
    } else {
        BS_WARN_F(Blacksite::LogCategory::CORE, "Unknown entity type: %s", type.c_str());
        return -1;
    }

    if (entityId >= 0) {
        // Set a default name for the entity
        auto* entitySystem = scene->GetEntitySystem();
        if (entitySystem) {
            std::string entityName = type + "_" + std::to_string(entityId);
            entitySystem->SetEntityName(entityId, entityName);
        }

        SetSelectedEntity(entityId);
        BS_INFO_F(Blacksite::LogCategory::CORE, "Created entity: %s (ID: %d)", type.c_str(), entityId);
    } else {
        BS_ERROR_F(Blacksite::LogCategory::CORE, "Failed to create entity: %s", type.c_str());
    }

    return entityId;
}

void EditorCore::DeleteEntity(int entityId) {
    auto* scene = GetActiveScene();
    if (!scene) {
        BS_WARN(Blacksite::LogCategory::CORE, "No active scene to delete entity from");
        return;
    }

    auto* entitySystem = scene->GetEntitySystem();
    if (!entitySystem || !entitySystem->IsValidEntity(entityId)) {
        BS_WARN_F(Blacksite::LogCategory::CORE, "Invalid entity ID for deletion: %d", entityId);
        return;
    }

    // Get entity name for logging
    std::string entityName = entitySystem->GetEntityName(entityId);

    // Remove the entity
    scene->RemoveEntity(entityId);

    // Clear selection if this entity was selected
    if (m_selectedEntityId == entityId) {
        m_selectedEntityId = -1;
    }

    BS_INFO_F(Blacksite::LogCategory::CORE, "Deleted entity: %s (ID: %d)", entityName.c_str(), entityId);
}

void EditorCore::DuplicateEntity(int entityId) {
    auto* scene = GetActiveScene();
    if (!scene) {
        BS_WARN(Blacksite::LogCategory::CORE, "No active scene to duplicate entity in");
        return;
    }

    auto* entitySystem = scene->GetEntitySystem();
    if (!entitySystem || !entitySystem->IsValidEntity(entityId)) {
        BS_WARN_F(Blacksite::LogCategory::CORE, "Invalid entity ID for duplication: %d", entityId);
        return;
    }

    auto* originalEntity = entitySystem->GetEntityPtr(entityId);
    if (!originalEntity) {
        BS_ERROR_F(Blacksite::LogCategory::CORE, "Could not get entity pointer for ID: %d", entityId);
        return;
    }

    // Create new entity of the same type
    glm::vec3 newPosition = originalEntity->transform.position + glm::vec3(2.0f, 0.0f, 0.0f);
    int newEntityId = -1;

    switch (originalEntity->shape) {
        case Blacksite::Entity::CUBE:
            newEntityId = scene->SpawnCube(newPosition, originalEntity->shader, originalEntity->color);
            break;
        case Blacksite::Entity::SPHERE:
            newEntityId = scene->SpawnSphere(newPosition, originalEntity->shader, originalEntity->color);
            break;
        case Blacksite::Entity::PLANE:
            newEntityId = scene->SpawnPlane(newPosition, originalEntity->transform.scale,
                                          originalEntity->shader, originalEntity->color);
            break;
        default:
            BS_ERROR_F(Blacksite::LogCategory::CORE, "Unknown entity shape for duplication: %d",
                       static_cast<int>(originalEntity->shape));
            return;
    }

    if (newEntityId >= 0) {
        // Copy properties from original
        auto newEntityHandle = scene->GetEntity(newEntityId);
        newEntityHandle.Scale(originalEntity->transform.scale);

        // Copy physics state
        if (originalEntity->hasPhysics) {
            if (originalEntity->isDynamic) {
                newEntityHandle.MakeDynamic();
            } else {
                newEntityHandle.MakeStatic();
            }
        }

        // Set name
        std::string originalName = entitySystem->GetEntityName(entityId);
        std::string newName = originalName + "_Copy";
        entitySystem->SetEntityName(newEntityId, newName);

        // Select the new entity
        SetSelectedEntity(newEntityId);

        BS_INFO_F(Blacksite::LogCategory::CORE, "Duplicated entity: %s -> %s (ID: %d -> %d)",
                  originalName.c_str(), newName.c_str(), entityId, newEntityId);
    } else {
        BS_ERROR_F(Blacksite::LogCategory::CORE, "Failed to duplicate entity ID: %d", entityId);
    }
}


void EditorCore::UpdatePerformanceStats(float deltaTime) {
    m_frameTime = deltaTime;
    m_frameCount++;

    // Update FPS every second
    static float timer = 0.0f;
    timer += deltaTime;

    if (timer >= 1.0f) {
        m_fps = m_frameCount / timer;
        m_frameCount = 0;
        timer = 0.0f;
    }
}

}  // namespace BlacksiteEditor
