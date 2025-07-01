#include "blacksite/graphics/GeometryManager.h"
#include "blacksite/core/Logger.h"

#include <cmath>

namespace Blacksite {

GeometryManager::GeometryManager() = default;

GeometryManager::~GeometryManager() {
    Cleanup();  // Don't leak GPU memory like a rookie
}

void GeometryManager::CreateCube(const std::string& name) {
    std::vector<float> vertices = {
        // Front face (z = 0.5) - the face that says "hello world"
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

        // Back face (z = -0.5) - the mysterious back side
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.5f, 0.5f, -0.5f, 0.0f, 0.0f,
        -1.0f, 0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, -0.5f, -0.5f, -0.5f, 0.0f,
        0.0f, -1.0f,

        // Left face (x = -0.5) - for the southpaws
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,
        0.0f, -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, -0.5f, 0.5f, 0.5f, -1.0f,
        0.0f, 0.0f,

        // Right face (x = 0.5) - the dominant hand's favorite
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,

        // Bottom face (y = -0.5) - where dreams go to die
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.5f, -0.5f, 0.5f, 0.0f, -1.0f,
        0.0f, 0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f,
        -1.0f, 0.0f,

        // Top face (y = 0.5) - the crown jewel
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f};

    m_meshes[name] = CreateMesh(vertices);
    m_meshes[name].vertexCount = 36;  // 6 faces * 2 triangles * 3 vertices

    BS_DEBUG_F(LogCategory::RENDERER, "Cube geometry '%s' created", name.c_str());
}

void GeometryManager::CreateSphere(const std::string& name, float radius, int sectors, int stacks) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    // Here be dragons
    GenerateSphere(vertices, indices, radius, sectors, stacks);

    m_meshes[name] = CreateMesh(vertices, indices);
    m_meshes[name].useIndices = true;
    m_meshes[name].indexCount = indices.size();

    BS_DEBUG_F(LogCategory::RENDERER, "Sphere geometry '%s' created with %zu vertices, %zu indices", 
               name.c_str(), vertices.size() / 6, indices.size());
}

void GeometryManager::CreatePlane(const std::string& name) {
    // Simple ground plane - two triangles forming a square
    std::vector<float> vertices = {
        // Triangle 1
        -0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f,  // Bottom-left
        0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f,   // Bottom-right
        0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f,    // Top-right

        // Triangle 2
        0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f,   // Top-right
        -0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f,  // Top-left
        -0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f  // Bottom-left
    };

    m_meshes[name] = CreateMesh(vertices);
    m_meshes[name].vertexCount = 6;  // 2 triangles * 3 vertices

    BS_DEBUG_F(LogCategory::RENDERER, "Plane geometry '%s' created", name.c_str());
}

const Mesh* GeometryManager::GetMesh(const std::string& name) const {
    auto it = m_meshes.find(name);
    if (it != m_meshes.end()) {
        return &it->second;
    }

    BS_ERROR_F(LogCategory::RENDERER, "Mesh '%s' not found! Did you forget to create it?", name.c_str());
    return nullptr;
}

void GeometryManager::Cleanup() {
    // Delete all GPU resources
    for (auto& pair : m_meshes) {
        Mesh& mesh = pair.second;
        if (mesh.VAO != 0) {
            glDeleteVertexArrays(1, &mesh.VAO);
        }
        if (mesh.VBO != 0) {
            glDeleteBuffers(1, &mesh.VBO);
        }
        if (mesh.EBO != 0) {
            glDeleteBuffers(1, &mesh.EBO);
        }
    }
    m_meshes.clear();

    BS_DEBUG(LogCategory::RENDERER, "GeometryManager cleaned up");
}

void GeometryManager::GenerateSphere(std::vector<float>& vertices, std::vector<unsigned int>& indices, float radius,
                                     int sectors, int stacks) {
    vertices.clear();
    indices.clear();

    const float PI = 3.14159265359f;  // Because M_PI isn't always available

    // Generate vertices using spherical coordinates
    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = PI / 2 - i * PI / stacks;  // from pi/2 to -pi/2 (top to bottom)
        float xy = radius * cosf(stackAngle);         // r * cos(u)
        float z = radius * sinf(stackAngle);          // r * sin(u)

        // Add vertices around the current stack
        for (int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * 2 * PI / sectors;  // from 0 to 2pi (around the circle)

            // Calculate vertex position
            float x = xy * cosf(sectorAngle);  // r * cos(u) * cos(v)
            float y = xy * sinf(sectorAngle);  // r * cos(u) * sin(v)

            // Add position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Add normal (for a unit sphere, normal = normalized position)
            vertices.push_back(x / radius);
            vertices.push_back(y / radius);
            vertices.push_back(z / radius);
        }
    }

    // Generate triangle indices
    for (int i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1);  // Current stack
        int k2 = k1 + sectors + 1;   // Next stack

        for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
            // Skip the top cap
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            // Skip the bottom cap
            if (i != (stacks - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}

Mesh GeometryManager::CreateMesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
    Mesh mesh;

    // Generate OpenGL objects
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);

    glBindVertexArray(mesh.VAO);

    // Upload vertex data to GPU
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    // Tell OpenGL how to interpret our vertex data
    // Position attribute (location 0): 3 floats starting at offset 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute (location 1): 3 floats starting at offset 3
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Setup element buffer if we have indices
    if (!indices.empty()) {
        glGenBuffers(1, &mesh.EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        mesh.useIndices = true;
        mesh.indexCount = indices.size();
    }

    // Unbind to prevent accidental modification
    glBindVertexArray(0);

    return mesh;
}

}  // namespace Blacksite