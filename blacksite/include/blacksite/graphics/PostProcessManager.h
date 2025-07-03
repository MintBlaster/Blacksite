#pragma once
#include <GL/glew.h>
#include <string>

namespace Blacksite {

class ShaderSystem;

struct PostProcessSettings {
    bool enableBloom = true;
    bool enableFXAA = true;
    bool enableToneMapping = true;
    bool showBloomTexture = false;

    // Bloom
    float bloomThreshold = 1.2f;  // Only really bright pixels bloom (prevents flashlight vomit)
    float bloomStrength = 0.6f;   // Less aggressive glow
    int bloomBlurPasses = 3;      // Less blur for sharper bloom edges

    // Tonemapping & exposure
    float exposure = 1.1f;  // Slightly boosted but not overblown
    float gamma = 2.0f;     // Less contrasty than 2.2, keeps mids readable
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

    GLuint GetSceneTexture() const { return m_mainBuffer.colorTexture; }
    GLuint GetDepthTexture() const { return m_mainBuffer.depthTexture; }
    bool IsInitialized() const { return m_initialized; }

    // Optional: Get specific buffer textures
    GLuint GetMainColorTexture() const { return m_mainBuffer.colorTexture; }
    GLuint GetBloomTexture() const { return m_bloomBuffer.colorTexture; }

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
