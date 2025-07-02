#include "blacksite/graphics/PostProcessManager.h"
#include <iostream>
#include "blacksite/core/Logger.h"

namespace Blacksite {

PostProcessManager::PostProcessManager() = default;

PostProcessManager::~PostProcessManager() {
    Shutdown();
}

bool PostProcessManager::Initialize(int width, int height, ShaderSystem* shaderSystem) {
    m_shaderSystem = shaderSystem;
    m_width = width;
    m_height = height;

    if (!m_shaderSystem) {
        BS_ERROR(LogCategory::RENDERER, "PostProcessManager: ShaderSystem is null!");
        return false;
    }

    // - > They are already loaded for now.
    // // Load post-processing shaders if not already loaded
    // if (!m_shaderSystem->HasShader("postprocess")) {
    //     m_shaderSystem->LoadShaderFromLibrary("postprocess");
    // }
    // if (!m_shaderSystem->HasShader("blur")) {
    //     m_shaderSystem->LoadShaderFromLibrary("blur");
    // }
    // if (!m_shaderSystem->HasShader("bloom")) {
    //     m_shaderSystem->LoadShaderFromLibrary("bloom");
    // }
    // if (!m_shaderSystem->HasShader("fxaa")) {
    //     m_shaderSystem->LoadShaderFromLibrary("fxaa");
    // }

    CreateFrameBuffers();
    CreateScreenQuad();

    BS_INFO(LogCategory::RENDERER, "PostProcessManager initialized successfully");
    return true;
}

void PostProcessManager::Shutdown() {
    DeleteFrameBuffers();

    if (m_quadVAO) {
        glDeleteVertexArrays(1, &m_quadVAO);
        glDeleteBuffers(1, &m_quadVBO);
        m_quadVAO = m_quadVBO = 0;
    }

    BS_INFO(LogCategory::RENDERER, "PostProcessManager shut down");
}

void PostProcessManager::OnWindowResize(int width, int height) {
    m_width = width;
    m_height = height;

    DeleteFrameBuffers();
    CreateFrameBuffers();

    BS_DEBUG_F(LogCategory::RENDERER, "PostProcessManager resized to %dx%d", width, height);
}

void PostProcessManager::BeginFrame() {
    // Bind main framebuffer for scene rendering
    BindFrameBuffer(m_mainBuffer);
    glViewport(0, 0, m_width, m_height);

    // Clear with default clear color
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Ensure depth testing is enabled for 3D rendering
    glEnable(GL_DEPTH_TEST);
}

void PostProcessManager::EndFrame() {
    // Unbind framebuffer first
    UnbindFrameBuffer();

    // Execute post-processing pipeline
    if (m_settings.enableBloom) {
        ExtractBloom();
        BlurBloom();
    }

    // Final composite and tone mapping
    CombineAndToneMap();

    // Apply FXAA if enabled
    if (m_settings.enableFXAA) {
        ApplyFXAA();
    }
}

void PostProcessManager::CreateFrameBuffers() {
    // Main scene buffer (full resolution)
    if (!CreateFrameBuffer(m_mainBuffer, m_width, m_height, true)) {
        BS_ERROR(LogCategory::RENDERER, "Failed to create main framebuffer");
        return;
    }

    // Bloom buffers (half resolution for performance)
    int bloomWidth = m_width / 2;
    int bloomHeight = m_height / 2;

    if (!CreateFrameBuffer(m_bloomBuffer, bloomWidth, bloomHeight, false)) {
        BS_ERROR(LogCategory::RENDERER, "Failed to create bloom framebuffer");
        return;
    }

    if (!CreateFrameBuffer(m_blurBuffer1, bloomWidth, bloomHeight, false)) {
        BS_ERROR(LogCategory::RENDERER, "Failed to create blur buffer 1");
        return;
    }

    if (!CreateFrameBuffer(m_blurBuffer2, bloomWidth, bloomHeight, false)) {
        BS_ERROR(LogCategory::RENDERER, "Failed to create blur buffer 2");
        return;
    }

    BS_DEBUG(LogCategory::RENDERER, "Post-processing framebuffers created");
}

void PostProcessManager::CreateScreenQuad() {
    // Full-screen quad vertices (position + texcoord)
    float quadVertices[] = {// positions   // texCoords
                            -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,

                            -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f};

    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);

    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // Texture coordinate attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}

void PostProcessManager::DeleteFrameBuffers() {
    auto deleteFrameBuffer = [](FrameBuffer& fb) {
        if (fb.FBO)
            glDeleteFramebuffers(1, &fb.FBO);
        if (fb.colorTexture)
            glDeleteTextures(1, &fb.colorTexture);
        if (fb.depthTexture)
            glDeleteTextures(1, &fb.depthTexture);
        fb = FrameBuffer{};  // Reset to default state
    };

    deleteFrameBuffer(m_mainBuffer);
    deleteFrameBuffer(m_bloomBuffer);
    deleteFrameBuffer(m_blurBuffer1);
    deleteFrameBuffer(m_blurBuffer2);
}

bool PostProcessManager::CreateFrameBuffer(FrameBuffer& fb, int width, int height, bool needDepth) {
    fb.width = width;
    fb.height = height;

    // Create framebuffer
    glGenFramebuffers(1, &fb.FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, fb.FBO);

    // Create color texture
    glGenTextures(1, &fb.colorTexture);
    glBindTexture(GL_TEXTURE_2D, fb.colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb.colorTexture, 0);

    // Create depth texture if needed
    if (needDepth) {
        glGenTextures(1, &fb.depthTexture);
        glBindTexture(GL_TEXTURE_2D, fb.depthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fb.depthTexture, 0);
    }

    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        BS_ERROR_F(LogCategory::RENDERER, "Framebuffer not complete! Status: %d",
                   glCheckFramebufferStatus(GL_FRAMEBUFFER));
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

void PostProcessManager::BindFrameBuffer(const FrameBuffer& fb) {
    glBindFramebuffer(GL_FRAMEBUFFER, fb.FBO);
}

void PostProcessManager::UnbindFrameBuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessManager::ExtractBloom() {
    // Extract bright pixels for bloom
    BindFrameBuffer(m_bloomBuffer);
    glViewport(0, 0, m_bloomBuffer.width, m_bloomBuffer.height);
    glClear(GL_COLOR_BUFFER_BIT);

    auto& shaderManager = m_shaderSystem->GetShaderManager();
    shaderManager.UseShader("postprocess");

    // Bind main scene texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_mainBuffer.colorTexture);
    shaderManager.SetUniform("uScreenTexture", 0);

    // Set bloom threshold
    shaderManager.SetUniform("uBloomThreshold", m_settings.bloomThreshold);
    shaderManager.SetUniform("uExtractBrightPixels", true);

    RenderScreenQuad();
}

void PostProcessManager::BlurBloom() {
    auto& shaderManager = m_shaderSystem->GetShaderManager();
    shaderManager.UseShader("blur");

    // Ping-pong blur
    bool horizontal = true;
    bool firstIteration = true;

    for (int i = 0; i < m_settings.bloomBlurPasses * 2; i++) {
        FrameBuffer& targetBuffer = horizontal ? m_blurBuffer2 : m_blurBuffer1;
        BindFrameBuffer(targetBuffer);
        glViewport(0, 0, targetBuffer.width, targetBuffer.height);

        shaderManager.SetUniform("uHorizontal", horizontal);
        shaderManager.SetUniform("uBlurSize", 1.0f);

        // Bind source texture
        glActiveTexture(GL_TEXTURE0);
        if (firstIteration) {
            glBindTexture(GL_TEXTURE_2D, m_bloomBuffer.colorTexture);
            firstIteration = false;
        } else {
            FrameBuffer& sourceBuffer = horizontal ? m_blurBuffer1 : m_blurBuffer2;
            glBindTexture(GL_TEXTURE_2D, sourceBuffer.colorTexture);
        }
        shaderManager.SetUniform("uTexture", 0);

        RenderScreenQuad();
        horizontal = !horizontal;
    }
}

void PostProcessManager::CombineAndToneMap() {
    // Render to screen (framebuffer 0)
    UnbindFrameBuffer();
    glViewport(0, 0, m_width, m_height);
    glClear(GL_COLOR_BUFFER_BIT);

    auto& shaderManager = m_shaderSystem->GetShaderManager();

    if (m_settings.enableBloom) {
        shaderManager.UseShader("bloom");

        // Bind scene texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_mainBuffer.colorTexture);
        shaderManager.SetUniform("uScene", 0);

        // Bind bloom texture
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_blurBuffer1.colorTexture);  // Final blur result
        shaderManager.SetUniform("uBloomBlur", 1);

        shaderManager.SetUniform("uBloomStrength", m_settings.bloomStrength);
    } else {
        shaderManager.UseShader("postprocess");

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_mainBuffer.colorTexture);
        shaderManager.SetUniform("uScreenTexture", 0);
    }

    // Tone mapping settings
    shaderManager.SetUniform("uExposure", m_settings.exposure);
    shaderManager.SetUniform("uGamma", m_settings.gamma);

    RenderScreenQuad();
}

void PostProcessManager::ApplyFXAA() {
    // TODO: Implement FXAA as additional pass
    // For now, the previous pass already renders to screen
}

void PostProcessManager::RenderScreenQuad() {
    // Disable depth testing for post-processing
    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // Re-enable depth testing
    glEnable(GL_DEPTH_TEST);
}

void PostProcessManager::RenderDebugUI() {
    // TODO: ImGui integration for tweaking post-process settings
}

}  // namespace Blacksite
