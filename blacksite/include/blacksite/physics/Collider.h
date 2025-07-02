#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Blacksite {

enum class ColliderType {
    Box,
    Sphere,
    Capsule
};

struct Collider {
    ColliderType type = ColliderType::Box;
    glm::vec3 center = glm::vec3(0.0f); // Local offset
    glm::quat rotation = glm::quat();   // Local rotation
    glm::vec3 size = glm::vec3(1.0f);   // Extents for box, radius/height for others
};

}  // namespace Blacksite
