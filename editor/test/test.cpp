// #define GLFW_INCLUDE_NONE
// #include <GL/glew.h>
// #include <GLFW/glfw3.h>

// #include <blacksite/app/Application.h>
// #include <blacksite/scene/Scene.h>
// #include <blacksite/core/Logger.h>
// #include <iostream>
// #include <chrono>
// #include <vector>

// class ShaderShowcaseScene : public Blacksite::Scene {
// public:
//     ShaderShowcaseScene(const std::string& name) : Scene(name) {}

//     void OnEnter() override {
//         BS_INFO(Blacksite::LogCategory::GAME, "Shader Showcase Scene loaded");
//         SetupShowcaseScene();
//     }

//     void OnExit() override {
//         BS_INFO(Blacksite::LogCategory::GAME, "Exiting shader showcase scene");
//     }

//     void Update(float deltaTime) override {
//         Scene::Update(deltaTime);
//         m_time += deltaTime;
//         AnimateObjects();
//     }

// private:
//     float m_time = 0.0f;
//     std::vector<int> m_animatedObjects;

//     void SetupShowcaseScene() {
//         // Create ground platform
//         int ground = SpawnPlane({0, -3, 0}, {30, 0.2f, 30}, "basic", {0.4f, 0.4f, 0.4f});
//         GetEntity(ground).MakeStatic();

//         CreateShaderTestGrid();
//         CreateLightingTestArea();
//         CreateMaterialTestSpheres();
//         CreateTransparencyTests();

//         SetCameraPosition({0, 12, 20});
//         SetCameraTarget({0, 0, 0});

//         BS_INFO(Blacksite::LogCategory::GAME, "Shader showcase scene created");
//     }

//     void CreateShaderTestGrid() {
//         std::vector<std::string> shaders = {"basic", "unlit", "debug", "wireframe"};
//         std::vector<glm::vec3> colors = {
//             {0.8f, 0.2f, 0.2f}, {0.2f, 0.8f, 0.2f}, {0.2f, 0.2f, 0.8f},
//             {0.8f, 0.8f, 0.2f}, {0.8f, 0.2f, 0.8f}, {0.2f, 0.8f, 0.8f}
//         };

//         for (int row = 0; row < 4; ++row) {
//             for (int col = 0; col < 6; ++col) {
//                 glm::vec3 pos = {-12.5f + col * 5.0f, 0.0f, -7.5f + row * 5.0f};
//                 std::string shader = shaders[row];
//                 glm::vec3 color = colors[col];

//                 int entity;
//                 if ((row + col) % 2 == 0) {
//                     entity = SpawnCube(pos, shader, color);
//                 } else {
//                     entity = SpawnSphere(pos, shader, color);
//                 }

//                 GetEntity(entity).Scale(1.2f);

//                 if (col < 2) {
//                     m_animatedObjects.push_back(entity);
//                 }
//             }
//         }
//     }

//     void CreateLightingTestArea() {
//         int brightRed = SpawnCube({-15, 3, -15}, "unlit", {3.0f, 0.5f, 0.5f});
//         GetEntity(brightRed).Scale(1.5f);

//         int brightGreen = SpawnSphere({-10, 3, -15}, "unlit", {0.5f, 3.0f, 0.5f});
//         GetEntity(brightGreen).Scale(1.5f);

//         int brightBlue = SpawnCube({-5, 3, -15}, "unlit", {0.5f, 0.5f, 3.0f});
//         GetEntity(brightBlue).Scale(1.5f);

//         int normalCube = SpawnCube({0, 3, -15}, "basic", {0.7f, 0.7f, 0.7f});
//         GetEntity(normalCube).Scale(1.5f);
//     }

//     void CreateMaterialTestSpheres() {
//         std::vector<glm::vec3> materialColors = {
//             {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f},
//             {1.0f, 1.0f, 1.0f}, {0.5f, 0.5f, 0.5f}, {0.1f, 0.1f, 0.1f}
//         };

//         for (int i = 0; i < 6; ++i) {
//             glm::vec3 pos = {-12.5f + i * 5.0f, 5, 15};
//             int sphere = SpawnSphere(pos, "basic", materialColors[i]);
//             GetEntity(sphere).Scale(1.8f);
//         }
//     }

//     void CreateTransparencyTests() {
//         int transparentCube1 = SpawnCube({15, 2, -10}, "transparent", {1.0f, 0.0f, 0.0f});
//         GetEntity(transparentCube1).Scale(2.0f);

//         int transparentCube2 = SpawnCube({15, 2, -5}, "transparent", {0.0f, 1.0f, 0.0f});
//         GetEntity(transparentCube2).Scale(2.0f);

//         int transparentCube3 = SpawnCube({15, 2, 0}, "transparent", {0.0f, 0.0f, 1.0f});
//         GetEntity(transparentCube3).Scale(2.0f);
//     }

//     void AnimateObjects() {
//         float rotationSpeed = 30.0f;
//         for (int entityId : m_animatedObjects) {
//             auto entity = GetEntity(entityId);
//             entity.Rotate({0, rotationSpeed * m_time, 0});
//         }
//     }
// };

// class ShaderShowcaseApp : public Blacksite::Application {
// public:
//     ShaderShowcaseApp() : Application("Shader Showcase Game", 1920, 1080) {}

// protected:
//     void OnInitialize() override {
//         SetupPostProcessing();

//         auto scene = GetEngine().CreateScene<ShaderShowcaseScene>("ShaderShowcase");
//         GetEngine().SwitchToScene("ShaderShowcase");

