#include "blacksite/scene/Scene.h"
#include "blacksite/core/Logger.h"
#include "blacksite/graphics/Renderer.h"
#include "blacksite/physics/PhysicsSystem.h"

namespace Blacksite {

Scene::Scene(const std::string& name) : m_name(name) {
    BS_DEBUG_F(LogCategory::CORE, "Scene '%s' created", m_name.c_str());
}

Scene::~Scene() {
    Shutdown();
    BS_DEBUG_F(LogCategory::CORE, "Scene '%s' destroyed", m_name.c_str());
}

bool Scene::Initialize(PhysicsSystem* physicsSystem, float aspectRatio) {
    if (m_initialized) {
        BS_ERROR_F(LogCategory::CORE, "Scene '%s' already initialized!", m_name.c_str());
        return false;
    }

    if (!physicsSystem) {
        BS_ERROR_F(LogCategory::CORE, "Scene '%s' requires valid PhysicsSystem!", m_name.c_str());
        return false;
    }

    m_physicsSystem = physicsSystem;

    // Initialize EntitySystem
    m_entitySystem = std::make_unique<EntitySystem>();
    m_entitySystem->SetPhysicsSystem(m_physicsSystem);

    // Initialize CameraSystem
    m_cameraSystem = std::make_unique<CameraSystem>();
    m_cameraSystem->Initialize(aspectRatio);

    m_initialized = true;
    BS_INFO_F(LogCategory::CORE, "Scene '%s' initialized successfully", m_name.c_str());
    return true;
}

// Add to Scene::Update method
void Scene::Update(float deltaTime) {
    if (!m_active || !m_initialized)
        return;

    // Update all entities
    auto& entities = m_entitySystem->GetEntities();
    for (auto& entity : entities) {
        if (entity.active) {
            entity.Update(deltaTime);
        }
    }

    // Sync physics to graphics
    SyncPhysicsToGraphics();

    // Call user update callback
    if (m_updateCallback) {
        m_updateCallback(*this, deltaTime);
    }
}

void Scene::Render(Renderer* renderer) {
    if (!m_initialized || !m_active || !renderer)
        return;

    // Set this scene's camera to the renderer
    renderer->SetCamera(&m_cameraSystem->GetCamera());

    // Render all entities
    RenderEntities(renderer);

    // Call user render callback for custom rendering
    if (m_renderCallback) {
        m_renderCallback(*this, renderer);
    }
}

void Scene::Shutdown() {
    if (!m_initialized)
        return;

    BS_INFO_F(LogCategory::CORE, "Shutting down scene '%s'...", m_name.c_str());

    m_cameraSystem.reset();
    m_entitySystem.reset();

    m_physicsSystem = nullptr;
    m_active = false;
    m_initialized = false;

    BS_INFO_F(LogCategory::CORE, "Scene '%s' shutdown complete", m_name.c_str());
}

void Scene::Clear() {
    if (m_entitySystem) {
        // Clear all entities (you'll need to implement this in EntitySystem)
        // m_entitySystem->Clear();
    }
}

void Scene::SyncPhysicsToGraphics() {
    if (!m_entitySystem || !m_physicsSystem)
        return;

    for (auto& entity : m_entitySystem->GetEntities()) {
        if (entity.hasPhysics && entity.active) {
            entity.transform.position = m_physicsSystem->GetBodyPosition(entity.physicsBody);
            entity.transform.rotation = m_physicsSystem->GetBodyRotation(entity.physicsBody);
        }
    }
}

void Scene::RenderEntities(Renderer* renderer) {
    if (!m_entitySystem || !renderer)
        return;

    for (const auto& entity : m_entitySystem->GetEntities()) {
        if (!entity.active)
            continue;

        switch (entity.shape) {
            case Entity::CUBE:
                renderer->DrawCube(entity.transform, entity.shader, entity.color);
                break;
            case Entity::SPHERE:
                renderer->DrawSphere(entity.transform, entity.shader, entity.color);
                break;
            case Entity::PLANE:
                renderer->DrawPlane(entity.transform, entity.shader, entity.color);
                break;
            default:
                BS_ERROR(LogCategory::RENDERER, "Unknown entity shape");
                break;
        }
    }
}

// --- Entity Management Implementation ---
int Scene::SpawnCube(const glm::vec3& position) {
    return m_entitySystem ? m_entitySystem->SpawnCube(position) : -1;
}

int Scene::SpawnSphere(const glm::vec3& position) {
    return m_entitySystem ? m_entitySystem->SpawnSphere(position) : -1;
}

int Scene::SpawnPlane(const glm::vec3& position, const glm::vec3& size) {
    return m_entitySystem ? m_entitySystem->SpawnPlane(position, size) : -1;
}

int Scene::SpawnCube(const glm::vec3& position, const std::string& shader, const glm::vec3& color) {
    return m_entitySystem ? m_entitySystem->SpawnCube(position, shader, color) : -1;
}

int Scene::SpawnSphere(const glm::vec3& position, const std::string& shader, const glm::vec3& color) {
    return m_entitySystem ? m_entitySystem->SpawnSphere(position, shader, color) : -1;
}

int Scene::SpawnPlane(const glm::vec3& position, const glm::vec3& size, const std::string& shader,
                      const glm::vec3& color) {
    return m_entitySystem ? m_entitySystem->SpawnPlane(position, size, shader, color) : -1;
}

int Scene::SpawnEntity(Entity::VisualShape shape, const glm::vec3& position, const std::string& shader,
                       const glm::vec3& color) {
    return m_entitySystem ? m_entitySystem->SpawnEntity(shape, position, shader, color) : -1;
}

EntityHandle Scene::GetEntity(int id) {
    return EntityHandle(m_entitySystem.get(), m_physicsSystem, id);
}

void Scene::RemoveEntity(int id) {
    // Implementation depends on your EntitySystem
    // m_entitySystem->RemoveEntity(id);
}

// --- Camera Management Implementation ---
void Scene::SetCameraPosition(const glm::vec3& position) {
    if (m_cameraSystem)
        m_cameraSystem->SetPosition(position);
}

void Scene::SetCameraTarget(const glm::vec3& target) {
    if (m_cameraSystem)
        m_cameraSystem->SetTarget(target);
}

glm::vec3 Scene::GetCameraPosition() const {
    return m_cameraSystem ? m_cameraSystem->GetPosition() : glm::vec3(0.0f);
}

glm::vec3 Scene::GetCameraTarget() const {
    return m_cameraSystem ? m_cameraSystem->GetTarget() : glm::vec3(0.0f);
}

}  // namespace Blacksite
