#include <cstdlib>
#include <glm/gtc/type_ptr.hpp>

#include "blacksite/graphics/Renderer.h"
#include "blacksite/core/Logger.h"

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

    // Setup camera with reasonable defaults
    m_camera.SetPerspective(45.0f, (float)width / height, 0.1f, 100.0f);

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
    // Clear the screen - dark gray because pure black is for amateurs
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Clear the render queue for this frame
    m_renderQueue.clear();
}

void Renderer::EndFrame() {
    // Execute all queued render commands
    Flush();
}

void Renderer::DrawCube(const Transform& transform, const glm::vec3& color) {
    RenderCommand cmd;
    cmd.meshName = "cube";
    cmd.transform = transform;
    cmd.color = color;
    Submit(cmd);  // Queue it up for later rendering
}

void Renderer::DrawSphere(const Transform& transform, const glm::vec3& color) {
    RenderCommand cmd;
    cmd.meshName = "sphere";
    cmd.transform = transform;
    cmd.color = color;
    Submit(cmd);
}

void Renderer::DrawPlane(const Transform& transform, const glm::vec3& color) {
    RenderCommand cmd;
    cmd.meshName = "plane";
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
    m_camera.SetAspectRatio((float)width / height);

    BS_DEBUG_F(LogCategory::RENDERER, "Renderer resized to %dx%d", width, height);
}

void Renderer::SetupDefaultShaders() {
    // Load our basic shader
    bool success = m_shaderManager.LoadShader("basic", BASIC_VERTEX_SHADER, BASIC_FRAGMENT_SHADER);
    if (!success) {
        BS_FATAL(LogCategory::RENDERER, "Failed to load basic shader - everything will be broken!");
        std::exit(EXIT_FAILURE);
    }
}

void Renderer::SetupDefaultGeometry() {
    // Create our basic geometric primitives
    m_geometryManager.CreateCube();
    m_geometryManager.CreateSphere();
    m_geometryManager.CreatePlane();

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
    m_shaderManager.UseShader(command.shaderName);

    // Set up matrices for the GPU
    glm::mat4 model = CreateModelMatrix(command.transform);
    m_shaderManager.SetUniform("uModel", model);
    m_shaderManager.SetUniform("uView", m_camera.GetViewMatrix());
    m_shaderManager.SetUniform("uProjection", m_camera.GetProjectionMatrix());

    // Set up lighting uniforms
    m_shaderManager.SetUniform("uLightPos", m_lightPosition);
    m_shaderManager.SetUniform("uViewPos", m_camera.GetPosition());
    m_shaderManager.SetUniform("uColor", command.color);

    // Actually draw the mesh
    glBindVertexArray(mesh->VAO);
    if (mesh->useIndices) {
        // Draw using indices
        glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);
    } else {
        // Draw raw triangles
        glDrawArrays(GL_TRIANGLES, 0, mesh->vertexCount);
    }
    glBindVertexArray(0);  // Unbind to be safe
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

}  // namespace Blacksite