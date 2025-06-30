#include "blacksite/core/Engine.h"
#include <GL/glew.h>
#include <chrono>
#include <iostream>

namespace Blacksite {

Engine::Engine() = default;

Engine::~Engine() {
    Shutdown();
}

bool Engine::Initialize(int width, int height, const std::string& title) {
    if (m_initialized) {
        std::cout << "Engine already initialized - ignoring duplicate call" << std::endl;
        return true;
    }

    std::cout << "Initializing Blacksite Engine... (buckle up)" << std::endl;

    // Create the window
    m_window = std::make_unique<Window>();
    if (!m_window->Initialize(width, height, title)) {
        std::cerr << "Failed to initialize window (probably a driver issue)" << std::endl;
        return false;
    }

    // Initialize the renderer
    m_renderer = std::make_unique<Renderer>();
    if (!m_renderer->Initialize(width, height)) {
        std::cerr << "Failed to initialize renderer (graphics gods are angry)" << std::endl;
        return false;
    }

    // Set up a reasonable default camera position
    m_renderer->GetCamera().SetPosition({0.0f, 2.0f, 10.0f});
    m_renderer->GetCamera().SetTarget({0.0f, 0.0f, 0.0f});

    std::cout << "Blacksite Engine initialized successfully (miracles do happen)" << std::endl;
    m_initialized = true;
    return true;
}

int Engine::Run() {
    if (!m_initialized) {
        std::cerr << "Engine not initialized - call Initialize() first, genius" << std::endl;
        return -1;
    }

    m_running = true;
    auto lastTime = std::chrono::high_resolution_clock::now();

    std::cout << "Starting main loop... (here we go again)" << std::endl;

    // The main loop
    while (m_running && !m_window->ShouldClose()) {
        // Calculate delta time
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        m_window->PollEvents();
        Update(deltaTime);
        Render();
        m_window->SwapBuffers();
    }

    std::cout << "Main loop ended (probably for the best)" << std::endl;
    return 0;
}

void Engine::Shutdown() {
    if (!m_initialized) {
        return;  // Nothing to clean up
    }

    std::cout << "Shutting down Blacksite Engine... (end of an era)" << std::endl;

    // Clean up in reverse order of initialization
    m_entities.clear();
    m_renderer.reset();
    m_window.reset();

    m_running = false;
    m_initialized = false;

    std::cout << "Engine shutdown complete (everything is dead now)" << std::endl;
}

int Engine::SpawnCube(const glm::vec3& position) {
    Entity entity(Entity::CUBE);
    entity.transform.position = position;

    // Add it to our collection of digital pets
    m_entities.push_back(entity);

    int id = m_nextEntityId++;
    std::cout << "Spawned cube with ID " << id << " at (" << position.x << ", " << position.y << ", " << position.z
              << ")" << std::endl;

    return id;
}

int Engine::SpawnSphere(const glm::vec3& position) {
    Entity entity(Entity::SPHERE);
    entity.transform.position = position;

    m_entities.push_back(entity);

    int id = m_nextEntityId++;
    std::cout << "Spawned sphere with ID " << id << " (it's round!)" << std::endl;

    return id;
}

int Engine::SpawnPlane(const glm::vec3& position) {
    Entity entity(Entity::PLANE);
    entity.transform.position = position;

    m_entities.push_back(entity);

    int id = m_nextEntityId++;
    std::cout << "Spawned plane with ID " << id << " (flat as your gameplay)" << std::endl;

    return id;
}

Engine::EntityHandle Engine::GetEntity(int id) {
    // Return a handle even if the entity doesn't exist
    // The handle will check validity on each operation
    return EntityHandle(this, id);
}

Engine::EntityHandle& Engine::EntityHandle::At(const glm::vec3& position) {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        entity->transform.position = position;
    } else {
        std::cerr << "Tried to move non-existent entity " << m_id << std::endl;
    }
    return *this;
}

Engine::EntityHandle& Engine::EntityHandle::Rotate(const glm::vec3& rotation) {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        entity->transform.rotation = rotation;
    } else {
        std::cerr << "Tried to rotate non-existent entity " << m_id << std::endl;
    }
    return *this;
}

