#pragma once
#include "blacksite/core/Logger.h"
#include "blacksite/scene/Scene.h"
#include <memory>
#include <unordered_map>
#include <string>
#include <functional>

namespace Blacksite {

class PhysicsSystem;
class Renderer;

class SceneSystem {
public:
    SceneSystem();
    ~SceneSystem();

    // --- System Lifecycle ---
    bool Initialize(PhysicsSystem* physicsSystem, Renderer* renderer, float aspectRatio = 16.0f / 9.0f);
    void Update(float deltaTime);
    void Render();
    void Shutdown();

    // --- Scene Management ---
    template<typename T = Scene>
    std::shared_ptr<T> CreateScene(const std::string& name);

    bool AddScene(const std::string& name, std::shared_ptr<Scene> scene);
    bool RemoveScene(const std::string& name);
    bool LoadScene(const std::string& name);
    bool UnloadScene(const std::string& name);
    void UnloadAllScenes();

    // --- Scene Switching ---
    bool SwitchToScene(const std::string& name);
    Scene* GetActiveScene() { return m_activeScene; }
    Scene* GetScene(const std::string& name);

    // --- Scene Queries ---
    bool HasScene(const std::string& name) const;
    std::vector<std::string> GetSceneNames() const;
    size_t GetSceneCount() const { return m_scenes.size(); }

    // --- Transition Management ---
    using TransitionCallback = std::function<void(Scene* from, Scene* to)>;
    void SetTransitionCallback(TransitionCallback callback) { m_transitionCallback = callback; }

    // --- Convenience API (delegates to active scene) ---
    int SpawnCube(const glm::vec3& position);
    int SpawnSphere(const glm::vec3& position);
    int SpawnPlane(const glm::vec3& position, const glm::vec3& size);
    EntityHandle GetEntity(int id);
    void SetCameraPosition(const glm::vec3& position);
    void SetCameraTarget(const glm::vec3& target);

private:
    // --- Scene Storage ---
    std::unordered_map<std::string, std::shared_ptr<Scene>> m_scenes;
    Scene* m_activeScene = nullptr;
    std::string m_activeSceneName;

    // --- System References ---
    PhysicsSystem* m_physicsSystem = nullptr;
    Renderer* m_renderer = nullptr;
    float m_aspectRatio = 16.0f / 9.0f;

    // --- Callbacks ---
    TransitionCallback m_transitionCallback;

    bool m_initialized = false;
};

// Template implementation
template<typename T>
std::shared_ptr<T> SceneSystem::CreateScene(const std::string& name) {
    static_assert(std::is_base_of_v<Scene, T>, "T must derive from Scene");

    if (HasScene(name)) {
        BS_ERROR_F(LogCategory::CORE, "Scene '%s' already exists!", name.c_str());
        return nullptr;
    }

    auto scene = std::make_shared<T>(name);
    if (m_initialized && !scene->Initialize(m_physicsSystem, m_aspectRatio)) {
        BS_ERROR_F(LogCategory::CORE, "Failed to initialize scene '%s'", name.c_str());
        return nullptr;
    }

    m_scenes[name] = scene;
    BS_INFO_F(LogCategory::CORE, "Scene '%s' created and added to SceneSystem", name.c_str());
    return scene;
}

} // namespace Blacksite
