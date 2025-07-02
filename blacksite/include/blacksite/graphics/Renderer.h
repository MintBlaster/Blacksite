#pragma once
#include <vector>
#include "blacksite/core/Entity.h"
#include "Camera.h"
#include "GeometryManager.h"
#include "PostProcessManager.h"
#include "RenderCommand.h"
#include "ShaderSystem.h"

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
    void DrawSphere(const Transform& transform, const std::string& shaderName,
                    const glm::vec3& color = glm::vec3(1.0f));
    void DrawPlane(const Transform& transform, const std::string& shaderName, const glm::vec3& color = glm::vec3(1.0f));

    // Advanced rendering
    void Submit(const RenderCommand& command);  // Queue up a draw command
    void Flush();                               // Execute all queued commands

    void EnablePostProcessing(bool enable) { m_postProcessingEnabled = enable; }
    bool IsPostProcessingEnabled() const { return m_postProcessingEnabled; }
    PostProcessManager* GetPostProcessManager() { return m_postProcessManager.get(); }

    // Camera control
    void SetCamera(Camera* camera);
    Camera& GetCamera() { return *m_camera; }
    const Camera& GetCamera() const { return *m_camera; }

    // Window management
    void OnWindowResize(int width, int height);

    // Shader system integration
    void SetShaderSystem(ShaderSystem* shaderSystem);

    // Debug methods
    void DebugOpenGLState();
    void DebugShaderCompilation();
    void DebugGeometry();
    void DebugMatrices();

    void DrawColliders(const std::vector<Entity>& entities);
    void DrawEntityCollider(const Entity& entity);

    void SetShowColliders(bool show) { m_showColliders = show; }
    bool GetShowColliders() const { return m_showColliders; }

  private:
    ShaderSystem* m_shaderSystem = nullptr;  // Reference to engine's shader system
    GeometryManager m_geometryManager;       // Keeps track of our meshes
    Camera* m_camera = nullptr;              // External camera (not owned)
    Camera m_internalCamera;                 // Fallback internal camera

    std::unique_ptr<PostProcessManager> m_postProcessManager;
    bool m_postProcessingEnabled = true;

    // Command queue for batched rendering
    std::vector<RenderCommand> m_renderQueue;

    // Current viewport dimensions
    int m_width = 0;
    int m_height = 0;

    // Basic lighting setup [hardcoded]
    glm::vec3 m_lightPosition{2.0f, 4.0f, 2.0f};

    // Setup functions
    void SetupDefaultShaders();   // Verify required shaders are available
    void SetupDefaultGeometry();  // Create cube, sphere, plane

    void ExecuteRenderCommand(const RenderCommand& command);

    // Transform math
    glm::mat4 CreateModelMatrix(const Transform& transform);

    bool m_showColliders = false;
};
}  // namespace Blacksite