Engine::EntityHandle& Engine::EntityHandle::Scale(const glm::vec3& scale) {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        entity->transform.scale = scale;
    } else {
        std::cerr << "Tried to scale non-existent entity " << m_id << std::endl;
    }
    return *this;
}

Engine::EntityHandle& Engine::EntityHandle::Scale(float x, float y, float z) {
    glm::vec3 scale = {x, y, z};
    return Scale(scale);
}
Engine::EntityHandle& Engine::EntityHandle::Scale(float uniformScale) {
    return Scale({uniformScale, uniformScale, uniformScale});
}

Engine::EntityHandle& Engine::EntityHandle::Color(float r, float g, float b) {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        entity->color = {r, g, b};  // Now using the correct glm::vec3 color field
    } else {
        std::cerr << "Tried to color non-existent entity " << m_id << std::endl;
    }
    return *this;
}

Engine::EntityHandle& Engine::EntityHandle::Color(const glm::vec3& color) {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        entity->color = color;
    } else {
        std::cerr << "Tried to color non-existent entity " << m_id << std::endl;
    }
    return *this;
}

Engine::EntityHandle& Engine::EntityHandle::SetActive(bool active) {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        entity->active = active;
        std::cout << "Entity " << m_id << " is now " << (active ? "active" : "inactive") << std::endl;
    } else {
        std::cerr << "Tried to set state of non-existent entity " << m_id << std::endl;
    }
    return *this;
}

void Engine::EntityHandle::Destroy() {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        entity->active = false;
        std::cout << "Entity " << m_id << " destroyed (marked for digital death)" << std::endl;
    } else {
        std::cerr << "Tried to destroy non-existent entity " << m_id << " (it's already dead, Jim)" << std::endl;
    }
}

void Engine::SetCameraPosition(const glm::vec3& position) {
    if (m_renderer) {
        m_renderer->GetCamera().SetPosition(position);
        std::cout << "Camera moved to (" << position.x << ", " << position.y << ", " << position.z
                  << ") - new perspective unlocked" << std::endl;
    }
}

void Engine::SetCameraTarget(const glm::vec3& target) {
    if (m_renderer) {
        m_renderer->GetCamera().SetTarget(target);
        std::cout << "Camera now staring at (" << target.x << ", " << target.y << ", " << target.z << ") - how creepy"
                  << std::endl;
    }
}

void Engine::Update(float deltaTime) {
    if (m_updateCallback) {
        m_updateCallback(*this, deltaTime);
    }

    (void)deltaTime;  // Suppress unused parameter warning
}

void Engine::SetUpdateCallback(UpdateCallback callback) {
    m_updateCallback = callback;
    std::cout << "Game update callback registered" << std::endl;
}

void Engine::Render() {
    if (!m_renderer) {
        std::cerr << "No renderer available - everything is invisible!" << std::endl;
        return;
    }

    m_renderer->BeginFrame();

    // Draw all active entities
    int drawnCount = 0;
    for (const auto& entity : m_entities) {
        if (!entity.active) {
            continue;  // Skip dead entities
        }

        // Draw based on shape type
        switch (entity.shape) {
            case Entity::CUBE:
                m_renderer->DrawCube(entity.transform, entity.color);
                break;
            case Entity::SPHERE:
                m_renderer->DrawSphere(entity.transform, entity.color);
                break;
            case Entity::PLANE:
                m_renderer->DrawPlane(entity.transform, entity.color);
                break;
            default:
                std::cerr << "Unknown entity shape - what sorcery is this?" << std::endl;
                break;
        }

        drawnCount++;
    }

    m_renderer->EndFrame();

    // Uncomment for debugging render performance
    std::cout << "Rendered " << drawnCount << " entities this frame" << std::endl;
}

bool Engine::IsValidEntity(int id) const {
    // Check bounds first
    if (id < 0 || id >= static_cast<int>(m_entities.size())) {
        return false;
    }

    // Check if entity is active
    return m_entities[id].active;
}

Entity* Engine::GetEntityPtr(int id) {
    // Only check bounds - let caller decide how to handle inactive entities
    if (id >= 0 && id < static_cast<int>(m_entities.size())) {
        return &m_entities[id];
    }

    return nullptr;  // Entity doesn't exist
}

}  // namespace Blacksite