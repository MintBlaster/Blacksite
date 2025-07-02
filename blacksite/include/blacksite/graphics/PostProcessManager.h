#pragma once
#include <GL/glew.h>
#include <string>
#include <vector>

namespace Blacksite {

class ShaderSystem;

struct PostProcessSettings {
    bool enableBloom = true;
    bool enableFXAA = true;
    bool enableToneMapping = true;
    bool showBloomTexture = false;
    float bloomThreshold = 0.8f;
    float bloomStrength = 1.0f;
    int bloomBlurPasses = 5;
    float exposure = 1.0f;
    float gamma = 2.2f;
};

struct FrameBuffer {
    GLuint FBO = 0;
    GLuint colorTexture = 0;
    GLuint depthTexture = 0;
    int width = 0;
    int height = 0;
};

class PostProcessManager {
  public:
    PostProcessManager();
    ~PostProcessManager();

    bool Initialize(int width, int height, ShaderSystem* shaderSystem);
    void Shutdown();
    void OnWindowResize(int width, int height);

    void BeginFrame();
    void EndFrame();

    PostProcessSettings& GetSettings() { return m_settings; }

    void RenderDebugUI();

  private:
    // Core functionality
    void CreateFrameBuffers();
    void DeleteFrameBuffers();
    bool CreateFrameBuffer(FrameBuffer& fb, int width, int height, bool needDepth);
    void CreateScreenQuad();

    // Rendering
    void BindFrameBuffer(const FrameBuffer& fb);
    void UnbindFrameBuffer();
    void RenderScreenQuad();

    // Post-processing pipeline
    void ExtractBloom();
    void BlurBloom();
    void CombineAndToneMap();
    void ApplyFXAA();

    // Debugging
    void DebugFrameBuffers();
    void CheckGLError(const std::string& operation);
    bool ValidateShader(const std::string& shaderName);

    // Member variables
    ShaderSystem* m_shaderSystem = nullptr;
    PostProcessSettings m_settings;

    int m_width = 0;
    int m_height = 0;
    bool m_initialized = false;

    // Framebuffers
    FrameBuffer m_mainBuffer;
    FrameBuffer m_bloomBuffer;
    FrameBuffer m_blurBuffer1;
    FrameBuffer m_blurBuffer2;
    FrameBuffer m_fxaaBuffer;

    // Screen quad
    GLuint m_quadVAO = 0;
    GLuint m_quadVBO = 0;
};

}  // namespace Blacksite
