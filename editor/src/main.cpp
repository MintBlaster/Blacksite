#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Editor.h"
#include "blacksite/core/Engine.h"
#include <iostream>
#include <chrono>

class ShaderShowcaseScene : public Blacksite::Scene {
public:
    ShaderShowcaseScene(const std::string& name) : Scene(name) {}

    void OnEnter() override {
        BS_INFO(Blacksite::LogCategory::GAME, "Shader Showcase Scene loaded");
        SetupShowcaseScene();
    }

    void OnExit() override {
        BS_INFO(Blacksite::LogCategory::GAME, "Exiting shader showcase scene");
    }

    void Update(float deltaTime) override {
        Scene::Update(deltaTime);
        m_time += deltaTime;
        AnimateObjects();
    }

private:
    float m_time = 0.0f;
    std::vector<int> m_animatedObjects;

    void SetupShowcaseScene() {
        // Create ground platform
        int ground = SpawnPlane({0, -3, 0}, {30, 0.2f, 30}, "basic", {0.4f, 0.4f, 0.4f});
        GetEntity(ground).MakeStatic();

        // Shader test grid - different shaders in rows
        CreateShaderTestGrid();

        // Lighting test area
        CreateLightingTestArea();

        // Material test spheres
        CreateMaterialTestSpheres();

        // Transparency test objects
        CreateTransparencyTests();

        // Set camera for good overview
        SetCameraPosition({0, 12, 20});
        SetCameraTarget({0, 0, 0});

        BS_INFO(Blacksite::LogCategory::GAME, "Shader showcase scene created");
        BS_INFO(Blacksite::LogCategory::GAME, "Different shader types arranged in grid pattern");
    }

    void CreateShaderTestGrid() {
        std::vector<std::string> shaders = {"basic", "unlit", "debug", "wireframe"};
        std::vector<glm::vec3> colors = {
            {0.8f, 0.2f, 0.2f},  // Red
            {0.2f, 0.8f, 0.2f},  // Green
            {0.2f, 0.2f, 0.8f},  // Blue
            {0.8f, 0.8f, 0.2f},  // Yellow
            {0.8f, 0.2f, 0.8f},  // Magenta
            {0.2f, 0.8f, 0.8f}   // Cyan
        };

        // Create 4x6 grid of objects
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 6; ++col) {
                glm::vec3 pos = {
                    -12.5f + col * 5.0f,
                    0.0f,
                    -7.5f + row * 5.0f
                };

                std::string shader = shaders[row];
                glm::vec3 color = colors[col];

                // Alternate between cubes and spheres
                int entity;
                if ((row + col) % 2 == 0) {
                    entity = SpawnCube(pos, shader, color);
                } else {
                    entity = SpawnSphere(pos, shader, color);
                }

                GetEntity(entity).Scale(1.2f);

                // Store some for animation
                if (col < 2) {
                    m_animatedObjects.push_back(entity);
                }
            }
        }
    }

    void CreateLightingTestArea() {
        // Bright objects for HDR bloom testing
        int brightRed = SpawnCube({-15, 3, -15}, "unlit", {3.0f, 0.5f, 0.5f});
        GetEntity(brightRed).Scale(1.5f);

        int brightGreen = SpawnSphere({-10, 3, -15}, "unlit", {0.5f, 3.0f, 0.5f});
        GetEntity(brightGreen).Scale(1.5f);

        int brightBlue = SpawnCube({-5, 3, -15}, "unlit", {0.5f, 0.5f, 3.0f});
        GetEntity(brightBlue).Scale(1.5f);

        // Normal lighting comparison
        int normalCube = SpawnCube({0, 3, -15}, "basic", {0.7f, 0.7f, 0.7f});
        GetEntity(normalCube).Scale(1.5f);
    }

    void CreateMaterialTestSpheres() {
        // Row of spheres with different material properties
        std::vector<glm::vec3> materialColors = {
            {1.0f, 0.0f, 0.0f},  // Pure red
            {0.0f, 1.0f, 0.0f},  // Pure green
            {0.0f, 0.0f, 1.0f},  // Pure blue
            {1.0f, 1.0f, 1.0f},  // White
            {0.5f, 0.5f, 0.5f},  // Gray
            {0.1f, 0.1f, 0.1f}   // Dark
        };

        for (int i = 0; i < 6; ++i) {
            glm::vec3 pos = {-12.5f + i * 5.0f, 5, 15};
            int sphere = SpawnSphere(pos, "basic", materialColors[i]);
            GetEntity(sphere).Scale(1.8f);
        }
    }

    void CreateTransparencyTests() {
        // Test transparent objects if transparency works
        int transparentCube1 = SpawnCube({15, 2, -10}, "transparent", {1.0f, 0.0f, 0.0f});
        GetEntity(transparentCube1).Scale(2.0f);

        int transparentCube2 = SpawnCube({15, 2, -5}, "transparent", {0.0f, 1.0f, 0.0f});
        GetEntity(transparentCube2).Scale(2.0f);

        int transparentCube3 = SpawnCube({15, 2, 0}, "transparent", {0.0f, 0.0f, 1.0f});
        GetEntity(transparentCube3).Scale(2.0f);
    }

    void AnimateObjects() {
        // Simple rotation animation for some objects
        float rotationSpeed = 30.0f; // degrees per second

        for (int entityId : m_animatedObjects) {
            auto entity = GetEntity(entityId);
            entity.Rotate({0, rotationSpeed * m_time, 0});
        }
    }
};

