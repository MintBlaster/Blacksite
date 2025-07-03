#pragma once
#include <functional>
#include <memory>
#include <string>
#include "blacksite/core/CameraSystem.h"
#include "blacksite/core/EntityHandle.h"
#include "blacksite/core/EntitySystem.h"

namespace Blacksite {

class PhysicsSystem;
class Renderer;

class Scene {
  public:
    Scene(const std::string& name = "Untitled Scene");
    ~Scene();

    // --- Scene Lifecycle ---
    virtual bool Initialize(PhysicsSystem* physicsSystem, float aspectRatio = 16.0f / 9.0f);
    virtual void OnEnter() {}  // Called when scene becomes active
    virtual void OnExit() {}   // Called when scene becomes inactive
    virtual void Update(float deltaTime);
    virtual void Render(Renderer* renderer);
    virtual void Shutdown();
    void Clear();

    // --- Scene Properties ---
    const std::string& GetName() const { return m_name; }
    void SetName(const std::string& name) { m_name = name; }
    bool IsActive() const { return m_active; }
    void SetActive(bool active) { m_active = active; }
    bool IsInitialized() const { return m_initialized; }

    // --- Entity Management ---
    int SpawnCube(const glm::vec3& position);
    int SpawnSphere(const glm::vec3& position);
    int SpawnPlane(const glm::vec3& position, const glm::vec3& size);

    template<typename T>
    EntityHandle SpawnEntity() {
        static_assert(std::is_base_of_v<Entity, T>, "T must derive from Entity");
        return m_entitySystem->Spawn<T>();
    }

    // Shader-specific methods
    int SpawnCube(const glm::vec3& position, const std::string& shader, const glm::vec3& color = glm::vec3(1.0f));
    int SpawnSphere(const glm::vec3& position, const std::string& shader, const glm::vec3& color = glm::vec3(1.0f));
    int SpawnPlane(const glm::vec3& position, const glm::vec3& size, const std::string& shader,
                   const glm::vec3& color = glm::vec3(1.0f));

    // Generic spawn method
    int SpawnEntity(Entity::VisualShape shape, const glm::vec3& position, const std::string& shader = "basic",
                    const glm::vec3& color = glm::vec3(1.0f));

    EntityHandle GetEntity(int id);
    void RemoveEntity(int id);

    // --- Camera Management ---
    void SetCameraPosition(const glm::vec3& position);
    void SetCameraTarget(const glm::vec3& target);
    glm::vec3 GetCameraPosition() const;
    glm::vec3 GetCameraTarget() const;
    Camera& GetCamera() { return m_cameraSystem->GetCamera(); }

    // --- System Access ---
    EntitySystem* GetEntitySystem() { return m_entitySystem.get(); }
    CameraSystem* GetCameraSystem() { return m_cameraSystem.get(); }

    // --- Scene Callbacks ---
    using UpdateCallback = std::function<void(Scene& scene, float deltaTime)>;
    using RenderCallback = std::function<void(Scene& scene, Renderer* renderer)>;

    void SetUpdateCallback(UpdateCallback callback) { m_updateCallback = callback; }
    void SetRenderCallback(RenderCallback callback) { m_renderCallback = callback; }

  protected:
    std::string m_name;
    bool m_active = false;
    bool m_initialized = false;

    // --- Scene Systems ---
    std::unique_ptr<EntitySystem> m_entitySystem;
    std::unique_ptr<CameraSystem> m_cameraSystem;

    // --- External System References ---
    PhysicsSystem* m_physicsSystem = nullptr;

    // --- Callbacks ---
    UpdateCallback m_updateCallback;
    RenderCallback m_renderCallback;

  private:
    void SyncPhysicsToGraphics();
    void RenderEntities(Renderer* renderer);
};

}  // namespace Blacksite
