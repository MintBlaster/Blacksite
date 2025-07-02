#include "blacksite/graphics/shaders/DebugShaders.h"

namespace Blacksite {
namespace Shaders {

    const char* WIREFRAME_VERTEX_SHADER = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec2 aTexCoord;

        uniform mat4 uModel;
        uniform mat4 uView;
        uniform mat4 uProjection;

        out vec3 barycentric;

        void main()
        {
            // Set barycentric coordinates for wireframe effect
            if (gl_VertexID % 3 == 0) barycentric = vec3(1.0, 0.0, 0.0);
            else if (gl_VertexID % 3 == 1) barycentric = vec3(0.0, 1.0, 0.0);
            else barycentric = vec3(0.0, 0.0, 1.0);

            gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
        }
    )";

    const char* WIREFRAME_FRAGMENT_SHADER = R"(
        #version 330 core
        in vec3 barycentric;
        out vec4 FragColor;

        uniform vec3 uColor;
        uniform float uLineWidth;

        void main()
        {
            float minDist = min(min(barycentric.x, barycentric.y), barycentric.z);
            float lineWidth = uLineWidth * 0.01;

            if (minDist < lineWidth) {
                FragColor = vec4(uColor, 1.0);
            } else {
                discard; // Only show wireframe lines
            }
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
    in vec2 TexCoord;
    out vec4 FragColor;

    uniform vec3 uColor;
    uniform int uDebugMode; // 0=solid, 1=normals, 2=UVs, 3=checkerboard

    void main()
    {
        if (uDebugMode == 1) {
            // Visualize normals
            vec3 normalColor = normalize(Normal) * 0.5 + 0.5;
            FragColor = vec4(normalColor, 1.0);
        } else if (uDebugMode == 2) {
            // Visualize UVs
            FragColor = vec4(TexCoord, 0.0, 1.0);
        } else if (uDebugMode == 3) {
            // Checkerboard pattern
            float checker = mod(floor(TexCoord.x * 8.0) + floor(TexCoord.y * 8.0), 2.0);
            FragColor = vec4(vec3(checker), 1.0);
        } else {
            // Solid color
            FragColor = vec4(uColor, 1.0);
        }
    }
)";


} // namespace Shaders
} // namespace Blacksite
