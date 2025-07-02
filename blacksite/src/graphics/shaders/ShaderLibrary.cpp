#include "blacksite/graphics/shaders/ShaderLibrary.h"
#include "blacksite/graphics/shaders/CoreShaders.h"
#include "blacksite/graphics/shaders/DebugShaders.h"
#include "blacksite/graphics/shaders/PostProcessShaders.h"

namespace Blacksite {

// Static member definitions
std::unordered_map<std::string, ShaderSource> ShaderLibrary::s_shaders;
std::unordered_map<ShaderType, std::string> ShaderLibrary::s_typeToName;
bool ShaderLibrary::s_initialized = false;

const ShaderSource* ShaderLibrary::GetShader(const std::string& name) {
    if (!s_initialized) {
        InitializeShaders();
    }

    auto it = s_shaders.find(name);
    return (it != s_shaders.end()) ? &it->second : nullptr;
}

const ShaderSource* ShaderLibrary::GetShader(ShaderType type) {
    if (!s_initialized) {
        InitializeShaders();
    }

    auto it = s_typeToName.find(type);
    if (it != s_typeToName.end()) {
        return GetShader(it->second);
    }
    return nullptr;
}

std::vector<std::string> ShaderLibrary::GetAvailableShaderNames() {
    if (!s_initialized) {
        InitializeShaders();
    }

    std::vector<std::string> names;
    names.reserve(s_shaders.size());

    for (const auto& pair : s_shaders) {
        names.push_back(pair.first);
    }

    return names;
}

std::vector<ShaderType> ShaderLibrary::GetAvailableShaderTypes() {
    if (!s_initialized) {
        InitializeShaders();
    }

    std::vector<ShaderType> types;
    types.reserve(s_typeToName.size());

    for (const auto& pair : s_typeToName) {
        types.push_back(pair.first);
    }

    return types;
}

bool ShaderLibrary::HasShader(const std::string& name) {
    if (!s_initialized) {
        InitializeShaders();
    }

    return s_shaders.find(name) != s_shaders.end();
}

bool ShaderLibrary::HasShader(ShaderType type) {
    if (!s_initialized) {
        InitializeShaders();
    }

    return s_typeToName.find(type) != s_typeToName.end();
}

std::string ShaderLibrary::GetShaderDescription(const std::string& name) {
    const ShaderSource* shader = GetShader(name);
    return shader ? shader->description : "Shader not found";
}

bool ShaderLibrary::ShaderRequiresTime(const std::string& name) {
    const ShaderSource* shader = GetShader(name);
    return shader ? shader->requiresTime : false;
}

bool ShaderLibrary::ShaderRequiresLighting(const std::string& name) {
    const ShaderSource* shader = GetShader(name);
    return shader ? shader->requiresLighting : true; // Default to true for safety
}

void ShaderLibrary::InitializeShaders() {
    if (s_initialized) return;

    // Core rendering shaders
    s_shaders["basic"] = {
        Shaders::BASIC_VERTEX_SHADER,
        Shaders::BASIC_FRAGMENT_SHADER,
        "basic",
        "Standard Phong lighting with texture support",
        false, // requiresTime
        true   // requiresLighting
    };

    s_shaders["unlit"] = {
        Shaders::UNLIT_VERTEX_SHADER,
        Shaders::UNLIT_FRAGMENT_SHADER,
        "unlit",
        "No lighting, just color and texture",
        false, // requiresTime
        false  // requiresLighting
    };

    s_shaders["transparent"] = {
        Shaders::TRANSPARENT_VERTEX_SHADER,
        Shaders::TRANSPARENT_FRAGMENT_SHADER,
        "transparent",
        "Basic lighting with alpha blending support",
        false, // requiresTime
        true   // requiresLighting
    };

    // Debug shaders
    s_shaders["wireframe"] = {
        Shaders::WIREFRAME_VERTEX_SHADER,
        Shaders::WIREFRAME_FRAGMENT_SHADER,
        "wireframe",
        "Simple wireframe rendering for debug visualization",
        false, // requiresTime
        false  // requiresLighting
    };

    s_shaders["debug"] = {
        Shaders::DEBUG_VERTEX_SHADER,
        Shaders::DEBUG_FRAGMENT_SHADER,
        "debug",
        "Debug shader with normal visualization",
        false, // requiresTime
        false  // requiresLighting
    };

    // Post-processing shaders
    s_shaders["postprocess"] = {
        Shaders::POSTPROCESS_VERTEX_SHADER,
        Shaders::POSTPROCESS_FRAGMENT_SHADER,
        "postprocess",
        "Basic post-processing with tone mapping and gamma correction",
        false, // requiresTime
        false  // requiresLighting
    };

    s_shaders["blur"] = {
        Shaders::BLUR_VERTEX_SHADER,
        Shaders::BLUR_FRAGMENT_SHADER,
        "blur",
        "Gaussian blur for post-processing effects",
        false, // requiresTime
        false  // requiresLighting
    };

    s_shaders["bloom"] = {
        Shaders::BLOOM_VERTEX_SHADER,
        Shaders::BLOOM_FRAGMENT_SHADER,
        "bloom",
        "Bloom effect combining scene and blur textures",
        false, // requiresTime
        false  // requiresLighting
    };

    s_shaders["fxaa"] = {
        Shaders::FXAA_VERTEX_SHADER,
        Shaders::FXAA_FRAGMENT_SHADER,
        "fxaa",
        "Fast Approximate Anti-Aliasing",
        false, // requiresTime
        false  // requiresLighting
    };

    // Map shader types to names
    s_typeToName[ShaderType::Basic] = "basic";
    s_typeToName[ShaderType::Unlit] = "unlit";
    s_typeToName[ShaderType::Transparent] = "transparent";
    s_typeToName[ShaderType::Wireframe] = "wireframe";
    s_typeToName[ShaderType::Debug] = "debug";
    s_typeToName[ShaderType::PostProcess] = "postprocess";
    s_typeToName[ShaderType::Blur] = "blur";
    s_typeToName[ShaderType::Bloom] = "bloom";
    s_typeToName[ShaderType::FXAA] = "fxaa";

    s_initialized = true;
}

} // namespace Blacksite
