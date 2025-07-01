#include "blacksite/core/Engine.h"
#include "blacksite/core/Logger.h"
#include <GL/glew.h>
#include <chrono>
#include <iostream>
#include <memory>

namespace Blacksite {

Engine::Engine() = default;

Engine::~Engine() {
    Shutdown();
}

bool Engine::Initialize(int width, int height, const std::string& title) {
    if (m_initialized) {
        BS_ERROR(LogCategory::CORE, "Engine already initialized!");
        return false;
    }

    // Initialize window
    m_window = std::make_unique<Window>();
    if (!m_window->Initialize(width, height, title)) {
        BS_ERROR(LogCategory::CORE, "Failed to initialize window!");
        return false;
    }

    // Initialize renderer
    m_renderer = std::make_unique<Renderer>();
    if (!m_renderer->Initialize(width, height)) {
        BS_ERROR(LogCategory::CORE, "Failed to initialize renderer!");
        return false;
    }

    // Initialize physics system
    m_physicsSystem = std::make_unique<PhysicsSystem>();
    if (!m_physicsSystem->Initialize()) {
        BS_ERROR(LogCategory::CORE, "Failed to initialize physics system!");
        return false;
    }

    // Initialize Editor Core (replaces ImGuiManager)
    m_editorCore = std::make_unique<Editor::EditorCore>();
    if (!m_editorCore->Initialize(m_window->GetGLFWindow())) {
        BS_ERROR(LogCategory::CORE, "Failed to initialize editor core!");
        return false;
    }

    m_initialized = true;
    BS_INFO(LogCategory::CORE, "Blacksite Engine initialized successfully");
    BS_INFO_F(LogCategory::CORE, "Build: %s %s", __DATE__, __TIME__);
    return true;
}

int Engine::Run() {
    if (!m_initialized) {
        BS_ERROR(LogCategory::CORE, "Engine not initialized - call Initialize() first!");
        return -1;
    }

    m_running = true;
    auto lastTime = std::chrono::high_resolution_clock::now();

    BS_INFO(LogCategory::CORE, "Starting main loop...");

    while (m_running && !m_window->ShouldClose()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        m_window->PollEvents();
        HandleInput();

        // Editor frame management
        m_editorCore->BeginFrame();

        Update(deltaTime);

        // Render editor UI
        m_editorCore->Render(*this);

        Render();

        m_editorCore->EndFrame();
        m_window->SwapBuffers();
    }

    BS_INFO(LogCategory::CORE, "Main loop ended");
    return 0;
}

void Engine::HandleInput() {
    // F1 key - Toggle Editor
    bool f1Current = glfwGetKey(m_window->GetGLFWindow(), GLFW_KEY_F1) == GLFW_PRESS;
    if (f1Current && !m_f1KeyPressed) {
        m_editorCore->ToggleVisibility();
    }
    m_f1KeyPressed = f1Current;

    // ESC key - Close engine
    if (glfwGetKey(m_window->GetGLFWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        m_running = false;
    }
}

void Engine::Shutdown() {
    if (!m_initialized) {
        return;
    }

    BS_INFO(LogCategory::CORE, "Shutting down Blacksite Engine...");

    if (m_editorCore) {
        m_editorCore->Shutdown();
        m_editorCore.reset();
    }

    m_entities.clear();
    m_entityNames.clear();
    m_renderer.reset();
    m_physicsSystem.reset();
    m_window.reset();

    m_running = false;
    m_initialized = false;

    BS_INFO(LogCategory::CORE, "Engine shutdown complete");
}

// Physics by default
int Engine::SpawnCube(const glm::vec3& position) {
    Entity entity(Entity::CUBE);
    entity.transform.position = position;

    // Always create physics body
    entity.physicsBody = m_physicsSystem->CreateBoxBody(position, {1.0f, 1.0f, 1.0f}, false);
    entity.hasPhysics = true;

    int id = m_nextEntityId++;
    m_entities.push_back(entity);
    m_entityNames.push_back("Cube_" + std::to_string(id));

    BS_INFO_F(LogCategory::PHYSICS, "Spawned cube entity (ID: %d, dynamic: true)", id);
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
    m_entityNames.push_back("Sphere_" + std::to_string(id));

    BS_INFO_F(LogCategory::PHYSICS, "Spawned sphere entity (ID: %d, dynamic: true)", id);
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
    m_entityNames.push_back("Plane_" + std::to_string(id));

    BS_INFO_F(LogCategory::PHYSICS, "Spawned plane entity (ID: %d, size: %.1fx%.1fx%.1f, static: false)", 
              id, size.x, size.y, size.z);
    return id;
}

// Editor-specific entity operations
void Engine::RemoveEntity(int id) {
    if (IsValidEntity(id)) {
        m_entities[id].active = false;
        BS_INFO_F(LogCategory::EDITOR, "Entity removed (ID: %d)", id);
    }
}

void Engine::DuplicateEntity(int id) {
    if (IsValidEntity(id)) {
        const Entity& original = m_entities[id];

        // Create new entity based on original
        int newId = -1;
        glm::vec3 newPos = original.transform.position + glm::vec3(1.0f, 0.0f, 0.0f);

        switch (original.shape) {
            case Entity::CUBE:
                newId = SpawnCube(newPos);
                break;
            case Entity::SPHERE:
                newId = SpawnSphere(newPos);
                break;
            case Entity::PLANE:
                newId = SpawnPlane(newPos, original.transform.scale);
                break;
        }

        if (newId >= 0) {
            GetEntity(newId).Color(original.color);
            BS_INFO_F(LogCategory::EDITOR, "Entity duplicated (original: %d, copy: %d)", id, newId);
        }
    }
}

void Engine::SetEntityName(int id, const std::string& name) {
    if (id >= 0 && id < static_cast<int>(m_entityNames.size())) {
        m_entityNames[id] = name;
    }
}

std::string Engine::GetEntityName(int id) const {
    if (id >= 0 && id < static_cast<int>(m_entityNames.size())) {
        return m_entityNames[id];
    }
    return "Unknown";
}

glm::vec3 Engine::GetCameraPosition() const {
    if (m_renderer) {
        return m_renderer->GetCamera().GetPosition();
    }
    return {0, 0, 0};
}

glm::vec3 Engine::GetCameraTarget() const {
    if (m_renderer) {
        return m_renderer->GetCamera().GetTarget();
    }
    return {0, 0, 0};
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

// EntityHandle implementations remain the same...
Engine::EntityHandle& Engine::EntityHandle::At(const glm::vec3& position) {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        entity->transform.position = position;
        if (entity->hasPhysics) {
            m_engine->GetPhysicsSystem()->SetBodyPosition(entity->physicsBody, position);
        }
    } else {
        BS_ERROR_F(LogCategory::CORE, "Tried to move non-existent entity %d", m_id);
    }
    return *this;
}

Engine::EntityHandle& Engine::EntityHandle::Rotate(const glm::vec3& rotation) {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        entity->transform.rotation = rotation;
    } else {
        BS_ERROR_F(LogCategory::CORE, "Tried to rotate non-existent entity %d", m_id);
    }
    return *this;
}

Engine::EntityHandle& Engine::EntityHandle::Scale(const glm::vec3& scale) {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        entity->transform.scale = scale;
    } else {
        BS_ERROR_F(LogCategory::CORE, "Tried to scale non-existent entity %d", m_id);
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
        BS_ERROR_F(LogCategory::CORE, "Tried to color non-existent entity %d", m_id);
    }
    return *this;
}

