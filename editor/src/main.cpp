#include "Editor.h"
#include "blacksite/core/Engine.h"
#include <iostream>
#include <chrono>

// Custom scene for the editor test
class EditorTestScene : public Blacksite::Scene {
public:
    EditorTestScene(const std::string& name) : Scene(name) {}

    void OnEnter() override {
        BS_INFO(Blacksite::LogCategory::EDITOR, "Entering Editor Test Scene...");
        SetupTestScene();
    }

    void OnExit() override {
        BS_INFO(Blacksite::LogCategory::EDITOR, "Exiting Editor Test Scene...");
    }

private:
    void SetupTestScene() {
        // Add some test entities for the editor to work with
        int cube = SpawnCube(glm::vec3(0, 0, 0), "basic", glm::vec3(1, 0, 0));
        int sphere = SpawnSphere(glm::vec3(2, 0, 0), "basic", glm::vec3(0, 1, 0));
        int plane = SpawnPlane(glm::vec3(0, -1, 0), glm::vec3(10, 1, 10), "basic", glm::vec3(0.5f, 0.5f, 0.5f));

        // Make the plane static for physics
        GetEntity(plane).MakeStatic();

        // Set up camera
        SetCameraPosition(glm::vec3(0, 2, 5));
        SetCameraTarget(glm::vec3(0, 0, 0));

        BS_INFO(Blacksite::LogCategory::EDITOR, "Editor test scene setup complete with 3 entities");
    }
};

int main() {
    // Initialize logging first
    Blacksite::Logger::Instance().Initialize();

    std::cout << "Starting Blacksite Editor with Scene System..." << std::endl;
    BS_INFO(Blacksite::LogCategory::EDITOR, "🎨 Blacksite Editor - Scene System Integration");

    // Initialize engine
    Blacksite::Engine engine;
    if (!engine.Initialize(1920, 1080, "Blacksite Editor - Scene System")) {
        std::cerr << "Failed to initialize Blacksite Engine!" << std::endl;
        BS_ERROR(Blacksite::LogCategory::EDITOR, "Engine initialization failed");
        return -1;
    }

    // Get GLFW window from engine
    GLFWwindow* window = engine.GetGLFWWindow();
    if (!window) {
        std::cerr << "Failed to get GLFW window!" << std::endl;
        BS_ERROR(Blacksite::LogCategory::EDITOR, "Failed to get GLFW window");
        return -1;
    }

    // Create and setup initial scene
    auto editorScene = engine.CreateScene<EditorTestScene>("EditorTest");
    if (!editorScene) {
        std::cerr << "Failed to create editor test scene!" << std::endl;
        BS_ERROR(Blacksite::LogCategory::EDITOR, "Failed to create editor test scene");
        return -1;
    }

    // Switch to the editor scene
    if (!engine.SwitchToScene("EditorTest")) {
        std::cerr << "Failed to switch to editor test scene!" << std::endl;
        BS_ERROR(Blacksite::LogCategory::EDITOR, "Failed to switch to editor test scene");
        return -1;
    }

    // Create additional scenes for testing scene management
    auto gameScene = engine.CreateScene("GameLevel");
    if (gameScene) {
        // Add some entities to the game scene
        gameScene->SpawnCube(glm::vec3(-2, 1, -2), "glow", glm::vec3(0, 0, 1));
        gameScene->SpawnSphere(glm::vec3(2, 1, -2), "rainbow");
        gameScene->SpawnPlane(glm::vec3(0, -1, -2), glm::vec3(8, 1, 8), "basic", glm::vec3(0.3f, 0.3f, 0.3f));
        gameScene->SetCameraPosition(glm::vec3(0, 3, 3));
        gameScene->SetCameraTarget(glm::vec3(0, 0, -2));
        BS_INFO(Blacksite::LogCategory::EDITOR, "Created GameLevel scene with test entities");
    }

    auto menuScene = engine.CreateScene("MainMenu");
    if (menuScene) {
        // Add a simple menu scene setup
        menuScene->SpawnCube(glm::vec3(0, 0, -3), "holo", glm::vec3(1, 1, 0));
        menuScene->SetCameraPosition(glm::vec3(0, 0, 0));
        menuScene->SetCameraTarget(glm::vec3(0, 0, -1));
        BS_INFO(Blacksite::LogCategory::EDITOR, "Created MainMenu scene");
    }

    // Initialize editor
    BlacksiteEditor::Editor editor;
    if (!editor.Initialize(&engine, window)) {
        std::cerr << "Failed to initialize editor!" << std::endl;
        BS_ERROR(Blacksite::LogCategory::EDITOR, "Editor initialization failed");
        return -1;
    }

    std::cout << "Editor initialized successfully!" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  F1 - Toggle editor UI" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << "  Use Scene menu to switch between scenes" << std::endl;
    std::cout << "  Available scenes: EditorTest, GameLevel, MainMenu" << std::endl;

    BS_INFO(Blacksite::LogCategory::EDITOR, "Editor initialized successfully!");
    BS_INFO(Blacksite::LogCategory::EDITOR, "Available scenes:");
    auto sceneNames = engine.GetSceneSystem()->GetSceneNames();
    for (const auto& name : sceneNames) {
        BS_INFO_F(Blacksite::LogCategory::EDITOR, "  - %s", name.c_str());
    }

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
            BS_INFO(Blacksite::LogCategory::EDITOR, "Exit requested via ESC key");
        }

        // Handle F1 for editor toggle (backup - editor also handles this)
        static bool f1Pressed = false;
        bool f1Current = glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS;
        if (f1Current && !f1Pressed) {
            editor.ToggleEnabled();
            BS_INFO_F(Blacksite::LogCategory::EDITOR, "Editor UI %s", editor.IsEnabled() ? "enabled" : "disabled");
        }
        f1Pressed = f1Current;

        // Editor updates first
        editor.Update(deltaTime);

        engine.UpdateFrame(deltaTime);  // Updates physics, scenes, callbacks
        engine.RenderFrame();           // Renders active scene

        // Editor renders UI on top
        editor.Render();

        glfwSwapBuffers(window);

        // Log scene switches for debugging
        static std::string lastActiveScene;
        auto* activeScene = engine.GetActiveScene();
        if (activeScene) {
            std::string currentScene = activeScene->GetName();
            if (currentScene != lastActiveScene) {
                BS_INFO_F(Blacksite::LogCategory::EDITOR, "Active scene changed to: %s", currentScene.c_str());
                lastActiveScene = currentScene;
            }
        }
    }

    // Cleanup
    std::cout << "Shutting down editor..." << std::endl;
    BS_INFO(Blacksite::LogCategory::EDITOR, "Shutting down editor...");

    editor.Shutdown();

    BS_INFO(Blacksite::LogCategory::EDITOR, "Editor shutdown complete");
    Blacksite::Logger::Instance().Shutdown();

    return 0;
}
