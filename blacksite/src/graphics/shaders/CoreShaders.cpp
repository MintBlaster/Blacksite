#include "blacksite/graphics/shaders/CoreShaders.h"

namespace Blacksite {
namespace Shaders {

const char* BASIC_VERTEX_SHADER = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;
    layout (location = 2) in vec2 aTexCoord;

    uniform mat4 uModel;
    uniform mat4 uView;
    uniform mat4 uProjection;

    out vec3 FragPos;
    out vec3 Normal;
    out vec2 TexCoord;

    void main()
    {
        FragPos = vec3(uModel * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(uModel))) * aNormal;
        TexCoord = aTexCoord;

        gl_Position = uProjection * uView * vec4(FragPos, 1.0);
    }
)";

const char* BASIC_FRAGMENT_SHADER = R"(
    #version 330 core
    in vec3 FragPos;
    in vec3 Normal;
    in vec2 TexCoord;

    out vec4 FragColor;

    uniform vec3 uColor;
    uniform vec3 uLightPos;
    uniform vec3 uViewPos;
    uniform bool uHasTexture;
    uniform sampler2D uTexture;

    void main()
    {
        vec3 baseColor = uColor;
        if (uHasTexture) {
            baseColor *= texture(uTexture, TexCoord).rgb;
        }

        vec3 lightColor = vec3(1.0);
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(uLightPos - FragPos);

        // Ambient
        float ambientStrength = 0.1;
        vec3 ambient = ambientStrength * lightColor;

        // Diffuse
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        // Specular
        float specularStrength = 0.5;
        vec3 viewDir = normalize(uViewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * lightColor;

        vec3 result = (ambient + diffuse + specular) * baseColor;
        FragColor = vec4(result, 1.0);
    }
)";

const char* UNLIT_VERTEX_SHADER = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 2) in vec2 aTexCoord;

    uniform mat4 uModel;
    uniform mat4 uView;
    uniform mat4 uProjection;

    out vec2 TexCoord;

    void main()
    {
        TexCoord = aTexCoord;
        gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
    }
)";

const char* UNLIT_FRAGMENT_SHADER = R"(
    #version 330 core
    in vec2 TexCoord;

    out vec4 FragColor;

    uniform vec3 uColor;
    uniform bool uHasTexture;
    uniform sampler2D uTexture;

    void main()
    {
        vec3 color = uColor;
        if (uHasTexture) {
            color *= texture(uTexture, TexCoord).rgb;
        }
        FragColor = vec4(color, 1.0);
    }
)";

const char* TRANSPARENT_VERTEX_SHADER = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;
    layout (location = 2) in vec2 aTexCoord;

    uniform mat4 uModel;
    uniform mat4 uView;
    uniform mat4 uProjection;

    out vec3 FragPos;
    out vec3 Normal;
    out vec2 TexCoord;

    void main()
    {
        FragPos = vec3(uModel * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(uModel))) * aNormal;
        TexCoord = aTexCoord;

        gl_Position = uProjection * uView * vec4(FragPos, 1.0);
    }
)";

const char* TRANSPARENT_FRAGMENT_SHADER = R"(
    #version 330 core
    in vec3 FragPos;
    in vec3 Normal;
    in vec2 TexCoord;

    out vec4 FragColor;

    uniform vec3 uColor;
    uniform float uAlpha;
    uniform vec3 uLightPos;
    uniform vec3 uViewPos;
    uniform bool uHasTexture;
    uniform sampler2D uTexture;

    void main()
    {
        vec4 texColor = vec4(uColor, uAlpha);
        if (uHasTexture) {
            texColor *= texture(uTexture, TexCoord);
        }

        vec3 lightColor = vec3(1.0);
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(uLightPos - FragPos);

        float ambientStrength = 0.3;
        vec3 ambient = ambientStrength * lightColor;

        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        vec3 result = (ambient + diffuse) * texColor.rgb;
        FragColor = vec4(result, texColor.a);
    }
)";

} // namespace Shaders
} // namespace Blacksite