Engine::EntityHandle& Engine::EntityHandle::Color(const glm::vec3& color) {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        entity->color = color;
    } else {
        BS_ERROR_F(LogCategory::CORE, "Tried to color non-existent entity %d", m_id);
    }
    return *this;
}

Engine::EntityHandle& Engine::EntityHandle::SetActive(bool active) {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        entity->active = active;
        BS_DEBUG_F(LogCategory::CORE, "Entity %d is now %s", m_id, (active ? "active" : "inactive"));
    } else {
        BS_ERROR_F(LogCategory::CORE, "Tried to set state of non-existent entity %d", m_id);
    }
    return *this;
}

void Engine::EntityHandle::Destroy() {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        entity->active = false;
        BS_DEBUG_F(LogCategory::CORE, "Entity %d destroyed", m_id);
    } else {
        BS_ERROR_F(LogCategory::CORE, "Tried to destroy non-existent entity %d", m_id);
    }
}

void Engine::SetCameraPosition(const glm::vec3& position) {
    if (m_renderer) {
        m_renderer->GetCamera().SetPosition(position);
        BS_DEBUG_F(LogCategory::RENDERER, "Camera position updated: (%.1f, %.1f, %.1f)", 
                   position.x, position.y, position.z);
    }
}

void Engine::SetCameraTarget(const glm::vec3& target) {
    if (m_renderer) {
        m_renderer->GetCamera().SetTarget(target);
        BS_DEBUG_F(LogCategory::RENDERER, "Camera target updated: (%.1f, %.1f, %.1f)", 
                   target.x, target.y, target.z);
    }
}

