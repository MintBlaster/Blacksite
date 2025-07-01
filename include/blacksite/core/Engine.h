#pragma once

#include "blacksite/core/CameraSystem.h"
#include "blacksite/core/EntityHandle.h"
#include "blacksite/core/EntitySystem.h"
#include "blacksite/core/InputSystem.h"
#include "blacksite/core/Window.h"
#include "blacksite/graphics/Renderer.h"
#include "blacksite/physics/PhysicsSystem.h"

#include <functional>
#include <memory>

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

    // --- Convenience Entity API (delegates to EntitySystem) ---
    int SpawnCube(const glm::vec3& position);
    int SpawnSphere(const glm::vec3& position);
    int SpawnPlane(const glm::vec3& position, const glm::vec3& size);

    // Shader-specific methods
    int SpawnCube(const glm::vec3& position, const std::string& shader, const glm::vec3& color = glm::vec3(1.0f));
    int SpawnSphere(const glm::vec3& position, const std::string& shader, const glm::vec3& color = glm::vec3(1.0f));
    int SpawnPlane(const glm::vec3& position, const glm::vec3& size, const std::string& shader,
                   const glm::vec3& color = glm::vec3(1.0f));

    // Generic spawn method
    int SpawnEntity(Entity::Shape shape, const glm::vec3& position, const std::string& shader = "basic",
                    const glm::vec3& color = glm::vec3(1.0f));

    EntityHandle GetEntity(int id);

    // --- Convenience Camera API (delegates to CameraSystem) ---
    void SetCameraPosition(const glm::vec3& position);
    void SetCameraTarget(const glm::vec3& target);
    glm::vec3 GetCameraPosition() const;
    glm::vec3 GetCameraTarget() const;

    // --- System Access ---
    EntitySystem* GetEntitySystem() { return m_entitySystem.get(); }
    CameraSystem* GetCameraSystem() { return m_cameraSystem.get(); }
    InputSystem* GetInputSystem() { return m_inputSystem.get(); }
    PhysicsSystem* GetPhysicsSystem() { return m_physicsSystem.get(); }
    Renderer* GetRenderer() { return m_renderer.get(); }

  private:
    // --- Core Systems ---
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<PhysicsSystem> m_physicsSystem;
    std::unique_ptr<EntitySystem> m_entitySystem;
    std::unique_ptr<CameraSystem> m_cameraSystem;
    std::unique_ptr<InputSystem> m_inputSystem;

    UpdateCallback m_updateCallback;
    bool m_running = false;
    bool m_initialized = false;

    // --- Internal Methods ---
    void Update(float deltaTime);
    void Render();
    void SyncPhysicsToGraphics();
    void HandleInput();
};

}  // namespace Blacksite
