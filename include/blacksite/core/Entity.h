#pragma once

#include <blacksite/physics/JoltConfig.h>

#include <Jolt/Physics/Body/BodyID.h>
#include <glm/glm.hpp>
#include <string>
#include "blacksite/math/Transform.h"

namespace Blacksite {

struct Entity {
    Transform transform;
    JPH::BodyID physicsBody;  // Jolt Physics body ID
    bool hasPhysics = false;  // Whether this entity has a physics body

    enum Shape { CUBE, SPHERE, PLANE } shape;

    glm::vec3 color{1.0f, 1.0f, 1.0f};
    std::string shader{"basic"};
    bool active = true;

    Entity(Shape s) : shape(s) {}

    // Convenience constructor with shader
    Entity(Shape s, const std::string& shaderName) : shape(s), shader(shaderName) {}
};

}  // namespace Blacksite
