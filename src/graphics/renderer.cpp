#include "blacksite/graphics/renderer.h"

#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <iostream>

namespace Blacksite {
    // Basic 3D vertex shader - nothing fancy, just MVP transformation
    // NOTE: Using raw string literals because we're not savages who escape quotes
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        
        uniform mat4 uModel;
        uniform mat4 uView;
        uniform mat4 uProjection;
        
        out vec3 FragPos;
        out vec3 Normal;
        
        void main()
        {
            // Transform vertex to world space for lighting calculations
            FragPos = vec3(uModel * vec4(aPos, 1.0));
            // Transform normals properly - this is THE critical part everyone screws up
            // We need transpose(inverse(model)) to handle non-uniform scaling correctly
            Normal = mat3(transpose(inverse(uModel))) * aNormal;
            
            // Standard MVP transformation - bread and butter 3D graphics
            gl_Position = uProjection * uView * vec4(FragPos, 1.0);
        }
    )";

    // Fragment shader with Phong-ish lighting model
    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec3 FragPos;
        in vec3 Normal;
        
        out vec4 FragColor;
        
        uniform vec3 uColor;
        uniform vec3 uLightPos;
        uniform vec3 uViewPos;
        
        void main()
        {
            // Basic Phong lighting - ambient + diffuse (no specular because we're not fancy yet)
            vec3 lightColor = vec3(1.0);
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(uLightPos - FragPos);
            
            // Ambient lighting - prevents everything from being pitch black
            float ambientStrength = 0.3;
            vec3 ambient = ambientStrength * lightColor;
            
            // Diffuse lighting - the main show
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * lightColor;
            
            // Combine and call it a day
            vec3 result = (ambient + diffuse) * uColor;
            FragColor = vec4(result, 1.0);
        }
    )";

    Renderer::Renderer()
        : m_shaderProgram(0),
          m_cubeVAO(0),
          m_cubeVBO(0),
          m_sphereVAO(0),
          m_sphereVBO(0),
          m_sphereEBO(0),
          m_planeVAO(0),
          m_planeVBO(0),
          m_width(0),
          m_height(0)
    {
        // Initialize camera defaults - looking down negative Z axis (OpenGL convention)
        m_cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);
        m_cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        m_cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    }

    Renderer::~Renderer()
    {
        Shutdown();
    }

    bool Renderer::Initialize(int windowWidth, int windowHeight)
    {
        m_width = windowWidth;
        m_height = windowHeight;

        // Enable depth testing - CRITICAL or everything will render like a hot mess
        glEnable(GL_DEPTH_TEST);

        // Create and compile our shaders
        if (!CreateShaders())
        {
            std::cerr << "Failed to create shaders" << std::endl;
            return false;
        }

        // Setup all our basic geometry - cube, sphere, plane
        // TODO: This should probably be data-driven rather than hardcoded
        SetupCube();
        SetupSphere();
        SetupPlane();

        // Set viewport to match window dimensions
        glViewport(0, 0, windowWidth, windowHeight);

        std::cout << "Renderer initialized successfully" << std::endl;
        return true;
    }

    void Renderer::Shutdown()
    {
        // Clean up OpenGL resources - always clean up your mess!
        // NOTE: Checking for 0 because OpenGL generates non-zero IDs

        // Clean up cube resources
        if (m_cubeVAO != 0)
        {
            glDeleteVertexArrays(1, &m_cubeVAO);
            glDeleteBuffers(1, &m_cubeVBO);
        }

        // Clean up sphere resources
        if (m_sphereVAO != 0)
        {
            glDeleteVertexArrays(1, &m_sphereVAO);
            glDeleteBuffers(1, &m_sphereVBO);
            glDeleteBuffers(1, &m_sphereEBO);
        }

        // Clean up plane resources
        if (m_planeVAO != 0)
        {
            glDeleteVertexArrays(1, &m_planeVAO);
            glDeleteBuffers(1, &m_planeVBO);
        }

        // Clean up shader program
        if (m_shaderProgram != 0)
        {
            glDeleteProgram(m_shaderProgram);
        }
    }

    void Renderer::BeginFrame()
    {
        // Clear screen with dark gray - because pure black is for amateurs
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Bind our shader program for the frame
        glUseProgram(m_shaderProgram);

        // Set global lighting parameters - hardcoded for now (TODO: make configurable)
        // Light position is arbitrary but provides nice lighting angles
        glUniform3f(glGetUniformLocation(m_shaderProgram, "uLightPos"), 2.0f, 4.0f, 2.0f);
        glUniform3fv(glGetUniformLocation(m_shaderProgram, "uViewPos"), 1, glm::value_ptr(m_cameraPosition));
    }

    void Renderer::EndFrame()
    {
        // Unbind shader program - good practice for state management
        glUseProgram(0);
    }

    void Renderer::DrawCube(const Transform& transform, float r, float g, float b)
    {
        glBindVertexArray(m_cubeVAO);

        // Set per-object color uniform
        glUniform3f(glGetUniformLocation(m_shaderProgram, "uColor"), r, g, b);

        // Apply transformation matrices (model, view, projection)
        SetMatrices(transform);

        // Draw cube using triangle primitives - 36 vertices for 12 triangles (6 faces * 2 triangles each)
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);
    }

    void Renderer::DrawSphere(const Transform& transform, float r, float g, float b)
    {
        glBindVertexArray(m_sphereVAO);

        // Set per-object color uniform
        glUniform3f(glGetUniformLocation(m_shaderProgram, "uColor"), r, g, b);

        // Apply transformation matrices
        SetMatrices(transform);

        // Draw sphere using indexed rendering - more efficient than individual triangles
        glDrawElements(GL_TRIANGLES, m_sphereIndices.size(), GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
    }

    void Renderer::DrawPlane(const Transform& transform, float r, float g, float b)
    {
        glBindVertexArray(m_planeVAO);

        // Set per-object color uniform
        glUniform3f(glGetUniformLocation(m_shaderProgram, "uColor"), r, g, b);

        // Apply transformation matrices
        SetMatrices(transform);

        // Draw plane as 2 triangles (6 vertices)
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindVertexArray(0);
    }

    void Renderer::SetCameraPosition(const glm::vec3& position)
    {
        m_cameraPosition = position;
    }

    void Renderer::SetCameraLookAt(const glm::vec3& target)
    {
        m_cameraTarget = target;
    }

    void Renderer::OnWindowResize(int width, int height)
    {
        m_width = width;
        m_height = height;
        // Update OpenGL viewport to match new window dimensions
        glViewport(0, 0, width, height);
        // NOTE: Projection matrix will be recalculated automatically on next frame
    }

    bool Renderer::CreateShaders()
    {
        // Compile vertex shader
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
        glCompileShader(vertexShader);

        // Check for compilation errors - because debugging shader issues is hell
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
            std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
            return false;
        }

        // Compile fragment shader - same deal as vertex shader
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
            std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
            return false;
        }

        // Create shader program and link both shaders
        m_shaderProgram = glCreateProgram();
        glAttachShader(m_shaderProgram, vertexShader);
        glAttachShader(m_shaderProgram, fragmentShader);
        glLinkProgram(m_shaderProgram);

        // Check linking status - another potential point of failure
        glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(m_shaderProgram, 512, nullptr, infoLog);
            std::cerr << "Shader program linking failed: " << infoLog << std::endl;
            return false;
        }

        // Clean up individual shader objects - we don't need them after linking
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return true;
    }

    void Renderer::SetupCube()
    {
        // Cube vertices with positions and normals - 36 vertices total (6 faces * 6 vertices)
        // This is inefficient compared to indexed rendering but simpler to understand
        // Each vertex has: x, y, z, nx, ny, nz (position + normal)
        float vertices[] = {// Front face (z = 0.5)
                            -0.5f,
                            -0.5f,
                            0.5f,
                            0.0f,
                            0.0f,
                            1.0f,
                            0.5f,
                            -0.5f,
                            0.5f,
                            0.0f,
                            0.0f,
                            1.0f,
                            0.5f,
                            0.5f,
                            0.5f,
                            0.0f,
                            0.0f,
                            1.0f,
                            0.5f,
                            0.5f,
                            0.5f,
                            0.0f,
                            0.0f,
                            1.0f,
                            -0.5f,
                            0.5f,
                            0.5f,
                            0.0f,
                            0.0f,
                            1.0f,
                            -0.5f,
                            -0.5f,
                            0.5f,
                            0.0f,
                            0.0f,
                            1.0f,

                            // Back face (z = -0.5)
                            -0.5f,
                            -0.5f,
                            -0.5f,
                            0.0f,
                            0.0f,
                            -1.0f,
                            0.5f,
                            -0.5f,
                            -0.5f,
                            0.0f,
                            0.0f,
                            -1.0f,
                            0.5f,
                            0.5f,
                            -0.5f,
                            0.0f,
                            0.0f,
                            -1.0f,
                            0.5f,
                            0.5f,
                            -0.5f,
                            0.0f,
                            0.0f,
                            -1.0f,
                            -0.5f,
                            0.5f,
                            -0.5f,
                            0.0f,
                            0.0f,
                            -1.0f,
                            -0.5f,
                            -0.5f,
                            -0.5f,
                            0.0f,
                            0.0f,
                            -1.0f,

                            // Left face (x = -0.5)
                            -0.5f,
                            0.5f,
                            0.5f,
                            -1.0f,
                            0.0f,
                            0.0f,
                            -0.5f,
                            0.5f,
                            -0.5f,
                            -1.0f,
                            0.0f,
                            0.0f,
                            -0.5f,
                            -0.5f,
                            -0.5f,
                            -1.0f,
                            0.0f,
                            0.0f,
                            -0.5f,
                            -0.5f,
                            -0.5f,
                            -1.0f,
                            0.0f,
                            0.0f,
                            -0.5f,
                            -0.5f,
                            0.5f,
                            -1.0f,
                            0.0f,
                            0.0f,
                            -0.5f,
                            0.5f,
                            0.5f,
                            -1.0f,
                            0.0f,
                            0.0f,

                            // Right face (x = 0.5)
                            0.5f,
                            0.5f,
                            0.5f,
                            1.0f,
                            0.0f,
                            0.0f,
                            0.5f,
                            0.5f,
                            -0.5f,
                            1.0f,
                            0.0f,
                            0.0f,
                            0.5f,
                            -0.5f,
                            -0.5f,
                            1.0f,
                            0.0f,
                            0.0f,
                            0.5f,
                            -0.5f,
                            -0.5f,
                            1.0f,
                            0.0f,
                            0.0f,
                            0.5f,
                            -0.5f,
                            0.5f,
                            1.0f,
                            0.0f,
                            0.0f,
                            0.5f,
                            0.5f,
                            0.5f,
                            1.0f,
                            0.0f,
                            0.0f,

                            // Bottom face (y = -0.5)
                            -0.5f,
                            -0.5f,
                            -0.5f,
                            0.0f,
                            -1.0f,
                            0.0f,
                            0.5f,
                            -0.5f,
                            -0.5f,
                            0.0f,
                            -1.0f,
                            0.0f,
                            0.5f,
                            -0.5f,
                            0.5f,
                            0.0f,
                            -1.0f,
                            0.0f,
                            0.5f,
                            -0.5f,
                            0.5f,
                            0.0f,
                            -1.0f,
                            0.0f,
                            -0.5f,
                            -0.5f,
                            0.5f,
                            0.0f,
                            -1.0f,
                            0.0f,
                            -0.5f,
                            -0.5f,
                            -0.5f,
                            0.0f,
                            -1.0f,
                            0.0f,

                            // Top face (y = 0.5)
                            -0.5f,
                            0.5f,
                            -0.5f,
                            0.0f,
                            1.0f,
                            0.0f,
                            0.5f,
                            0.5f,
                            -0.5f,
                            0.0f,
                            1.0f,
                            0.0f,
                            0.5f,
                            0.5f,
                            0.5f,
                            0.0f,
                            1.0f,
                            0.0f,
                            0.5f,
                            0.5f,
                            0.5f,
                            0.0f,
                            1.0f,
                            0.0f,
                            -0.5f,
                            0.5f,
                            0.5f,
                            0.0f,
                            1.0f,
                            0.0f,
                            -0.5f,
                            0.5f,
                            -0.5f,
                            0.0f,
                            1.0f,
                            0.0f};

        // Generate and setup VAO/VBO for cube
        glGenVertexArrays(1, &m_cubeVAO);
        glGenBuffers(1, &m_cubeVBO);

        glBindVertexArray(m_cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Setup vertex attributes - position at location 0
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
        glEnableVertexAttribArray(0);

        // Setup vertex attributes - normal at location 1
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void Renderer::SetupSphere()
    {
        // Generate sphere geometry procedurally - 30x30 tessellation for decent quality
        GenerateSphere(0.5f, 30, 30);

        // Setup VAO/VBO/EBO for sphere
        glGenVertexArrays(1, &m_sphereVAO);
        glGenBuffers(1, &m_sphereVBO);
        glGenBuffers(1, &m_sphereEBO);

        glBindVertexArray(m_sphereVAO);

        // Upload vertex data
        glBindBuffer(GL_ARRAY_BUFFER, m_sphereVBO);
        glBufferData(GL_ARRAY_BUFFER, m_sphereVertices.size() * sizeof(float), &m_sphereVertices[0], GL_STATIC_DRAW);

        // Upload index data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_sphereEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     m_sphereIndices.size() * sizeof(unsigned int),
                     &m_sphereIndices[0],
                     GL_STATIC_DRAW);

        // Setup vertex attributes - same layout as cube
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void Renderer::SetupPlane()
    {
        // Simple horizontal plane - just 2 triangles forming a quad
        // Normal points up (0, 1, 0) for consistent lighting
        float vertices[] = {// Triangle 1
                            -0.5f,
                            0.0f,
                            -0.5f,
                            0.0f,
                            1.0f,
                            0.0f,
                            0.5f,
                            0.0f,
                            -0.5f,
                            0.0f,
                            1.0f,
                            0.0f,
                            0.5f,
                            0.0f,
                            0.5f,
                            0.0f,
                            1.0f,
                            0.0f,

                            // Triangle 2
                            0.5f,
                            0.0f,
                            0.5f,
                            0.0f,
                            1.0f,
                            0.0f,
                            -0.5f,
                            0.0f,
                            0.5f,
                            0.0f,
                            1.0f,
                            0.0f,
                            -0.5f,
                            0.0f,
                            -0.5f,
                            0.0f,
                            1.0f,
                            0.0f};

        // Setup VAO/VBO for plane
        glGenVertexArrays(1, &m_planeVAO);
        glGenBuffers(1, &m_planeVBO);

        glBindVertexArray(m_planeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_planeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Setup vertex attributes - same as cube/sphere
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void Renderer::GenerateSphere(float radius, int sectors, int stacks)
    {
        // Clear previous sphere data
        m_sphereVertices.clear();
        m_sphereIndices.clear();

        const float PI = 3.14159265359f;

        // Generate vertices using spherical coordinates
        // Stack angle goes from +90 to -90 degrees (top to bottom)
        // Sector angle goes from 0 to 360 degrees (around)
        for (int i = 0; i <= stacks; ++i)
        {
            float stackAngle = PI / 2 - i * PI / stacks;  // from pi/2 to -pi/2
            float xy = radius * cosf(stackAngle);  // r * cos(u)
            float z = radius * sinf(stackAngle);  // r * sin(u)

            for (int j = 0; j <= sectors; ++j)
            {
                float sectorAngle = j * 2 * PI / sectors;  // from 0 to 2pi

                // Calculate vertex position
                float x = xy * cosf(sectorAngle);  // r * cos(u) * cos(v)
                float y = xy * sinf(sectorAngle);  // r * cos(u) * sin(v)

                // Add position to vertex buffer
                m_sphereVertices.push_back(x);
                m_sphereVertices.push_back(y);
                m_sphereVertices.push_back(z);

                // Add normal (for unit sphere, normal = normalized position)
                m_sphereVertices.push_back(x / radius);
                m_sphereVertices.push_back(y / radius);
                m_sphereVertices.push_back(z / radius);
            }
        }

        // Generate triangle indices for the sphere
        // Each quad (except poles) gets split into 2 triangles
        for (int i = 0; i < stacks; ++i)
        {
            int k1 = i * (sectors + 1);  // beginning of current stack
            int k2 = k1 + sectors + 1;  // beginning of next stack

            for (int j = 0; j < sectors; ++j, ++k1, ++k2)
            {
                // Skip triangles at the top pole
                if (i != 0)
                {
                    m_sphereIndices.push_back(k1);
                    m_sphereIndices.push_back(k2);
                    m_sphereIndices.push_back(k1 + 1);
                }

                // Skip triangles at the bottom pole
                if (i != (stacks - 1))
                {
                    m_sphereIndices.push_back(k1 + 1);
                    m_sphereIndices.push_back(k2);
                    m_sphereIndices.push_back(k2 + 1);
                }
            }
        }
    }

    void Renderer::SetMatrices(const Transform& transform)
    {
        // Calculate transformation matrices
        glm::mat4 model = CreateTransformMatrix(transform);
        glm::mat4 view = CreateViewMatrix();
        glm::mat4 projection = CreateProjectionMatrix();

        // Upload matrices to shader uniforms
        // NOTE: OpenGL expects column-major matrices, GLM provides them correctly
        glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "uModel"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "uView"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "uProjection"),
                           1,
                           GL_FALSE,
                           glm::value_ptr(projection));
    }

    glm::mat4 Renderer::CreateTransformMatrix(const Transform& transform)
    {
        glm::mat4 model = glm::mat4(1.0f);

        // Apply transformations in TRS order: Translation * Rotation * Scale
        // This is the standard order - changing it will break expectations
        model = glm::translate(model, transform.position);

        // Apply rotations in XYZ order (Euler angles)
        // WARNING: This suffers from gimbal lock - consider quaternions for production
        model = glm::rotate(model, glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        model = glm::scale(model, transform.scale);

        return model;
    }

    glm::mat4 Renderer::CreateViewMatrix()
    {
        // Create view matrix using lookAt function
        // This creates a camera that looks from m_cameraPosition towards m_cameraTarget
        // with m_cameraUp defining the "up" direction
        return glm::lookAt(m_cameraPosition, m_cameraTarget, m_cameraUp);
    }

    glm::mat4 Renderer::CreateProjectionMatrix()
    {
        // Create perspective projection matrix
        // 45 degree FOV, aspect ratio based on window dimensions
        // Near plane at 0.1, far plane at 100.0
        float aspect = (float) m_width / (float) m_height;
        return glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    }

}  // namespace Blacksite