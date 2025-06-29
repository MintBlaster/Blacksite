#include "blacksite/core/engine.h"
#include "blacksite/graphics/renderer.h"
#include "blacksite/scene/scene.h"
#include <iostream>
#include <GL/glew.h>

namespace Blacksite {

    Engine::Engine() = default;

    Engine::~Engine()
    {
        Shutdown();
    }

    bool Engine::Initialize(int width, int height, const std::string& title)
    {
        if (m_initialized) {
            std::cerr << "Engine already initialized!" << std::endl;
            return false;
        }

        std::cout << "Initializing Blacksite Engine..." << std::endl;

        // --- Create and initialize window ---
        m_window = std::make_unique<Window>();
        if (!m_window->Initialize(width, height, title)) {
            std::cerr << "Failed to initialize window system" << std::endl;
            return false;
        }

        // --- Create renderer ---
        m_renderer = std::make_unique<Renderer>();
        if (!m_renderer->Initialize()) {
            std::cerr << "Failed to initialize renderer" << std::endl;
            return false;
        }

        // --- Initialize timing ---
        m_lastFrameTime = std::chrono::high_resolution_clock::now();
        m_deltaTime = 0.0f;
        m_totalTime = 0.0f;

        m_initialized = true;
        std::cout << "Blacksite Engine initialized successfully!" << std::endl;

        // Print OpenGL info
        std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;

        return true;
    }

    std::shared_ptr<Scene> Engine::CreateScene()
    {
        m_currentScene = std::make_shared<Scene>();
        if (!m_currentScene->Initialize()) {
            std::cerr << "Failed to initialize scene" << std::endl;
            m_currentScene.reset();
            return nullptr;
        }
        return m_currentScene;
    }

    int Engine::Run()
    {
        if (!m_initialized) {
            std::cerr << "Engine not initialized! Call Initialize() first." << std::endl;
            return -1;
        }

        if (!m_currentScene) {
            std::cerr << "No scene created! Call CreateScene() first." << std::endl;
            return -1;
        }

        std::cout << "Starting main loop..." << std::endl;
        m_running = true;

        // --- Main game loop ---
        while (m_running && !m_window->ShouldClose()) {
            // Process input
            m_window->PollEvents();

            // Update timing
            UpdateTime();

            // Update game logic
            Update();

            // Render frame
            Render();

            // Present frame
            m_window->SwapBuffers();
        }

        std::cout << "Main loop ended." << std::endl;
        return 0;
    }

    void Engine::UpdateTime()
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - m_lastFrameTime);
        m_deltaTime = duration.count() / 1000000.0f; // Convert to seconds
        m_totalTime += m_deltaTime;
        m_lastFrameTime = currentTime;

        // Cap delta time to prevent spiral of death
        if (m_deltaTime > 0.016f) { // ~60 FPS minimum
            m_deltaTime = 0.016f;
        }
    }

    void Engine::Update()
    {
        if (m_currentScene) {
            m_currentScene->Update(m_deltaTime);
        }
    }

    void Engine::Render()
    {
        // Clear screen
        m_renderer->Clear();

        // Render current scene
        if (m_currentScene) {
            m_renderer->RenderScene(*m_currentScene);
        }
    }

    void Engine::Shutdown()
    {
        if (!m_initialized) return;

        std::cout << "Shutting down Blacksite Engine..." << std::endl;

        m_currentScene.reset();
        m_renderer.reset();
        m_window.reset();

        m_running = false;
        m_initialized = false;

        std::cout << "Engine shutdown complete." << std::endl;
    }

} // namespace Blacksite