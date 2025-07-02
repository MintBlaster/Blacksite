#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "blacksite/core/Entity.h"

namespace Blacksite {

class PhysicsSystem;

class EntitySystem {
  public:
    EntitySystem();
    ~EntitySystem();

    void SetPhysicsSystem(PhysicsSystem* physics) { m_physicsSystem = physics; }

    // Default shader versions (backward compatible)
    int SpawnCube(const glm::vec3& position);
    int SpawnSphere(const glm::vec3& position);
    int SpawnPlane(const glm::vec3& position, const glm::vec3& size);

    // Shader-specific versions
    int SpawnCube(const glm::vec3& position, const std::string& shader, const glm::vec3& color = glm::vec3(1.0f));
    int SpawnSphere(const glm::vec3& position, const std::string& shader, const glm::vec3& color = glm::vec3(1.0f));
    int SpawnPlane(const glm::vec3& position, const glm::vec3& size, const std::string& shader,
                   const glm::vec3& color = glm::vec3(1.0f));

    // Generic spawn method
    int SpawnEntity(Entity::VisualShape shape, const glm::vec3& position, const std::string& shader = "basic",
                    const glm::vec3& color = glm::vec3(1.0f));

    void RemoveEntity(int id);
    void DuplicateEntity(int id);

    // Shader and color management methods
    void SetEntityShader(int id, const std::string& shader);
    void SetEntityColor(int id, const glm::vec3& color);
    std::string GetEntityShader(int id) const;
    glm::vec3 GetEntityColor(int id) const;

    void SetEntityName(int id, const std::string& name);
    std::string GetEntityName(int id) const;

    Entity* GetEntityPtr(int id);
    bool IsValidEntity(int id) const;

    const std::vector<Entity>& GetEntities() const { return m_entities; }
    std::vector<Entity>& GetEntities() { return m_entities; }

  private:
    std::vector<Entity> m_entities;
    std::vector<std::string> m_entityNames;
    PhysicsSystem* m_physicsSystem = nullptr;
    int m_nextEntityId = 0;
};

}  // namespace Blacksite
