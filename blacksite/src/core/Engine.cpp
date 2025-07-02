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

    m_inputSystem = std::make_unique<InputSystem>();
    m_inputSystem->Initialize(m_window->GetGLFWindow());

    // Initialize SceneSystem
    m_sceneSystem = std::make_unique<SceneSystem>();
    float aspectRatio = static_cast<float>(width) / height;
    if (!m_sceneSystem->Initialize(m_physicsSystem.get(), m_renderer.get(), aspectRatio)) {
        BS_ERROR(LogCategory::CORE, "Failed to initialize scene system!");
        return false;
    }

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

int Blacksite::Engine::Run() {
    if (!m_initialized) {
        BS_ERROR(LogCategory::CORE, "Engine not initialized - call Initialize() first!");
        return -1;
    }

    m_running = true;
    auto lastTime = std::chrono::high_resolution_clock::now();
    BS_INFO(LogCategory::CORE, "Starting main loop (refactored engine with scene system)...");

    while (m_running && !m_window->ShouldClose()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        m_window->PollEvents();
        HandleInput();

        Update(deltaTime);
        Render();

        m_window->SwapBuffers();
    }

    BS_INFO(LogCategory::CORE, "Main loop ended");
    return 0;
}

void Blacksite::Engine::HandleInput() {
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
    // Update physics system
    m_physicsSystem->Update(deltaTime);

    // Update scene system (which updates the active scene)
    m_sceneSystem->Update(deltaTime);

    // Call user update callback
    if (m_updateCallback) {
        m_updateCallback(*this, deltaTime);
    }
}

void Engine::Render() {
    if (!m_renderer || !m_sceneSystem) {
        BS_ERROR(LogCategory::RENDERER, "Missing renderer or scene system!");
        return;
    }

    // SceneSystem handles all rendering including BeginFrame/EndFrame
    m_sceneSystem->Render();
}

void Engine::Shutdown() {
    if (!m_initialized)
        return;

    BS_INFO(LogCategory::CORE, "Shutting down Blacksite Engine (refactored with scenes)...");

    // Shutdown systems in reverse order
    m_sceneSystem.reset();
    m_inputSystem.reset();
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

// --- Convenience Entity API (delegates to SceneSystem) ---
int Engine::SpawnCube(const glm::vec3& position) {
    return m_sceneSystem ? m_sceneSystem->SpawnCube(position) : -1;
}

int Engine::SpawnSphere(const glm::vec3& position) {
    return m_sceneSystem ? m_sceneSystem->SpawnSphere(position) : -1;
}

int Engine::SpawnPlane(const glm::vec3& position, const glm::vec3& size) {
    return m_sceneSystem ? m_sceneSystem->SpawnPlane(position, size) : -1;
}

int Engine::SpawnCube(const glm::vec3& position, const std::string& shader, const glm::vec3& color) {
    Scene* activeScene = GetActiveScene();
    return activeScene ? activeScene->SpawnCube(position, shader, color) : -1;
}

int Engine::SpawnSphere(const glm::vec3& position, const std::string& shader, const glm::vec3& color) {
    Scene* activeScene = GetActiveScene();
    return activeScene ? activeScene->SpawnSphere(position, shader, color) : -1;
}

int Engine::SpawnPlane(const glm::vec3& position, const glm::vec3& size, const std::string& shader,
                       const glm::vec3& color) {
    Scene* activeScene = GetActiveScene();
    return activeScene ? activeScene->SpawnPlane(position, size, shader, color) : -1;
}

int Engine::SpawnEntity(Entity::Shape shape, const glm::vec3& position, const std::string& shader,
                        const glm::vec3& color) {
    Scene* activeScene = GetActiveScene();
    return activeScene ? activeScene->SpawnEntity(shape, position, shader, color) : -1;
}

EntityHandle Engine::GetEntity(int id) {
    return m_sceneSystem ? m_sceneSystem->GetEntity(id) : EntityHandle(nullptr, nullptr, -1);
}

// --- Convenience Camera API (delegates to SceneSystem) ---
void Engine::SetCameraPosition(const glm::vec3& position) {
    if (m_sceneSystem)
        m_sceneSystem->SetCameraPosition(position);
}

void Engine::SetCameraTarget(const glm::vec3& target) {
    if (m_sceneSystem)
        m_sceneSystem->SetCameraTarget(target);
}

glm::vec3 Engine::GetCameraPosition() const {
    Scene* activeScene = const_cast<Engine*>(this)->GetActiveScene();
    return activeScene ? activeScene->GetCameraPosition() : glm::vec3(0.0f);
}

glm::vec3 Engine::GetCameraTarget() const {
    Scene* activeScene = const_cast<Engine*>(this)->GetActiveScene();
    return activeScene ? activeScene->GetCameraTarget() : glm::vec3(0.0f);
}

void Engine::UpdateFrame(float deltaTime) {
    if (!m_initialized)
        return;
    Update(deltaTime);
}

void Engine::RenderFrame() {
    if (!m_initialized)
        return;
    Render();
}

}  // namespace Blacksite
