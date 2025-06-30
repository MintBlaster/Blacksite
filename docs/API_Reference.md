# Blacksite Engine API Reference

## Overview

Blacksite is a modern C++ game engine with a clean, fluent API design. The engine uses OpenGL for rendering and provides simple primitives for rapid prototyping and game development.

**Version:** 1.0.0  
**Namespace:** `Blacksite`  
**Dependencies:** OpenGL, GLEW, GLFW, GLM

---

## Quick Start

```cpp
#include "blacksite/Blacksite.h"

int main() {
    Blacksite::Engine engine;
    
    if (!engine.Initialize(1280, 720, "My Game")) {
        return -1;
    }
    
    // Spawn some objects
    int cubeId = engine.SpawnCube({0, 0, 0});
    engine.GetEntity(cubeId).Color(1.0f, 0.0f, 0.0f).Scale(2.0f);
    
    // Set up camera
    engine.SetCameraPosition({5, 5, 5});
    engine.SetCameraTarget({0, 0, 0});
    
    // Game loop
    engine.SetUpdateCallback([&](Blacksite::Engine& engine, float deltaTime) {
        // Your game logic here
    });
    
    return engine.Run();
}
```

---

## Core Classes

### Engine

The main engine class that manages the entire application lifecycle.

#### Initialization & Lifecycle

```cpp
Blacksite::Engine engine;
```

**`bool Initialize(int width = 1280, int height = 720, const std::string& title = "Blacksite Engine")`**
- Initializes the engine, creates window and graphics context
- **Parameters:**
  - `width` - Window width in pixels
  - `height` - Window height in pixels  
  - `title` - Window title
- **Returns:** `true` if successful, `false` on failure
- **Note:** Must be called before any other engine operations

**`int Run()`**
- Starts the main engine loop
- **Returns:** Exit code (0 for success)
- **Note:** Blocks until application exits

**`void Shutdown()`**
- Cleanly shuts down the engine and releases resources
- **Note:** Called automatically in destructor

**`bool IsRunning() const`**
- **Returns:** `true` if engine is currently running

#### Entity Creation

**`int SpawnCube(const glm::vec3& position = {0, 0, 0})`**
- Creates a cube entity at the specified position
- **Parameters:** `position` - World position (default: origin)
- **Returns:** Unique entity ID for manipulation

**`int SpawnSphere(const glm::vec3& position = {0, 0, 0})`**
- Creates a sphere entity at the specified position
- **Parameters:** `position` - World position (default: origin)  
- **Returns:** Unique entity ID for manipulation

**`int SpawnPlane(const glm::vec3& position = {0, 0, 0})`**
- Creates a plane entity at the specified position
- **Parameters:** `position` - World position (default: origin)
- **Returns:** Unique entity ID for manipulation

#### Entity Manipulation

**`EntityHandle GetEntity(int id)`**
- Gets a handle for fluent-style entity manipulation
- **Parameters:** `id` - Entity ID returned from spawn functions
- **Returns:** EntityHandle for chaining operations
- **Example:**
```cpp
int cubeId = engine.SpawnCube();
engine.GetEntity(cubeId).At({1, 2, 3}).Color(1, 0, 0).Scale(2.0f);
```

#### Camera Control

**`void SetCameraPosition(const glm::vec3& position)`**
- Sets the camera's world position
- **Parameters:** `position` - Camera position in world space

**`void SetCameraTarget(const glm::vec3& target)`**
- Sets what the camera is looking at
- **Parameters:** `target` - Target position in world space

#### Game Loop Integration

**`void SetUpdateCallback(UpdateCallback callback)`**
- Sets the per-frame update function
- **Parameters:** `callback` - Function called every frame
- **Callback Signature:** `void(Engine& engine, float deltaTime)`
- **Example:**
```cpp
engine.SetUpdateCallback([&](Blacksite::Engine& engine, float deltaTime) {
    // Update game logic
    gameTime += deltaTime;
    
    // Move entities
    engine.GetEntity(playerId).At({sin(gameTime), 0, 0});
});
```

---

### EntityHandle (Fluent API)

