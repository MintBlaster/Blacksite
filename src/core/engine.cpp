#include "blacksite/core/engine.h"
#include <GL/glew.h>
#include "blacksite/core/window.h"
#include <chrono>
#include <iostream>
#include <memory>

namespace Blacksite
{

    Engine::Engine() = default;

    Engine::~Engine()
    {
        Shutdown();
    }

    bool Engine::Initialize(int width, int height, const std::string& title)
    {

        if (m_initialized)
        {
            return true;
        }

        // --- Create window ---
        m_window = std::make_unique<Window>();
        if (!m_window->Initialize(width, height, title))
        {
            std::cerr << "Failed to initialize window" << std::endl;
            return false;
        }

        std::cout << "Blacksite engine initialized successfully" << std::endl;
        m_initialized = true;
        return m_initialized;
    }

    int Engine::Run()
    {

        if (!m_initialized)
        {
            std::cerr << "Engine not initialized. Call Initialize() first" << std::endl;
            return -1;
        }

        m_running = true;

        // --- Time tracking for delta time ---
        auto lastTime = std::chrono::high_resolution_clock::now();

        std::cout << "Starting main game loop..." << std::endl;

        // --- Main game loop ---
        while (m_running && !m_window->ShouldClose())
        {

            // --- Calculate delta time ---
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;

            // --- Process Inputs ---
            m_window->PollEvents();

            // --- Update game logic ---
            Update(deltaTime);

            // --- Render frame ---
            Render();

            // --- Present frame ---
            m_window->SwapBuffers();
        }

        std::cout << "Game loop ended" << std::endl;
        return 0;
    }

    void Engine::Shutdown()
    {
        if (!m_initialized)
        {
            return;
        }

        std::cout << "Shutting down blacksite engine..." << std::endl;

        m_window.reset();

        m_running = false;
        m_initialized = false;
    }

    void Engine::Update(float deltaTime)
    {

        // [TODO] Update game logic here.
    }

    void Engine::Render()
    {
        // --- Clear the screen ---
        glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // [TODO] Render game objects here.
    }
} // namespace Blacksite