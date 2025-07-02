#include "blacksite/graphics/ShaderSystem.h"
#include "blacksite/core/Logger.h"
#include <fstream>
#include <sstream>

namespace Blacksite {

ShaderSystem::ShaderSystem() = default;

ShaderSystem::~ShaderSystem() {
    Shutdown();
}

bool ShaderSystem::Initialize(const std::string& shaderDirectory) {
    m_shaderDirectory = shaderDirectory;

    BS_INFO(LogCategory::RENDERER, "Initializing Shader System...");

    // Load built-in shaders first
    if (!LoadBuiltInShaders()) {
        BS_ERROR(LogCategory::RENDERER, "Failed to load built-in shaders!");
        return false;
    }

    // Create shader directory if it doesn't exist
    if (!std::filesystem::exists(m_shaderDirectory)) {
        std::filesystem::create_directories(m_shaderDirectory);
        BS_INFO_F(LogCategory::RENDERER, "Created shader directory: %s", m_shaderDirectory.c_str());
    }

    // Scan for user shaders
    ScanShaderDirectory();

    BS_INFO_F(LogCategory::RENDERER, "Shader System initialized with %zu total shaders",
              m_shaderFiles.size());

    return true;
}

void ShaderSystem::Shutdown() {
    m_shaderManager.Cleanup();
    m_shaderFiles.clear();
    m_builtInShaders.clear();

    BS_INFO(LogCategory::RENDERER, "Shader System shut down");
}

void ShaderSystem::Update() {
    if (m_hotReloadEnabled) {
        CheckForShaderChanges();
    }
}

bool ShaderSystem::LoadBuiltInShaders() {
    bool success = true;

    // Get all available shaders from the library
    auto availableShaders = ShaderLibrary::GetAvailableShaderNames();

    for (const auto& shaderName : availableShaders) {
        if (m_shaderManager.LoadShaderFromLibrary(shaderName)) {
            m_builtInShaders.push_back(shaderName);

            // Add to tracking (built-in shaders don't have file paths)
            ShaderFileInfo info;
            info.name = shaderName;
            info.isBuiltIn = true;
            m_shaderFiles[shaderName] = info;

            BS_DEBUG_F(LogCategory::RENDERER, "Loaded built-in shader: %s", shaderName.c_str());
        } else {
            BS_ERROR_F(LogCategory::RENDERER, "Failed to load built-in shader: %s", shaderName.c_str());
            success = false;
        }
    }

    BS_INFO_F(LogCategory::RENDERER, "Loaded %zu built-in shaders", m_builtInShaders.size());
    return success;
}

bool ShaderSystem::LoadUserShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) {
    if (IsBuiltInShader(name)) {
        BS_WARN_F(LogCategory::RENDERER, "Cannot override built-in shader: %s", name.c_str());
        return false;
    }

    return LoadShaderFromFile(name, vertexPath, fragmentPath);
}

bool ShaderSystem::ReloadShader(const std::string& name) {
    auto it = m_shaderFiles.find(name);
    if (it == m_shaderFiles.end()) {
        BS_ERROR_F(LogCategory::RENDERER, "Cannot reload unknown shader: %s", name.c_str());
        return false;
    }

    if (it->second.isBuiltIn) {
        // Reload from library
        return m_shaderManager.LoadShaderFromLibrary(name);
    } else {
        // Reload from file
        return LoadShaderFromFile(name, it->second.vertexPath, it->second.fragmentPath);
    }
}

bool ShaderSystem::HasShader(const std::string& name) const {
    return m_shaderFiles.find(name) != m_shaderFiles.end();
}

// Replace the problematic section in ScanShaderDirectory() with this:

void ShaderSystem::ScanShaderDirectory() {
    if (!std::filesystem::exists(m_shaderDirectory)) {
        return;
    }

    BS_DEBUG_F(LogCategory::RENDERER, "Scanning shader directory: %s", m_shaderDirectory.c_str());

    std::unordered_map<std::string, std::vector<std::string>> shaderGroups;

    // Group files by base name
    for (const auto& entry : std::filesystem::recursive_directory_iterator(m_shaderDirectory)) {
        if (!entry.is_regular_file()) continue;

        std::string extension = entry.path().extension().string();
        if (!IsShaderFile(extension)) continue;

        std::string baseName = entry.path().stem().string();

        // Remove common shader suffixes to group vertex/fragment pairs (C++17 compatible)
        if (HasSuffix(baseName, "_vert") || HasSuffix(baseName, ".vert")) {
            size_t pos = baseName.find_last_of("_.");
            if (pos != std::string::npos) {
                baseName = baseName.substr(0, pos);
            }
        } else if (HasSuffix(baseName, "_frag") || HasSuffix(baseName, ".frag")) {
            size_t pos = baseName.find_last_of("_.");
            if (pos != std::string::npos) {
                baseName = baseName.substr(0, pos);
            }
        }

        shaderGroups[baseName].push_back(entry.path().string());
    }

    // Load shader pairs
    for (const auto& [baseName, files] : shaderGroups) {
        auto [vertexPath, fragmentPath] = FindShaderPair(baseName);

        if (!vertexPath.empty() && !fragmentPath.empty()) {
            LoadUserShader(baseName, vertexPath, fragmentPath);
        }
    }
}


