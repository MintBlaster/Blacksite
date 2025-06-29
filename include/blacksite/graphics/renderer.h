#pragma once

#include <GL/glew.h>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

namespace Blacksite
{

    struct Transform
    {
        glm::vec3 position{0.0f};
        glm::vec3 rotation{0.0f};
        glm::vec3 scale{1.0f};
    };

    class Renderer
    {

      public:
        Renderer();
        ~Renderer();

        // --- Initialize renderer ---
        bool Initialize(int width, int height);
        void Shutdown();

        // --- Frame Management ---
        void BeginFrame();
        void EndFrame();

        // --- 3D rendering commands ---
        void DrawCube(const Transform& transform, float r = 1.0f, float g = 1.0f, float b = 1.0f);
        void DrawSphere(const Transform& transform, float r = 1.0f, float g = 1.0f, float b = 1.0f);
        void DrawPlane(const Transform& transform, float r = 1.0f, float g = 1.0f, float b = 1.0f);

        // --- Camera controls ---
        void SetCameraPosition(const glm::vec3& position);
        void SetCameraLookAt(const glm::vec3& target);

        // --- Window Resizing ---
        void OnWindowResize(int width, int height);

      private:
        // --- Shader program ---
        unsigned int m_shaderProgram;

        // --- Geometry buffers ---
        unsigned int m_cubeVAO, m_cubeVBO;
        unsigned int m_sphereVAO, m_sphereVBO, m_sphereEBO;
        unsigned int m_planeVAO, m_planeVBO;

        // --- Sphere data ---
        std::vector<float> m_sphereVertices;
        std::vector<unsigned int> m_sphereIndices;

        // --- Camera data ---
        glm::vec3 m_cameraPosition{0.0f, 0.0f, 5.0f};
        glm::vec3 m_cameraTarget{0.0f, 0.0f, 0.0f};
        glm::vec3 m_cameraUp{0.0f, 1.0f, 0.0f};

        // --- Viewport info ---
        int m_width, m_height;

        // --- Internal methods ---
        bool CreateShaders();                                        // Compiles and links shader program
        void SetupCube();                                            // Uploads cube vertex data to GPU
        void SetupSphere();                                          // Uses GenerateSphere(), uploads to GPU
        void SetupPlane();                                           // Creates basic ground plane geometry
        void GenerateSphere(float radius, int sectors, int stacks);  // Generates sphere vertex/index buffers
        void SetMatrices(const Transform& transform);                // Applies model/view/proj to shader
        glm::mat4 CreateTransformMatrix(const Transform& transform); // Build model matrix from position/rot/scale
        glm::mat4 CreateViewMatrix();                                // Camera transform → view matrix
        glm::mat4 CreateProjectionMatrix();                          // Perspective projection from screen size
    };

} // namespace Blacksite
