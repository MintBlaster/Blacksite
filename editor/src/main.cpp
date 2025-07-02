#include "Editor.h"
#include "blacksite/core/Engine.h"
#include <iostream>
#include <chrono>
#include <random>

class EditorTestScene : public Blacksite::Scene {
public:
    EditorTestScene(const std::string& name) : Scene(name) {}

    void OnEnter() override {
        BS_INFO(Blacksite::LogCategory::EDITOR, "🧪 Entering Arena Testbed Scene...");
        SetupTestScene();
    }

    void OnExit() override {
        BS_INFO(Blacksite::LogCategory::EDITOR, "👋 Exiting Arena Testbed Scene...");
    }

    void TriggerCubeRain() { CreateCubeRain(); }
    void TriggerSphereRain() { CreateSphereRain(); }
    void TriggerCubeStack() { CreateCubeStack(); }
    void TriggerMixedChaos() { CreateMixedChaos(); }
    void ClearDynamicObjects() { ClearAllDynamic(); }

private:
    std::mt19937 rng{std::random_device{}()};

    float RandomFloat(float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(rng);
    }

    int RandomInt(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(rng);
    }

    void CreateCubeRain() {
        BS_INFO(Blacksite::LogCategory::EDITOR, "🌧️ Cube Rain Triggered!");
        for (int i = 0; i < 12; i++) {
            glm::vec3 pos(RandomFloat(-10.f, 10.f), RandomFloat(18.f, 25.f), RandomFloat(-10.f, 8.f));
            glm::vec3 color(RandomFloat(0.3f, 1.0f), RandomFloat(0.3f, 1.0f), RandomFloat(0.3f, 1.0f));
            std::vector<std::string> shaders = {"basic", "glow", "holo", "rainbow", "plasma"};
            std::string shader = shaders[RandomInt(0, shaders.size() - 1)];
            int cube = SpawnCube(pos, shader, color);
            float scale = RandomFloat(0.4f, 1.2f);
            GetEntity(cube).Scale(scale).MakeDynamic().SetVelocity(glm::vec3(
                RandomFloat(-1.5f, 1.5f), RandomFloat(-0.8f, 0.2f), RandomFloat(-1.5f, 1.5f)));
        }
    }

    void CreateSphereRain() {
        BS_INFO(Blacksite::LogCategory::EDITOR, "⚽ Sphere Rain Triggered!");
        for (int i = 0; i < 10; i++) {
            glm::vec3 pos(RandomFloat(-10.f, 10.f), RandomFloat(20.f, 28.f), RandomFloat(-10.f, 8.f));
            glm::vec3 color(RandomFloat(0.4f, 1.0f), RandomFloat(0.4f, 1.0f), RandomFloat(0.4f, 1.0f));
            std::vector<std::string> shaders = {"basic", "glow", "holo", "rainbow"};
            std::string shader = shaders[RandomInt(0, shaders.size() - 1)];
            int sphere = SpawnSphere(pos, shader, color);
            float scale = RandomFloat(0.5f, 1.8f);
            GetEntity(sphere).Scale(scale).MakeDynamic().SetVelocity(glm::vec3(
                RandomFloat(-1.2f, 1.2f), RandomFloat(-0.6f, 0.3f), RandomFloat(-1.2f, 1.2f)));
        }
    }

    void CreateCubeStack() {
        BS_INFO(Blacksite::LogCategory::EDITOR, "🏗️ Cube Stack Triggered!");
        glm::vec3 basePos(RandomFloat(-6.f, 6.f), -2, RandomFloat(-6.f, 4.f));
        for (int i = 0; i < 8; i++) {
            glm::vec3 pos = basePos + glm::vec3(0, i * 1.05f, 0);
            glm::vec3 color = (i % 3 == 0) ? glm::vec3(0.8f, 0.2f, 0.2f) :
                             (i % 3 == 1) ? glm::vec3(0.2f, 0.8f, 0.2f) : glm::vec3(0.2f, 0.2f, 0.8f);
            std::string shader = (i % 2 == 0) ? "basic" : "glow";
            int cube = SpawnCube(pos, shader, color);
            float scale = 0.9f - (i * 0.03f);
            GetEntity(cube).Scale(scale, 0.9f, scale).MakeDynamic();
        }
    }

    void CreateMixedChaos() {
        BS_INFO(Blacksite::LogCategory::EDITOR, "💥 Mixed Chaos Triggered!");
        for (int i = 0; i < 15; i++) {
            glm::vec3 pos(RandomFloat(-10.f, 10.f), RandomFloat(18.f, 26.f), RandomFloat(-10.f, 8.f));
            glm::vec3 color(RandomFloat(0.6f, 1.0f), RandomFloat(0.6f, 1.0f), RandomFloat(0.6f, 1.0f));
            int entity = (i % 3 == 0) ? SpawnCube(pos, "plasma", color) :
                        (i % 3 == 1) ? SpawnSphere(pos, "rainbow", color) :
                                      SpawnCube(pos, "holo", color);

            float scale = RandomFloat(0.5f, 1.4f);
            if (i % 3 == 0)
                GetEntity(entity).Scale(scale, RandomFloat(0.6f, 1.3f), scale);
            else
                GetEntity(entity).Scale(scale);

            GetEntity(entity).MakeDynamic()
                .SetVelocity(glm::vec3(RandomFloat(-2.5f, 2.5f), RandomFloat(-1.2f, 0.8f), RandomFloat(-2.5f, 2.5f)))
                .SetAngularVelocity(glm::vec3(RandomFloat(-4.f, 4.f), RandomFloat(-4.f, 4.f), RandomFloat(-4.f, 4.f)));
        }
    }

