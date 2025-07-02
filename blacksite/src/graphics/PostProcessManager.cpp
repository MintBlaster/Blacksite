#include "blacksite/graphics/PostProcessManager.h"
#include "blacksite/core/Logger.h"
#include "blacksite/graphics/ShaderSystem.h"
#include <iostream>

namespace Blacksite {

PostProcessManager::PostProcessManager() {
    BS_DEBUG(LogCategory::RENDERER, "PostProcessManager created");
}

PostProcessManager::~PostProcessManager() {
    Shutdown();
}

bool PostProcessManager::Initialize(int width, int height, ShaderSystem* shaderSystem) {
    if (m_initialized) {
        BS_WARN(LogCategory::RENDERER, "PostProcessManager already initialized");
        return true;
    }

    if (width <= 0 || height <= 0) {
        BS_ERROR_F(LogCategory::RENDERER, "Invalid dimensions: %dx%d", width, height);
        return false;
    }

    if (!shaderSystem) {
        BS_ERROR(LogCategory::RENDERER, "PostProcessManager: ShaderSystem is null!");
        return false;
    }

    m_shaderSystem = shaderSystem;
    m_width = width;
    m_height = height;

    BS_INFO_F(LogCategory::RENDERER, "Initializing PostProcessManager (%dx%d)", width, height);

    // Load and validate all required shaders
    auto& shaderManager = m_shaderSystem->GetShaderManager();
    std::vector<std::string> requiredShaders = {"postprocess", "blur", "bloom", "fxaa"};

    for (const auto& shaderName : requiredShaders) {
        if (!m_shaderSystem->HasShader(shaderName)) {
            BS_INFO_F(LogCategory::RENDERER, "Loading shader: %s", shaderName.c_str());
            if (!shaderManager.LoadShaderFromLibrary(shaderName)) {
                BS_ERROR_F(LogCategory::RENDERER, "Failed to load shader: %s", shaderName.c_str());
                return false;
            }
        }

        if (!ValidateShader(shaderName)) {
            BS_ERROR_F(LogCategory::RENDERER, "Shader validation failed: %s", shaderName.c_str());
            return false;
        }
    }

    // Initialize settings
    m_settings.enableBloom = true;
    m_settings.enableFXAA = true;
    m_settings.bloomThreshold = 0.8f;
    m_settings.bloomStrength = 1.0f;
    m_settings.bloomBlurPasses = 5;
    m_settings.exposure = 1.0f;
    m_settings.gamma = 2.2f;

    // Create framebuffers and screen quad
    CreateFrameBuffers();
    CreateScreenQuad();

    // Validate everything was created successfully
    if (!m_mainBuffer.FBO || !m_quadVAO) {
        BS_ERROR(LogCategory::RENDERER, "Failed to create required resources");
        return false;
    }

    DebugFrameBuffers();

    m_initialized = true;
    BS_INFO(LogCategory::RENDERER, "PostProcessManager initialized successfully");
    return true;
}

void PostProcessManager::Shutdown() {
    if (!m_initialized) {
        return;
    }

    BS_INFO(LogCategory::RENDERER, "Shutting down PostProcessManager");

    DeleteFrameBuffers();

    if (m_quadVAO) {
        glDeleteVertexArrays(1, &m_quadVAO);
        m_quadVAO = 0;
    }

    if (m_quadVBO) {
        glDeleteBuffers(1, &m_quadVBO);
        m_quadVBO = 0;
    }

    m_shaderSystem = nullptr;
    m_initialized = false;

    CheckGLError("PostProcessManager shutdown");
    BS_INFO(LogCategory::RENDERER, "PostProcessManager shut down successfully");
}

void PostProcessManager::OnWindowResize(int width, int height) {
    if (!m_initialized || width <= 0 || height <= 0) {
        return;
    }

    BS_INFO_F(LogCategory::RENDERER, "Resizing PostProcessManager: {}x{} -> {}x{}",
            m_width, m_height, width, height);

    m_width = width;
    m_height = height;

    DeleteFrameBuffers();
    CreateFrameBuffers();

    CheckGLError("Window resize");
}

void PostProcessManager::BeginFrame() {
    if (!m_initialized) {
        BS_ERROR(LogCategory::RENDERER, "PostProcessManager not initialized!");
        return;
    }

    // Bind main framebuffer for scene rendering
    BindFrameBuffer(m_mainBuffer);
    glViewport(0, 0, m_width, m_height);

    // Clear with default clear color
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Ensure proper 3D rendering state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    CheckGLError("BeginFrame");
}

void PostProcessManager::EndFrame() {
    if (!m_initialized) {
        return;
    }

    // Store current OpenGL state
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

    GLboolean depthTest = glIsEnabled(GL_DEPTH_TEST);
    GLboolean cullFace = glIsEnabled(GL_CULL_FACE);

    // Unbind framebuffer and prepare for post-processing
    UnbindFrameBuffer();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    try {
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
    catch (const std::exception& e) {
        BS_ERROR_F(LogCategory::RENDERER, "Post-processing error: {}", e.what());
    }

    // Cleanup and restore state
    glUseProgram(0);

    if (depthTest) glEnable(GL_DEPTH_TEST);
    if (cullFace) glEnable(GL_CULL_FACE);

    CheckGLError("EndFrame");
}

void PostProcessManager::CreateFrameBuffers() {
    BS_DEBUG(LogCategory::RENDERER, "Creating framebuffers");

    // Main scene buffer (full resolution with depth)
    if (!CreateFrameBuffer(m_mainBuffer, m_width, m_height, true)) {
        BS_ERROR(LogCategory::RENDERER, "Failed to create main framebuffer");
        return;
    }

    // Bloom buffers (half resolution for performance)
    int bloomWidth = std::max(1, m_width / 2);
    int bloomHeight = std::max(1, m_height / 2);

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

    // FXAA buffer (full resolution)
    if (!CreateFrameBuffer(m_fxaaBuffer, m_width, m_height, false)) {
        BS_ERROR(LogCategory::RENDERER, "Failed to create FXAA framebuffer");
        return;
    }

    CheckGLError("CreateFrameBuffers");
    BS_DEBUG(LogCategory::RENDERER, "All framebuffers created successfully");
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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fb.depthTexture, 0);
    }

