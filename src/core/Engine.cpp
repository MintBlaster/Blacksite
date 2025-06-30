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
        std::cerr << "Engine already initialized!" << std::endl;
        return false;
    }

    // Initialize window
    m_window = std::make_unique<Window>();
    if (!m_window->Initialize(width, height, title)) {
        std::cerr << "Failed to initialize window!" << std::endl;
        return false;
    }

    // Initialize renderer
    m_renderer = std::make_unique<Renderer>();
    if (!m_renderer->Initialize(width, height)) {
        std::cerr << "Failed to initialize renderer!" << std::endl;
        return false;
    }

    // Initialize physics system
    m_physicsSystem = std::make_unique<PhysicsSystem>();
    if (!m_physicsSystem->Initialize()) {
        std::cerr << "Failed to initialize physics system!" << std::endl;
        return false;
    }

    m_initialized = true;
    std::cout << "Blacksite Engine initialized successfully! Physics is ready!" << std::endl;
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

    while (m_running && !m_window->ShouldClose()) {
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
        return;
    }

    std::cout << "Shutting down Blacksite Engine... (end of an era)" << std::endl;

    m_entities.clear();
    m_renderer.reset();
    m_window.reset();

    m_running = false;
    m_initialized = false;

    std::cout << "Engine shutdown complete (everything is dead now)" << std::endl;
}

// Physics by default - no more enablePhysics parameter!
int Engine::SpawnCube(const glm::vec3& position) {
    Entity entity(Entity::CUBE);
    entity.transform.position = position;

    // Always create physics body
    entity.physicsBody = m_physicsSystem->CreateBoxBody(position, {1.0f, 1.0f, 1.0f}, false);
    entity.hasPhysics = true;

    int id = m_nextEntityId++;
    m_entities.push_back(entity);
    std::cout << "Spawned cube with ID " << id << " (physics enabled)" << std::endl;
    return id;
}

int Engine::SpawnSphere(const glm::vec3& position) {
    Entity entity(Entity::SPHERE);
    entity.transform.position = position;

    // Always create physics body
    entity.physicsBody = m_physicsSystem->CreateSphereBody(position, 0.5f, false);
    entity.hasPhysics = true;

    int id = m_nextEntityId++;
    m_entities.push_back(entity);
    std::cout << "Spawned sphere with ID " << id << " (physics enabled)" << std::endl;
    return id;
}

int Engine::SpawnPlane(const glm::vec3& position, const glm::vec3& size) {
    Entity entity(Entity::PLANE);
    entity.transform.position = position;
    entity.transform.scale = size;

    // Always create physics body
    entity.physicsBody = m_physicsSystem->CreatePlaneBody(position, size);
    entity.hasPhysics = true;

    int id = m_nextEntityId++;
    m_entities.push_back(entity);
    std::cout << "Spawned plane with ID " << id << " (size: " << size.x << "x" << size.y << "x" << size.z
              << ") (physics enabled)" << std::endl;
    return id;
}

void Engine::SyncPhysicsToGraphics() {
    for (auto& entity : m_entities) {
        if (entity.hasPhysics && entity.active) {
            entity.transform.position = m_physicsSystem->GetBodyPosition(entity.physicsBody);
            entity.transform.rotation = m_physicsSystem->GetBodyRotation(entity.physicsBody);
        }
    }
}

Engine::EntityHandle Engine::GetEntity(int id) {
    return EntityHandle(this, id);
}

// Improved method names and always assume physics exists
Engine::EntityHandle& Engine::EntityHandle::At(const glm::vec3& position) {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        entity->transform.position = position;
        if (entity->hasPhysics) {
            m_engine->GetPhysicsSystem()->SetBodyPosition(entity->physicsBody, position);
        }
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
    return Scale({x, y, z});
}

Engine::EntityHandle& Engine::EntityHandle::Scale(float uniformScale) {
    return Scale({uniformScale, uniformScale, uniformScale});
}

Engine::EntityHandle& Engine::EntityHandle::Color(float r, float g, float b) {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        entity->color = {r, g, b};
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
        std::cout << "Camera moved to (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    }
}

void Engine::SetCameraTarget(const glm::vec3& target) {
    if (m_renderer) {
        m_renderer->GetCamera().SetTarget(target);
        std::cout << "Camera now staring at (" << target.x << ", " << target.y << ", " << target.z << ")" << std::endl;
    }
}

// Better method names for physics operations
Engine::EntityHandle& Engine::EntityHandle::Push(const glm::vec3& force) {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        if (entity->hasPhysics) {
            m_engine->GetPhysicsSystem()->AddForce(entity->physicsBody, force);
        } else {
            std::cerr << "Entity " << m_id << " has no physics body" << std::endl;
        }
    }
    return *this;
}

Engine::EntityHandle& Engine::EntityHandle::Impulse(const glm::vec3& impulse) {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        if (entity->hasPhysics) {
            m_engine->GetPhysicsSystem()->AddImpulse(entity->physicsBody, impulse);
        } else {
            std::cerr << "Entity " << m_id << " has no physics body" << std::endl;
        }
    }
    return *this;
}

Engine::EntityHandle& Engine::EntityHandle::SetVelocity(const glm::vec3& velocity) {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        if (entity->hasPhysics) {
            m_engine->GetPhysicsSystem()->SetVelocity(entity->physicsBody, velocity);
        } else {
            std::cerr << "Entity " << m_id << " has no physics body" << std::endl;
        }
    }
    return *this;
}

Engine::EntityHandle& Engine::EntityHandle::SetAngularVelocity(const glm::vec3& angularVel) {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        if (entity->hasPhysics) {
            m_engine->GetPhysicsSystem()->SetAngularVelocity(entity->physicsBody, angularVel);
        } else {
            std::cerr << "Entity " << m_id << " has no physics body" << std::endl;
        }
    }
    return *this;
}

Engine::EntityHandle& Engine::EntityHandle::MakeStatic() {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        if (entity->hasPhysics) {
            // Convert to static body in physics system
            m_engine->GetPhysicsSystem()->MakeBodyStatic(entity->physicsBody);
            std::cout << "Entity " << m_id << " is now static" << std::endl;
        }
    }
    return *this;
}

Engine::EntityHandle& Engine::EntityHandle::MakeDynamic() {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        if (entity->hasPhysics) {
            // Convert to dynamic body in physics system
            m_engine->GetPhysicsSystem()->MakeBodyDynamic(entity->physicsBody);
            std::cout << "Entity " << m_id << " is now dynamic" << std::endl;
        }
    }
    return *this;
}

void Engine::Update(float deltaTime) {
    m_physicsSystem->Update(deltaTime);
    SyncPhysicsToGraphics();

    if (m_updateCallback) {
        m_updateCallback(*this, deltaTime);
    }
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

    int drawnCount = 0;
    for (const auto& entity : m_entities) {
        if (!entity.active) {
            continue;
        }

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
                std::cerr << "Unknown entity shape" << std::endl;
                break;
        }

        drawnCount++;
    }

    m_renderer->EndFrame();
}

bool Engine::IsValidEntity(int id) const {
    if (id < 0 || id >= static_cast<int>(m_entities.size())) {
        return false;
    }
    return m_entities[id].active;
}

Entity* Engine::GetEntityPtr(int id) {
    if (id >= 0 && id < static_cast<int>(m_entities.size())) {
        return &m_entities[id];
    }
    return nullptr;
}

}  // namespace Blacksite