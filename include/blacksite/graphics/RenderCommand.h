#pragma once
#include <glm/glm.hpp>
#include <string>
#include "blacksite/math/Transform.h"

namespace Blacksite {


struct RenderCommand {
    std::string meshName;               // What geometry to draw
    Transform transform;                // Where to put it and how big
    glm::vec3 color{1.0f, 1.0f, 1.0f};  // What color (white by default)
    std::string shaderName = "basic";   // Which shader to use
};

}  // namespace Blacksite