//         std::cout << "Game Controls:\n";
//         std::cout << "  Mouse + Right Click - Look around\n";
//         std::cout << "  WASD+QE - Move camera\n";
//         std::cout << "  Tab - Toggle mouse look\n";
//         std::cout << "  F6 - Toggle post-processing\n";
//         std::cout << "  R - Reset camera\n";
//         std::cout << "  1-4 - Camera presets\n";
//         std::cout << "  Shift/Ctrl - Speed modifiers\n";
//         std::cout << "  Q+ESC - Exit game\n\n";
//     }

//     void OnUpdate(float deltaTime) override {
//         HandleCameraInput(deltaTime);
//     }

// private:
//     glm::vec3 m_cameraPos = {0, 12, 20};
//     float m_pitch = -15.0f;
//     float m_yaw = -90.0f;
//     float m_sensitivity = 0.1f;
//     bool m_firstMouse = true;
//     bool m_mouseLookEnabled = true;

//     void SetupPostProcessing() {
//         auto* renderer = GetEngine().GetRenderer();
//         if (renderer && renderer->GetPostProcessManager()) {
//             renderer->EnablePostProcessing(true);
//             auto& settings = renderer->GetPostProcessManager()->GetSettings();
//             settings.exposure = 0.3f;
//             settings.bloomStrength = 0.4f;
//             settings.bloomThreshold = 0.7f;
//             settings.enableFXAA = true;
//             BS_INFO(Blacksite::LogCategory::GAME, "Post-processing configured");
//         }
//     }

//     void HandleCameraInput(float deltaTime) {
//         auto* input = GetEngine().GetInputSystem();
//         auto* scene = GetEngine().GetActiveScene();
//         if (!scene) return;

//         // Toggle post-processing
//         if (input->KeyPressed(Blacksite::Key::F6)) {
//             auto* renderer = GetEngine().GetRenderer();
//             bool currentState = renderer->IsPostProcessingEnabled();
//             renderer->EnablePostProcessing(!currentState);
//             BS_INFO_F(Blacksite::LogCategory::GAME, "Post-processing: %s",
//                      !currentState ? "ON" : "OFF");
//         }

//         // Toggle mouse look
//         if (input->KeyPressed(Blacksite::Key::Tab)) {
//             m_mouseLookEnabled = !m_mouseLookEnabled;
//             BS_INFO_F(Blacksite::LogCategory::GAME, "Mouse look: %s",
//                      m_mouseLookEnabled ? "ON" : "OFF");
//         }

//         // Mouse look
//         if (m_mouseLookEnabled && input->ShouldCaptureMouse()) {
//             if (input->MouseDown(Blacksite::Mouse::Right)) {
//                 glm::vec2 mouseDelta = input->MouseDelta();
//                 if (glm::length(mouseDelta) > 0.1f && !m_firstMouse) {
//                     m_yaw += mouseDelta.x * m_sensitivity;
//                     m_pitch -= mouseDelta.y * m_sensitivity;
//                     m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);
//                 } else if (m_firstMouse) {
//                     m_firstMouse = false;
//                 }
//             } else {
//                 m_firstMouse = true;
//             }
//         }

//         // Camera movement
//         glm::vec3 movement = input->Movement3D();
//         float speed = input->GetSpeedMultiplier() * 8.0f * deltaTime;

//         // Calculate camera vectors
//         glm::vec3 forward;
//         forward.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
//         forward.y = sin(glm::radians(m_pitch));
//         forward.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
//         forward = normalize(forward);

//         glm::vec3 right = normalize(cross(forward, glm::vec3(0, 1, 0)));
//         glm::vec3 up = cross(right, forward);

//         // Apply movement
//         m_cameraPos += right * movement.x * speed;
//         m_cameraPos += up * movement.y * speed;
//         m_cameraPos += forward * movement.z * speed;

//         // Update camera
//         scene->SetCameraPosition(m_cameraPos);
//         scene->SetCameraTarget(m_cameraPos + forward);

//         // Camera presets
//         if (input->KeyPressed(Blacksite::Key::Num1)) {
//             m_cameraPos = {0, 12, 20}; m_pitch = -15.0f; m_yaw = -90.0f;
//         }
//         if (input->KeyPressed(Blacksite::Key::Num2)) {
//             m_cameraPos = {25, 8, 0}; m_pitch = -10.0f; m_yaw = -180.0f;
//         }
//         if (input->KeyPressed(Blacksite::Key::Num3)) {
//             m_cameraPos = {0, 25, 0}; m_pitch = -70.0f; m_yaw = -90.0f;
//         }
//         if (input->KeyPressed(Blacksite::Key::Num4)) {
//             m_cameraPos = {-20, 15, 15}; m_pitch = -20.0f; m_yaw = -45.0f;
//         }

//         // Reset camera
//         if (input->KeyPressed('r')) {
//             m_cameraPos = {0, 12, 20};
//             m_pitch = -15.0f;
//             m_yaw = -90.0f;
//             m_firstMouse = true;
//         }
//     }
// };

// int main() {
//     Blacksite::Logger::Instance().Initialize();

//     std::cout << "🎮 Blacksite Engine - Shader Showcase Game\n";
//     std::cout << "==========================================\n";
//     BS_INFO(Blacksite::LogCategory::GAME, "Starting Shader Showcase Game");

//     ShaderShowcaseApp game;

//     if (!game.Initialize()) {
//         std::cerr << "❌ Failed to initialize game\n";
//         return -1;
//     }

//     std::cout << "✅ Game initialized successfully\n";
//     std::cout << "🚀 Starting game loop...\n\n";

//     int result = game.Run();

//     if (result == 0) {
//         std::cout << "✅ Game completed successfully\n";
//     } else {
//         std::cout << "⚠️ Game exited with code: " << result << "\n";
//     }

//     BS_INFO(Blacksite::LogCategory::GAME, "Shader showcase game completed");
//     return result;
// }
