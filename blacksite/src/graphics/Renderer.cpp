#include "blacksite/graphics/Renderer.h"
#include <cstdlib>
#include <glm/gtc/type_ptr.hpp>
#include "blacksite/core/Logger.h"

namespace Blacksite {

Renderer::Renderer() = default;

Renderer::~Renderer() {
    Shutdown();  // Clean up
}

bool Renderer::Initialize(int width, int height) {
    m_width = width;
    m_height = height;

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height);

    // Setup internal camera as fallback
    m_internalCamera.SetPerspective(45.0f, (float)width / height, 0.1f, 100.0f);

    // Use internal camera by default
    if (!m_camera) {
        m_camera = &m_internalCamera;
    }

    m_lightPosition = glm::vec3(10.0f, 10.0f, 10.0f);

    // Initialize our subsystems
    SetupDefaultShaders();
    SetupDefaultGeometry();

    m_postProcessingEnabled = true;

    // Initialize post-processing
    m_postProcessManager = std::make_unique<PostProcessManager>();
    if (!m_postProcessManager->Initialize(width, height, m_shaderSystem)) {
        BS_ERROR(LogCategory::RENDERER, "Failed to initialize post-processing!");
        return false;
    }

    BS_INFO(LogCategory::RENDERER, "Renderer initialized successfully");
    return true;
}

void Renderer::Shutdown() {
    // Clean up all our GPU resources
    // Note: ShaderSystem is owned by Engine, so we don't clean it up here
    m_geometryManager.Cleanup();

    BS_INFO(LogCategory::RENDERER, "Renderer shut down cleanly");
}

