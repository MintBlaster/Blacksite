#include "blacksite/physics/PhysicsSystem.h"
#include <cstdarg>
#include <cstdio>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "blacksite/core/Logger.h"
#include "Core/IssueReporting.h"
#include "Core/Memory.h"

namespace Blacksite {

// Static callback functions for Jolt (instead of lambdas with variadic args)
static void TraceImpl(const char* inFMT, ...) {
    va_list list;
    va_start(list, inFMT);
    char buffer[1024];
    std::vsnprintf(buffer, sizeof(buffer), inFMT, list);
    va_end(list);
    BS_DEBUG_F(Blacksite::LogCategory::PHYSICS, "[Jolt] %s", buffer);
}

#ifdef JPH_ENABLE_ASSERTS
static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, JPH::uint inLine) {
    BS_ERROR_F(Blacksite::LogCategory::PHYSICS, "[Jolt Assert] %s:%d: (%s) %s",
               inFile, inLine, inExpression, (inMessage != nullptr ? inMessage : ""));
    return true;
}
#endif

// Utility conversion functions
JPH::Vec3 ToJoltPos(const glm::vec3& v) {
    return JPH::Vec3(v.x, v.y, v.z);
}

glm::vec3 ToGLMPos(const JPH::Vec3& v) {
    return glm::vec3(v.GetX(), v.GetY(), v.GetZ());
}

JPH::Quat ToJoltRot(const glm::vec3& eulerAngles) {
    glm::mat4 rotationMatrix =
        glm::eulerAngleXYZ(glm::radians(eulerAngles.x), glm::radians(eulerAngles.y), glm::radians(eulerAngles.z));

    glm::quat glmQuat = glm::quat_cast(rotationMatrix);
    return JPH::Quat(glmQuat.x, glmQuat.y, glmQuat.z, glmQuat.w);
}

glm::vec3 ToGLMRot(const JPH::Quat& q) {
    glm::quat glmQuat(q.GetW(), q.GetX(), q.GetY(), q.GetZ());
    glm::vec3 eulerAngles = glm::eulerAngles(glmQuat);
    return glm::degrees(eulerAngles);
}

// Layer interfaces for Jolt Physics - FIXED VERSION
class PhysicsSystem::BPLayerInterfaceImpl : public JPH::BroadPhaseLayerInterface {
  public:
    BPLayerInterfaceImpl() {
        // Create a mapping table from object to broad phase layer
        mObjectToBroadPhase[NON_MOVING] = JPH::BroadPhaseLayer(NON_MOVING);
        mObjectToBroadPhase[MOVING] = JPH::BroadPhaseLayer(MOVING);
    }

    virtual JPH::uint GetNumBroadPhaseLayers() const override { return NUM_LAYERS; }

    virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override {
        JPH_ASSERT(inLayer < NUM_LAYERS);
        return mObjectToBroadPhase[inLayer];
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override {
        switch ((JPH::BroadPhaseLayer::Type)inLayer) {
            case (JPH::BroadPhaseLayer::Type)NON_MOVING:
                return "NON_MOVING";
            case (JPH::BroadPhaseLayer::Type)MOVING:
                return "MOVING";
            default:
                JPH_ASSERT(false);
                return "INVALID";
        }
    }
#endif

  private:
    JPH::BroadPhaseLayer mObjectToBroadPhase[NUM_LAYERS];
};

class PhysicsSystem::ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
  public:
    virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override {
        switch (inLayer1) {
            case NON_MOVING:
                return inLayer2 == JPH::BroadPhaseLayer(MOVING);
            case MOVING:
                return true;
            default:
                JPH_ASSERT(false);
                return false;
        }
    }
};

class PhysicsSystem::ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
  public:
    virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override {
        switch (inObject1) {
            case NON_MOVING:
                return inObject2 == MOVING;
            case MOVING:
                return true;
            default:
                JPH_ASSERT(false);
                return false;
        }
    }
};

PhysicsSystem::PhysicsSystem() = default;

PhysicsSystem::~PhysicsSystem() {
    Shutdown();
}

