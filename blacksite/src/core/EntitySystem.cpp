#include "blacksite/core/EntitySystem.h"
#include "blacksite/core/Logger.h"
#include "blacksite/physics/PhysicsSystem.h"

namespace Blacksite {

EntitySystem::EntitySystem() = default;

EntitySystem::~EntitySystem() = default;

// Template spawn method implementation (moved to header as template)
// The Spawn<T>() method is now in the header file

// Default shader versions (backward compatible) - these now call the shader versions
int EntitySystem::SpawnCube(const glm::vec3& position) {
    return SpawnCube(position, "basic");
}

int EntitySystem::SpawnSphere(const glm::vec3& position) {
    return SpawnSphere(position, "basic");
}

int EntitySystem::SpawnPlane(const glm::vec3& position, const glm::vec3& size) {
    return SpawnPlane(position, size, "basic");
}

// Shader-specific versions - updated to work with new entity class system
int EntitySystem::SpawnCube(const glm::vec3& position, const std::string& shader, const glm::vec3& color) {
    if (!m_physicsSystem) {
        BS_ERROR(LogCategory::CORE, "EntitySystem: No physics system available!");
        return -1;
    }

    // Create basic entity (not using template system for backward compatibility)
    Entity entity(Entity::CUBE, shader);
    entity.transform.position = position;
    entity.transform.scale = glm::vec3(1.0f);
    entity.color = color;
    entity.id = m_nextEntityId;
    entity.name = "Cube_" + std::to_string(m_nextEntityId);

    // Add entity to vector FIRST
    int id = m_nextEntityId++;

    // Ensure vector is large enough
    if (id >= static_cast<int>(m_entities.size())) {
        m_entities.resize(id + 1);
    }

    m_entities[id] = entity;

    // NOW create physics body (entity is in vector)
    Entity& storedEntity = m_entities[id];
    storedEntity.physicsBody = m_physicsSystem->CreatePhysicsBody(storedEntity);
    storedEntity.hasPhysics = true;

    // Call OnSpawn for any custom initialization
    storedEntity.OnSpawn();

    BS_INFO_F(LogCategory::PHYSICS, "EntitySystem: Spawned cube with %zu colliders", storedEntity.colliders.size());
    return id;
}

int EntitySystem::SpawnSphere(const glm::vec3& position, const std::string& shader, const glm::vec3& color) {
    if (!m_physicsSystem) {
        BS_ERROR(LogCategory::CORE, "EntitySystem: No physics system available!");
        return -1;
    }

    Entity entity(Entity::SPHERE, shader);
    entity.transform.position = position;
    entity.transform.scale = glm::vec3(1.0f);
    entity.color = color;
    entity.id = m_nextEntityId;
    entity.name = "Sphere_" + std::to_string(m_nextEntityId);

    int id = m_nextEntityId++;

    if (id >= static_cast<int>(m_entities.size())) {
        m_entities.resize(id + 1);
    }

    m_entities[id] = entity;

    Entity& storedEntity = m_entities[id];
    storedEntity.physicsBody = m_physicsSystem->CreatePhysicsBody(storedEntity);
    storedEntity.hasPhysics = true;

    storedEntity.OnSpawn();

    BS_INFO_F(LogCategory::PHYSICS, "EntitySystem: Spawned sphere with %zu colliders", storedEntity.colliders.size());
    return id;
}

int EntitySystem::SpawnPlane(const glm::vec3& position, const glm::vec3& size, const std::string& shader, const glm::vec3& color) {
    if (!m_physicsSystem) {
        BS_ERROR(LogCategory::CORE, "EntitySystem: No physics system available!");
        return -1;
    }

    Entity entity(Entity::PLANE, shader);
    entity.transform.position = position;
    entity.transform.scale = size;
    entity.color = color;
    entity.id = m_nextEntityId;
    entity.name = "Plane_" + std::to_string(m_nextEntityId);
    entity.isDynamic = false;

    int id = m_nextEntityId++;

    if (id >= static_cast<int>(m_entities.size())) {
        m_entities.resize(id + 1);
    }

    m_entities[id] = entity;

    Entity& storedEntity = m_entities[id];
    storedEntity.physicsBody = m_physicsSystem->CreatePhysicsBody(storedEntity);
    storedEntity.hasPhysics = true;

    storedEntity.OnSpawn();

    BS_INFO_F(LogCategory::PHYSICS, "EntitySystem: Spawned plane with %zu colliders", storedEntity.colliders.size());
    return id;
}

// Generic spawn method
int EntitySystem::SpawnEntity(Entity::VisualShape shape, const glm::vec3& position, const std::string& shader, const glm::vec3& color) {
    switch (shape) {
        case Entity::CUBE:
            return SpawnCube(position, shader, color);
        case Entity::SPHERE:
            return SpawnSphere(position, shader, color);
        case Entity::PLANE:
            return SpawnPlane(position, glm::vec3(1.0f), shader, color);
        default:
            BS_ERROR(LogCategory::CORE, "Unknown entity shape in SpawnEntity");
            return -1;
    }
}

void EntitySystem::RemoveEntity(int id) {
    if (IsValidEntity(id)) {
        // Call OnDestroy before deactivating
        m_entities[id].OnDestroy();
        m_entities[id].active = false;
        BS_INFO_F(LogCategory::CORE, "EntitySystem: Entity removed (ID: %d)", id);
    } else {
        BS_WARN_F(LogCategory::CORE, "EntitySystem: Tried to remove invalid entity (ID: %d)", id);
    }
}

void EntitySystem::DuplicateEntity(int id) {
    if (!IsValidEntity(id)) {
        BS_WARN_F(LogCategory::CORE, "EntitySystem: Tried to duplicate invalid entity (ID: %d)", id);
        return;
    }

    const Entity& original = m_entities[id];
    glm::vec3 newPos = original.transform.position + glm::vec3(1.0f, 0.0f, 0.0f);

    int newId = -1;
    switch (original.shape) {
        case Entity::CUBE:
            newId = SpawnCube(newPos, original.shader, original.color);
            break;
        case Entity::SPHERE:
            newId = SpawnSphere(newPos, original.shader, original.color);
            break;
        case Entity::PLANE:
            newId = SpawnPlane(newPos, original.transform.scale, original.shader, original.color);
            break;
    }

    if (newId >= 0) {
        BS_INFO_F(LogCategory::CORE, "EntitySystem: Entity duplicated (original: %d, copy: %d, shader: %s)",
                  id, newId, original.shader.c_str());
    }
}

// Shader and color management methods
void EntitySystem::SetEntityShader(int id, const std::string& shader) {
    if (IsValidEntity(id)) {
        m_entities[id].shader = shader;
        BS_DEBUG_F(LogCategory::CORE, "EntitySystem: Entity %d shader changed to '%s'", id, shader.c_str());
    } else {
        BS_WARN_F(LogCategory::CORE, "EntitySystem: Tried to set shader on invalid entity (ID: %d)", id);
    }
}

void EntitySystem::SetEntityColor(int id, const glm::vec3& color) {
    if (IsValidEntity(id)) {
        m_entities[id].color = color;
        BS_DEBUG_F(LogCategory::CORE, "EntitySystem: Entity %d color changed to (%.2f, %.2f, %.2f)",
                  id, color.r, color.g, color.b);
    } else {
        BS_WARN_F(LogCategory::CORE, "EntitySystem: Tried to set color on invalid entity (ID: %d)", id);
    }
}

std::string EntitySystem::GetEntityShader(int id) const {
    if (IsValidEntity(id)) {
        return m_entities[id].shader;
    }
    return "basic";
}

glm::vec3 EntitySystem::GetEntityColor(int id) const {
    if (IsValidEntity(id)) {
        return m_entities[id].color;
    }
    return glm::vec3(1.0f);
}

void EntitySystem::SetEntityName(int id, const std::string& name) {
    if (IsValidEntity(id)) {
        m_entities[id].name = name;
        BS_DEBUG_F(LogCategory::CORE, "EntitySystem: Entity %d renamed to '%s'", id, name.c_str());
    }
}

std::string EntitySystem::GetEntityName(int id) const {
    if (IsValidEntity(id)) {
        return m_entities[id].name;
    }
    return "Unknown";
}

Entity* EntitySystem::GetEntityPtr(int id) {
    if (id >= 0 && id < static_cast<int>(m_entities.size())) {
        return &m_entities[id];
    }
    return nullptr;
}

bool EntitySystem::IsValidEntity(int id) const {
    if (id < 0 || id >= static_cast<int>(m_entities.size())) {
        return false;
    }
    return m_entities[id].active;
}

}  // namespace Blacksite