int main() {
    Blacksite::Logger::Instance().Initialize();

    std::cout << "Shader Showcase Test\n";
    BS_INFO(Blacksite::LogCategory::GAME, "Starting Shader Showcase");

    Blacksite::Engine engine;
    if (!engine.Initialize(1920, 1080, "Shader Showcase")) return -1;

    auto* input = engine.GetInputSystem();
    if (!input) {
        BS_ERROR(Blacksite::LogCategory::GAME, "InputSystem not available");
        return -1;
    }

    // Setup post-processing
    auto* renderer = engine.GetRenderer();
    if (renderer && renderer->GetPostProcessManager()) {
        renderer->EnablePostProcessing(true);
        auto& settings = renderer->GetPostProcessManager()->GetSettings();
        settings.exposure = 0.3f;
        settings.bloomStrength = 0.4f;
        settings.bloomThreshold = 0.7f;
        settings.enableFXAA = true;
        BS_INFO(Blacksite::LogCategory::GAME, "Post-processing configured");
    }

    GLFWwindow* window = engine.GetGLFWWindow();
    if (!window) return -1;

    auto testScene = engine.CreateScene<ShaderShowcaseScene>("ShaderShowcase");
    if (!testScene || !engine.SwitchToScene("ShaderShowcase")) return -1;

    BlacksiteEditor::Editor editor;
    if (!editor.Initialize(&engine, window)) return -1;

    // Camera state
    glm::vec3 cameraPos = {0, 12, 20};
    float pitch = -15.0f;
    float yaw = -90.0f;
    float sensitivity = 0.1f;
    bool firstMouse = true;
    bool mouseLookEnabled = true;

    std::cout << "Controls:\n";
    std::cout << "  Mouse - Look around\n";
    std::cout << "  WASD+QE - Move camera\n";
    std::cout << "  Tab - Toggle mouse look\n";
    std::cout << "  F1 - Toggle editor\n";
    std::cout << "  F6 - Toggle post-processing\n";
    std::cout << "  R - Reset camera\n";
    std::cout << "  1-4 - Camera presets\n";
    std::cout << "  Shift/Ctrl - Speed modifiers\n";

    auto lastTime = std::chrono::high_resolution_clock::now();
    while (!glfwWindowShouldClose(window)) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        glfwPollEvents();

        input->SetEditorMode(editor.IsEnabled());

        if (input->KeyDown('q') && input->KeyDown(Blacksite::Key::Escape)) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        if (input->KeyPressed(Blacksite::Key::F1)) {
            editor.ToggleEnabled();
        }

        if (input->KeyPressed(Blacksite::Key::F6)) {
            bool currentState = renderer->IsPostProcessingEnabled();
            renderer->EnablePostProcessing(!currentState);
            BS_INFO_F(Blacksite::LogCategory::GAME, "Post-processing: %s",
                      !currentState ? "ON" : "OFF");
        }

        if (input->KeyPressed(Blacksite::Key::Tab)) {
            mouseLookEnabled = !mouseLookEnabled;
            BS_INFO_F(Blacksite::LogCategory::GAME, "Mouse look: %s",
                     mouseLookEnabled ? "ON" : "OFF");
        }

        // Mouse look
        if (mouseLookEnabled && input->ShouldCaptureMouse()) {
            // Only rotate camera when right mouse button is held down
            if (input->MouseDown(Blacksite::Mouse::Right)) {
                glm::vec2 mouseDelta = input->MouseDelta();
                if (glm::length(mouseDelta) > 0.1f && !firstMouse) {
                    yaw += mouseDelta.x * sensitivity;
                    pitch -= mouseDelta.y * sensitivity;
                    pitch = glm::clamp(pitch, -89.0f, 89.0f);
                } else if (firstMouse) {
                    firstMouse = false;
                }
            } else {
                // Reset first mouse when not holding button
                firstMouse = true;
            }
        }

        // Camera movement using new input system
        glm::vec3 movement = input->Movement3D();
        float speed = input->GetSpeedMultiplier() * 8.0f * deltaTime;

        // Calculate camera vectors
        glm::vec3 forward;
        forward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        forward.y = sin(glm::radians(pitch));
        forward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        forward = normalize(forward);

        glm::vec3 right = normalize(cross(forward, glm::vec3(0, 1, 0)));
        glm::vec3 up = cross(right, forward);

        // Apply movement
        cameraPos += right * movement.x * speed;
        cameraPos += up * movement.y * speed;
        cameraPos += forward * movement.z * speed;

        // Update camera
        testScene->SetCameraPosition(cameraPos);
        testScene->SetCameraTarget(cameraPos + forward);

        // Camera presets
        if (input->KeyPressed(Blacksite::Key::Num1)) {
            cameraPos = {0, 12, 20}; pitch = -15.0f; yaw = -90.0f;
        }
        if (input->KeyPressed(Blacksite::Key::Num2)) {
            cameraPos = {25, 8, 0}; pitch = -10.0f; yaw = -180.0f;
        }
        if (input->KeyPressed(Blacksite::Key::Num3)) {
            cameraPos = {0, 25, 0}; pitch = -70.0f; yaw = -90.0f;
        }
        if (input->KeyPressed(Blacksite::Key::Num4)) {
            cameraPos = {-20, 15, 15}; pitch = -20.0f; yaw = -45.0f;
        }

        if (input->KeyPressed('r')) {
            cameraPos = {0, 12, 20};
            pitch = -15.0f;
            yaw = -90.0f;
            firstMouse = true;
        }

        editor.Update(deltaTime);
        engine.UpdateFrame(deltaTime);
        engine.RenderFrame();
        editor.Render();
        glfwSwapBuffers(window);
    }

    editor.Shutdown();
    BS_INFO(Blacksite::LogCategory::GAME, "Shader showcase completed");
    return 0;
}