// Physics operations (same as before)
Engine::EntityHandle& Engine::EntityHandle::Push(const glm::vec3& force) {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        if (entity->hasPhysics) {
            m_engine->GetPhysicsSystem()->AddForce(entity->physicsBody, force);
        } else {
            BS_ERROR_F(LogCategory::PHYSICS, "Entity %d has no physics body", m_id);
        }
    }
    return *this;
}

Engine::EntityHandle& Engine::EntityHandle::Impulse(const glm::vec3& impulse) {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        if (entity->hasPhysics) {
            m_engine->GetPhysicsSystem()->AddImpulse(entity->physicsBody, impulse);
        } else {
            BS_ERROR_F(LogCategory::PHYSICS, "Entity %d has no physics body", m_id);
        }
    }
    return *this;
}

Engine::EntityHandle& Engine::EntityHandle::SetVelocity(const glm::vec3& velocity) {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        if (entity->hasPhysics) {
            m_engine->GetPhysicsSystem()->SetVelocity(entity->physicsBody, velocity);
        } else {
            BS_ERROR_F(LogCategory::PHYSICS, "Entity %d has no physics body", m_id);
        }
    }
    return *this;
}

Engine::EntityHandle& Engine::EntityHandle::SetAngularVelocity(const glm::vec3& angularVel) {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        if (entity->hasPhysics) {
            m_engine->GetPhysicsSystem()->SetAngularVelocity(entity->physicsBody, angularVel);
        } else {
            BS_ERROR_F(LogCategory::PHYSICS, "Entity %d has no physics body", m_id);
        }
    }
    return *this;
}

Engine::EntityHandle& Engine::EntityHandle::MakeStatic() {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        if (entity->hasPhysics) {
            m_engine->GetPhysicsSystem()->MakeBodyStatic(entity->physicsBody);
            BS_DEBUG_F(LogCategory::PHYSICS, "Entity %d is now static", m_id);
        }
    }
    return *this;
}

Engine::EntityHandle& Engine::EntityHandle::MakeDynamic() {
    if (auto* entity = m_engine->GetEntityPtr(m_id)) {
        if (entity->hasPhysics) {
            m_engine->GetPhysicsSystem()->MakeBodyDynamic(entity->physicsBody);
            BS_DEBUG_F(LogCategory::PHYSICS, "Entity %d is now dynamic", m_id);
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
    BS_DEBUG(LogCategory::CORE, "Update callback registered");
}

void Engine::Render() {
    if (!m_renderer) {
        BS_ERROR(LogCategory::RENDERER, "No renderer available!");
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
                BS_ERROR(LogCategory::RENDERER, "Unknown entity shape");
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