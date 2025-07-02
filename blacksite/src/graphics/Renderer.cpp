#include <cstdlib>
#include <glm/gtc/type_ptr.hpp>

#include "blacksite/core/Logger.h"
#include "blacksite/graphics/Renderer.h"

namespace Blacksite {

// Shader sources - keeping them here for now.

const char* BASIC_VERTEX_SHADER = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;      // Vertex position
    layout (location = 1) in vec3 aNormal;   // Vertex normal

    uniform mat4 uModel;      // Model matrix (object -> world)
    uniform mat4 uView;       // View matrix (world -> camera)
    uniform mat4 uProjection; // Projection matrix (camera -> screen)

    out vec3 FragPos;   // World space position for lighting
    out vec3 Normal;    // World space normal for lighting

    void main()
    {
        // Transform vertex to world space for lighting calculations
        FragPos = vec3(uModel * vec4(aPos, 1.0));

        // Transform normal to world space (properly handling non-uniform scaling)
        Normal = mat3(transpose(inverse(uModel))) * aNormal;

        // Final vertex position in clip space
        gl_Position = uProjection * uView * vec4(FragPos, 1.0);
    }
)";

const char* BASIC_FRAGMENT_SHADER = R"(
    #version 330 core
    in vec3 FragPos;    // World space position from vertex shader
    in vec3 Normal;     // World space normal from vertex shader

    out vec4 FragColor; // Final pixel color

    uniform vec3 uColor;    // Object color
    uniform vec3 uLightPos; // Light position in world space
    uniform vec3 uViewPos;  // Camera position in world space

    void main()
    {
        // Basic Phong lighting model - not fancy but gets the job done
        vec3 lightColor = vec3(1.0);  // White light
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(uLightPos - FragPos);

        // Ambient lighting - so objects aren't completely black
        float ambientStrength = 0.3;
        vec3 ambient = ambientStrength * lightColor;

        // Diffuse lighting - the main lighting component
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        // Combine ambient and diffuse (no specular because we're keeping it simple)
        vec3 result = (ambient + diffuse) * uColor;
        FragColor = vec4(result, 1.0);
    }
)";

// Rainbow animated shader
const char* RAINBOW_VERTEX_SHADER = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;

    uniform mat4 uModel;
    uniform mat4 uView;
    uniform mat4 uProjection;
    uniform float uTime;

    out vec3 FragPos;
    out vec3 Normal;
    out float Time;

    void main()
    {
        FragPos = vec3(uModel * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(uModel))) * aNormal;
        Time = uTime;

        gl_Position = uProjection * uView * vec4(FragPos, 1.0);
    }
)";

const char* RAINBOW_FRAGMENT_SHADER = R"(
    #version 330 core
    in vec3 FragPos;
    in vec3 Normal;
    in float Time;

    out vec4 FragColor;

    uniform vec3 uLightPos;
    uniform vec3 uViewPos;

    vec3 hsv2rgb(vec3 c) {
        vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    void main()
    {
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(uLightPos - FragPos);

        float hue = fract(Time * 0.5 + FragPos.x * 0.1 + FragPos.y * 0.1);
        vec3 rainbowColor = hsv2rgb(vec3(hue, 0.8, 1.0));

        float ambientStrength = 0.3;
        vec3 ambient = ambientStrength * vec3(1.0);

        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * vec3(1.0);

        vec3 result = (ambient + diffuse) * rainbowColor;
        FragColor = vec4(result, 1.0);
    }
)";

// Pulsing glow shader
const char* GLOW_VERTEX_SHADER = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;

    uniform mat4 uModel;
    uniform mat4 uView;
    uniform mat4 uProjection;
    uniform float uTime;

    out vec3 FragPos;
    out vec3 Normal;
    out float Time;

    void main()
    {
        FragPos = vec3(uModel * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(uModel))) * aNormal;
        Time = uTime;

        gl_Position = uProjection * uView * vec4(FragPos, 1.0);
    }
)";

