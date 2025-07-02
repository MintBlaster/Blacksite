#pragma once
#include <functional>
#include <memory>
#include "blacksite/core/InputSystem.h"
#include "blacksite/core/Window.h"
#include "blacksite/graphics/Renderer.h"
#include "blacksite/physics/PhysicsSystem.h"
#include "blacksite/scene/SceneSystem.h"

namespace Blacksite {

class Engine {
  public:
    Engine();
    ~Engine();

    // --- Core Engine Lifecycle ---
    bool Initialize(int width = 1280, int height = 720, const std::string& title = "Blacksite Engine");
    int Run();
    void Shutdown();
    bool IsRunning() const { return m_running; }

    // --- Game Code Interface ---
    using UpdateCallback = std::function<void(Engine& engine, float deltaTime)>;
    void SetUpdateCallback(UpdateCallback callback);

    // --- Scene Management ---
    SceneSystem* GetSceneSystem() { return m_sceneSystem.get(); }

    // --- Convenience Scene API (delegates to SceneSystem) ---
    template <typename T = Scene>
    std::shared_ptr<T> CreateScene(const std::string& name) {
        return m_sceneSystem ? m_sceneSystem->CreateScene<T>(name) : nullptr;
    }

    bool SwitchToScene(const std::string& name) { return m_sceneSystem ? m_sceneSystem->SwitchToScene(name) : false; }

    Scene* GetActiveScene() { return m_sceneSystem ? m_sceneSystem->GetActiveScene() : nullptr; }

    Scene* GetScene(const std::string& name) { return m_sceneSystem ? m_sceneSystem->GetScene(name) : nullptr; }

    // --- Convenience Entity API (delegates to active scene via SceneSystem) ---
    int SpawnCube(const glm::vec3& position);
    int SpawnSphere(const glm::vec3& position);
    int SpawnPlane(const glm::vec3& position, const glm::vec3& size);

    // Shader-specific methods
    int SpawnCube(const glm::vec3& position, const std::string& shader, const glm::vec3& color = glm::vec3(1.0f));
    int SpawnSphere(const glm::vec3& position, const std::string& shader, const glm::vec3& color = glm::vec3(1.0f));
    int SpawnPlane(const glm::vec3& position, const glm::vec3& size, const std::string& shader,
                   const glm::vec3& color = glm::vec3(1.0f));

    // Generic spawn method
    int SpawnEntity(Entity::VisualShape shape, const glm::vec3& position, const std::string& shader = "basic",
                    const glm::vec3& color = glm::vec3(1.0f));

    EntityHandle GetEntity(int id);

    // --- Convenience Camera API (delegates to active scene via SceneSystem) ---
    void SetCameraPosition(const glm::vec3& position);
    void SetCameraTarget(const glm::vec3& target);
    glm::vec3 GetCameraPosition() const;
    glm::vec3 GetCameraTarget() const;

    // Single frame update/render for external main loops.
    void UpdateFrame(float deltaTime);
    void RenderFrame();

    // --- Core System Access ---
    InputSystem* GetInputSystem() { return m_inputSystem.get(); }
    PhysicsSystem* GetPhysicsSystem() { return m_physicsSystem.get(); }
    Renderer* GetRenderer() { return m_renderer.get(); }
    GLFWwindow* GetGLFWWindow() const { return m_window->GetGLFWindow(); }

  private:
    // --- Core Systems ---
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<PhysicsSystem> m_physicsSystem;
    std::unique_ptr<InputSystem> m_inputSystem;
    std::unique_ptr<SceneSystem> m_sceneSystem;

    // --- Engine State ---
    UpdateCallback m_updateCallback;
    bool m_running = false;
    bool m_initialized = false;

    // --- Internal Methods ---
    void Update(float deltaTime);
    void Render();
    void HandleInput();
};

}  // namespace Blacksite
