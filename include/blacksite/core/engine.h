#pragma once

#include "blacksite/core/window.h"
#include <memory>

namespace Blacksite
{

    class Engine
    {

      public:
        Engine();
        ~Engine();

        // --- Initialize all engine systems ---
        bool Initialize(int width = 1280, int height = 720, const std::string& title = "Blacksite Engine");

        // --- Main Game loop ---
        int Run();

        // --- Shutdown all systems ---
        void Shutdown();

        // --- Engine States ---
        bool IsRunning() const { return m_running; }

      private:
        // --- Core systems ---
        std::unique_ptr<Window> m_window;

        // --- Engine state ---
        bool m_running = false;
        bool m_initialized = false;

        // --- Internal methods ---
        void Update(float deltaTime);
        void Render();
    };
} // namespace Blacksite