Provides chainable operations for entity manipulation.

#### Transform Operations

**`EntityHandle& At(const glm::vec3& position)`**
- Sets entity position in world space
- **Returns:** Self reference for chaining

**`EntityHandle& Rotate(const glm::vec3& rotation)`**
- Sets entity rotation (Euler angles in degrees)
- **Parameters:** `rotation` - X, Y, Z rotation in degrees

**`EntityHandle& Scale(const glm::vec3& scale)`**
- Sets non-uniform scale
- **Parameters:** `scale` - Scale factors for X, Y, Z axes

**`EntityHandle& Scale(float x, float y, float z)`**
- Sets non-uniform scale with individual components
- **Parameters:** Individual scale factors

**`EntityHandle& Scale(float uniformScale)`**
- Sets uniform scale for all axes
- **Parameters:** `uniformScale` - Scale factor applied to all axes

#### Appearance

**`EntityHandle& Color(float r, float g, float b)`**
- Sets entity color (RGB values 0.0-1.0)
- **Parameters:** Red, green, blue components

**`EntityHandle& Color(const glm::vec3& color)`**
- Sets entity color using GLM vector
- **Parameters:** `color` - RGB color vector

#### State Management

**`EntityHandle& SetActive(bool active)`**
- Sets entity visibility/active state
- **Parameters:** `active` - Whether entity should be rendered

**`void Destroy()`**
- Marks entity for destruction
- **Note:** Entity will be removed on next frame

**`int GetId() const`**
- **Returns:** The entity's unique ID

#### Chaining Examples

```cpp
// Chaining operations
engine.GetEntity(id)
    .At({10, 0, 0})
    .Rotate({0, 45, 0})
    .Scale(2.0f)
    .Color(1, 0, 0);

// Individual operations
auto entity = engine.GetEntity(id);
entity.At({5, 5, 5});
entity.Color(0, 1, 0);
```

---

## Rendering System

### Renderer

Core rendering subsystem that handles all graphics operations.

#### Lifecycle

**`bool Initialize(int width, int height)`**
- Initializes the rendering system
- **Parameters:** Initial viewport dimensions
- **Returns:** `true` if successful

**`void Shutdown()`**
- Cleans up all rendering resources

#### Frame Management

**`void BeginFrame()`**
- Clears the screen and prepares for rendering
- **Note:** Call once per frame before drawing

**`void EndFrame()`**
- Executes all queued render commands and presents frame
- **Note:** Call once per frame after all drawing

#### High-Level Drawing API

**`void DrawCube(const Transform& transform, const glm::vec3& color = glm::vec3(1.0f))`**
- Immediately draws a cube with given transform and color
- **Parameters:**
  - `transform` - Position, rotation, scale
  - `color` - RGB color (default: white)

**`void DrawSphere(const Transform& transform, const glm::vec3& color = glm::vec3(1.0f))`**
- Immediately draws a sphere with given transform and color

**`void DrawPlane(const Transform& transform, const glm::vec3& color = glm::vec3(1.0f))`**
- Immediately draws a plane with given transform and color

#### Advanced Rendering

**`void Submit(const RenderCommand& command)`**
- Queues a render command for batched execution
- **Parameters:** `command` - Complete rendering instruction

**`void Flush()`**
- Executes all queued render commands immediately

#### Camera Access

**`Camera& GetCamera()`**
- **Returns:** Reference to the internal camera for direct manipulation

#### Window Events

**`void OnWindowResize(int width, int height)`**
- Updates viewport and projection when window is resized
- **Parameters:** New window dimensions

---

### RenderCommand

Structure defining a complete rendering instruction:

```cpp
struct RenderCommand {
    std::string meshName;               // Geometry to render ("cube", "sphere", "plane")
    Transform transform;                // Position, rotation, scale
    glm::vec3 color{1.0f, 1.0f, 1.0f}; // RGB color (default: white)
    std::string shaderName = "basic";   // Shader program to use
};
```

#### Usage Example

