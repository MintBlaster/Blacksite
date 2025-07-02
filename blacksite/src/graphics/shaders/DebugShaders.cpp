#include "blacksite/graphics/shaders/DebugShaders.h"

namespace Blacksite {
namespace Shaders {

const char* WIREFRAME_VERTEX_SHADER = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;

    uniform mat4 uModel;
    uniform mat4 uView;
    uniform mat4 uProjection;

    void main()
    {
        gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
    }
)";

const char* WIREFRAME_FRAGMENT_SHADER = R"(
    #version 330 core
    out vec4 FragColor;

    uniform vec3 uColor;

    void main()
    {
        FragColor = vec4(uColor, 1.0);
    }
)";

const char* DEBUG_VERTEX_SHADER = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;

    uniform mat4 uModel;
    uniform mat4 uView;
    uniform mat4 uProjection;

    out vec3 Normal;

    void main()
    {
        Normal = mat3(transpose(inverse(uModel))) * aNormal;
        gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
    }
)";

const char* DEBUG_FRAGMENT_SHADER = R"(
    #version 330 core
    in vec3 Normal;

    out vec4 FragColor;

    uniform vec3 uColor;
    uniform bool uShowNormals;

    void main()
    {
        if (uShowNormals) {
            // Visualize normals as colors
            vec3 normalColor = normalize(Normal) * 0.5 + 0.5;
            FragColor = vec4(normalColor, 1.0);
        } else {
            FragColor = vec4(uColor, 1.0);
        }
    }
)";

} // namespace Shaders
} // namespace Blacksite
