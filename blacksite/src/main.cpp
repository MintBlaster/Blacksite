#include <memory>
#include <cstdlib>
#include <ctime>
#include "blacksite/Blacksite.h"

using namespace Blacksite;

class SimpleTest {
public:
    SimpleTest(Engine& engine) : m_engine(engine) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
    }

    void Initialize() {
        CreateTestScene();
        SetupCallbacks();
    }

    void CreateTestScene() {
        BS_INFO(LogCategory::GAME, "Setting up enhanced shader test scene...");

        // Create main showcase objects with different shaders
        int rainbowCube = m_engine.SpawnCube({0, 0, -5}, "rainbow");
        m_engine.GetEntity(rainbowCube).Scale(2.0f);

        int glowCube = m_engine.SpawnCube({4, 0, -5}, "glow", {1.0f, 0.0f, 0.0f});
        m_engine.GetEntity(glowCube).Scale(1.5f);

        int holoCube = m_engine.SpawnCube({0, 4, -5}, "holo", {0.0f, 1.0f, 1.0f});
        m_engine.GetEntity(holoCube).Scale(1.5f);

        int plasmaSphere = m_engine.SpawnSphere({-4, 0, -5}, "plasma");
        m_engine.GetEntity(plasmaSphere).Scale(1.5f);

        int wireframeCube = m_engine.SpawnCube({-4, 4, -5}, "wireframe", {1.0f, 1.0f, 0.0f});
        m_engine.GetEntity(wireframeCube).Scale(1.2f);

        // Basic ground plane
        int ground = m_engine.SpawnPlane({0, -5, -5}, {20, 0.1f, 20}, "basic", {0.3f, 0.3f, 0.3f});
        m_engine.GetEntity(ground).MakeStatic();

        // Create shader showcase area
        CreateShaderShowcase();

        m_engine.SetCameraPosition({0, 0, 0});
        m_engine.SetCameraTarget({0, 0, -1});

        SpawnShaderRain(150);

        BS_INFO(LogCategory::GAME, "Enhanced shader scene ready. Prepare for visual overload!");
    }

    void CreateShaderShowcase() {
        // Create a grid of objects showcasing different shaders
        std::vector<std::string> shaders = {"basic", "rainbow", "glow", "holo", "wireframe", "plasma"};
        std::vector<glm::vec3> colors = {
            {1.0f, 0.0f, 0.0f},  // Red
            {0.0f, 1.0f, 0.0f},  // Green
            {0.0f, 0.0f, 1.0f},  // Blue
            {1.0f, 1.0f, 0.0f},  // Yellow
            {1.0f, 0.0f, 1.0f},  // Magenta
            {0.0f, 1.0f, 1.0f}   // Cyan
        };

        for (int i = 0; i < 6; ++i) {
            glm::vec3 pos = {-10.0f + i * 4.0f, 0, -15};

            // Cubes on top
            int cube = m_engine.SpawnCube(pos + glm::vec3(0, 2, 0), shaders[i], colors[i]);
            m_engine.GetEntity(cube).Scale(0.8f);

            // Spheres on bottom
            int sphere = m_engine.SpawnSphere(pos + glm::vec3(0, -2, 0), shaders[i], colors[i]);
            m_engine.GetEntity(sphere).Scale(0.8f);
        }

        BS_INFO(LogCategory::GAME, "Shader showcase created with 6 different effects");
    }

    void SpawnShaderRain(int count = 100) {
        std::vector<std::string> shaders = {"basic", "rainbow", "glow", "holo", "plasma"};

        for (int i = 0; i < count; ++i) {
            glm::vec3 pos = {
                -15.f + static_cast<float>(std::rand() % 300) / 10.f,
                10.f + static_cast<float>(std::rand() % 100) / 10.f,
                -20.f + static_cast<float>(std::rand() % 300) / 10.f
            };

            // Random shader selection
            std::string shader = shaders[std::rand() % shaders.size()];

            // Random color
            glm::vec3 color = {
                static_cast<float>(std::rand() % 100) / 100.f,
                static_cast<float>(std::rand() % 100) / 100.f,
                static_cast<float>(std::rand() % 100) / 100.f
            };

            float scale = 0.2f + static_cast<float>(std::rand() % 30) / 100.f;

            // Mix of cubes and spheres
            if (std::rand() % 2 == 0) {
                int cube = m_engine.SpawnCube(pos, shader, color);
                m_engine.GetEntity(cube).Scale(scale);
            } else {
                int sphere = m_engine.SpawnSphere(pos, shader, color);
                m_engine.GetEntity(sphere).Scale(scale);
            }
        }
    }

    void SpawnShaderExplosion() {
        std::vector<std::string> explosionShaders = {"glow", "plasma", "rainbow", "holo"};

        for (int i = 0; i < 25; ++i) {
            glm::vec3 pos = {0, 2, -5};
            glm::vec3 offset = {
                (std::rand() % 400 - 200) / 100.f,
                (std::rand() % 400 - 200) / 100.f,
                (std::rand() % 400 - 200) / 100.f
            };

            std::string shader = explosionShaders[std::rand() % explosionShaders.size()];
            glm::vec3 color = {
                0.8f + static_cast<float>(std::rand() % 20) / 100.f,
                0.3f + static_cast<float>(std::rand() % 40) / 100.f,
                0.1f + static_cast<float>(std::rand() % 20) / 100.f
            };

            int cube = m_engine.SpawnCube(pos + offset, shader, color);
            m_engine.GetEntity(cube).Scale(0.3f + static_cast<float>(std::rand() % 40) / 100.f);
        }

        BS_INFO(LogCategory::GAME, "💥 Shader explosion! Multiple effects spawned.");
    }

    void SpawnShaderWave() {
        std::vector<std::string> waveShaders = {"rainbow", "plasma", "glow"};

        for (int i = 0; i < 15; ++i) {
            float angle = (i / 15.0f) * 2.0f * 3.14159f;
            float radius = 3.0f + i * 0.5f;

            glm::vec3 pos = {
                radius * std::cos(angle),
                1.0f + i * 0.2f,
                -5.0f + radius * std::sin(angle)
            };

            std::string shader = waveShaders[i % waveShaders.size()];
            glm::vec3 color = {
                0.5f + 0.5f * std::sin(i * 0.5f),
                0.5f + 0.5f * std::cos(i * 0.7f),
                0.5f + 0.5f * std::sin(i * 0.3f)
            };

            int sphere = m_engine.SpawnSphere(pos, shader, color);
            m_engine.GetEntity(sphere).Scale(0.5f);
        }

        BS_INFO(LogCategory::GAME, "🌊 Shader wave created in spiral pattern!");
    }

    void SetupCallbacks() {
        m_engine.SetUpdateCallback([this](Engine& engine, float dt) {
            Update(dt);
        });
    }

    void Update(float deltaTime) {
        m_time += deltaTime;

        static float debugTimer = 0;
        debugTimer += deltaTime;
        if (debugTimer > 2.0f) {
            auto pos = m_engine.GetCameraPosition();
            auto tgt = m_engine.GetCameraTarget();
            BS_INFO_F(LogCategory::GAME, "Camera: (%.1f, %.1f, %.1f) -> (%.1f, %.1f, %.1f)",
                      pos.x, pos.y, pos.z, tgt.x, tgt.y, tgt.z);
            debugTimer = 0;
        }

        if (auto* input = m_engine.GetInputSystem()) {
            auto camPos = m_engine.GetCameraPosition();
            float speed = 5.0f * deltaTime;

            // Camera movement
            if (input->IsKeyPressed(GLFW_KEY_W)) camPos.z -= speed;
            if (input->IsKeyPressed(GLFW_KEY_S)) camPos.z += speed;
            if (input->IsKeyPressed(GLFW_KEY_A)) camPos.x -= speed;
            if (input->IsKeyPressed(GLFW_KEY_D)) camPos.x += speed;
            if (input->IsKeyPressed(GLFW_KEY_Q)) camPos.y -= speed;
            if (input->IsKeyPressed(GLFW_KEY_E)) camPos.y += speed;
            m_engine.SetCameraPosition(camPos);

            // Camera presets
            if (input->IsKeyJustPressed(GLFW_KEY_R)) {
                m_engine.SetCameraPosition({0, 0, 0});
                m_engine.SetCameraTarget({0, 0, -1});
                BS_INFO(LogCategory::GAME, "Camera reset.");
            }

            // Shader effects spawning
            if (input->IsKeyJustPressed(GLFW_KEY_X)) {
                SpawnShaderExplosion();
            }

            if (input->IsKeyJustPressed(GLFW_KEY_Z)) {
                SpawnShaderWave();
            }

            if (input->IsKeyJustPressed(GLFW_KEY_C)) {
                SpawnShaderRain(50);
                BS_INFO(LogCategory::GAME, "More shader rain spawned!");
            }

            // Camera views
            if (input->IsKeyJustPressed(GLFW_KEY_1)) {
                m_engine.SetCameraPosition({0, 0, 0});
                m_engine.SetCameraTarget({0, 0, -1});
            }
            if (input->IsKeyJustPressed(GLFW_KEY_2)) {
                m_engine.SetCameraPosition({0, 8, 5});
                m_engine.SetCameraTarget({0, 0, -10});
            }
            if (input->IsKeyJustPressed(GLFW_KEY_3)) {
                m_engine.SetCameraPosition({15, 5, 0});
                m_engine.SetCameraTarget({0, 0, -10});
            }
            if (input->IsKeyJustPressed(GLFW_KEY_4)) {
                m_engine.SetCameraPosition({0, 15, -5});
                m_engine.SetCameraTarget({0, 0, -10});
            }

            // Orbit camera
            if (input->IsKeyPressed(GLFW_KEY_O)) {
                float orbitRadius = 12.0f;
                float speed = 0.3f;
                float angle = m_time * speed;
                glm::vec3 pos = {
                    orbitRadius * std::cos(angle),
                    5.0f + 3.0f * std::sin(m_time * 0.2f),
                    orbitRadius * std::sin(angle) - 8.0f
                };
                m_engine.SetCameraPosition(pos);
                m_engine.SetCameraTarget({0, 0, -8});
            }
        }
    }