```cpp
RenderCommand cmd;
cmd.meshName = "cube";
cmd.transform.position = {1, 2, 3};
cmd.transform.scale = {2, 2, 2};
cmd.color = {1, 0, 0}; // Red
cmd.shaderName = "basic";

renderer.Submit(cmd);
```

---

### Window

Platform abstraction for window management.

#### Lifecycle

**`bool Initialize(int width, int height, const std::string& title)`**
- Creates window with OpenGL context
- **Parameters:** Window dimensions and title
- **Returns:** `true` if successful

**`void Shutdown()`**
- Destroys window and cleanup

#### Window State

**`bool ShouldClose() const`**
- **Returns:** `true` if user requested window close

**`void SwapBuffers()`**
- Presents rendered frame to screen
- **Note:** Call after all rendering is complete

**`void PollEvents()`**
- Processes input and window events
- **Note:** Call once per frame

#### Properties

**`GLFWwindow* GetGLFWindow() const`**
- **Returns:** Native GLFW window handle

**`int GetWidth() const`**
- **Returns:** Current window width

**`int GetHeight() const`**
- **Returns:** Current window height

---

### Camera

Camera system for view control.

#### Construction
```cpp
Blacksite::Camera camera;  // Default constructor
```

#### Position & Orientation

**`void SetPosition(const glm::vec3& position)`**
- Sets camera world position
- **Default:** `{0, 0, 3}`

**`void SetTarget(const glm::vec3& target)`**
- Sets camera look-at target
- **Default:** `{0, 0, 0}` (origin)

**`void SetUp(const glm::vec3& up)`**
- Sets camera up vector
- **Default:** `{0, 1, 0}` (Y-up)

#### Projection

**`void SetPerspective(float fov, float aspect, float nearPlane, float farPlane)`**
- Configures perspective projection
- **Parameters:**
  - `fov` - Field of view in degrees (default: 45°)
  - `aspect` - Width/height ratio (default: 1.0)
  - `nearPlane` - Near clipping plane (default: 0.1)
  - `farPlane` - Far clipping plane (default: 100.0)

**`void SetAspectRatio(float aspect)`**
- Updates aspect ratio (for window resize)

#### Matrix Access

**`glm::mat4 GetViewMatrix() const`**
- **Returns:** View transformation matrix (camera → world)

**`glm::mat4 GetProjectionMatrix() const`**
- **Returns:** Projection transformation matrix (3D → 2D)

#### Property Access

**`const glm::vec3& GetPosition() const`**
- **Returns:** Current camera position

**`const glm::vec3& GetTarget() const`**
- **Returns:** Current camera target

---

### Transform

Mathematical transform representation:

```cpp
struct Transform {
    glm::vec3 position{0.0f};  // World position
    glm::vec3 rotation{0.0f};  // Euler angles (degrees)
    glm::vec3 scale{1.0f};     // Scale factors
    
    glm::mat4 GetMatrix() const;  // Computed 4x4 transformation matrix
};
```

#### Matrix Computation
The `GetMatrix()` method computes the transformation matrix as:
**Translation × Rotation × Scale**

Rotations are applied in order: X → Y → Z (Euler angles)

---

### GeometryManager

Manages primitive mesh generation and GPU resource caching.

#### Mesh Creation

**`void CreateCube(const std::string& name = "cube")`**
- Generates unit cube mesh geometry
- **Default name:** "cube"

**`void CreateSphere(const std::string& name = "sphere", float radius = 0.5f, int sectors = 30, int stacks = 30)`**
- Generates sphere mesh with customizable detail level
- **Parameters:**
  - `name` - Identifier for caching (default: "sphere")
  - `radius` - Sphere radius (default: 0.5)
  - `sectors` - Horizontal subdivisions (default: 30)
  - `stacks` - Vertical subdivisions (default: 30)
- **Note:** Higher subdivision values = smoother sphere, more vertices

**`void CreatePlane(const std::string& name = "plane")`**
- Generates unit plane mesh geometry
- **Default name:** "plane"

#### Mesh Access

**`const Mesh* GetMesh(const std::string& name) const`**
- Retrieves cached mesh data for rendering
- **Returns:** Mesh pointer or `nullptr` if not found