std::vector<std::string> ShaderSystem::GetAvailableShaders() const {
    std::vector<std::string> shaders;
    shaders.reserve(m_shaderFiles.size());

    for (const auto& [name, info] : m_shaderFiles) {
        shaders.push_back(name);
    }

    return shaders;
}

std::vector<std::string> ShaderSystem::GetBuiltInShaders() const {
    return m_builtInShaders;
}

std::vector<std::string> ShaderSystem::GetUserShaders() const {
    std::vector<std::string> userShaders;

    for (const auto& [name, info] : m_shaderFiles) {
        if (!info.isBuiltIn) {
            userShaders.push_back(name);
        }
    }

    return userShaders;
}

bool ShaderSystem::IsBuiltInShader(const std::string& name) const {
    auto it = m_shaderFiles.find(name);
    return it != m_shaderFiles.end() && it->second.isBuiltIn;
}

std::string ShaderSystem::GetShaderDescription(const std::string& name) const {
    if (IsBuiltInShader(name)) {
        return ShaderLibrary::GetShaderDescription(name);
    }
    return "User-defined shader";
}

bool ShaderSystem::LoadShaderFromFile(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexSource = ReadShaderFile(vertexPath);
    std::string fragmentSource = ReadShaderFile(fragmentPath);

    if (vertexSource.empty() || fragmentSource.empty()) {
        BS_ERROR_F(LogCategory::RENDERER, "Failed to read shader files for: %s", name.c_str());
        return false;
    }

    bool success = m_shaderManager.LoadShader(name, vertexSource.c_str(), fragmentSource.c_str());

    if (success) {
        // Update tracking info
        ShaderFileInfo info;
        info.name = name;
        info.vertexPath = vertexPath;
        info.fragmentPath = fragmentPath;
        info.lastModified = std::max(GetFileModificationTime(vertexPath),
                                   GetFileModificationTime(fragmentPath));
        info.isBuiltIn = false;

        m_shaderFiles[name] = info;

        BS_INFO_F(LogCategory::RENDERER, "Loaded user shader: %s", name.c_str());
    }

    return success;
}

std::string ShaderSystem::ReadShaderFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        BS_ERROR_F(LogCategory::RENDERER, "Cannot open shader file: %s", path.c_str());
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool ShaderSystem::IsShaderFile(const std::string& extension) {
    return extension == ".glsl" || extension == ".vert" || extension == ".frag" ||
           extension == ".vs" || extension == ".fs" || extension == ".hlsl";
}

std::filesystem::file_time_type ShaderSystem::GetFileModificationTime(const std::string& path) {
    try {
        return std::filesystem::last_write_time(path);
    } catch (const std::exception&) {
        return std::filesystem::file_time_type{};
    }
}

void ShaderSystem::CheckForShaderChanges() {
    for (auto& [name, info] : m_shaderFiles) {
        if (info.isBuiltIn) continue; // Skip built-in shaders

        auto vertexTime = GetFileModificationTime(info.vertexPath);
        auto fragmentTime = GetFileModificationTime(info.fragmentPath);
        auto latestTime = std::max(vertexTime, fragmentTime);

        if (latestTime > info.lastModified) {
            BS_INFO_F(LogCategory::RENDERER, "Detected changes in shader: %s, reloading...", name.c_str());

            if (ReloadShader(name)) {
                info.lastModified = latestTime;
                BS_INFO_F(LogCategory::RENDERER, "Successfully reloaded shader: %s", name.c_str());
            } else {
                BS_ERROR_F(LogCategory::RENDERER, "Failed to reload shader: %s", name.c_str());
            }
        }
    }
}

std::pair<std::string, std::string> ShaderSystem::FindShaderPair(const std::string& baseName) {
    std::string vertexPath, fragmentPath;

    // Common naming patterns
    std::vector<std::string> vertexSuffixes = {"_vert", ".vert", "_vs", ".vs"};
    std::vector<std::string> fragmentSuffixes = {"_frag", ".frag", "_fs", ".fs"};
    std::vector<std::string> extensions = {".glsl", ".hlsl", ""};

    // Try to find vertex shader
    for (const auto& suffix : vertexSuffixes) {
        for (const auto& ext : extensions) {
            std::string path = m_shaderDirectory + "/" + baseName + suffix + ext;
            if (std::filesystem::exists(path)) {
                vertexPath = path;
                break;
            }
        }
        if (!vertexPath.empty()) break;
    }

    // Try to find fragment shader
    for (const auto& suffix : fragmentSuffixes) {
        for (const auto& ext : extensions) {
            std::string path = m_shaderDirectory + "/" + baseName + suffix + ext;
            if (std::filesystem::exists(path)) {
                fragmentPath = path;
                break;
            }
        }
        if (!fragmentPath.empty()) break;
    }

    return {vertexPath, fragmentPath};
}

bool ShaderSystem::HasSuffix(const std::string& str, const std::string& suffix) const {
    if (suffix.length() > str.length()) {
        return false;
    }
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

} // namespace Blacksite
