#pragma once
#include <vector>
#include "Camera.h"
#include "GeometryManager.h"
#include "RenderCommand.h"
#include "ShaderManager.h"

namespace Blacksite {

class Renderer {
  public:
    Renderer();
    ~Renderer();

    // Core lifecycle
    bool Initialize(int width, int height);  // Get everything ready
    void Shutdown();                         // Clean up

    // Frame management
    void BeginFrame();  // Clear the screen
    void EndFrame();    // Actually draw all the queued commands

    // High-level rendering API
    void DrawCube(const Transform& transform, const glm::vec3& color = glm::vec3(1.0f));
    void DrawSphere(const Transform& transform, const glm::vec3& color = glm::vec3(1.0f));
    void DrawPlane(const Transform& transform, const glm::vec3& color = glm::vec3(1.0f));

    void DrawCube(const Transform& transform, const std::string& shaderName, const glm::vec3& color = glm::vec3(1.0f));
    void DrawSphere(const Transform& transform, const std::string& shaderName, const glm::vec3& color = glm::vec3(1.0f));
    void DrawPlane(const Transform& transform, const std::string& shaderName, const glm::vec3& color = glm::vec3(1.0f));

    // Advanced rendering
    void Submit(const RenderCommand& command);  // Queue up a draw command
    void Flush();                               // Execute all queued commands

    // Camera control
    void SetCamera(Camera* camera);
    Camera& GetCamera() { return *m_camera; }
    const Camera& GetCamera() const { return *m_camera; }

    // Window management
    void OnWindowResize(int width, int height);

    // Debug methods
    void DebugOpenGLState();
    void DebugShaderCompilation();
    void DebugGeometry();
    void DebugMatrices();

  private:
    ShaderManager m_shaderManager;      // Manages our compiled shaders
    GeometryManager m_geometryManager;  // Keeps track of our meshes
    Camera* m_camera = nullptr;         // External camera (not owned)
    Camera m_internalCamera;            // Fallback internal camera

    // Command queue for batched rendering
    std::vector<RenderCommand> m_renderQueue;

    // Current viewport dimensions
    int m_width = 0;
    int m_height = 0;

    // Basic lighting setup [hardcoded]
    glm::vec3 m_lightPosition{2.0f, 4.0f, 2.0f};

    // Setup functions
    void SetupDefaultShaders();   // Load our basic shaders
    void SetupDefaultGeometry();  // Create cube, sphere, plane

    void ExecuteRenderCommand(const RenderCommand& command);

    // Transform math
    glm::mat4 CreateModelMatrix(const Transform& transform);
};
}  // namespace Blacksite
