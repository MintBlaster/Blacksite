#include "blacksite/graphics/shaders/PostProcessShaders.h"

namespace Blacksite {
namespace Shaders {

const char* POSTPROCESS_VERTEX_SHADER = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec2 aTexCoord;

    out vec2 TexCoord;

    void main()
    {
        TexCoord = aTexCoord;
        gl_Position = vec4(aPos, 0.0, 1.0);
    }
)";

const char* POSTPROCESS_FRAGMENT_SHADER = R"(
    #version 330 core
    in vec2 TexCoord;

    out vec4 FragColor;

    uniform sampler2D uScreenTexture;
    uniform float uGamma;
    uniform float uExposure;

    void main()
    {
        vec3 color = texture(uScreenTexture, TexCoord).rgb;

        // Tone mapping
        color = vec3(1.0) - exp(-color * uExposure);

        // Gamma correction
        color = pow(color, vec3(1.0 / uGamma));

        FragColor = vec4(color, 1.0);
    }
)";

const char* BLUR_VERTEX_SHADER = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec2 aTexCoord;

    out vec2 TexCoord;

    void main()
    {
        TexCoord = aTexCoord;
        gl_Position = vec4(aPos, 0.0, 1.0);
    }
)";

const char* BLUR_FRAGMENT_SHADER = R"(
    #version 330 core
    in vec2 TexCoord;

    out vec4 FragColor;

    uniform sampler2D uTexture;
    uniform bool uHorizontal;
    uniform float uBlurSize;

    float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

    void main()
    {
        vec2 tex_offset = 1.0 / textureSize(uTexture, 0) * uBlurSize;
        vec3 result = texture(uTexture, TexCoord).rgb * weight[0];

        if(uHorizontal) {
            for(int i = 1; i < 5; ++i) {
                result += texture(uTexture, TexCoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
                result += texture(uTexture, TexCoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            }
        } else {
            for(int i = 1; i < 5; ++i) {
                result += texture(uTexture, TexCoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
                result += texture(uTexture, TexCoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            }
        }

        FragColor = vec4(result, 1.0);
    }
)";

const char* BLOOM_VERTEX_SHADER = POSTPROCESS_VERTEX_SHADER;

const char* BLOOM_FRAGMENT_SHADER = R"(
    #version 330 core
    in vec2 TexCoord;

    out vec4 FragColor;

    uniform sampler2D uScene;
    uniform sampler2D uBloomBlur;
    uniform float uBloomStrength;

    void main()
    {
        vec3 sceneColor = texture(uScene, TexCoord).rgb;
        vec3 bloomColor = texture(uBloomBlur, TexCoord).rgb;

        vec3 result = sceneColor + bloomColor * uBloomStrength;
        FragColor = vec4(result, 1.0);
    }
)";

const char* FXAA_VERTEX_SHADER = POSTPROCESS_VERTEX_SHADER;

const char* FXAA_FRAGMENT_SHADER = R"(
    #version 330 core
    in vec2 TexCoord;

    out vec4 FragColor;

    uniform sampler2D uTexture;
    uniform vec2 uInverseScreenSize;

    void main()
    {
        vec3 rgbNW = texture(uTexture, TexCoord + vec2(-1.0, -1.0) * uInverseScreenSize).rgb;
        vec3 rgbNE = texture(uTexture, TexCoord + vec2(1.0, -1.0) * uInverseScreenSize).rgb;
        vec3 rgbSW = texture(uTexture, TexCoord + vec2(-1.0, 1.0) * uInverseScreenSize).rgb;
        vec3 rgbSE = texture(uTexture, TexCoord + vec2(1.0, 1.0) * uInverseScreenSize).rgb;
        vec3 rgbM = texture(uTexture, TexCoord).rgb;

        vec3 luma = vec3(0.299, 0.587, 0.114);
        float lumaNW = dot(rgbNW, luma);
        float lumaNE = dot(rgbNE, luma);
        float lumaSW = dot(rgbSW, luma);
        float lumaSE = dot(rgbSE, luma);
        float lumaM = dot(rgbM, luma);

        float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
        float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

        vec2 dir = vec2(-((lumaNW + lumaNE) - (lumaSW + lumaSE)), ((lumaNW + lumaSW) - (lumaNE + lumaSE)));

        float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * 0.125), 0.0078125);
        float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);

        dir = min(vec2(8.0), max(vec2(-8.0), dir * rcpDirMin)) * uInverseScreenSize;

        vec3 rgbA = 0.5 * (texture(uTexture, TexCoord + dir * (1.0/3.0 - 0.5)).rgb +
                          texture(uTexture, TexCoord + dir * (2.0/3.0 - 0.5)).rgb);
        vec3 rgbB = rgbA * 0.5 + 0.25 * (texture(uTexture, TexCoord + dir * -0.5).rgb +
                                        texture(uTexture, TexCoord + dir * 0.5).rgb);

        float lumaB = dot(rgbB, luma);
        if ((lumaB < lumaMin) || (lumaB > lumaMax)) {
            FragColor = vec4(rgbA, 1.0);
        } else {
            FragColor = vec4(rgbB, 1.0);
        }
    }
)";

} // namespace Shaders
} // namespace Blacksite