bool PhysicsSystem::Initialize() {
    if (m_initialized) {
        BS_ERROR(LogCategory::PHYSICS, "PhysicsSystem already initialized");
        return false;
    }

    // Register allocation hook
    JPH::RegisterDefaultAllocator();

    // Install trace and assert callbacks - FIXED: Use function pointers instead of lambdas
    JPH::Trace = TraceImpl;

#ifdef JPH_ENABLE_ASSERTS
    JPH::AssertFailed = AssertFailedImpl;
#endif

    // Create factory and register types
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();

    // Create allocators and job system
    m_tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);

    int cNumThreads = std::thread::hardware_concurrency() - 1;
    if (cNumThreads < 1)
        cNumThreads = 1;
    m_jobSystem =
        std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, cNumThreads);

    // Create physics system
    m_physicsSystem = std::make_unique<JPH::PhysicsSystem>();

    // Create layer interfaces
    m_broadPhaseLayerInterface = std::make_unique<BPLayerInterfaceImpl>();
    m_objectVsBroadphaseLayerFilter = std::make_unique<ObjectVsBroadPhaseLayerFilterImpl>();
    m_objectVsObjectLayerFilter = std::make_unique<ObjectLayerPairFilterImpl>();

    // Initialize physics system
    const JPH::uint cMaxBodies = 1024;
    const JPH::uint cNumBodyMutexes = 0;
    const JPH::uint cMaxBodyPairs = 1024;
    const JPH::uint cMaxContactConstraints = 1024;

    m_physicsSystem->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints,
                          *m_broadPhaseLayerInterface, *m_objectVsBroadphaseLayerFilter, *m_objectVsObjectLayerFilter);

    m_initialized = true;
    BS_INFO(LogCategory::PHYSICS, "Physics system initialized successfully with Jolt Physics!");
    return true;
}

void PhysicsSystem::Shutdown() {
    if (!m_initialized)
        return;

    m_physicsSystem.reset();
    m_objectVsObjectLayerFilter.reset();
    m_objectVsBroadphaseLayerFilter.reset();
    m_broadPhaseLayerInterface.reset();
    m_jobSystem.reset();
    m_tempAllocator.reset();

    JPH::UnregisterTypes();
    if (JPH::Factory::sInstance) {
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;
    }

    m_initialized = false;
    BS_INFO(LogCategory::PHYSICS, "Physics system shut down");
}

void PhysicsSystem::Update(float deltaTime) {
    if (!m_initialized || !m_physicsSystem)
        return;

    const int cCollisionSteps = 1;
    m_physicsSystem->Update(deltaTime, cCollisionSteps, m_tempAllocator.get(), m_jobSystem.get());
}

JPH::BodyID PhysicsSystem::CreateBoxBody(const glm::vec3& position, const glm::vec3& size, bool isStatic) {
    if (!m_initialized)
        return JPH::BodyID();

    JPH::RefConst<JPH::Shape> box_shape = new JPH::BoxShape(JPH::Vec3(size.x * 0.5f, size.y * 0.5f, size.z * 0.5f));
    JPH::BodyCreationSettings body_settings(box_shape, ToJoltPos(position), JPH::Quat::sIdentity(),
                                            isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic,
                                            isStatic ? NON_MOVING : MOVING);

    JPH::Body* body = m_physicsSystem->GetBodyInterface().CreateBody(body_settings);
    if (body == nullptr) {
        BS_ERROR(LogCategory::PHYSICS, "Failed to create box body!");
        return JPH::BodyID();
    }

    JPH::BodyID bodyID = body->GetID();
    m_physicsSystem->GetBodyInterface().AddBody(bodyID, JPH::EActivation::Activate);
    return bodyID;
}

JPH::BodyID PhysicsSystem::CreateSphereBody(const glm::vec3& position, float radius, bool isStatic) {
    if (!m_initialized)
        return JPH::BodyID();

    JPH::RefConst<JPH::Shape> sphere_shape = new JPH::SphereShape(radius);
    JPH::BodyCreationSettings body_settings(sphere_shape, ToJoltPos(position), JPH::Quat::sIdentity(),
                                            isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic,
                                            isStatic ? NON_MOVING : MOVING);

    JPH::Body* body = m_physicsSystem->GetBodyInterface().CreateBody(body_settings);
    if (body == nullptr) {
        BS_ERROR(LogCategory::PHYSICS, "Failed to create sphere body!");
        return JPH::BodyID();
    }

    JPH::BodyID bodyID = body->GetID();
    m_physicsSystem->GetBodyInterface().AddBody(bodyID, JPH::EActivation::Activate);
    return bodyID;
}

