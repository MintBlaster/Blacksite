#pragma once

#include "blacksite/core/Window.h"
#include "blacksite/graphics/Renderer.h"
#include "blacksite/math/Transform.h"

#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace Blacksite {

/**
 * Entity
 * Hopefully will gets its own module soon.
 */
struct Entity {
    Transform transform;

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

    // --- Entity Spawning ---
    int SpawnCube(const glm::vec3& position = {0, 0, 0});
    int SpawnSphere(const glm::vec3& position = {0, 0, 0});
    int SpawnPlane(const glm::vec3& position = {0, 0, 0});
    // Returns an integer ID to keep track of it

    // --- Game Code Interface ---
    using UpdateCallback = std::function<void(Engine& engine, float deltaTime)>;
    void SetUpdateCallback(UpdateCallback callback);

    class EntityHandle {
      public:
        EntityHandle(Engine* engine, int id) : m_engine(engine), m_id(id) {}

        // Transform manipulation
        EntityHandle& At(const glm::vec3& position);
        EntityHandle& Rotate(const glm::vec3& rotation);
        EntityHandle& Scale(const glm::vec3& scale);
        EntityHandle& Scale(float x, float y, float z);
        EntityHandle& Scale(float uniformScale);

        // Appearance
        EntityHandle& Color(float r, float g, float b);
        EntityHandle& Color(const glm::vec3& color);

        // State management
        EntityHandle& SetActive(bool active);
        void Destroy();  // Marks entity as dead
        int GetId() const { return m_id; }

      private:
        Engine* m_engine;  // The engine that owns this entity
        int m_id;          // The entity's ID. hopefully it still exists
    };

    EntityHandle GetEntity(int id);  // Get a handle to manipulate an entity

    // --- Camera Controls ---
    void SetCameraPosition(const glm::vec3& position);
    void SetCameraTarget(const glm::vec3& target);

  private:
    // --- Core Systems ---
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    UpdateCallback m_updateCallback;

    // --- Entity Storage ---
    std::vector<Entity> m_entities;
    int m_nextEntityId = 0;

    // --- Engine State ---
    bool m_running = false;
    bool m_initialized = false;

    // --- Internal Methods ---
    void Update(float deltaTime);
    void Render();

    // --- Entity Management ---
    bool IsValidEntity(int id) const;
    Entity* GetEntityPtr(int id);
};

}  // namespace Blacksite