void Renderer::BeginFrame() {
    if (m_postProcessingEnabled) {
        m_postProcessManager->BeginFrame();
    } else {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    m_renderQueue.clear();
}

void Renderer::EndFrame() {
    // Execute all queued render commands
    Flush();

    if (m_postProcessingEnabled) {
        m_postProcessManager->EndFrame();
    }
}

// Default shader versions (use basic shader)
void Renderer::DrawCube(const Transform& transform, const glm::vec3& color) {
    DrawCube(transform, "basic", color);
}

void Renderer::DrawSphere(const Transform& transform, const glm::vec3& color) {
    DrawSphere(transform, "basic", color);
}

void Renderer::DrawPlane(const Transform& transform, const glm::vec3& color) {
    DrawPlane(transform, "basic", color);
}

// Shader-specific versions
void Renderer::DrawCube(const Transform& transform, const std::string& shaderName, const glm::vec3& color) {
    RenderCommand cmd;
    cmd.meshName = "cube";
    cmd.shaderName = shaderName;
    cmd.transform = transform;
    cmd.color = color;
    Submit(cmd);
}

void Renderer::DrawSphere(const Transform& transform, const std::string& shaderName, const glm::vec3& color) {
    RenderCommand cmd;
    cmd.meshName = "sphere";
    cmd.shaderName = shaderName;
    cmd.transform = transform;
    cmd.color = color;
    Submit(cmd);
}

void Renderer::DrawPlane(const Transform& transform, const std::string& shaderName, const glm::vec3& color) {
    RenderCommand cmd;
    cmd.meshName = "plane";
    cmd.shaderName = shaderName;
    cmd.transform = transform;
    cmd.color = color;
    Submit(cmd);
}

void Renderer::Submit(const RenderCommand& command) {
    // Add to our render queue
    m_renderQueue.push_back(command);

    // TODO: Could sort by material/depth here for optimization
}

void Renderer::Flush() {
    // Execute all queued render commands
    for (const auto& command : m_renderQueue) {
        ExecuteRenderCommand(command);
    }

    // Clear the queue
    m_renderQueue.clear();
}

void Renderer::OnWindowResize(int width, int height) {
    m_width = width;
    m_height = height;

    // Update OpenGL viewport
    glViewport(0, 0, width, height);

    // Update camera aspect ratio
    if (m_camera) {
        m_camera->SetAspectRatio((float)width / height);
    }

    // Update post process
    if (m_postProcessManager) {
        m_postProcessManager->OnWindowResize(width, height);
    }

    BS_DEBUG_F(LogCategory::RENDERER, "Renderer resized to %dx%d", width, height);
}

void Renderer::SetCamera(Camera* camera) {
    m_camera = camera;
}

void Renderer::SetShaderSystem(ShaderSystem* shaderSystem) {
    m_shaderSystem = shaderSystem;
}

void Renderer::SetupDefaultShaders() {
    if (!m_shaderSystem) {
        BS_FATAL(LogCategory::RENDERER, "ShaderSystem not set! Call SetShaderSystem() first!");
        std::exit(EXIT_FAILURE);
    }

    // Just verify they're available
    std::vector<std::string> requiredShaders = {"basic", "unlit", "wireframe", "debug"};

    // Load post-processing shaders
    std::vector<std::string> postProcessShaders = {"postprocess", "blur", "bloom", "fxaa"};

    for (const auto& shaderName : postProcessShaders) {
        if (!m_shaderSystem->HasShader(shaderName)) {
            if (!m_shaderSystem->GetShaderManager().LoadShaderFromLibrary(shaderName)) {
                BS_ERROR_F(LogCategory::RENDERER, "Failed to load post-process shader: %s", shaderName.c_str());
            }
        }
    }

    BS_INFO(LogCategory::RENDERER, "All required shaders are available");

    // List available shaders for debugging
    auto availableShaders = m_shaderSystem->GetAvailableShaders();
    std::string shaderList;
    for (const auto& name : availableShaders) {
        if (!shaderList.empty())
            shaderList += ", ";
        shaderList += name;
    }
    BS_DEBUG_F(LogCategory::RENDERER, "Available shaders: %s", shaderList.c_str());
}

void Renderer::SetupDefaultGeometry() {
    // Create our basic geometric primitives with proper names
    m_geometryManager.CreateCube("cube");
    m_geometryManager.CreateSphere("sphere");
    m_geometryManager.CreatePlane("plane");

    BS_DEBUG(LogCategory::RENDERER, "Default geometry created (cube, sphere, plane)");
}

void Renderer::ExecuteRenderCommand(const RenderCommand& command) {
    // Get the mesh we want to render
    const Mesh* mesh = m_geometryManager.GetMesh(command.meshName);
    if (!mesh) {
        BS_ERROR_F(LogCategory::RENDERER, "Mesh not found: %s", command.meshName.c_str());
        return;
    }

    // Use the correct shader for 3D rendering
    if (!m_shaderSystem->GetShaderManager().UseShader(command.shaderName)) {
        BS_ERROR_F(LogCategory::RENDERER, "Failed to use shader: %s", command.shaderName.c_str());
        return;
    }

    // DEBUG: Check camera
    if (!m_camera) {
        BS_ERROR(LogCategory::RENDERER, "No camera set!");
        return;
    }

    // Set up matrices for the GPU
    glm::mat4 model = CreateModelMatrix(command.transform);
    glm::mat4 view = m_camera->GetViewMatrix();
    glm::mat4 proj = m_camera->GetProjectionMatrix();

    auto& shaderManager = m_shaderSystem->GetShaderManager();

    // *** FIX: Actually set the MVP matrices! ***
    shaderManager.SetUniform("uModel", model);
    shaderManager.SetUniform("uView", view);
    shaderManager.SetUniform("uProjection", proj);

    // Set up lighting uniforms
    shaderManager.SetUniform("uLightPos", m_lightPosition);
    shaderManager.SetUniform("uViewPos", m_camera->GetPosition());
    shaderManager.SetUniform("uColor", command.color);

    // Set texture uniforms if the shader supports them
    shaderManager.SetUniform("uHasTexture", false);  // Default to no texture

    // Actually draw the mesh
    glBindVertexArray(mesh->VAO);
    if (mesh->useIndices) {
        glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, mesh->vertexCount);
    }
    glBindVertexArray(0);

    // Check for errors after drawing
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        BS_ERROR_F(LogCategory::RENDERER, "OpenGL error after drawing %s: %d", command.meshName.c_str(), error);
    }
}

