#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "ShaderSystem.h"

namespace Blacksite {

struct FrameBuffer {
    unsigned int FBO = 0;
    unsigned int colorTexture = 0;
    unsigned int depthTexture = 0;
    int width = 0;
    int height = 0;

    bool IsValid() const { return FBO != 0; }
};

struct PostProcessSettings {
    // Tone mapping
    bool enableToneMapping = true;
    float exposure = 1.0f;
    float gamma = 2.2f;

    // Bloom
    bool enableBloom = true;
    float bloomThreshold = 1.0f;
    float bloomStrength = 0.5f;
    int bloomBlurPasses = 5;

    // FXAA
    bool enableFXAA = true;

    // Debug
    bool showBloomTexture = false;
};

class PostProcessManager {
public:
    PostProcessManager();
    ~PostProcessManager();

    bool Initialize(int width, int height, ShaderSystem* shaderSystem);
    void Shutdown();
    void OnWindowResize(int width, int height);

    // Main post-processing pipeline
    void BeginFrame();  // Bind main framebuffer
    void EndFrame();    // Execute post-processing and present

    // Settings
    PostProcessSettings& GetSettings() { return m_settings; }
    const PostProcessSettings& GetSettings() const { return m_settings; }

    // Debug
    void RenderDebugUI(); // For ImGui integration

private:
    ShaderSystem* m_shaderSystem = nullptr;
    PostProcessSettings m_settings;

    // Screen dimensions
    int m_width = 0;
    int m_height = 0;

    // Framebuffers
    FrameBuffer m_mainBuffer;      // Main scene render target
    FrameBuffer m_bloomBuffer;     // Bloom extraction
    FrameBuffer m_blurBuffer1;     // Ping-pong blur buffer 1
    FrameBuffer m_blurBuffer2;     // Ping-pong blur buffer 2

    // Screen quad for post-processing
    unsigned int m_quadVAO = 0;
    unsigned int m_quadVBO = 0;

    // Pipeline methods
    void CreateFrameBuffers();
    void CreateScreenQuad();
    void DeleteFrameBuffers();

    bool CreateFrameBuffer(FrameBuffer& fb, int width, int height, bool needDepth = true);
    void BindFrameBuffer(const FrameBuffer& fb);
    void UnbindFrameBuffer();

    // Post-processing passes
    void ExtractBloom();
    void BlurBloom();
    void CombineAndToneMap();
    void ApplyFXAA();

    void RenderScreenQuad();
};

} // namespace Blacksite
