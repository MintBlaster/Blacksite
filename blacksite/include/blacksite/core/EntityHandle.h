#pragma once

#include <glm/glm.hpp>
#include "blacksite/core/Entity.h"

namespace Blacksite {

class EntitySystem;
class PhysicsSystem;

class EntityHandle {
  public:
    EntityHandle(EntitySystem* entitySystem, PhysicsSystem* physicsSystem, int id);

    // Transform manipulation (syncs with physics automatically)
    EntityHandle& At(const glm::vec3& position);
    EntityHandle& Rotate(const glm::vec3& rotation);
    EntityHandle& Scale(const glm::vec3& scale);
    EntityHandle& Scale(float x, float y, float z);
    EntityHandle& Scale(float uniformScale);

    // Physics operations
    EntityHandle& Push(const glm::vec3& force);
    EntityHandle& Impulse(const glm::vec3& impulse);
    EntityHandle& SetVelocity(const glm::vec3& velocity);
    EntityHandle& SetAngularVelocity(const glm::vec3& angularVel);
    EntityHandle& MakeStatic();
    EntityHandle& MakeDynamic();

    // Appearance
    EntityHandle& Color(float r, float g, float b);
    EntityHandle& Color(const glm::vec3& color);

    // Getters
    glm::vec3 GetPosition() const;
    glm::vec3 GetRotation() const;
    glm::vec3 GetScale() const;
    glm::vec3 GetVelocity() const;
    glm::vec3 GetAngularVelocity() const;
    glm::vec3 GetColor() const;
    bool IsActive() const;
    bool IsValid() const;

    // State management
    EntityHandle& SetActive(bool active);
    void Destroy();
    int GetId() const { return m_id; }

  private:
    void RecreatePhysicsBodyWithScale(Entity& entity, const glm::vec3& scale);
    EntitySystem* m_entitySystem;
    PhysicsSystem* m_physicsSystem;
    int m_id;
};

}  // namespace Blacksite