private:
    Engine& m_engine;
    float m_time = 0.0f;
};

int main() {
    Logger::Instance().Initialize();
    BS_INFO(LogCategory::CORE, "🌈 Blacksite Engine - Enhanced Shader Test Scene");

    Engine engine;
    if (!engine.Initialize(1920, 1080, "Enhanced Shader Chaos")) {
        BS_ERROR(LogCategory::CORE, "Engine initialization failed");
        return -1;
    }

    SimpleTest test(engine);
    test.Initialize();

    BS_INFO(LogCategory::CORE, "Enhanced shader scene initialized. Controls:");
    BS_INFO(LogCategory::CORE, "  WASD + QE = move camera");
    BS_INFO(LogCategory::CORE, "  R = reset camera");
    BS_INFO(LogCategory::CORE, "  1/2/3/4 = preset views");
    BS_INFO(LogCategory::CORE, "  X = shader explosion (mixed effects)");
    BS_INFO(LogCategory::CORE, "  Z = shader wave (spiral pattern)");
    BS_INFO(LogCategory::CORE, "  C = more shader rain");
    BS_INFO(LogCategory::CORE, "  O (hold) = orbit camera with vertical motion");
    BS_INFO(LogCategory::CORE, "  Enjoy the shader showcase!");

    int result = engine.Run();

    BS_INFO(LogCategory::CORE, "Shutdown complete.");
    Logger::Instance().Shutdown();

    return result;
}
