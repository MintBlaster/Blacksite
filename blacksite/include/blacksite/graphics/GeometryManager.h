#pragma once

#include <GL/glew.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace Blacksite {

/**
 * Mesh - A collection of vertices that hopefully forms something recognizable
 * Contains all the OpenGL handles
 */

struct Mesh {
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO = 0;
    unsigned int indexCount = 0;
    bool useIndices = false;
    unsigned int vertexCount = 0;
};

class GeometryManager {
  public:
    GeometryManager();
    ~GeometryManager();

    // Create basic primitives
    void CreateCube(const std::string& name = "cube");

    // Spheres are surprisingly annoying to generate correctly
    // Default params should work for most "I just need a ball" scenarios
    void CreateSphere(const std::string& name = "sphere", float radius = 0.5f, int sectors = 30, int stacks = 30);
    void CreatePlane(const std::string& name = "plane");

    // Get mesh for rendering
    const Mesh* GetMesh(const std::string& name) const;

    // Cleanup all the GPU resources
    void Cleanup();

  private:
    // Geometry cache
    std::unordered_map<std::string, Mesh> m_meshes;

    // Sphere generation math
    void GenerateSphere(std::vector<float>& vertices, std::vector<unsigned int>& indices, float radius, int sectors,
                        int stacks);

    // Creates the actual OpenGL mesh from vertex data
    Mesh CreateMesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices = {});
};
}  // namespace Blacksite