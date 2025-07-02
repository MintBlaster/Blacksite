#include "blacksite/scene/SceneSystem.h"
#include "blacksite/physics/PhysicsSystem.h"
#include "blacksite/graphics/Renderer.h"
#include "blacksite/core/Logger.h"

namespace Blacksite {

SceneSystem::SceneSystem() = default;

SceneSystem::~SceneSystem() {
    Shutdown();
}

bool SceneSystem::Initialize(PhysicsSystem* physicsSystem, Renderer* renderer, float aspectRatio) {
    if (m_initialized) {
        BS_ERROR(LogCategory::CORE, "SceneSystem already initialized!");
        return false;
    }

    if (!physicsSystem || !renderer) {
        BS_ERROR(LogCategory::CORE, "SceneSystem requires valid PhysicsSystem and Renderer!");
        return false;
    }

    m_physicsSystem = physicsSystem;
    m_renderer = renderer;
    m_aspectRatio = aspectRatio;

    // Initialize any existing scenes
    for (auto& [name, scene] : m_scenes) {
        if (!scene->Initialize(m_physicsSystem, m_aspectRatio)) {
            BS_ERROR_F(LogCategory::CORE, "Failed to initialize scene '%s'", name.c_str());
            return false;
        }
    }

    m_initialized = true;
    BS_INFO(LogCategory::CORE, "SceneSystem initialized successfully");
    return true;
}

void SceneSystem::Update(float deltaTime) {
    if (!m_initialized) return;

    if (m_activeScene) {
        m_activeScene->Update(deltaTime);
    }
}

void SceneSystem::Render() {
    if (!m_initialized || !m_renderer) return;

    m_renderer->BeginFrame();

    if (m_activeScene) {
        m_activeScene->Render(m_renderer);
    }

    m_renderer->EndFrame();
}

void SceneSystem::Shutdown() {
    if (!m_initialized) return;

    BS_INFO(LogCategory::CORE, "Shutting down SceneSystem...");

    UnloadAllScenes();
    m_scenes.clear();

    m_physicsSystem = nullptr;
    m_renderer = nullptr;
    m_activeScene = nullptr;
    m_activeSceneName.clear();

    m_initialized = false;
    BS_INFO(LogCategory::CORE, "SceneSystem shutdown complete");
}

bool SceneSystem::AddScene(const std::string& name, std::shared_ptr<Scene> scene) {
    if (HasScene(name)) {
        BS_ERROR_F(LogCategory::CORE, "Scene '%s' already exists!", name.c_str());
        return false;
    }

    if (m_initialized && !scene->IsInitialized()) {
        if (!scene->Initialize(m_physicsSystem, m_aspectRatio)) {
            BS_ERROR_F(LogCategory::CORE, "Failed to initialize scene '%s'", name.c_str());
            return false;
        }
    }

    m_scenes[name] = scene;
    BS_INFO_F(LogCategory::CORE, "Scene '%s' added to SceneSystem", name.c_str());
    return true;
}

bool SceneSystem::RemoveScene(const std::string& name) {
    auto it = m_scenes.find(name);
    if (it == m_scenes.end()) {
        BS_ERROR_F(LogCategory::CORE, "Scene '%s' not found!", name.c_str());
        return false;
    }

    // If this is the active scene, deactivate it
    if (m_activeScene == it->second.get()) {
        UnloadScene(name);
    }

    it->second->Shutdown();
    m_scenes.erase(it);
    BS_INFO_F(LogCategory::CORE, "Scene '%s' removed from SceneSystem", name.c_str());
    return true;
}

bool SceneSystem::LoadScene(const std::string& name) {
    auto it = m_scenes.find(name);
    if (it == m_scenes.end()) {
        BS_ERROR_F(LogCategory::CORE, "Scene '%s' not found!", name.c_str());
        return false;
    }

    if (!it->second->IsInitialized()) {
        if (!it->second->Initialize(m_physicsSystem, m_aspectRatio)) {
            BS_ERROR_F(LogCategory::CORE, "Failed to initialize scene '%s'", name.c_str());
            return false;
        }
    }

    it->second->SetActive(true);
    it->second->OnEnter();
    BS_INFO_F(LogCategory::CORE, "Scene '%s' loaded", name.c_str());
    return true;
}

bool SceneSystem::UnloadScene(const std::string& name) {
    auto it = m_scenes.find(name);
    if (it == m_scenes.end()) {
        BS_ERROR_F(LogCategory::CORE, "Scene '%s' not found!", name.c_str());
        return false;
    }

    if (it->second->IsActive()) {
        it->second->OnExit();
        it->second->SetActive(false);

        // If this was the active scene, clear the active scene
        if (m_activeScene == it->second.get()) {
            m_activeScene = nullptr;
            m_activeSceneName.clear();
        }
    }

    BS_INFO_F(LogCategory::CORE, "Scene '%s' unloaded", name.c_str());
    return true;
}

void SceneSystem::UnloadAllScenes() {
    for (auto& [name, scene] : m_scenes) {
        if (scene->IsActive()) {
            scene->OnExit();
            scene->SetActive(false);
        }
    }

    m_activeScene = nullptr;
    m_activeSceneName.clear();
    BS_INFO(LogCategory::CORE, "All scenes unloaded");
}

bool SceneSystem::SwitchToScene(const std::string& name) {
    auto it = m_scenes.find(name);
    if (it == m_scenes.end()) {
        BS_ERROR_F(LogCategory::CORE, "Scene '%s' not found!", name.c_str());
        return false;
    }

    Scene* oldScene = m_activeScene;
    Scene* newScene = it->second.get();

    // Call transition callback if set
    if (m_transitionCallback) {
        m_transitionCallback(oldScene, newScene);
    }

    // Unload current scene
    if (oldScene) {
        oldScene->OnExit();
        oldScene->SetActive(false);
    }

    // Load new scene
    if (!newScene->IsInitialized()) {
        if (!newScene->Initialize(m_physicsSystem, m_aspectRatio)) {
            BS_ERROR_F(LogCategory::CORE, "Failed to initialize scene '%s'", name.c_str());
            return false;
        }
    }

    newScene->SetActive(true);
    newScene->OnEnter();

    m_activeScene = newScene;
    m_activeSceneName = name;

    BS_INFO_F(LogCategory::CORE, "Switched to scene '%s'", name.c_str());
    return true;
}

Scene* SceneSystem::GetScene(const std::string& name) {
    auto it = m_scenes.find(name);
    return (it != m_scenes.end()) ? it->second.get() : nullptr;
}

bool SceneSystem::HasScene(const std::string& name) const {
    return m_scenes.find(name) != m_scenes.end();
}

std::vector<std::string> SceneSystem::GetSceneNames() const {
    std::vector<std::string> names;
    names.reserve(m_scenes.size());
    for (const auto& [name, scene] : m_scenes) {
        names.push_back(name);
    }
    return names;
}

// --- Convenience API Implementation ---
int SceneSystem::SpawnCube(const glm::vec3& position) {
    return m_activeScene ? m_activeScene->SpawnCube(position) : -1;
}

int SceneSystem::SpawnSphere(const glm::vec3& position) {
    return m_activeScene ? m_activeScene->SpawnSphere(position) : -1;
}

int SceneSystem::SpawnPlane(const glm::vec3& position, const glm::vec3& size) {
    return m_activeScene ? m_activeScene->SpawnPlane(position, size) : -1;
}

EntityHandle SceneSystem::GetEntity(int id) {
    return m_activeScene ? m_activeScene->GetEntity(id) : EntityHandle(nullptr, nullptr, -1);
}

void SceneSystem::SetCameraPosition(const glm::vec3& position) {
    if (m_activeScene) m_activeScene->SetCameraPosition(position);
}

void SceneSystem::SetCameraTarget(const glm::vec3& target) {
    if (m_activeScene) m_activeScene->SetCameraTarget(target);
}

} // namespace Blacksite
