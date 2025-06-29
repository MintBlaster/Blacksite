#include "blacksite/core/engine.h"
#include <GL/glew.h>
#include <chrono>
#include <iostream>

namespace Blacksite {
    Engine::Engine() = default;

    Engine::~Engine()
    {
        Shutdown();
    }

    bool Engine::Initialize(int width, int height, const std::string& title)
    {
        if (m_initialized)
            return true;

        std::cout << "Initializing Blacksite Engine..." << std::endl;

        // --- Initialize Window ---
        m_window = std::make_unique<Window>();
        if (!m_window->Initialize(width, height, title))
        {
            std::cerr << "Failed to initialize window" << std::endl;
            return false;
        }

        // --- Initialize Renderer ---
        m_renderer = std::make_unique<Renderer>();
        if (!m_renderer->Initialize(width, height))
        {
            std::cerr << "Failed to initialize renderer" << std::endl;
            return false;
        }

        // --- Setup default camera ---
        m_renderer->SetCameraPosition({0.0f, 2.0f, 10.0f});
        m_renderer->SetCameraLookAt({0.0f, 0.0f, 0.0f});

        std::cout << "Blacksite Engine initialized successfully" << std::endl;
        m_initialized = true;
        return true;
    }

    int Engine::Run()
    {
        if (!m_initialized)
        {
            std::cerr << "Engine not initialized. Call Initialize() first" << std::endl;
            return -1;
        }

        m_running = true;
        auto lastTime = std::chrono::high_resolution_clock::now();

        std::cout << "Starting main loop..." << std::endl;

        // --- Main game loop ---
        while (m_running && !m_window->ShouldClose())
        {
            // --- Calculate delta time ---
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;

            m_window->PollEvents();
            Update(deltaTime);
            Render();
            m_window->SwapBuffers();
        }

        std::cout << "Main loop ended" << std::endl;
        return 0;
    }

    void Engine::Shutdown()
    {
        if (!m_initialized)
            return;

        std::cout << "Shutting down Blacksite Engine..." << std::endl;

        m_entities.clear();  // Entities are just nuked; no cleanup logic per-entity yet
        m_renderer.reset();
        m_window.reset();

        m_running = false;
        m_initialized = false;
    }

    // --- Entity Spawning ---
    int Engine::SpawnCube(const glm::vec3& position)
    {
        Entity entity(Entity::CUBE);
        entity.transform.position = position;
        m_entities.push_back(entity);
        return m_nextEntityId++;  // IDs are fire-and-forget, never reused
    }

    int Engine::SpawnSphere(const glm::vec3& position)
    {
        Entity entity(Entity::SPHERE);
        entity.transform.position = position;
        m_entities.push_back(entity);
        return m_nextEntityId++;
    }

    int Engine::SpawnPlane(const glm::vec3& position)
    {
        Entity entity(Entity::PLANE);
        entity.transform.position = position;
        m_entities.push_back(entity);
        return m_nextEntityId++;
    }

    Engine::EntityHandle Engine::GetEntity(int id)
    {
        return EntityHandle(this, id);
    }

    // --- EntityHandle Implementation ---
    Engine::EntityHandle& Engine::EntityHandle::At(const glm::vec3& position)
    {
        if (auto* entity = m_engine->GetEntityPtr(m_id))
            entity->transform.position = position;
        return *this;
    }

    Engine::EntityHandle& Engine::EntityHandle::Rotate(const glm::vec3& rotation)
    {
        if (auto* entity = m_engine->GetEntityPtr(m_id))
            entity->transform.rotation = rotation;
        return *this;
    }

    Engine::EntityHandle& Engine::EntityHandle::Scale(const glm::vec3& scale)
    {
        if (auto* entity = m_engine->GetEntityPtr(m_id))
            entity->transform.scale = scale;
        return *this;
    }

    Engine::EntityHandle& Engine::EntityHandle::Scale(float uniformScale)
    {
        return Scale({uniformScale, uniformScale, uniformScale});  // Convenience overload
    }

    Engine::EntityHandle& Engine::EntityHandle::Color(float r, float g, float b)
    {
        if (auto* entity = m_engine->GetEntityPtr(m_id))
        {
            entity->r = r;
            entity->g = g;
            entity->b = b;
        }
        return *this;
    }

    Engine::EntityHandle& Engine::EntityHandle::Color(const glm::vec3& color)
    {
        return Color(color.r, color.g, color.b);
    }

    Engine::EntityHandle& Engine::EntityHandle::SetActive(bool active)
    {
        if (auto* entity = m_engine->GetEntityPtr(m_id))
            entity->active = active;
        return *this;
    }

    void Engine::EntityHandle::Destroy()
    {
        if (auto* entity = m_engine->GetEntityPtr(m_id))
            entity->active = false;  // Just disables it. Nothing gets deleted.
    }

    // --- Camera Controls ---
    void Engine::SetCameraPosition(const glm::vec3& position)
    {
        if (m_renderer)
            m_renderer->SetCameraPosition(position);
    }

    void Engine::SetCameraLookAt(const glm::vec3& target)
    {
        if (m_renderer)
            m_renderer->SetCameraLookAt(target);
    }

    // --- Internal Methods ---
    void Engine::Update(float deltaTime)
    {
        // TODO: Add systems like physics, AI, input dispatch, etc.
    }

    void Engine::Render()
    {
        if (!m_renderer)
            return;

        m_renderer->BeginFrame();

        for (const auto& entity : m_entities)
        {
            if (!entity.active)
                continue;

            // Entity draw dispatch. This is dumb but simple.
            switch (entity.shape)
            {
                case Entity::CUBE:
                    m_renderer->DrawCube(entity.transform, entity.r, entity.g, entity.b);
                    break;
                case Entity::SPHERE:
                    m_renderer->DrawSphere(entity.transform, entity.r, entity.g, entity.b);
                    break;
                case Entity::PLANE:
                    m_renderer->DrawPlane(entity.transform, entity.r, entity.g, entity.b);
                    break;
            }
        }

        m_renderer->EndFrame();
    }

    // --- Helper Methods ---
    bool Engine::IsValidEntity(int id) const
    {
        return id >= 0 && id < static_cast<int>(m_entities.size())
            && m_entities[id].active;  // Dead/inactive entities fail this check
    }

    Entity* Engine::GetEntityPtr(int id)
    {
        if (id >= 0 && id < static_cast<int>(m_entities.size()))
            return &m_entities[id];  // No nullptr on inactive; caller must check
        return nullptr;
    }

}  // namespace Blacksite
