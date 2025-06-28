//============================================================
// Blacksite Engine | Window | window.cpp
//------------------------------------------------------------
// Manages OpenGL windows creationg and events.
//============================================================


#include "blacksite/core/window.h"
#include <GLFW/glfw3.h>
#include <iostream>

namespace Blacksite
{

    Window::Window() = default;
    Window::~Window()
    {
        Shutdown();
    }

    bool Window::Initialize(int width, int height, const std::string &title){

        m_width = width;
        m_height = height;

        if (!glfwInit()){
            std::cerr << "Failed to initialize GLFW " << std::endl;
            return false;
        }

        // --- Request OpenGL 3.3 core ---
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // --- Create actual window ---
        m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

        if (!m_window){
            std::cerr << "Failed to create GLFW window." << std::endl;
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(m_window); // [OP] Make this window's context current

        // --- Load OpenGL Functions ---
        if (glewInit() != GLEW_OK){

            std::cerr << "Failed to initialize glew" << std::endl;
            return false;
        }

        // --- Set rendering area and enable depth ---
        glViewport(0, 0, width, height);
        glEnable(GL_DEPTH_TEST);

        return true;

    }

    void Window::Shutdown() {
        if (m_window){
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }
        glfwTerminate();
    }

    bool Window::ShouldClose() const {

        return glfwWindowShouldClose(m_window);
    }

    void Window::SwapBuffers() {
        glfwSwapBuffers(m_window);
    }

    void Window::PollEvents(){
        glfwPollEvents();
    }

} // namespace Blacksite