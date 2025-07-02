#pragma once
#include <blacksite/physics/JoltConfig.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "blacksite/math/Transform.h"
#include "blacksite/physics/Collider.h"

namespace Blacksite {

struct Entity {
    int id = -1;
    std::string name;
    bool active = true;

    Transform transform;

    enum VisualShape { CUBE, SPHERE, PLANE } shape = CUBE;  // Visual
    std::string shader{"basic"};
    glm::vec3 color{1.0f, 1.0f, 1.0f};

    // Physics
    bool hasPhysics = false;
    bool isDynamic = true;
    JPH::BodyID physicsBody;
    std::vector<Collider> colliders; // defines the body shape

    Entity() = default;
    Entity(VisualShape s) : shape(s) {}
    Entity(VisualShape s, const std::string& shaderName) : shape(s), shader(shaderName) {}
};

}  // namespace Blacksite