**`void Cleanup()`**
- Releases all GPU resources (VAOs, VBOs, EBOs)

#### Mesh Structure

```cpp
struct Mesh {
    unsigned int VAO = 0;        // Vertex Array Object
    unsigned int VBO = 0;        // Vertex Buffer Object  
    unsigned int EBO = 0;        // Element Buffer Object
    unsigned int indexCount = 0; // Number of indices
    bool useIndices = false;     // Whether to use indexed rendering
    unsigned int vertexCount = 0; // Number of vertices
};
```

---

### ShaderManager

Manages shader compilation, linking, and uniform setting.

#### Shader Management

**`bool LoadShader(const std::string& name, const char* vertexSource, const char* fragmentSource)`**
- Compiles and links vertex and fragment shaders
- **Parameters:**
  - `name` - Identifier for the shader program
  - `vertexSource` - GLSL vertex shader source code
  - `fragmentSource` - GLSL fragment shader source code  
- **Returns:** `true` if compilation and linking successful

**`bool UseShader(const std::string& name)`**
- Activates shader program for rendering
- **Parameters:** `name` - Previously loaded shader identifier
- **Returns:** `true` if shader exists and activated

#### Uniform Setting

**`void SetUniform(const std::string& name, const glm::mat4& matrix)`**
- Sets matrix uniform (typically for transforms)
- **Common names:** "u_model", "u_view", "u_projection"

**`void SetUniform(const std::string& name, const glm::vec3& vector)`**
- Sets vector uniform (positions, colors, etc.)
- **Common names:** "u_color", "u_lightPos"

**`void SetUniform(const std::string& name, float value)`**
- Sets scalar uniform
- **Common names:** "u_time", "u_intensity"

#### Resource Management

**`void Cleanup()`**
- Deletes all compiled shader programs

#### Usage Example

```cpp
const char* vertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 u_model;
uniform mat4 u_view; 
uniform mat4 u_projection;

void main() {
    gl_Position = u_projection * u_view * u_model * vec4(aPos, 1.0);
}
)";

const char* fragmentShader = R"(
#version 330 core
out vec4 FragColor;
uniform vec3 u_color;

void main() {
    FragColor = vec4(u_color, 1.0);
}
)";

shaderManager.LoadShader("basic", vertexShader, fragmentShader);
shaderManager.UseShader("basic");
shaderManager.SetUniform("u_color", glm::vec3(1, 0, 0));
```

---

## Common Usage Patterns

### Basic Scene Setup

```cpp
Blacksite::Engine engine;
engine.Initialize(1920, 1080, "My Game");

// Create scene objects
int ground = engine.SpawnPlane({0, -1, 0});
int player = engine.SpawnCube({0, 0, 0});
int enemy = engine.SpawnSphere({5, 0, 0});

// Style them
engine.GetEntity(ground).Scale({10, 1, 10}).Color(0.2f, 0.8f, 0.2f);
engine.GetEntity(player).Color(0.0f, 0.0f, 1.0f);
engine.GetEntity(enemy).Color(1.0f, 0.0f, 0.0f);

// Camera setup
engine.SetCameraPosition({10, 5, 10});
engine.SetCameraTarget({0, 0, 0});
```

### Animation Loop

```cpp
float gameTime = 0.0f;

engine.SetUpdateCallback([&](Blacksite::Engine& engine, float deltaTime) {
    gameTime += deltaTime;
    
    // Rotating cube
    engine.GetEntity(cubeId).Rotate({0, gameTime * 50, 0});
    
    // Bouncing sphere
    float height = abs(sin(gameTime * 2)) * 3;
    engine.GetEntity(sphereId).At({0, height, 0});
    
    // Camera orbit
    float radius = 15;
    engine.SetCameraPosition({
        cos(gameTime * 0.5f) * radius,
        5,
        sin(gameTime * 0.5f) * radius
    });
});
```

### Advanced Rendering Commands

