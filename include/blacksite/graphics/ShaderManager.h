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

    // Load and compile shader programs
    // If this return false -> check shader syntax
    bool LoadShader(const std::string& name, const char* vertexSource, const char* fragmentSource);
    bool UseShader(const std::string& name);

    // Uniform setters
    void SetUniform(const std::string& name, const glm::mat4& matrix);
    void SetUniform(const std::string& name, const glm::vec3& vector);
    void SetUniform(const std::string& name, float value);

    void Cleanup();

  private:
    // Collection of compiled shaders
    std::unordered_map<std::string, unsigned int> m_shaderPrograms;

    // Currently active program
    unsigned int m_currentProgram = 0;

    // Shader compilation
    bool CompileShader(unsigned int shader, const char* source);
    bool LinkProgram(unsigned int program);
};
}  // namespace Blacksite