    void ClearAllDynamic() {
        BS_INFO(Blacksite::LogCategory::EDITOR, "🧹 Clearing dynamic objects...");
        auto& entities = GetEntitySystem()->GetEntities();
        for (auto& e : entities) {
            if (e.active && e.hasPhysics && e.isDynamic)
                e.active = false;
        }
    }

    void SetupTestScene() {
        float arenaSize = 24.f;
        float wallHeight = 12.f;

        // Clean, neutral wall color
        glm::vec3 wallColor(0.15f, 0.15f, 0.18f);

        // Ground - larger and more centered
        int ground = SpawnPlane(glm::vec3(0, -3, 0), glm::vec3(arenaSize, 0.6f, arenaSize), "basic", glm::vec3(0.12f, 0.12f, 0.15f));

        // Three walls only - leaving the front open for viewing
        // Left wall
        int wallL = SpawnPlane(glm::vec3(-arenaSize / 2, wallHeight / 2 - 3, 0),
                              glm::vec3(0.8f, wallHeight, arenaSize), "basic", wallColor);

        // Right wall
        int wallR = SpawnPlane(glm::vec3(arenaSize / 2, wallHeight / 2 - 3, 0),
                              glm::vec3(0.8f, wallHeight, arenaSize), "basic", wallColor);

        // Back wall
        int wallB = SpawnPlane(glm::vec3(0, wallHeight / 2 - 3, -arenaSize / 2),
                              glm::vec3(arenaSize, wallHeight, 0.8f), "basic", wallColor);

        // Make all walls static
        GetEntity(ground).MakeStatic();
        GetEntity(wallL).MakeStatic();
        GetEntity(wallR).MakeStatic();
        GetEntity(wallB).MakeStatic();

        // Reference objects positioned for better visibility
        int ref1 = SpawnCube(glm::vec3(-6, 0.5f, -8), "basic", glm::vec3(0.9f, 0.3f, 0.3f));
        int ref2 = SpawnCube(glm::vec3(0, 0.5f, -8), "holo", glm::vec3(0.3f, 0.3f, 0.9f));
        int ref3 = SpawnCube(glm::vec3(6, 0.5f, -8), "rainbow", glm::vec3(0.9f, 0.9f, 0.3f));

        GetEntity(ref1).Scale(0.8f);
        GetEntity(ref2).Scale(1.2f);
        GetEntity(ref3).Scale(1.0f, 1.8f, 1.0f);

        // Optimal camera position - elevated and angled for cinematic view
        SetCameraPosition(glm::vec3(18.f, 12.f, 20.f));
        SetCameraTarget(glm::vec3(0, 3.f, -2.f));

        BS_INFO(Blacksite::LogCategory::EDITOR, "🎯 Three-Wall Arena Ready | Size: 24x24 | Height: 12 | Optimal Viewing Angle Set");
    }
};

int main() {
    Blacksite::Logger::Instance().Initialize();

    std::cout << "🚀 Starting Blacksite Editor - Cinematic Arena Testbed\n";
    BS_INFO(Blacksite::LogCategory::EDITOR, "🎮 Blacksite Arena Test - Enhanced View Mode");

    Blacksite::Engine engine;
    if (!engine.Initialize(1920, 1080, "Blacksite Editor - Cinematic Arena")) return -1;

    GLFWwindow* window = engine.GetGLFWWindow();
    if (!window) return -1;

    auto editorScene = engine.CreateScene<EditorTestScene>("ArenaTest");
    if (!editorScene || !engine.SwitchToScene("ArenaTest")) return -1;

    BlacksiteEditor::Editor editor;
    if (!editor.Initialize(&engine, window)) return -1;

    std::cout << "🎛️ Controls: [F1] Toggle Editor | [1-5] Trigger Effects | [ESC] Exit\n";
    std::cout << "   [1] Cube Rain | [2] Sphere Rain | [3] Cube Stack | [4] Mixed Chaos | [5] Clear All\n";

    auto lastTime = std::chrono::high_resolution_clock::now();
    while (!glfwWindowShouldClose(window)) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);

        static bool f1Pressed = false;
        bool f1Current = glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS;
        if (f1Current && !f1Pressed) editor.ToggleEnabled();
        f1Pressed = f1Current;

        static bool key1, key2, key3, key4, key5;
        bool cur1 = glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;
        bool cur2 = glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS;
        bool cur3 = glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS;
        bool cur4 = glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS;
        bool cur5 = glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS;

        if (cur1 && !key1) editorScene->TriggerCubeRain(); key1 = cur1;
        if (cur2 && !key2) editorScene->TriggerSphereRain(); key2 = cur2;
        if (cur3 && !key3) editorScene->TriggerCubeStack(); key3 = cur3;
        if (cur4 && !key4) editorScene->TriggerMixedChaos(); key4 = cur4;
        if (cur5 && !key5) editorScene->ClearDynamicObjects(); key5 = cur5;

        editor.Update(deltaTime);
        engine.UpdateFrame(deltaTime);
        engine.RenderFrame();
        editor.Render();
        glfwSwapBuffers(window);
    }

    editor.Shutdown();
    return 0;
}
