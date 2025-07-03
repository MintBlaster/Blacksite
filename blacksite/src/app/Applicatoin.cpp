#include "blacksite/app/Application.h"
#include "blacksite/core/Logger.h"
#include <chrono>

namespace Blacksite {

Application::Application(const std::string& name, int width, int height)
    : m_name(name), m_width(width), m_height(height) {}

bool Application::Initialize() {
    Blacksite::Logger::Instance().Initialize();

    if (!m_engine.Initialize(m_width, m_height, m_name)) {
        BS_ERROR(LogCategory::CORE, "Failed to initialize engine");
        return false;
    }

    // Call game-specific initialization
    OnInitialize();

    return true;
}

int Application::Run() {
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (!glfwWindowShouldClose(m_engine.GetGLFWWindow())) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        glfwPollEvents();
        HandleInput();

        // Update game
        OnUpdate(deltaTime);
        m_engine.UpdateFrame(deltaTime);

        // FIXED: Render 3D scene first, then UI on top
        m_engine.RenderFrame();  // 3D scene renders to back buffer
        OnRender();              // ImGui renders on top of 3D scene

        glfwSwapBuffers(m_engine.GetGLFWWindow());
    }

    return 0;
}

void Application::HandleInput() {
    auto* input = m_engine.GetInputSystem();


    if (input->KeyDown('q') && input->KeyDown(Key::Escape)) {
        glfwSetWindowShouldClose(m_engine.GetGLFWWindow(), GLFW_TRUE);
    }

}


void Application::Shutdown() {
    OnShutdown();

    m_engine.Shutdown();
}

}  // namespace Blacksite
