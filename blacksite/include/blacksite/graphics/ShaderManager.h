#pragma once

#include <GL/gl.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

namespace Blacksite {

class ShaderManager {
public:
    ShaderManager() = default;
    ~ShaderManager();

    // Load shader from the shader library
    bool LoadShaderFromLibrary(const std::string& libraryName);

    // Load and compile shader programs (existing method)
    bool LoadShader(const std::string& name, const char* vertexSource, const char* fragmentSource);
    bool UseShader(const std::string& name);

    // Uniform setters - Complete set
    void SetUniform(const std::string& name, const glm::mat4& matrix);
    void SetUniform(const std::string& name, const glm::mat3& matrix);
    void SetUniform(const std::string& name, const glm::vec4& vector);
    void SetUniform(const std::string& name, const glm::vec3& vector);
    void SetUniform(const std::string& name, const glm::vec2& vector);
    void SetUniform(const std::string& name, float value);
    void SetUniform(const std::string& name, int value);
    void SetUniform(const std::string& name, bool value);

    // Two-parameter overloads for convenience
    void SetUniform(const std::string& name, float x, float y);
    void SetUniform(const std::string& name, float x, float y, float z);
    void SetUniform(const std::string& name, float x, float y, float z, float w);
    void SetUniform(const std::string& name, int x, int y);

    // Utility methods
    bool HasShader(const std::string& name) const;
    std::string GetCurrentShaderName() const;
    GLint GetUniformLocation(const std::string& name) const;

    void Cleanup();

private:
    // Collection of compiled shaders
    std::unordered_map<std::string, unsigned int> m_shaderPrograms;

    // Currently active program
    unsigned int m_currentProgram = 0;
    std::string m_currentShaderName;

    // Shader compilation
    bool CompileShader(unsigned int shader, const char* source);
    bool LinkProgram(unsigned int program);

    // Helper for getting uniform location with error checking
    GLint GetUniformLocationSafe(const std::string& name) const;
};

} // namespace Blacksite
