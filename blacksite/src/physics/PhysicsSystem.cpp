#include "blacksite/physics/PhysicsSystem.h"
#include <cstdarg>
#include <cstdio>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "blacksite/core/Logger.h"
#include "Core/IssueReporting.h"
#include "Core/Memory.h"
#include "Physics/Collision/Shape/StaticCompoundShape.h"

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
    BS_ERROR_F(Blacksite::LogCategory::PHYSICS, "[Jolt Assert] %s:%d: (%s) %s", inFile, inLine, inExpression,
               (inMessage != nullptr ? inMessage : ""));
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

JPH::BodyID PhysicsSystem::CreatePlaneBody(const glm::vec3& position, const glm::vec3& size) {
    if (!m_initialized)
        return JPH::BodyID();

    // Use provided size instead of hardcoded 50x0.1x50
    JPH::RefConst<JPH::Shape> plane_shape = new JPH::BoxShape(JPH::Vec3(size.x * 0.5f, size.y * 0.5f, size.z * 0.5f));
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

JPH::BodyID PhysicsSystem::CreatePhysicsBody(Entity& entity) {
    if (!m_initialized) {
        BS_ERROR(LogCategory::PHYSICS, "Physics system not initialized");
        return JPH::BodyID();
    }

    // If no colliders exist, add default based on visual shape
    if (entity.colliders.empty()) {
        AddDefaultColliderToEntity(entity);
    }

    // Create compound shape from all colliders
    JPH::Ref<JPH::Shape> shape = CreateShapeFromColliders(entity.colliders, entity.transform.scale);
    if (!shape) {
        BS_ERROR_F(LogCategory::PHYSICS, "Failed to create shape for entity %d", entity.id);
        return JPH::BodyID();
    }

    // Create body settings
    JPH::BodyCreationSettings bodySettings(shape, ToJoltPos(entity.transform.position),
                                           ToJoltRot(entity.transform.rotation),
                                           entity.isDynamic ? JPH::EMotionType::Dynamic : JPH::EMotionType::Static,
                                           entity.isDynamic ? MOVING : NON_MOVING);

    // Create and add body
    JPH::Body* body = m_physicsSystem->GetBodyInterface().CreateBody(bodySettings);
    if (!body) {
        BS_ERROR_F(LogCategory::PHYSICS, "Failed to create physics body for entity %d", entity.id);
        return JPH::BodyID();
    }

    JPH::BodyID bodyID = body->GetID();
    m_physicsSystem->GetBodyInterface().AddBody(bodyID, JPH::EActivation::Activate);

    // Store mapping and update entity
    entity.physicsBody = bodyID;
    entity.hasPhysics = true;
    MapEntityToBody(entity.id, bodyID);

    BS_DEBUG_F(LogCategory::PHYSICS, "Created physics body for entity %d with %zu colliders", entity.id,
               entity.colliders.size());

    return bodyID;
}

void PhysicsSystem::UpdatePhysicsBody(Entity& entity) {
    if (!entity.hasPhysics || entity.physicsBody.IsInvalid()) {
        return;
    }

    // Remove old body
    JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
    bodyInterface.RemoveBody(entity.physicsBody);
    bodyInterface.DestroyBody(entity.physicsBody);

    // Create new body with updated colliders
    entity.hasPhysics = false;
    entity.physicsBody = JPH::BodyID();
    CreatePhysicsBody(entity);
}

void PhysicsSystem::RemovePhysicsBody(Entity& entity) {
    if (!entity.hasPhysics || entity.physicsBody.IsInvalid()) {
        return;
    }

    JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
    bodyInterface.RemoveBody(entity.physicsBody);
    bodyInterface.DestroyBody(entity.physicsBody);

    UnmapEntity(entity.id);
    entity.hasPhysics = false;
    entity.physicsBody = JPH::BodyID();

    BS_DEBUG_F(LogCategory::PHYSICS, "Removed physics body for entity %d", entity.id);
}

JPH::Ref<JPH::Shape> PhysicsSystem::CreateShapeFromColliders(const std::vector<Collider>& colliders,
                                                             const glm::vec3& entityScale) {
    if (colliders.empty()) {
        return nullptr;
    }

    if (colliders.size() == 1) {
        // Single collider - create simple shape
        return CreateSingleColliderShape(colliders[0], entityScale);
    }

    // Multiple colliders - create compound shape
    JPH::StaticCompoundShapeSettings compoundSettings;

    for (const auto& collider : colliders) {
        JPH::Ref<JPH::Shape> shape = CreateSingleColliderShape(collider, entityScale);
        if (shape) {
            // Apply local transform (center and rotation)
            glm::vec3 scaledCenter = collider.center * entityScale;
            JPH::Vec3 position(scaledCenter.x, scaledCenter.y, scaledCenter.z);
            JPH::Quat rotation(collider.rotation.x, collider.rotation.y, collider.rotation.z, collider.rotation.w);

            compoundSettings.AddShape(position, rotation, shape);
        }
    }

    JPH::Shape::ShapeResult result = compoundSettings.Create();
    if (result.HasError()) {
        BS_ERROR_F(LogCategory::PHYSICS, "Failed to create compound shape: %s", result.GetError().c_str());
        return nullptr;
    }

    return result.Get();
}

JPH::Ref<JPH::Shape> PhysicsSystem::CreateSingleColliderShape(const Collider& collider, const glm::vec3& entityScale) {
    // Apply entity scale to collider size
    glm::vec3 scaledSize = collider.size * entityScale;

    // Ensure minimum size to prevent physics issues
    scaledSize.x = std::max(scaledSize.x, 0.01f);
    scaledSize.y = std::max(scaledSize.y, 0.01f);
    scaledSize.z = std::max(scaledSize.z, 0.01f);

    BS_DEBUG(LogCategory::PHYSICS, "CreateSingleColliderShape:");
    BS_DEBUG_F(LogCategory::PHYSICS, "  Original collider size: (%.2f, %.2f, %.2f)", collider.size.x, collider.size.y,
               collider.size.z);
    BS_DEBUG_F(LogCategory::PHYSICS, "  Entity scale: (%.2f, %.2f, %.2f)", entityScale.x, entityScale.y, entityScale.z);
    BS_DEBUG_F(LogCategory::PHYSICS, "  Final scaled size: (%.2f, %.2f, %.2f)", scaledSize.x, scaledSize.y,
               scaledSize.z);

    switch (collider.type) {
        case ColliderType::Box: {
            JPH::Vec3 halfExtents(scaledSize.x * 0.5f, scaledSize.y * 0.5f, scaledSize.z * 0.5f);
            BS_DEBUG_F(LogCategory::PHYSICS, "  Box half extents: (%.2f, %.2f, %.2f)", halfExtents.GetX(),
                       halfExtents.GetY(), halfExtents.GetZ());
            return new JPH::BoxShape(halfExtents);
        }
        case ColliderType::Sphere: {
            // For spheres, use the largest component for uniform scaling
            float radius = std::max({scaledSize.x, scaledSize.y, scaledSize.z}) * 0.5f;
            BS_DEBUG_F(LogCategory::PHYSICS, "  Sphere radius: %.2f", radius);
            return new JPH::SphereShape(radius);
        }
        default:
            BS_ERROR_F(LogCategory::PHYSICS, "Unknown collider type: %d", static_cast<int>(collider.type));
            return nullptr;
    }
}

void PhysicsSystem::AddDefaultColliderToEntity(Entity& entity) {
    Collider defaultCollider;

    switch (entity.shape) {
        case Entity::CUBE:
            defaultCollider.type = ColliderType::Box;
            defaultCollider.size = glm::vec3(1.0f);
            break;
        case Entity::SPHERE:
            defaultCollider.type = ColliderType::Sphere;
            defaultCollider.size = glm::vec3(1.f, 0.0f, 0.0f);
            break;
        case Entity::PLANE:
            defaultCollider.type = ColliderType::Box;
            // Jolt Physics requires that all half extents must be >= convex radius (which is typically around 0.05).
            // Y would become too small on .1f.
            defaultCollider.size = glm::vec3(1.0f, .2f, 1.0f);
            break;
    }

    entity.colliders.push_back(defaultCollider);
    BS_DEBUG_F(LogCategory::PHYSICS, "Added default collider to entity %d (type: %d, size: %.2f,%.2f,%.2f)", entity.id,
               static_cast<int>(defaultCollider.type), defaultCollider.size.x, defaultCollider.size.y,
               defaultCollider.size.z);
}

void PhysicsSystem::AddColliderToEntity(Entity& entity, const Collider& collider) {
    entity.colliders.push_back(collider);

    // If entity already has physics, update the body
    if (entity.hasPhysics) {
        UpdatePhysicsBody(entity);
    }

    BS_DEBUG_F(LogCategory::PHYSICS, "Added collider to entity %d (total: %zu)", entity.id, entity.colliders.size());
}

void PhysicsSystem::RemoveColliderFromEntity(Entity& entity, size_t colliderIndex) {
    if (colliderIndex >= entity.colliders.size()) {
        BS_ERROR_F(LogCategory::PHYSICS, "Invalid collider index %zu for entity %d", colliderIndex, entity.id);
        return;
    }

    entity.colliders.erase(entity.colliders.begin() + colliderIndex);

    // If entity has physics, update the body
    if (entity.hasPhysics) {
        if (entity.colliders.empty()) {
            // No colliders left - remove physics body
            RemovePhysicsBody(entity);
        } else {
            // Update with remaining colliders
            UpdatePhysicsBody(entity);
        }
    }

    BS_DEBUG_F(LogCategory::PHYSICS, "Removed collider from entity %d (remaining: %zu)", entity.id,
               entity.colliders.size());
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
    return JPH::BodyID();  // Invalid body ID
}

void PhysicsSystem::MapEntityToBody(int entityId, JPH::BodyID bodyID) {
    m_entityToBodyMap[entityId] = bodyID;
}

void PhysicsSystem::UnmapEntity(int entityId) {
    auto it = m_entityToBodyMap.find(entityId);
    if (it != m_entityToBodyMap.end()) {
        JPH::BodyID bodyID = it->second;

        m_entityToBodyMap.erase(it);

        // Then safely remove and destroy the body
        if (!bodyID.IsInvalid() && m_physicsSystem) {
            JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
            bodyInterface.RemoveBody(bodyID);
            bodyInterface.DestroyBody(bodyID);
        }
    }
}

}  // namespace Blacksite
