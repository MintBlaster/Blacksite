#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include <blacksite/core/Logger.h>
#include <blacksite/graphics/ShaderManager.h>
#include <GL/glext.h>
#include "blacksite/graphics/shaders/ShaderLibrary.h"

namespace Blacksite {

ShaderManager::~ShaderManager() {
    Cleanup();
}

bool ShaderManager::LoadShaderFromLibrary(const std::string& libraryName) {
    const ShaderSource* shaderSource = ShaderLibrary::GetShader(libraryName);
    if (!shaderSource) {
        BS_ERROR_F(LogCategory::RENDERER, "Shader '%s' not found in library", libraryName.c_str());
        return false;
    }

    bool success = LoadShader(libraryName, shaderSource->vertexSource, shaderSource->fragmentSource);
    if (success) {
        BS_DEBUG_F(LogCategory::RENDERER, "Loaded shader '%s' from library: %s",
                   libraryName.c_str(), shaderSource->description.c_str());
    }

    return success;
}

void ShaderManager::SetUniform(const std::string& name, int value) {
    if (m_currentProgram == 0) return;

    int location = glGetUniformLocation(m_currentProgram, name.c_str());
    if (location != -1) {
        glUniform1i(location, value);
    }
}

void ShaderManager::SetUniform(const std::string& name, bool value) {
    SetUniform(name, static_cast<int>(value));
}

bool ShaderManager::HasShader(const std::string& name) const {
    return m_shaderPrograms.find(name) != m_shaderPrograms.end();
}

std::string ShaderManager::GetCurrentShaderName() const {
    return m_currentShaderName;
}

// Update the UseShader method to track current shader name:
bool ShaderManager::UseShader(const std::string& name) {
    auto it = m_shaderPrograms.find(name);
    if (it != m_shaderPrograms.end()) {
        m_currentProgram = it->second;
        m_currentShaderName = name;
        glUseProgram(m_currentProgram);
        return true;
    } else {
        BS_ERROR_F(LogCategory::RENDERER, "Shader not found: %s (did you forget to load it?)", name.c_str());
        return false;
    }
}


bool ShaderManager::LoadShader(const std::string& name, const char* vertexSource, const char* fragmentSource) {
    // Create vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    if (!CompileShader(vertexShader, vertexSource)) {
        glDeleteShader(vertexShader);
        BS_ERROR_F(LogCategory::RENDERER, "Vertex Shader compilation failed for: %s", name.c_str());
        return false;
    }

    // Create fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    if (!CompileShader(fragmentShader, fragmentSource)) {
        glDeleteShader(fragmentShader);
        BS_ERROR_F(LogCategory::RENDERER, "Fragment Shader compilation failed for: %s", name.c_str());
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
        BS_ERROR_F(LogCategory::RENDERER, "Shader program linking failed for: %s", name.c_str());
    }

    // Clean up individual shader - we don't need them anymore
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    m_shaderPrograms[name] = program;

    BS_DEBUG_F(LogCategory::RENDERER, "Shader '%s' loaded successfully", name.c_str());
    return true;
}

void ShaderManager::SetUniform(const std::string& name, const glm::mat4& matrix) {
    if (m_currentProgram == 0) {
        BS_ERROR(LogCategory::RENDERER, "No shader currently active - call UseShader() first!");
        return;
    }

    int location = glGetUniformLocation(m_currentProgram, name.c_str());
    if (location != -1) {
        // Upload the matrix to the GPU
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
    } else {
        // Uniform not found
        BS_WARN_F(LogCategory::RENDERER, "Uniform '%s' not found in current shader", name.c_str());
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

    BS_DEBUG(LogCategory::RENDERER, "ShaderManager cleaned up");
}

bool ShaderManager::CompileShader(unsigned int shader, const char* source) {
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        BS_ERROR_F(LogCategory::RENDERER, "Shader compilation failed: %s", infoLog);
        BS_ERROR(LogCategory::RENDERER, "Check your GLSL syntax, semicolons, and uniform names");
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
        BS_ERROR_F(LogCategory::RENDERER, "Shader program linking failed: %s", infoLog);
        BS_ERROR(LogCategory::RENDERER, "Usually means vertex/fragment outputs don't match up");
        return false;
    }
    return true;
}

}  // namespace Blacksite
