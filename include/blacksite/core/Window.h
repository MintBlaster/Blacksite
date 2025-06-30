#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>

namespace Blacksite {

    class Window
    {
      public:
        Window();
        ~Window();

        // --- Create window with OpenGL context ---
        bool Initialize(int width, int height, const std::string& title);
        void Shutdown();

        // --- Window State checks ---
        bool ShouldClose() const;
        void SwapBuffers();  // Presents current framebuffer to screen
        void PollEvents();  // Processes window events (keyboard, mouse, etc)

        // --- Getter for other systems ---
        GLFWwindow* GetGLFWindow() const { return m_window; }  // Renderer needs this
        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }

      private:
        GLFWwindow* m_window = nullptr;
        int m_width = 0;
        int m_height = 0;
    };

}  // namespace Blacksite
