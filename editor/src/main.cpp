#include "Editor.h"
#include "blacksite/core/Engine.h"
#include <iostream>
#include <chrono>

int main() {
    std::cout << "Starting Blacksite Editor..." << std::endl;

    // Initialize engine
    Blacksite::Engine engine;
    if (!engine.Initialize(1920, 1080, "Blacksite Editor")) {
        std::cerr << "Failed to initialize Blacksite Engine!" << std::endl;
        return -1;
    }

    // Get GLFW window from engine
    GLFWwindow* window = engine.GetGLFWWindow();
    if (!window) {
        std::cerr << "Failed to get GLFW window!" << std::endl;
        return -1;
    }

    // Initialize editor
    BlacksiteEditor::Editor editor;
    if (!editor.Initialize(&engine, window)) {
        std::cerr << "Failed to initialize editor!" << std::endl;
        return -1;
    }

    // Add some test entities
    engine.SpawnCube(glm::vec3(0, 0, 0), "basic", glm::vec3(1, 0, 0));
    engine.SpawnSphere(glm::vec3(2, 0, 0), "basic", glm::vec3(0, 1, 0));
    engine.SpawnPlane(glm::vec3(0, -1, 0), glm::vec3(10, 1, 10), "basic", glm::vec3(0.5f, 0.5f, 0.5f));

    // Set up camera
    engine.SetCameraPosition(glm::vec3(0, 2, 5));
    engine.SetCameraTarget(glm::vec3(0, 0, 0));

    std::cout << "Editor initialized successfully!" << std::endl;
    std::cout << "Controls: F1 - Toggle editor UI, ESC - Exit" << std::endl;

    // EDITOR CONTROLS THE MAIN LOOP (Clean Architecture)
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (!glfwWindowShouldClose(window)) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        glfwPollEvents();

        // Handle ESC key for exit
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        // Editor updates first
        editor.Update(deltaTime);

        // Engine updates systems
        engine.UpdateSystems(deltaTime);

        // Engine renders 3D scene
        engine.RenderScene();

        // Editor renders UI on top
        editor.Render();

        glfwSwapBuffers(window);
    }

    // Cleanup
    std::cout << "Shutting down editor..." << std::endl;
    editor.Shutdown();

    return 0;
}
