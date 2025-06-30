#pragma once

// Include our Jolt configuration first
#include "JoltConfig.h"

// Jolt Physics includes
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>

namespace Blacksite {

// Convert between GLM and Jolt types
JPH::Vec3 ToJoltPos(const glm::vec3& v);
glm::vec3 ToGLMPos(const JPH::Vec3& v);
JPH::Quat ToJoltRot(const glm::vec3& eulerAngles);
glm::vec3 ToGLMRot(const JPH::Quat& q);

class PhysicsSystem {
  public:
    PhysicsSystem();
    ~PhysicsSystem();

    bool Initialize();
    void Shutdown();
    void Update(float deltaTime);

    // Body creation
    JPH::BodyID CreateBoxBody(const glm::vec3& position, const glm::vec3& size, bool isStatic = false);
    JPH::BodyID CreateSphereBody(const glm::vec3& position, float radius, bool isStatic = false);
    JPH::BodyID CreatePlaneBody(const glm::vec3& position, const glm::vec3& normal = {0, 1, 0});

    // Body manipulation
    void SetBodyPosition(JPH::BodyID bodyID, const glm::vec3& position);
    void SetBodyRotation(JPH::BodyID bodyID, const glm::vec3& eulerAngles);
    glm::vec3 GetBodyPosition(JPH::BodyID bodyID);
    glm::vec3 GetBodyRotation(JPH::BodyID bodyID);

    // Forces and impulses
    void AddForce(JPH::BodyID bodyID, const glm::vec3& force);
    void AddImpulse(JPH::BodyID bodyID, const glm::vec3& impulse);
    void SetVelocity(JPH::BodyID bodyID, const glm::vec3& velocity);
    void SetAngularVelocity(JPH::BodyID bodyID, const glm::vec3& angularVel);

    // Static/Dynamic conversion - NEW!
    void MakeBodyStatic(JPH::BodyID bodyID);
    void MakeBodyDynamic(JPH::BodyID bodyID);
    bool IsBodyStatic(JPH::BodyID bodyID);

    // Get the Jolt physics system for advanced usage
    JPH::PhysicsSystem* GetJoltSystem() { return m_physicsSystem.get(); }

  private:
    // Jolt Physics objects
    std::unique_ptr<JPH::TempAllocatorImpl> m_tempAllocator;
    std::unique_ptr<JPH::JobSystemThreadPool> m_jobSystem;
    std::unique_ptr<JPH::PhysicsSystem> m_physicsSystem;

    // Layers and filters (we'll keep it simple for now)
    static constexpr JPH::ObjectLayer NON_MOVING = 0;
    static constexpr JPH::ObjectLayer MOVING = 1;
    static constexpr JPH::ObjectLayer NUM_LAYERS = 2;

    class BPLayerInterfaceImpl;
    class ObjectVsBroadPhaseLayerFilterImpl;
    class ObjectLayerPairFilterImpl;

    std::unique_ptr<BPLayerInterfaceImpl> m_broadPhaseLayerInterface;
    std::unique_ptr<ObjectVsBroadPhaseLayerFilterImpl> m_objectVsBroadphaseLayerFilter;
    std::unique_ptr<ObjectLayerPairFilterImpl> m_objectVsObjectLayerFilter;

    bool m_initialized = false;
};

}  // namespace Blacksite