JPH::BodyID PhysicsSystem::CreatePlaneBody(const glm::vec3& position, const glm::vec3& normal) {
    if (!m_initialized)
        return JPH::BodyID();

    JPH::RefConst<JPH::Shape> plane_shape = new JPH::BoxShape(JPH::Vec3(50.0f, 0.1f, 50.0f));
    JPH::BodyCreationSettings body_settings(plane_shape, ToJoltPos(position), JPH::Quat::sIdentity(),
                                            JPH::EMotionType::Static, NON_MOVING);

    JPH::Body* body = m_physicsSystem->GetBodyInterface().CreateBody(body_settings);
    if (body == nullptr) {
        BS_ERROR(LogCategory::PHYSICS, "Failed to create plane body!");
        return JPH::BodyID();
    }

    JPH::BodyID bodyID = body->GetID();
    m_physicsSystem->GetBodyInterface().AddBody(bodyID, JPH::EActivation::DontActivate);
    return bodyID;
}

void PhysicsSystem::SetBodyPosition(JPH::BodyID bodyID, const glm::vec3& position) {
    if (!m_initialized)
        return;
    m_physicsSystem->GetBodyInterface().SetPosition(bodyID, ToJoltPos(position), JPH::EActivation::Activate);
}

void PhysicsSystem::SetBodyRotation(JPH::BodyID bodyID, const glm::vec3& eulerAngles) {
    if (!m_initialized)
        return;
    m_physicsSystem->GetBodyInterface().SetRotation(bodyID, ToJoltRot(eulerAngles), JPH::EActivation::Activate);
}

glm::vec3 PhysicsSystem::GetBodyPosition(JPH::BodyID bodyID) {
    if (!m_initialized)
        return glm::vec3(0.0f);
    return ToGLMPos(m_physicsSystem->GetBodyInterface().GetCenterOfMassPosition(bodyID));
}

glm::vec3 PhysicsSystem::GetBodyRotation(JPH::BodyID bodyID) {
    if (!m_initialized)
        return glm::vec3(0.0f);
    return ToGLMRot(m_physicsSystem->GetBodyInterface().GetRotation(bodyID));
}

void PhysicsSystem::AddForce(JPH::BodyID bodyID, const glm::vec3& force) {
    if (!m_initialized)
        return;
    m_physicsSystem->GetBodyInterface().AddForce(bodyID, ToJoltPos(force));
}

void PhysicsSystem::AddImpulse(JPH::BodyID bodyID, const glm::vec3& impulse) {
    if (!m_initialized)
        return;
    m_physicsSystem->GetBodyInterface().AddImpulse(bodyID, ToJoltPos(impulse));
}

void PhysicsSystem::SetVelocity(JPH::BodyID bodyID, const glm::vec3& velocity) {
    if (!m_initialized)
        return;
    m_physicsSystem->GetBodyInterface().SetLinearVelocity(bodyID, ToJoltPos(velocity));
}

void PhysicsSystem::SetAngularVelocity(JPH::BodyID bodyID, const glm::vec3& angularVel) {
    if (!m_initialized)
        return;
    m_physicsSystem->GetBodyInterface().SetAngularVelocity(bodyID, ToJoltPos(angularVel));
}

// NEW METHODS - Static/Dynamic conversion!
void PhysicsSystem::MakeBodyStatic(JPH::BodyID bodyID) {
    if (!m_initialized)
        return;

    // Change motion type to static and move to NON_MOVING layer
    m_physicsSystem->GetBodyInterface().SetMotionType(bodyID, JPH::EMotionType::Static, JPH::EActivation::DontActivate);
    m_physicsSystem->GetBodyInterface().SetObjectLayer(bodyID, NON_MOVING);

    BS_DEBUG_F(LogCategory::PHYSICS, "Body %d is now static", bodyID.GetIndex());
}

