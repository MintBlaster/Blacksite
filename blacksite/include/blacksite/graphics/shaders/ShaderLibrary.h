#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace Blacksite {

enum class ShaderType {
    // Core rendering shaders
    Basic,          // Standard Phong lighting
    Unlit,          // No lighting, just color/texture

    // Debug shaders
    Wireframe,      // For collider visualization
    Debug,          // Solid color for debugging

    // Effect shaders
    Transparent,    // Alpha blending support

    // Post-processing shaders
    PostProcess,    // Basic post-process quad
    Blur,           // Gaussian blur
    Bloom,          // Bloom effect
    FXAA,           // Anti-aliasing
};

struct ShaderSource {
    const char* vertexSource;
    const char* fragmentSource;
    std::string name;
    std::string description;
    bool requiresTime = false;      // Does this shader need time uniform?
    bool requiresLighting = false;  // Does this shader need lighting uniforms?
};

class ShaderLibrary {
public:
    // Get shader source by name
    static const ShaderSource* GetShader(const std::string& name);
    static const ShaderSource* GetShader(ShaderType type);

    // Get all available shaders
    static std::vector<std::string> GetAvailableShaderNames();
    static std::vector<ShaderType> GetAvailableShaderTypes();

    // Check if shader exists
    static bool HasShader(const std::string& name);
    static bool HasShader(ShaderType type);

    // Get shader info
    static std::string GetShaderDescription(const std::string& name);
    static bool ShaderRequiresTime(const std::string& name);
    static bool ShaderRequiresLighting(const std::string& name);

private:
    static void InitializeShaders();
    static std::unordered_map<std::string, ShaderSource> s_shaders;
    static std::unordered_map<ShaderType, std::string> s_typeToName;
    static bool s_initialized;
};

} // namespace Blacksite