    // Check framebuffer completeness
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        BS_ERROR_F(LogCategory::RENDERER, "Framebuffer not complete! Status: 0x{:X}", status);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CheckGLError("CreateFrameBuffer");
    return true;
}

void PostProcessManager::CreateScreenQuad() {
    // Full-screen quad vertices (position + texcoord)
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

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
    CheckGLError("CreateScreenQuad");
}

void PostProcessManager::DeleteFrameBuffers() {
    auto deleteFrameBuffer = [](FrameBuffer& fb) {
        if (fb.FBO) {
            glDeleteFramebuffers(1, &fb.FBO);
            fb.FBO = 0;
        }
        if (fb.colorTexture) {
            glDeleteTextures(1, &fb.colorTexture);
            fb.colorTexture = 0;
        }
        if (fb.depthTexture) {
            glDeleteTextures(1, &fb.depthTexture);
            fb.depthTexture = 0;
        }
        fb.width = fb.height = 0;
    };

    deleteFrameBuffer(m_mainBuffer);
    deleteFrameBuffer(m_bloomBuffer);
    deleteFrameBuffer(m_blurBuffer1);
    deleteFrameBuffer(m_blurBuffer2);
    deleteFrameBuffer(m_fxaaBuffer);

    CheckGLError("DeleteFrameBuffers");
}

void PostProcessManager::BindFrameBuffer(const FrameBuffer& fb) {
    glBindFramebuffer(GL_FRAMEBUFFER, fb.FBO);
    CheckGLError("BindFrameBuffer");
}

void PostProcessManager::UnbindFrameBuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CheckGLError("UnbindFrameBuffer");
}


void PostProcessManager::ExtractBloom() {
    if (!ValidateShader("postprocess")) {  // Use postprocess shader for extraction
        BS_ERROR(LogCategory::RENDERER, "Postprocess shader not available");
        return;
    }

    BindFrameBuffer(m_bloomBuffer);
    glViewport(0, 0, m_bloomBuffer.width, m_bloomBuffer.height);
    glClear(GL_COLOR_BUFFER_BIT);

    auto& shaderManager = m_shaderSystem->GetShaderManager();
    if (!shaderManager.UseShader("postprocess")) {  // Use postprocess shader
        BS_ERROR(LogCategory::RENDERER, "Failed to use postprocess shader");
        return;
    }

    // Bind main scene texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_mainBuffer.colorTexture);
    shaderManager.SetUniform("uScreenTexture", 0);  // Correct uniform name
    shaderManager.SetUniform("uBloomThreshold", m_settings.bloomThreshold);
    shaderManager.SetUniform("uExtractBrightPixels", true);  // Enable extraction mode

    // Set dummy values for required uniforms
    shaderManager.SetUniform("uExposure", 1.0f);
    shaderManager.SetUniform("uGamma", 2.2f);

    RenderScreenQuad();
    CheckGLError("ExtractBloom");
}