glm::mat4 Renderer::CreateModelMatrix(const Transform& transform) {
    // Build the model matrix from translation, rotation, and scale
    // Order matters: Scale -> Rotate -> Translate (applied right to left)
    glm::mat4 model = glm::mat4(1.0f);  // Start with identity matrix

    // Apply transformations in the correct order
    model = glm::translate(model, transform.position);  // Move to world position

    // Apply rotations (Euler angles - not the best but simple to understand)
    model = glm::rotate(model, glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    model = glm::scale(model, transform.scale);  // Apply scaling

    return model;
}

void Renderer::DebugOpenGLState() {
    BS_INFO(LogCategory::RENDERER, "=== OpenGL State Debug ===");

    // Check OpenGL version
    const char* version = (const char*)glGetString(GL_VERSION);
    const char* vendor = (const char*)glGetString(GL_VENDOR);
    const char* renderer = (const char*)glGetString(GL_RENDERER);

    BS_INFO_F(LogCategory::RENDERER, "OpenGL Version: %s", version ? version : "NULL");
    BS_INFO_F(LogCategory::RENDERER, "Vendor: %s", vendor ? vendor : "NULL");
    BS_INFO_F(LogCategory::RENDERER, "Renderer: %s", renderer ? renderer : "NULL");

    // Check viewport
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    BS_INFO_F(LogCategory::RENDERER, "Viewport: x=%d, y=%d, w=%d, h=%d", viewport[0], viewport[1], viewport[2],
              viewport[3]);

    // Check if depth testing is enabled
    GLboolean depthTest = glIsEnabled(GL_DEPTH_TEST);
    BS_INFO_F(LogCategory::RENDERER, "Depth test enabled: %s", depthTest ? "YES" : "NO");

    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        BS_ERROR_F(LogCategory::RENDERER, "OpenGL Error: %d", error);
    } else {
        BS_INFO(LogCategory::RENDERER, "No OpenGL errors detected");
    }
}

void Renderer::DebugShaderCompilation() {
    BS_INFO(LogCategory::RENDERER, "=== Shader Debug ===");

    if (!m_shaderSystem->GetShaderManager().UseShader("basic")) {
        BS_ERROR(LogCategory::RENDERER, "Cannot use basic shader!");
        return;
    }

    // Get current shader program ID
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    BS_INFO_F(LogCategory::RENDERER, "Current shader program ID: %d", currentProgram);

    if (currentProgram == 0) {
        BS_ERROR(LogCategory::RENDERER, "No shader program is currently bound!");
        return;
    }

    // Check uniform locations
    GLint modelLoc = glGetUniformLocation(currentProgram, "uModel");
    GLint viewLoc = glGetUniformLocation(currentProgram, "uView");
    GLint projLoc = glGetUniformLocation(currentProgram, "uProjection");
    GLint colorLoc = glGetUniformLocation(currentProgram, "uColor");

    BS_INFO_F(LogCategory::RENDERER, "Uniform locations - Model: %d, View: %d, Proj: %d, Color: %d", modelLoc, viewLoc,
              projLoc, colorLoc);

    if (modelLoc == -1 || viewLoc == -1 || projLoc == -1 || colorLoc == -1) {
        BS_ERROR(LogCategory::RENDERER, "Some uniforms not found in shader!");
    }
}

void Renderer::DebugGeometry() {
    BS_INFO(LogCategory::RENDERER, "=== Geometry Debug ===");

    // Check if our basic meshes exist
    const char* meshNames[] = {"cube", "sphere", "plane"};

    for (const char* meshName : meshNames) {
        const Mesh* mesh = m_geometryManager.GetMesh(meshName);
        if (!mesh) {
            BS_ERROR_F(LogCategory::RENDERER, "Mesh '%s' not found!", meshName);
            continue;
        }

        BS_INFO_F(LogCategory::RENDERER, "Mesh '%s': VAO=%d, VBO=%d, vertices=%d, indices=%d, useIndices=%s", meshName,
                  mesh->VAO, mesh->VBO, mesh->vertexCount, mesh->indexCount, mesh->useIndices ? "YES" : "NO");

        // Check if VAO is valid
        if (mesh->VAO == 0) {
            BS_ERROR_F(LogCategory::RENDERER, "Mesh '%s' has invalid VAO!", meshName);
        }

        // Bind VAO and check vertex attributes
        glBindVertexArray(mesh->VAO);

        GLint maxAttribs;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttribs);

        for (int i = 0; i < 2; ++i) {  // Check first 2 attributes (position and normal)
            GLint enabled;
            glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
            if (enabled) {
                GLint size;
                GLenum type;
                GLint stride;
                GLvoid* pointer;
                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_SIZE, &size);
                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_TYPE, (GLint*)&type);
                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &stride);
                glGetVertexAttribPointerv(i, GL_VERTEX_ATTRIB_ARRAY_POINTER, &pointer);

                BS_INFO_F(LogCategory::RENDERER, "  Attribute %d: size=%d, type=%d, stride=%d, ptr=%p", i, size, type,
                          stride, pointer);
            } else {
                BS_INFO_F(LogCategory::RENDERER, "  Attribute %d: DISABLED", i);
            }
        }

        glBindVertexArray(0);
    }
}

