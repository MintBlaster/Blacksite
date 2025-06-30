#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include <blacksite/graphics/ShaderManager.h>
#include <GL/glext.h>

namespace Blacksite {

ShaderManager::~ShaderManager() {
    Cleanup();
}

bool ShaderManager::LoadShader(const std::string& name, const char* vertexSource, const char* fragmentSource) {
    // Create vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    if (!CompileShader(vertexShader, vertexSource)) {
        glDeleteShader(vertexShader);
        std::cerr << "Vertex Shader compilation failed for : " << name << std::endl;
        return false;
    }

    // Create fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    if (!CompileShader(fragmentShader, fragmentSource)) {
        glDeleteShader(fragmentShader);
        std::cerr << "Fragment Shader compilation failed for : " << name << std::endl;
        return false;
    }

    // Create program and link
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    if (!LinkProgram(program)) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(program);
        std::cerr << "Shader program linking failed for : " << name << std::endl;
    }

    // Clean up individual shader - we don't need them anymore
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    m_shaderPrograms[name] = program;

    std::cout << "Shader " << name << " loaded successfully." << std::endl;
    return true;
}

bool ShaderManager::UseShader(const std::string& name) {
    auto it = m_shaderPrograms.find(name);
    if (it != m_shaderPrograms.end()) {
        m_currentProgram = it->second;
        glUseProgram(m_currentProgram);
        return true;
    } else {
        std::cerr << "Shader not found : " << name << " (did you forgot to load it?)" << std::endl;
        return false;
    }
}

void ShaderManager::SetUniform(const std::string& name, const glm::mat4& matrix) {
    if (m_currentProgram == 0) {
        std::cerr << "No shader currently active - call UseShader() first!" << std::endl;
        return;
    }

    int location = glGetUniformLocation(m_currentProgram, name.c_str());
    if (location != -1) {
        // Upload the matrix to the GPU
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
    } else {
        // Uniform not found
        std::cerr << "Uniform '" << name << "' not found in current shader" << std::endl;
    }
}

void ShaderManager::SetUniform(const std::string& name, const glm::vec3& vector) {
    if (m_currentProgram == 0)
        return;

    int location = glGetUniformLocation(m_currentProgram, name.c_str());
    if (location != -1) {
        glUniform3fv(location, 1, glm::value_ptr(vector));
    }
}

void ShaderManager::SetUniform(const std::string& name, float value) {
    if (m_currentProgram == 0)
        return;

    int location = glGetUniformLocation(m_currentProgram, name.c_str());
    if (location != -1) {
        glUniform1f(location, value);
    }
}

void ShaderManager::Cleanup() {
    // Delete all our shader programs.
    for (auto& pair : m_shaderPrograms) {
        glDeleteProgram(pair.second);
    }

    m_shaderPrograms.clear();
    m_currentProgram = 0;

    std::cout << "ShaderManager cleaned up." << std::endl;
}

bool ShaderManager::CompileShader(unsigned int shader, const char* source) {
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed : " << infoLog << std::endl;
        std::cerr << "Pro tip: Check your GLSL syntax, semicolons, and uniform names" << std::endl;
        return false;
    }

    return true;
}

bool ShaderManager::LinkProgram(unsigned int program) {
    glLinkProgram(program);

    // Check if linking succeeded
    int success;
    char infoLog[512];

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        std::cerr << "Usually means vertex/fragment outputs don't match up" << std::endl;
        return false;
    }
    return true;
}

}  // namespace Blacksite