void PostProcessManager::BlurBloom() {
    if (!ValidateShader("blur")) {
        BS_ERROR(LogCategory::RENDERER, "Blur shader not available");
        return;
    }

    auto& shaderManager = m_shaderSystem->GetShaderManager();
    if (!shaderManager.UseShader("blur")) {
        BS_ERROR(LogCategory::RENDERER, "Failed to use blur shader");
        return;
    }

    bool horizontal = true;
    bool firstIteration = true;

    for (int i = 0; i < m_settings.bloomBlurPasses * 2; i++) {
        FrameBuffer& targetBuffer = horizontal ? m_blurBuffer2 : m_blurBuffer1;
        BindFrameBuffer(targetBuffer);
        glViewport(0, 0, targetBuffer.width, targetBuffer.height);
        glClear(GL_COLOR_BUFFER_BIT);

        shaderManager.SetUniform("uHorizontal", horizontal);

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

    CheckGLError("BlurBloom");
}

void PostProcessManager::CombineAndToneMap() {
    if (!ValidateShader("bloom")) {  // Use bloom shader for combining
        BS_ERROR(LogCategory::RENDERER, "Bloom shader not available");
        return;
    }

    // Render to FXAA buffer if FXAA is enabled, otherwise to screen
    if (m_settings.enableFXAA) {
        BindFrameBuffer(m_fxaaBuffer);
        glViewport(0, 0, m_fxaaBuffer.width, m_fxaaBuffer.height);
    } else {
        UnbindFrameBuffer();
        glViewport(0, 0, m_width, m_height);
    }

    glClear(GL_COLOR_BUFFER_BIT);

    auto& shaderManager = m_shaderSystem->GetShaderManager();
    if (!shaderManager.UseShader("bloom")) {  // Use bloom shader for combining
        BS_ERROR(LogCategory::RENDERER, "Failed to use bloom shader");
        return;
    }

    // Bind main scene texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_mainBuffer.colorTexture);
    shaderManager.SetUniform("uScene", 0);  // Correct uniform name

    // Bind bloom texture
    glActiveTexture(GL_TEXTURE1);
    if (m_settings.enableBloom) {
        glBindTexture(GL_TEXTURE_2D, m_blurBuffer1.colorTexture); // Final blur result
        shaderManager.SetUniform("uBloomBlur", 1);  // Correct uniform name
        shaderManager.SetUniform("uBloomStrength", m_settings.bloomStrength);
    } else {
        // Bind a black texture or set bloom strength to 0
        shaderManager.SetUniform("uBloomBlur", 1);
        shaderManager.SetUniform("uBloomStrength", 0.0f);
    }

    // Set tone mapping parameters
    shaderManager.SetUniform("uExposure", m_settings.exposure);
    shaderManager.SetUniform("uGamma", m_settings.gamma);

    RenderScreenQuad();
    CheckGLError("CombineAndToneMap");
}

void PostProcessManager::ApplyFXAA() {
    if (!ValidateShader("fxaa")) {
        BS_ERROR(LogCategory::RENDERER, "FXAA shader not available");
        return;
    }

    UnbindFrameBuffer();
    glViewport(0, 0, m_width, m_height);
    glClear(GL_COLOR_BUFFER_BIT);

    auto& shaderManager = m_shaderSystem->GetShaderManager();
    if (!shaderManager.UseShader("fxaa")) {
        BS_ERROR(LogCategory::RENDERER, "Failed to use FXAA shader");
        return;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fxaaBuffer.colorTexture);
    shaderManager.SetUniform("uTexture", 0);
    // Use correct uniform name from shader
    shaderManager.SetUniform("uInverseScreenSize", glm::vec2(1.0f / m_width, 1.0f / m_height));

    RenderScreenQuad();
    CheckGLError("ApplyFXAA");
}

void PostProcessManager::RenderScreenQuad() {
    if (!m_quadVAO) {
        BS_ERROR(LogCategory::RENDERER, "Screen quad not initialized");
        return;
    }

    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    CheckGLError("RenderScreenQuad");
}

bool PostProcessManager::ValidateShader(const std::string& shaderName) {
    if (!m_shaderSystem || !m_shaderSystem->HasShader(shaderName)) {
        return false;
    }

    // Additional validation could be added here
    return true;
}

void PostProcessManager::CheckGLError(const std::string& operation) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        BS_ERROR_F(LogCategory::RENDERER, "OpenGL error in %s: 0x%X", operation.c_str(), error);
    }
}

void PostProcessManager::DebugFrameBuffers() {
    BS_DEBUG(LogCategory::RENDERER, "=== PostProcess Debug ===");
    BS_DEBUG_F(LogCategory::RENDERER, "Main buffer: FBO={}, Color={}, Depth={} ({}x{})",
             m_mainBuffer.FBO, m_mainBuffer.colorTexture, m_mainBuffer.depthTexture,
             m_mainBuffer.width, m_mainBuffer.height);
    BS_DEBUG_F(LogCategory::RENDERER, "Bloom buffer: FBO={}, Color={} ({}x{})",
             m_bloomBuffer.FBO, m_bloomBuffer.colorTexture,
             m_bloomBuffer.width, m_bloomBuffer.height);

    // Validate main framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_mainBuffer.FBO);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    BS_DEBUG_F(LogCategory::RENDERER, "Main framebuffer status: 0x{:X} (complete=0x{:X})",
             status, GL_FRAMEBUFFER_COMPLETE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CheckGLError("DebugFrameBuffers");
}

void PostProcessManager::RenderDebugUI() {
    // TODO: Add ImGui integration for real-time parameter tweaking
    // This would allow adjusting bloom threshold, strength, blur passes, etc.
}

} // namespace Blacksite
