#include "blacksite/core/EntityHandle.h"
#include "blacksite/core/EntitySystem.h"
#include "blacksite/physics/PhysicsSystem.h"
#include "blacksite/core/Logger.h"

namespace Blacksite {

EntityHandle::EntityHandle(EntitySystem* entitySystem, PhysicsSystem* physicsSystem, int id)
    : m_entitySystem(entitySystem), m_physicsSystem(physicsSystem), m_id(id) {
}

EntityHandle& EntityHandle::At(const glm::vec3& position) {
    if (auto* entity = m_entitySystem->GetEntityPtr(m_id)) {
        entity->transform.position = position;
        if (entity->hasPhysics && m_physicsSystem) {
            m_physicsSystem->SetBodyPosition(entity->physicsBody, position);
        }
    } else {
        BS_ERROR_F(LogCategory::CORE, "EntityHandle: Tried to move non-existent entity %d", m_id);
    }
    return *this;
}

EntityHandle& EntityHandle::Rotate(const glm::vec3& rotation) {
    if (auto* entity = m_entitySystem->GetEntityPtr(m_id)) {
        entity->transform.rotation = rotation;
        // TODO: Sync rotation with physics system
    } else {
        BS_ERROR_F(LogCategory::CORE, "EntityHandle: Tried to rotate non-existent entity %d", m_id);
    }
    return *this;
}

EntityHandle& EntityHandle::Scale(const glm::vec3& scale) {
    if (auto* entity = m_entitySystem->GetEntityPtr(m_id)) {
        entity->transform.scale = scale;
        // Note: Physics bodies don't typically change scale after creation
    } else {
        BS_ERROR_F(LogCategory::CORE, "EntityHandle: Tried to scale non-existent entity %d", m_id);
    }
    return *this;
}

EntityHandle& EntityHandle::Scale(float x, float y, float z) {
    return Scale({x, y, z});
}

EntityHandle& EntityHandle::Scale(float uniformScale) {
    return Scale({uniformScale, uniformScale, uniformScale});
}

EntityHandle& EntityHandle::Color(float r, float g, float b) {
    if (auto* entity = m_entitySystem->GetEntityPtr(m_id)) {
        entity->color = {r, g, b};
    } else {
        BS_ERROR_F(LogCategory::CORE, "EntityHandle: Tried to color non-existent entity %d", m_id);
    }
    return *this;
}

EntityHandle& EntityHandle::Color(const glm::vec3& color) {
    if (auto* entity = m_entitySystem->GetEntityPtr(m_id)) {
        entity->color = color;
    } else {
        BS_ERROR_F(LogCategory::CORE, "EntityHandle: Tried to color non-existent entity %d", m_id);
    }
    return *this;
}

glm::vec3 EntityHandle::GetPosition() const {
    if (!IsValid()) return glm::vec3(0.0f);

    const auto& entities = m_entitySystem->GetEntities();
    if (m_id >= 0 && m_id < static_cast<int>(entities.size())) {
        return entities[m_id].transform.position;
    }
    return glm::vec3(0.0f);
}

glm::vec3 EntityHandle::GetRotation() const {
    if (!IsValid()) return glm::vec3(0.0f);

    const auto& entities = m_entitySystem->GetEntities();
    if (m_id >= 0 && m_id < static_cast<int>(entities.size())) {
        return entities[m_id].transform.rotation;
    }
    return glm::vec3(0.0f);
}

glm::vec3 EntityHandle::GetScale() const {
    if (!IsValid()) return glm::vec3(1.0f);

    const auto& entities = m_entitySystem->GetEntities();
    if (m_id >= 0 && m_id < static_cast<int>(entities.size())) {
        return entities[m_id].transform.scale;
    }
    return glm::vec3(1.0f);
}

glm::vec3 EntityHandle::GetVelocity() const {
    if (!IsValid()) return glm::vec3(0.0f);

    // Get velocity from physics system
    return m_physicsSystem->GetVelocity(m_id);
}

glm::vec3 EntityHandle::GetAngularVelocity() const {
    if (!IsValid()) return glm::vec3(0.0f);

    // Get angular velocity from physics system
    return m_physicsSystem->GetAngularVelocity(m_id);
}

glm::vec3 EntityHandle::GetColor() const {
    if (!IsValid()) return glm::vec3(1.0f);

    const auto& entities = m_entitySystem->GetEntities();
    if (m_id >= 0 && m_id < static_cast<int>(entities.size())) {
        return entities[m_id].color;
    }
    return glm::vec3(1.0f, 1.0f, 1.0f);
}

bool EntityHandle::IsActive() const {
    if (!IsValid()) return false;

    const auto& entities = m_entitySystem->GetEntities();
    if (m_id >= 0 && m_id < static_cast<int>(entities.size())) {
        return entities[m_id].active;
    }
    return false;
}

bool EntityHandle::IsValid() const {
    return m_id >= 0 && m_entitySystem != nullptr && m_physicsSystem != nullptr;
}

EntityHandle& EntityHandle::SetActive(bool active) {
    if (auto* entity = m_entitySystem->GetEntityPtr(m_id)) {
        entity->active = active;
        BS_DEBUG_F(LogCategory::CORE, "EntityHandle: Entity %d is now %s", m_id, (active ? "active" : "inactive"));
    } else {
        BS_ERROR_F(LogCategory::CORE, "EntityHandle: Tried to set state of non-existent entity %d", m_id);
    }
    return *this;
}

void EntityHandle::Destroy() {
    if (auto* entity = m_entitySystem->GetEntityPtr(m_id)) {
        entity->active = false;
        BS_DEBUG_F(LogCategory::CORE, "EntityHandle: Entity %d destroyed", m_id);
    } else {
        BS_ERROR_F(LogCategory::CORE, "EntityHandle: Tried to destroy non-existent entity %d", m_id);
    }
}

// Physics operations
EntityHandle& EntityHandle::Push(const glm::vec3& force) {
    if (auto* entity = m_entitySystem->GetEntityPtr(m_id)) {
        if (entity->hasPhysics && m_physicsSystem) {
            m_physicsSystem->AddForce(entity->physicsBody, force);
        } else {
            BS_ERROR_F(LogCategory::PHYSICS, "EntityHandle: Entity %d has no physics body", m_id);
        }
    }
    return *this;
}

EntityHandle& EntityHandle::Impulse(const glm::vec3& impulse) {
    if (auto* entity = m_entitySystem->GetEntityPtr(m_id)) {
        if (entity->hasPhysics && m_physicsSystem) {
            m_physicsSystem->AddImpulse(entity->physicsBody, impulse);
        } else {
            BS_ERROR_F(LogCategory::PHYSICS, "EntityHandle: Entity %d has no physics body", m_id);
        }
    }
    return *this;
}

EntityHandle& EntityHandle::SetVelocity(const glm::vec3& velocity) {
    if (auto* entity = m_entitySystem->GetEntityPtr(m_id)) {
        if (entity->hasPhysics && m_physicsSystem) {
            m_physicsSystem->SetVelocity(entity->physicsBody, velocity);
        } else {
            BS_ERROR_F(LogCategory::PHYSICS, "EntityHandle: Entity %d has no physics body", m_id);
        }
    }
    return *this;
}

EntityHandle& EntityHandle::SetAngularVelocity(const glm::vec3& angularVel) {
    if (auto* entity = m_entitySystem->GetEntityPtr(m_id)) {
        if (entity->hasPhysics && m_physicsSystem) {
            m_physicsSystem->SetAngularVelocity(entity->physicsBody, angularVel);
        } else {
            BS_ERROR_F(LogCategory::PHYSICS, "EntityHandle: Entity %d has no physics body", m_id);
        }
    }
    return *this;
}

EntityHandle& EntityHandle::MakeStatic() {
    if (auto* entity = m_entitySystem->GetEntityPtr(m_id)) {
        if (entity->hasPhysics && m_physicsSystem) {
            m_physicsSystem->MakeBodyStatic(entity->physicsBody);
            BS_DEBUG_F(LogCategory::PHYSICS, "EntityHandle: Entity %d is now static", m_id);
        }
    }
    return *this;
}

EntityHandle& EntityHandle::MakeDynamic() {
    if (auto* entity = m_entitySystem->GetEntityPtr(m_id)) {
        if (entity->hasPhysics && m_physicsSystem) {
            m_physicsSystem->MakeBodyDynamic(entity->physicsBody);
            BS_DEBUG_F(LogCategory::PHYSICS, "EntityHandle: Entity %d is now dynamic", m_id);
        }
    }
    return *this;
}

}
