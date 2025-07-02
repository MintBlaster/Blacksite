#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <memory>
#include "ShaderManager.h"
#include "shaders/ShaderLibrary.h"

namespace Blacksite {

struct ShaderFileInfo {
    std::string name;
    std::string vertexPath;
    std::string fragmentPath;
    std::filesystem::file_time_type lastModified;
    bool isBuiltIn = false;
};

class ShaderSystem {
public:
    ShaderSystem();
    ~ShaderSystem();

    // Core lifecycle - called by main engine
    bool Initialize(const std::string& shaderDirectory = "assets/shaders/");
    void Shutdown();
    void Update(); // Call this each frame for hot-reloading

    // Shader management
    bool LoadBuiltInShaders();
    bool LoadUserShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);
    bool ReloadShader(const std::string& name);
    bool HasShader(const std::string& name) const;

    // Directory scanning for user shaders
    void ScanShaderDirectory();
    void EnableHotReloading(bool enable) { m_hotReloadEnabled = enable; }

    // Access to shader manager
    ShaderManager& GetShaderManager() { return m_shaderManager; }
    const ShaderManager& GetShaderManager() const { return m_shaderManager; }

    // Utility
    std::vector<std::string> GetAvailableShaders() const;
    std::vector<std::string> GetBuiltInShaders() const;
    std::vector<std::string> GetUserShaders() const;

    // Shader info
    bool IsBuiltInShader(const std::string& name) const;
    std::string GetShaderDescription(const std::string& name) const;

    bool GetHotReloadEnabled() { return m_hotReloadEnabled; }

private:
    ShaderManager m_shaderManager;
    std::string m_shaderDirectory;

    // Tracking
    std::unordered_map<std::string, ShaderFileInfo> m_shaderFiles;
    std::vector<std::string> m_builtInShaders;

    // Hot reloading
    bool m_hotReloadEnabled = true;

    // Helper methods
    bool LoadShaderFromFile(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);
    std::string ReadShaderFile(const std::string& path);
    bool IsShaderFile(const std::string& extension);
    std::filesystem::file_time_type GetFileModificationTime(const std::string& path);
    void CheckForShaderChanges();

    // Shader file naming conventions
    std::pair<std::string, std::string> FindShaderPair(const std::string& baseName);

    bool HasSuffix(const std::string& str, const std::string& suffix) const;
};

} // namespace Blacksite
