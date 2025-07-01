#include "blacksite/core/Engine.h"
#include <GL/glew.h>
#include <chrono>
#include "blacksite/core/Logger.h"

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

    // Initialize core systems first
    m_window = std::make_unique<Window>();
    if (!m_window->Initialize(width, height, title)) {
        BS_ERROR(LogCategory::CORE, "Failed to initialize window!");
        return false;
    }

    m_renderer = std::make_unique<Renderer>();
    if (!m_renderer->Initialize(width, height)) {
        BS_ERROR(LogCategory::CORE, "Failed to initialize renderer!");
        return false;
    }

    m_physicsSystem = std::make_unique<PhysicsSystem>();
    if (!m_physicsSystem->Initialize()) {
        BS_ERROR(LogCategory::CORE, "Failed to initialize physics system!");
        return false;
    }

    // Initialize new systems
    m_entitySystem = std::make_unique<EntitySystem>();
    m_entitySystem->SetPhysicsSystem(m_physicsSystem.get());

    m_cameraSystem = std::make_unique<CameraSystem>();
    m_cameraSystem->Initialize(static_cast<float>(width) / height);

    m_inputSystem = std::make_unique<InputSystem>();
    m_inputSystem->Initialize(m_window->GetGLFWindow());

    // Connect camera to renderer
    m_renderer->SetCamera(&m_cameraSystem->GetCamera());

    BS_INFO(LogCategory::CORE, "Running renderer diagnostics...");
    m_renderer->DebugOpenGLState();
    m_renderer->DebugShaderCompilation();
    m_renderer->DebugGeometry();
    m_renderer->DebugMatrices();

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
    BS_INFO(LogCategory::CORE, "Starting main loop (refactored engine)...");

    while (m_running && !m_window->ShouldClose()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        m_window->PollEvents();
        HandleInput();

        // Render 3D scene FIRST
        Render();
        Update(deltaTime);

        m_window->SwapBuffers();

    }

    BS_INFO(LogCategory::CORE, "Main loop ended");
    return 0;
}

void Engine::HandleInput() {
    m_inputSystem->Update();

    // Handle engine-level inputs
    if (m_inputSystem->IsEscapePressed()) {
        m_running = false;
        BS_INFO(LogCategory::CORE, "Engine shutdown requested via ESC key");
    }

    // Future: Handle other global inputs like F1 for editor toggle
    if (m_inputSystem->IsF1JustPressed()) {
        BS_DEBUG(LogCategory::CORE, "F1 pressed - future editor toggle");
    }
}

void Engine::Update(float deltaTime) {
    // Update all systems
    m_cameraSystem->Update(deltaTime);
    m_physicsSystem->Update(deltaTime);
    SyncPhysicsToGraphics();

    // Call user update callback
    if (m_updateCallback) {
        m_updateCallback(*this, deltaTime);
    }
}

void Engine::SyncPhysicsToGraphics() {
    // Sync physics positions back to transform positions
    for (auto& entity : m_entitySystem->GetEntities()) {
        if (entity.hasPhysics && entity.active) {
            entity.transform.position = m_physicsSystem->GetBodyPosition(entity.physicsBody);
            entity.transform.rotation = m_physicsSystem->GetBodyRotation(entity.physicsBody);
        }
    }
}

void Engine::Render() {
    if (!m_renderer) {
        BS_ERROR(LogCategory::RENDERER, "No renderer available!");
        return;
    }

    m_renderer->BeginFrame();

    int drawnCount = 0;
    for (const auto& entity : m_entitySystem->GetEntities()) {
        if (!entity.active)
            continue;

        switch (entity.shape) {
            case Entity::CUBE:
                m_renderer->DrawCube(entity.transform, entity.shader, entity.color);
                break;
            case Entity::SPHERE:
                m_renderer->DrawSphere(entity.transform, entity.shader, entity.color);
                break;
            case Entity::PLANE:
                m_renderer->DrawPlane(entity.transform, entity.shader, entity.color);
                break;
            default:
                BS_ERROR(LogCategory::RENDERER, "Unknown entity shape");
                break;
        }
        drawnCount++;
    }

    m_renderer->EndFrame();
}


void Engine::Shutdown() {
    if (!m_initialized)
        return;

    BS_INFO(LogCategory::CORE, "Shutting down Blacksite Engine (refactored)...");

    // Shutdown systems in reverse order
    m_inputSystem.reset();
    m_cameraSystem.reset();
    m_entitySystem.reset();

    m_renderer.reset();
    m_physicsSystem.reset();
    m_window.reset();

    m_running = false;
    m_initialized = false;
    BS_INFO(LogCategory::CORE, "Engine shutdown complete");
}

void Engine::SetUpdateCallback(UpdateCallback callback) {
    m_updateCallback = callback;
    BS_DEBUG(LogCategory::CORE, "Update callback registered");
}

// Convenience methods that delegate to systems
int Engine::SpawnCube(const glm::vec3& position) {
    return m_entitySystem->SpawnCube(position);
}

int Engine::SpawnSphere(const glm::vec3& position) {
    return m_entitySystem->SpawnSphere(position);
}

int Engine::SpawnPlane(const glm::vec3& position, const glm::vec3& size) {
    return m_entitySystem->SpawnPlane(position, size);
}

// Shader specific

int Engine::SpawnCube(const glm::vec3& position, const std::string& shader, const glm::vec3& color) {
    return m_entitySystem->SpawnCube(position, shader, color);
}

int Engine::SpawnSphere(const glm::vec3& position, const std::string& shader, const glm::vec3& color) {
    return m_entitySystem->SpawnSphere(position, shader, color);
}

int Engine::SpawnPlane(const glm::vec3& position, const glm::vec3& size, const std::string& shader, const glm::vec3& color) {
    return m_entitySystem->SpawnPlane(position, size, shader, color);
}

int Engine::SpawnEntity(Entity::Shape shape, const glm::vec3& position, const std::string& shader, const glm::vec3& color) {
    return m_entitySystem->SpawnEntity(shape, position, shader, color);
}

EntityHandle Engine::GetEntity(int id) {
    return EntityHandle(m_entitySystem.get(), m_physicsSystem.get(), id);
}

void Engine::SetCameraPosition(const glm::vec3& position) {
    m_cameraSystem->SetPosition(position);
}

void Engine::SetCameraTarget(const glm::vec3& target) {
    m_cameraSystem->SetTarget(target);
}

glm::vec3 Engine::GetCameraPosition() const {
    return m_cameraSystem->GetPosition();
}

glm::vec3 Engine::GetCameraTarget() const {
    return m_cameraSystem->GetTarget();
}

}  // namespace Blacksite