const char* GLOW_FRAGMENT_SHADER = R"(
    #version 330 core
    in vec3 FragPos;
    in vec3 Normal;
    in float Time;

    out vec4 FragColor;

    uniform vec3 uColor;
    uniform vec3 uLightPos;
    uniform vec3 uViewPos;

    void main()
    {
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(uLightPos - FragPos);
        vec3 viewDir = normalize(uViewPos - FragPos);

        // Pulsing glow effect
        float pulse = sin(Time * 3.0) * 0.5 + 0.5;
        float glow = pow(pulse, 2.0) * 2.0;

        // Enhanced lighting with glow
        float ambientStrength = 0.4 + glow * 0.3;
        vec3 ambient = ambientStrength * uColor;

        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * uColor;

        // Add specular highlight
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = spec * vec3(1.0) * glow;

        vec3 result = ambient + diffuse + specular;
        FragColor = vec4(result, 1.0);
    }
)";

// Holographic shader
const char* HOLO_VERTEX_SHADER = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;

    uniform mat4 uModel;
    uniform mat4 uView;
    uniform mat4 uProjection;
    uniform float uTime;

    out vec3 FragPos;
    out vec3 Normal;
    out float Time;
    out vec3 WorldPos;

    void main()
    {
        FragPos = vec3(uModel * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(uModel))) * aNormal;
        Time = uTime;
        WorldPos = aPos;

        gl_Position = uProjection * uView * vec4(FragPos, 1.0);
    }
)";

const char* HOLO_FRAGMENT_SHADER = R"(
    #version 330 core
    in vec3 FragPos;
    in vec3 Normal;
    in float Time;
    in vec3 WorldPos;

    out vec4 FragColor;

    uniform vec3 uColor;
    uniform vec3 uLightPos;
    uniform vec3 uViewPos;

    void main()
    {
        vec3 norm = normalize(Normal);
        vec3 viewDir = normalize(uViewPos - FragPos);

        // Holographic interference pattern
        float interference = sin(WorldPos.y * 20.0 + Time * 2.0) * 0.5 + 0.5;
        interference *= sin(WorldPos.x * 15.0 + Time * 1.5) * 0.5 + 0.5;

        // Fresnel effect for holographic look
        float fresnel = 1.0 - max(dot(norm, viewDir), 0.0);
        fresnel = pow(fresnel, 2.0);

        // Combine effects
        vec3 holoColor = mix(uColor * 0.3, uColor * 2.0, interference);
        holoColor = mix(holoColor, vec3(0.0, 1.0, 1.0), fresnel * 0.5);

        // Add transparency based on interference
        float alpha = 0.7 + interference * 0.3;

        FragColor = vec4(holoColor, alpha);
    }
)";

// Wireframe shader
const char* WIREFRAME_VERTEX_SHADER = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;

    uniform mat4 uModel;
    uniform mat4 uView;
    uniform mat4 uProjection;

    void main()
    {
        gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
    }
)";

const char* WIREFRAME_FRAGMENT_SHADER = R"(
    #version 330 core
    out vec4 FragColor;

    uniform vec3 uColor;

    void main()
    {
        FragColor = vec4(uColor, 1.0);
    }
)";


// Plasma shader
const char* PLASMA_VERTEX_SHADER = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;

    uniform mat4 uModel;
    uniform mat4 uView;
    uniform mat4 uProjection;
    uniform float uTime;

    out vec3 FragPos;
    out vec3 Normal;
    out float Time;
    out vec3 LocalPos;

    void main()
    {
        FragPos = vec3(uModel * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(uModel))) * aNormal;
        Time = uTime;
        LocalPos = aPos;

        gl_Position = uProjection * uView * vec4(FragPos, 1.0);
    }
)";

