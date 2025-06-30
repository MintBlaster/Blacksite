#include "blacksite/core/Window.h"
#include <GLFW/glfw3.h>
#include <iostream>

namespace Blacksite {

Window::Window() = default;

Window::~Window() {
    Shutdown();
}

bool Window::Initialize(int width, int height, const std::string& title) {
    m_width = width;
    m_height = height;

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW " << std::endl;
        Shutdown();  // Defensive—should be safe even if glfwInit failed
        return false;
    }

    // --- Request OpenGL 3.3 Core Profile ---
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // --- Create window ---
    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window." << std::endl;
        glfwTerminate();  // glfwInit succeeded, must clean it up
        return false;
    }

    glfwMakeContextCurrent(m_window);  // Required before glewInit()

    // --- Load OpenGL functions via GLEW ---
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return false;  // No recovery—OpenGL functions won’t be available
    }

    // --- Setup default GL state ---
    glViewport(0, 0, width, height);  // Viewport = full window
    glEnable(GL_DEPTH_TEST);          // Enable Z-buffering for 3D rendering

    return true;
}

void Window::Shutdown() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();  // Always call this—GLFW doesn't track internal state
}

bool Window::ShouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Window::SwapBuffers() {
    glfwSwapBuffers(m_window);  // Push back buffer to front
}

void Window::PollEvents() {
    glfwPollEvents();  // Needed every frame to keep window responsive
}

}  // namespace Blacksite