```cpp
// Direct rendering approach
engine.SetUpdateCallback([&](Blacksite::Engine& engine, float deltaTime) {
    auto& renderer = engine.GetRenderer(); // Hypothetical access
    
    renderer.BeginFrame();
    
    // Manual render commands
    for (int i = 0; i < 10; ++i) {
        Transform t;
        t.position = {i * 2.0f, 0, 0};
        t.scale = {1 + sin(gameTime + i), 1, 1};
        
        glm::vec3 color = {
            sin(gameTime + i) * 0.5f + 0.5f,
            cos(gameTime + i) * 0.5f + 0.5f,
            0.5f
        };
        
        renderer.DrawCube(t, color);
    }
    
    renderer.EndFrame();
});
```

---

## Rendering Pipeline

### Frame Structure
```cpp
// Each frame follows this pattern:
renderer.BeginFrame();        // Clear buffers
  // Entity rendering happens here
  renderer.DrawCube(...);     // High-level API
  // OR
  renderer.Submit(command);   // Queue commands
renderer.EndFrame();          // Execute queue & present
```

### Internal Rendering Flow
1. **BeginFrame()** - Clear color and depth buffers
2. **Draw calls** - Either immediate or queued
3. **Shader binding** - Activate appropriate shader program  
4. **Uniform setting** - Upload matrices and material data
5. **Geometry binding** - Bind VAO for the mesh
6. **Draw call** - Issue OpenGL draw command
7. **EndFrame()** - Swap buffers, present to screen

### Lighting
Currently uses hardcoded lighting setup:
- **Light Position:** `{2.0f, 4.0f, 2.0f}` (world space)
- **Light Type:** Directional/Point (implementation dependent)
- **Shader:** Basic lighting in "basic" shader program

---

## Status & Capabilities

### ✅ Fully Working
- **Window Management** - Creation, events, cleanup
- **OpenGL Context** - GLFW + GLEW integration
- **Basic Primitives** - Cube, sphere, plane generation
- **Entity System** - ID-based entity management
- **Fluent API** - Chainable entity operations
- **Transform System** - Position, rotation, scale
- **Camera System** - View and projection matrices
- **Shader Management** - Compilation, linking, uniforms
- **Geometry Caching** - GPU resource management
- **Command Queue** - Batched rendering support
- **Frame Management** - Clear, draw, present cycle

### 🚧 Partially Implemented
- **Lighting** - Basic hardcoded lighting
- **Entity Destruction** - Marked but cleanup needs verification
- **Error Handling** - Basic error returns, limited diagnostics

### ❌ Not Yet Implemented
- **Input Handling** - No keyboard/mouse API exposed
- **Texture System** - No texture loading or binding
- **Model Loading** - No external asset support
- **Audio System** - No sound capabilities
- **Physics Integration** - No collision or physics
- **Advanced Lighting** - No multiple lights, shadows, etc.
- **Post-Processing** - No framebuffer effects
- **GUI System** - No immediate mode or retained UI

### 📋 Recommended Next Steps
1. **Input System** - Expose keyboard/mouse input
2. **Texture Loading** - Add image loading and GPU upload
3. **Model Loading** - Support for .obj, .gltf formats  
4. **Physics Integration** - Collision detection and response
5. **Advanced Lighting** - Multiple lights, shadows
6. **Audio System** - Sound loading and playback
7. **GUI Framework** - Immediate mode UI for debug/tools

---

## Performance Notes

### Rendering Efficiency
- **Geometry Caching** - Meshes uploaded once, reused
- **Command Batching** - Multiple objects can be queued
- **Shader State** - Minimizes shader program switches
- **Transform Math** - Computed per-frame as needed

### Memory Management
- **RAII Design** - Automatic cleanup in destructors
- **GPU Resources** - Properly released on shutdown
- **Entity Storage** - Vector-based for cache efficiency

### Limitations
- **No Frustum Culling** - All entities rendered regardless of visibility
- **No Instancing** - Each entity issues separate draw call
- **No LOD System** - All geometry at full detail always
- **Fixed Pipeline** - Single shader, no material system

This engine is excellent for learning, prototyping, and small-scale games. For production use, consider the missing features listed above.