const char* PLASMA_FRAGMENT_SHADER = R"(
    #version 330 core
    in vec3 FragPos;
    in vec3 Normal;
    in float Time;
    in vec3 LocalPos;

    out vec4 FragColor;

    uniform vec3 uLightPos;
    uniform vec3 uViewPos;

    void main()
    {
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(uLightPos - FragPos);

        // Plasma effect
        float plasma = sin(LocalPos.x * 4.0 + Time);
        plasma += sin(LocalPos.y * 3.0 + Time * 1.5);
        plasma += sin((LocalPos.x + LocalPos.y) * 2.0 + Time * 2.0);
        plasma += sin(sqrt(LocalPos.x * LocalPos.x + LocalPos.y * LocalPos.y) * 5.0 + Time * 0.5);
        plasma = plasma / 4.0;

        // Convert to color
        vec3 plasmaColor;
        plasmaColor.r = sin(plasma * 3.14159 + Time) * 0.5 + 0.5;
        plasmaColor.g = sin(plasma * 3.14159 + Time + 2.0) * 0.5 + 0.5;
        plasmaColor.b = sin(plasma * 3.14159 + Time + 4.0) * 0.5 + 0.5;

        // Basic lighting
        float ambientStrength = 0.4;
        vec3 ambient = ambientStrength * plasmaColor;

        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * plasmaColor;

        vec3 result = ambient + diffuse;
        FragColor = vec4(result, 1.0);
    }
)";

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

    BS_INFO(LogCategory::RENDERER, "Renderer initialized successfully");
    return true;
}

void Renderer::Shutdown() {
    // Clean up all our GPU resources
    m_shaderManager.Cleanup();
    m_geometryManager.Cleanup();

    BS_INFO(LogCategory::RENDERER, "Renderer shut down cleanly");
}

void Renderer::BeginFrame() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_renderQueue.clear();
}

void Renderer::EndFrame() {
    // Execute all queued render commands
    Flush();
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

    BS_DEBUG_F(LogCategory::RENDERER, "Renderer resized to %dx%d", width, height);
}

void Renderer::SetCamera(Camera* camera) {
    m_camera = camera;
}

void Renderer::SetupDefaultShaders() {
    bool success = true;

    // Load all our cool shaders
    success &= m_shaderManager.LoadShader("basic", BASIC_VERTEX_SHADER, BASIC_FRAGMENT_SHADER);
    success &= m_shaderManager.LoadShader("rainbow", RAINBOW_VERTEX_SHADER, RAINBOW_FRAGMENT_SHADER);
    success &= m_shaderManager.LoadShader("glow", GLOW_VERTEX_SHADER, GLOW_FRAGMENT_SHADER);
    success &= m_shaderManager.LoadShader("holo", HOLO_VERTEX_SHADER, HOLO_FRAGMENT_SHADER);
    success &= m_shaderManager.LoadShader("wireframe", WIREFRAME_VERTEX_SHADER, WIREFRAME_FRAGMENT_SHADER);
    success &= m_shaderManager.LoadShader("plasma", PLASMA_VERTEX_SHADER, PLASMA_FRAGMENT_SHADER);

    if (!success) {
        BS_FATAL(LogCategory::RENDERER, "Failed to load one or more shaders - everything will be broken!");
        std::exit(EXIT_FAILURE);
    }

    BS_INFO(LogCategory::RENDERER, "All shaders loaded successfully (basic, rainbow, glow, holo, wireframe, plasma)");
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

    // Use the specified shader
    if (!m_shaderManager.UseShader(command.shaderName)) {
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

    m_shaderManager.SetUniform("uModel", model);
    m_shaderManager.SetUniform("uView", view);
    m_shaderManager.SetUniform("uProjection", proj);

    // Set up lighting uniforms
    m_shaderManager.SetUniform("uLightPos", m_lightPosition);
    m_shaderManager.SetUniform("uViewPos", m_camera->GetPosition());
    m_shaderManager.SetUniform("uColor", command.color);

    // DEBUG: Check OpenGL state before drawing
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        BS_ERROR_F(LogCategory::RENDERER, "OpenGL error before draw: %d", error);
    }

    // Actually draw the mesh
    glBindVertexArray(mesh->VAO);
    if (mesh->useIndices) {
        glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, mesh->vertexCount);
    }
    glBindVertexArray(0);

    // DEBUG: Check OpenGL state after drawing
    error = glGetError();
    if (error != GL_NO_ERROR) {
        BS_ERROR_F(LogCategory::RENDERER, "OpenGL error after draw: %d", error);
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

    if (!m_shaderManager.UseShader("basic")) {
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

}  // namespace Blacksite