void Renderer::DebugMatrices() {
    BS_INFO(LogCategory::RENDERER, "=== Matrix Debug ===");

    if (!m_camera) {
        BS_ERROR(LogCategory::RENDERER, "No camera set for matrix debug!");
        return;
    }

    // Get camera matrices
    glm::mat4 view = m_camera->GetViewMatrix();
    glm::mat4 proj = m_camera->GetProjectionMatrix();
    glm::vec3 camPos = m_camera->GetPosition();

    BS_INFO_F(LogCategory::RENDERER, "Camera position: (%.2f, %.2f, %.2f)", camPos.x, camPos.y, camPos.z);

    // Print first row of each matrix to check for obvious issues
    BS_INFO_F(LogCategory::RENDERER, "View matrix [0]: %.2f, %.2f, %.2f, %.2f", view[0][0], view[0][1], view[0][2],
              view[0][3]);
    BS_INFO_F(LogCategory::RENDERER, "Proj matrix [0]: %.2f, %.2f, %.2f, %.2f", proj[0][0], proj[0][1], proj[0][2],
              proj[0][3]);

    // Create a simple test model matrix
    Transform testTransform;
    testTransform.position = glm::vec3(0, 0, -5);
    testTransform.scale = glm::vec3(1, 1, 1);
    testTransform.rotation = glm::vec3(0, 0, 0);

    glm::mat4 model = CreateModelMatrix(testTransform);
    BS_INFO_F(LogCategory::RENDERER, "Test Model matrix [0]: %.2f, %.2f, %.2f, %.2f", model[0][0], model[0][1],
              model[0][2], model[0][3]);

    // Test MVP multiplication
    glm::mat4 mvp = proj * view * model;
    BS_INFO_F(LogCategory::RENDERER, "MVP matrix [0]: %.2f, %.2f, %.2f, %.2f", mvp[0][0], mvp[0][1], mvp[0][2],
              mvp[0][3]);
}

void Renderer::DrawColliders(const std::vector<Entity>& entities) {
    if (!m_showColliders)
        return;

    // Enable wireframe mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_CULL_FACE);
    glLineWidth(2.0f);

    for (const auto& entity : entities) {
        if (entity.active && entity.hasPhysics && !entity.colliders.empty()) {
            DrawEntityCollider(entity);
        }
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
    glLineWidth(1.0f);
}

void Renderer::DrawEntityCollider(const Entity& entity) {
    // Choose collider color: green for dynamic, red for static
    glm::vec3 colliderColor = entity.isDynamic ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);

    for (const auto& collider : entity.colliders) {
        // Calculate collider transform
        Transform colliderTransform = entity.transform;

        // Apply collider local offset and scale
        colliderTransform.position += collider.center * entity.transform.scale;

        // Apply collider size to the entity scale
        glm::vec3 colliderScale = collider.size * entity.transform.scale;
        colliderTransform.scale = colliderScale;

        // Render based on collider type
        switch (collider.type) {
            case ColliderType::Box:
                DrawCube(colliderTransform, "wireframe", colliderColor);
                break;
            case ColliderType::Sphere:
                // For sphere, make scale uniform using radius
                colliderTransform.scale = glm::vec3(collider.size.x * entity.transform.scale.x * 2.0f);
                DrawSphere(colliderTransform, "wireframe", colliderColor);
                break;
            case ColliderType::Capsule:
                break;
        }
    }
}

GLuint Renderer::GetSceneTexture() const {
    if (m_postProcessManager && m_postProcessManager->IsInitialized()) {
        return m_postProcessManager->GetSceneTexture();
    }
    return 0;
}

}  // namespace Blacksite
