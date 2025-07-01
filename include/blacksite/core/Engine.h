#pragma once

#include "blacksite/core/Window.h"
#include "blacksite/graphics/Renderer.h"
#include "blacksite/math/Transform.h"
#include "blacksite/physics/PhysicsSystem.h"
#include "blacksite/ui/UI.h"



#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace Blacksite {

/**
 * Entity - Physics-first!
 */
struct Entity {
    Transform transform;
    JPH::BodyID physicsBody;
    bool hasPhysics = true;  // Always has physics by default!

    enum Shape { CUBE, SPHERE, PLANE } shape;

    glm::vec3 color{1.0f, 1.0f, 1.0f};
    bool active = true;

    Entity(Shape s) : shape(s) {}
};

class Engine {
  public:
    Engine();
    ~Engine();

    // --- Core Engine Lifecycle ---
    bool Initialize(int width = 1280, int height = 720, const std::string& title = "Blacksite Engine");
    int Run();        // The main loop
    void Shutdown();  // Clean up your mess
    bool IsRunning() const { return m_running; }

    // --- Entity Spawning (physics by default!) ---
    int SpawnCube(const glm::vec3& position = {0, 0, 0});
    int SpawnSphere(const glm::vec3& position = {0, 0, 0});
    int SpawnPlane(const glm::vec3& position = {0, 0, 0}, const glm::vec3& size = {10.0f, 0.2f, 10.0f});

    // --- Game Code Interface ---
    using UpdateCallback = std::function<void(Engine& engine, float deltaTime)>;
    void SetUpdateCallback(UpdateCallback callback);

    // --- Getters ---
    const std::vector<Entity>& GetEntities() const { return m_entities; }
    std::vector<Entity>& GetEntities() { return m_entities; }  // Non-const version for editor

    // --- Editor Interface ---
    Entity* GetEntityPtr(int id);      // Made public for editor access
    bool IsValidEntity(int id) const;  // Made public for editor access
    int GetNextEntityId() const { return m_nextEntityId; }

    // Editor-specific entity operations
    void RemoveEntity(int id);
    void DuplicateEntity(int id);
    void SetEntityName(int id, const std::string& name);
    std::string GetEntityName(int id) const;

    class EntityHandle {
      public:
        EntityHandle(Engine* engine, int id) : m_engine(engine), m_id(id) {}

        // Transform manipulation (syncs with physics automatically)
        EntityHandle& At(const glm::vec3& position);
        EntityHandle& Rotate(const glm::vec3& rotation);
        EntityHandle& Scale(const glm::vec3& scale);
        EntityHandle& Scale(float x, float y, float z);
        EntityHandle& Scale(float uniformScale);

        // Physics operations (work immediately - everything has physics!)
        EntityHandle& Push(const glm::vec3& force);       // Better name than AddForce
        EntityHandle& Impulse(const glm::vec3& impulse);  // Better name than AddImpulse
        EntityHandle& SetVelocity(const glm::vec3& velocity);
        EntityHandle& SetAngularVelocity(const glm::vec3& angularVel);
        EntityHandle& MakeStatic();   // Only way to disable dynamics
        EntityHandle& MakeDynamic();  // Re-enable dynamics

        // Appearance
        EntityHandle& Color(float r, float g, float b);
        EntityHandle& Color(const glm::vec3& color);

        // State management
        EntityHandle& SetActive(bool active);
        void Destroy();
        int GetId() const { return m_id; }

      private:
        Engine* m_engine;
        int m_id;
    };

    EntityHandle GetEntity(int id);

    // --- Camera Controls ---
    void SetCameraPosition(const glm::vec3& position);
    void SetCameraTarget(const glm::vec3& target);
    glm::vec3 GetCameraPosition() const;
    glm::vec3 GetCameraTarget() const;

    // --- Direct Physics Access ---
    PhysicsSystem* GetPhysicsSystem() { return m_physicsSystem.get(); }

    // --- System Access ---
    Renderer* GetRenderer() { return m_renderer.get(); }
    UI::UISystem* GetUI() { return m_uiSystem.get(); }

  private:
    // --- Core Systems ---
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<PhysicsSystem> m_physicsSystem;
    std::unique_ptr<UI::UISystem> m_uiSystem;
    UpdateCallback m_updateCallback;

    // --- Entity Storage ---
    std::vector<Entity> m_entities;
    std::vector<std::string> m_entityNames;
    int m_nextEntityId = 0;

    // --- Engine State ---
    bool m_running = false;
    bool m_initialized = false;

    // --- Internal Methods ---
    void Update(float deltaTime);
    void Render();
    void RenderUI();
    void SyncPhysicsToGraphics();

    // --- Input Management ---
    void HandleInput();
    bool m_f1KeyPressed = false;
};

}  // namespace Blacksite