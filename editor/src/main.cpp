#include "Editor.h"
#include "blacksite/core/Engine.h"
#include <iostream>
#include <chrono>

class BasicPostProcessTest : public Blacksite::Scene {
public:
    BasicPostProcessTest(const std::string& name) : Scene(name) {}

    void OnEnter() override {
        BS_INFO(Blacksite::LogCategory::GAME, "🔬 Basic Post-Processing Test Scene");
        SetupBasicScene();
    }

    void OnExit() override {
        BS_INFO(Blacksite::LogCategory::GAME, "Exiting basic test scene");
    }

    void Update(float deltaTime) override {
        Scene::Update(deltaTime);
        // No input handling here - it's done in main loop like physics playground
    }

    void HandleCameraInput(GLFWwindow* window, float deltaTime) {
        // Camera controls
        auto camPos = GetCameraPosition();
        float speed = 5.0f * deltaTime;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camPos.z -= speed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camPos.z += speed;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camPos.x -= speed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camPos.x += speed;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) camPos.y -= speed;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) camPos.y += speed;

        SetCameraPosition(camPos);
    }

private:
    void SetupBasicScene() {
        // Ground plane
        int ground = SpawnPlane({0, -2, 0}, {20, 0.3f, 20}, "basic", {0.2f, 0.2f, 0.2f});
        GetEntity(ground).MakeStatic();

        // VERY bright cube for bloom testing (should glow RED)
        int brightRed = SpawnCube({0, 0, -5}, "basic", {10.0f, 2.0f, 2.0f});
        GetEntity(brightRed).Scale(1.5f);

        // Normal cube for comparison (should NOT glow)
        int normalCube = SpawnCube({3, 0, -5}, "basic", {0.5f, 0.5f, 0.5f});
        GetEntity(normalCube).Scale(1.0f);

        // Bright GREEN cube (should glow)
        int brightGreen = SpawnCube({-3, 0, -5}, "basic", {2.0f, 10.0f, 2.0f});
        GetEntity(brightGreen).Scale(1.2f);

        // Bright BLUE cube (should glow)
        int brightBlue = SpawnCube({0, 3, -5}, "basic", {2.0f, 2.0f, 10.0f});
        GetEntity(brightBlue).Scale(1.0f);

        // Bright YELLOW cube (should glow)
        int brightYellow = SpawnCube({-3, 3, -5}, "basic", {8.0f, 8.0f, 2.0f});
        GetEntity(brightYellow).Scale(0.8f);

        // Set camera position - higher and farther back, no x offset
        SetCameraPosition({0, 8, 10});
        SetCameraTarget({0, 0, -5});

        BS_INFO(Blacksite::LogCategory::GAME, "✨ Basic scene created for post-processing test");
        BS_INFO(Blacksite::LogCategory::GAME, "🔴 RED cube should glow if bloom is working");
        BS_INFO(Blacksite::LogCategory::GAME, "🟢 GREEN cube should glow if bloom is working");
        BS_INFO(Blacksite::LogCategory::GAME, "🔵 BLUE cube should glow if bloom is working");
        BS_INFO(Blacksite::LogCategory::GAME, "🟡 YELLOW cube should glow if bloom is working");
        BS_INFO(Blacksite::LogCategory::GAME, "⚫ GRAY cube should NOT glow (normal brightness)");
    }
};

int main() {
    Blacksite::Logger::Instance().Initialize();

    std::cout << "🔬 Post-Processing Test - No Projectiles\n";
    BS_INFO(Blacksite::LogCategory::GAME, "🚀 Starting Post-Processing Test");

    Blacksite::Engine engine;
    if (!engine.Initialize(1920, 1080, "Post-Processing Test")) return -1;

    // Enable post-processing with strong bloom settings
    auto* renderer = engine.GetRenderer();
    if (renderer && renderer->GetPostProcessManager()) {
        renderer->EnablePostProcessing(true);
        auto& settings = renderer->GetPostProcessManager()->GetSettings();
        settings.enableBloom = true;
        settings.enableToneMapping = true;
        settings.enableFXAA = true;
        settings.bloomThreshold = 1.0f;
        settings.bloomStrength = 1.0f;
        settings.exposure = 1.0f;
        settings.gamma = 2.2f;

        BS_INFO(Blacksite::LogCategory::GAME, "✅ Post-processing enabled with bloom");
        std::cout << "✅ Post-processing ON - Look for glow around bright cubes!\n";
    } else {
        BS_ERROR(Blacksite::LogCategory::GAME, "❌ Failed to enable post-processing");
    }

    GLFWwindow* window = engine.GetGLFWWindow();
    if (!window) return -1;

    // Create and switch to basic test scene
    auto testScene = engine.CreateScene<BasicPostProcessTest>("PostProcessTest");
    if (!testScene || !engine.SwitchToScene("PostProcessTest")) return -1;

    BlacksiteEditor::Editor editor;
    if (!editor.Initialize(&engine, window)) return -1;

    std::cout << "🔬 POST-PROCESSING TEST\n";
    std::cout << "🎮 Controls:\n";
    std::cout << "   [WASD + QE] Move Camera\n";
    std::cout << "   [F1] Toggle Editor\n";
    std::cout << "   [F6] Toggle Post-Processing ON/OFF\n";
    std::cout << "   [ESC] Exit\n";
    std::cout << "🔍 What to look for:\n";
    std::cout << "   - Bright colored cubes should have GLOW/BLOOM effect\n";
    std::cout << "   - Gray cube should look normal (no glow)\n";
    std::cout << "   - Press F6 to compare with/without post-processing\n";

    auto lastTime = std::chrono::high_resolution_clock::now();
    while (!glfwWindowShouldClose(window)) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        // Editor toggle
        static bool f1Pressed = false;
        bool f1Current = glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS;
        if (f1Current && !f1Pressed) {
            editor.ToggleEnabled();
        }
        f1Pressed = f1Current;

        // Post-processing toggle
        static bool f6Pressed = false;
        bool f6Current = glfwGetKey(window, GLFW_KEY_F6) == GLFW_PRESS;
        if (f6Current && !f6Pressed) {
            auto* renderer = engine.GetRenderer();
            bool currentState = renderer->IsPostProcessingEnabled();
            renderer->EnablePostProcessing(!currentState);
            BS_INFO_F(Blacksite::LogCategory::GAME, "📷 Post-processing: %s",
                      !currentState ? "ENABLED" : "DISABLED");
            std::cout << (!currentState ? "✅ Post-processing ON - Look for glow!\n" :
                                         "❌ Post-processing OFF - No glow\n");
        }
        f6Pressed = f6Current;

        // Camera controls (same pattern as physics playground)
        testScene->HandleCameraInput(window, deltaTime);

        editor.Update(deltaTime);
        engine.UpdateFrame(deltaTime);
        engine.RenderFrame();
        editor.Render();
        glfwSwapBuffers(window);
    }

    editor.Shutdown();
    BS_INFO(Blacksite::LogCategory::GAME, "🔬 Post-processing test completed");
    return 0;
}