void PhysicsSystem::MakeBodyDynamic(JPH::BodyID bodyID) {
    if (!m_initialized)
        return;

    // Change motion type to dynamic and move to MOVING layer
    m_physicsSystem->GetBodyInterface().SetMotionType(bodyID, JPH::EMotionType::Dynamic, JPH::EActivation::Activate);
    m_physicsSystem->GetBodyInterface().SetObjectLayer(bodyID, MOVING);

    BS_DEBUG_F(LogCategory::PHYSICS, "Body %d is now dynamic", bodyID.GetIndex());
}

bool PhysicsSystem::IsBodyStatic(JPH::BodyID bodyID) {
    if (!m_initialized)
        return false;

    return m_physicsSystem->GetBodyInterface().GetMotionType(bodyID) == JPH::EMotionType::Static;
}

glm::vec3 PhysicsSystem::GetVelocity(JPH::BodyID bodyID) {
    if (!m_initialized || !m_physicsSystem) {
        return glm::vec3(0.0f);
    }

    JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
    JPH::Vec3 velocity = bodyInterface.GetLinearVelocity(bodyID);
    return ToGLMPos(velocity);
}

glm::vec3 PhysicsSystem::GetAngularVelocity(JPH::BodyID bodyID) {
    if (!m_initialized || !m_physicsSystem) {
        return glm::vec3(0.0f);
    }

    JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
    JPH::Vec3 angularVel = bodyInterface.GetAngularVelocity(bodyID);
    return ToGLMPos(angularVel);
}

glm::vec3 PhysicsSystem::GetVelocity(int entityId) {
    JPH::BodyID bodyID = GetBodyIDFromEntityID(entityId);
    if (bodyID.IsInvalid()) {
        return glm::vec3(0.0f);
    }
    return GetVelocity(bodyID);
}

glm::vec3 PhysicsSystem::GetAngularVelocity(int entityId) {
    JPH::BodyID bodyID = GetBodyIDFromEntityID(entityId);
    if (bodyID.IsInvalid()) {
        return glm::vec3(0.0f);
    }
    return GetAngularVelocity(bodyID);
}

bool PhysicsSystem::IsBodyActive(JPH::BodyID bodyID) {
    if (!m_initialized || !m_physicsSystem) {
        return false;
    }

    JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
    return bodyInterface.IsActive(bodyID);
}

float PhysicsSystem::GetBodyMass(JPH::BodyID bodyID) {
    if (!m_initialized || !m_physicsSystem) {
        return 0.0f;
    }

    JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
    JPH::BodyLockRead lock(m_physicsSystem->GetBodyLockInterface(), bodyID);
    if (lock.Succeeded()) {
        const JPH::Body& body = lock.GetBody();
        return 1.0f / body.GetMotionProperties()->GetInverseMass();
    }
    return 0.0f;
}

glm::vec3 PhysicsSystem::GetBodyCenterOfMass(JPH::BodyID bodyID) {
    if (!m_initialized || !m_physicsSystem) {
        return glm::vec3(0.0f);
    }

    JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
    JPH::Vec3 centerOfMass = bodyInterface.GetCenterOfMassPosition(bodyID);
    return ToGLMPos(centerOfMass);
}

JPH::BodyID PhysicsSystem::GetBodyIDFromEntityID(int entityId) {
    auto it = m_entityToBodyMap.find(entityId);
    if (it != m_entityToBodyMap.end()) {
        return it->second;
    }
    return JPH::BodyID(); // Invalid body ID
}

void PhysicsSystem::MapEntityToBody(int entityId, JPH::BodyID bodyID) {
    m_entityToBodyMap[entityId] = bodyID;
}

void PhysicsSystem::UnmapEntity(int entityId) {
    auto it = m_entityToBodyMap.find(entityId);
    if (it != m_entityToBodyMap.end()) {
        // Optionally remove the body from physics system here
        JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
        bodyInterface.RemoveBody(it->second);
        bodyInterface.DestroyBody(it->second);

        m_entityToBodyMap.erase(it);
    }
}

}  // namespace Blacksite
