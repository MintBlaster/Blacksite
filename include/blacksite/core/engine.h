#pragma once

#include "blacksite/core/window.h"
#include "blacksite/graphics/renderer.h"

#include <memory>
#include <vector>

namespace Blacksite {
    // --- Entity System ---
    struct Entity
    {
        Transform transform;

        enum Shape
        {
            CUBE,
            SPHERE,
            PLANE
        } shape;

        float r = 1.0f, g = 1.0f, b = 1.0f;  // Basic per-entity color
        bool active = true;

        Entity(Shape s)
            : shape(s)
        {}
    };

    class Engine
    {
      public:
        Engine();
        ~Engine();

        // --- Core Engine Lifecycle ---
        bool Initialize(int width = 1280, int height = 720, const std::string& title = "Blacksite Engine");
        int Run();
        void Shutdown();
        bool IsRunning() const { return m_running; }

        // --- Entity Spawning ---
        int SpawnCube(const glm::vec3& position = {0, 0, 0});
        int SpawnSphere(const glm::vec3& position = {0, 0, 0});
        int SpawnPlane(const glm::vec3& position = {0, 0, 0});
        // Returns an integer ID for the spawned entity, used as a handle

        // --- Entity Manipulation ---
        class EntityHandle
        {
          public:
            EntityHandle(Engine* engine, int id)
                : m_engine(engine),
                  m_id(id)
            {}

            // Transform
            EntityHandle& At(const glm::vec3& position);
            EntityHandle& Rotate(const glm::vec3& rotation);
            EntityHandle& Scale(const glm::vec3& scale);
            EntityHandle& Scale(float uniformScale);

            // Appearance
            EntityHandle& Color(float r, float g, float b);
            EntityHandle& Color(const glm::vec3& color);

            // State
            EntityHandle& SetActive(bool active);
            void Destroy();  // Marks entity inactive—does not remove from memory or list

            int GetId() const { return m_id; }

          private:
            Engine* m_engine;  // Reference to owning engine; assumed to outlive handle
            int m_id;
        };

        EntityHandle GetEntity(int id);  // Returns a wrapper to manipulate the entity safely

        // --- Camera Controls ---
        void SetCameraPosition(const glm::vec3& position);
        void SetCameraLookAt(const glm::vec3& target);  // Sets view target; assumes LookAt matrix usage

      private:
        // --- Core Systems ---
        std::unique_ptr<Window> m_window;
        std::unique_ptr<Renderer> m_renderer;

        // --- Entity Storage ---
        std::vector<Entity> m_entities;
        int m_nextEntityId = 0;  // Increment-only, IDs are never reused

        // --- Engine State ---
        bool m_running = false;
        bool m_initialized = false;

        // --- Internal Methods ---
        void Update(float deltaTime);  // Main simulation step
        void Render();  // Pushes current entity state to renderer

        // --- Entity Management ---
        bool IsValidEntity(int id) const;  // Bounds + state check before access
        Entity* GetEntityPtr(int id);  // Raw pointer access; no safety net
    };

}  // namespace Blacksite
