#pragma once
#include <blacksite/physics/JoltConfig.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
// #include <nlohmann/json.hpp>
#include "blacksite/math/Transform.h"
#include "blacksite/physics/Collider.h"

namespace Blacksite {

class Entity {
public:
    int id = -1;
    std::string name;
    bool active = true;
    Transform transform;

    // Visual properties
    enum VisualShape { CUBE, SPHERE, PLANE } shape = CUBE;
    std::string shader{"basic"};
    glm::vec3 color{1.0f, 1.0f, 1.0f};

    // Physics
    bool hasPhysics = false;
    bool isDynamic = true;
    JPH::BodyID physicsBody;
    std::vector<Collider> colliders;

    // Space management
    enum class Space {
        SceneSpace,
        EditorSpace,
        RuntimeSpace,
        UISpace
    };
    Space space = Space::SceneSpace;

    // Constructors
    Entity() = default;
    Entity(VisualShape s) : shape(s) {}
    Entity(VisualShape s, const std::string& shaderName) : shape(s), shader(shaderName) {}
    virtual ~Entity() = default;

    // Virtual methods for custom behavior
    virtual void OnSpawn() {}
    virtual void Update(float deltaTime) {}
    virtual void OnCollision(Entity& other) {}
    virtual void OnDestroy() {}

    // Serialization support
    // virtual void Serialize(nlohmann::json& data) const {}
    // virtual void Deserialize(const nlohmann::json& data) {}
    virtual std::string GetTypeName() const { return "Entity"; }
};

}  // namespace Blacksite
