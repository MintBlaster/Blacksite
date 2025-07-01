# 🚀 Engine Core API

The `Engine` class is the heart of Blacksite Engine, managing initialization, the main loop, and all core systems.

---

## 🧩 Class Overview

```cpp
namespace Blacksite {
    class Engine {
    public:
        // --- Lifecycle ---
        Engine();
        ~Engine();
        bool Initialize(int width = 1280, int height = 720,
                        const std::string& title = "Blacksite Engine");
        int Run();
        void Shutdown();

        // --- Entity spawning (physics-first) ---
        int SpawnCube(const glm::vec3& position = {0, 0, 0});
        int SpawnSphere(const glm::vec3& position = {0, 0, 0});
        int SpawnPlane(const glm::vec3& position = {0, 0, 0},
                       const glm::vec3& size = {10, 0.2f, 10});

        // --- Entity manipulation ---
        EntityHandle GetEntity(int id);

        // --- Camera control (manual) ---
        void SetCameraPosition(const glm::vec3& position);
        void SetCameraTarget(const glm::vec3& target);
        glm::vec3 GetCameraPosition() const;
        glm::vec3 GetCameraTarget() const;

        // --- System access ---
        PhysicsSystem* GetPhysicsSystem();
        EntitySystem* GetEntitySystem();
        Renderer* GetRenderer();

        // --- Optional update callback ---
        void SetUpdateCallback(std::function<void(Engine&, float)> callback);
    };
}
```

---

## 🔄 Lifecycle Management

### 🏗️ Constructor

```cpp
Engine engine;
```
- **Lightweight construction** – no heavy initialization
- **Safe to create** on stack or heap
- **No OpenGL context** created yet

### ⚡ Initialize

```cpp
bool Initialize(int width = 1280, int height = 720,
                const std::string& title = "Blacksite Engine");
```

**Purpose:**  
Sets up all engine systems and creates the application window.

**Parameters:**  
- `width`: Window width in pixels  
- `height`: Window height in pixels  
- `title`: Window title string  

**Returns:**  
- `true` if successful, `false` if any system fails to initialize

**What It Does:**  
1. Creates GLFW window with OpenGL 3.3 context  
2. Initializes OpenGL (GLEW) and checks capabilities  
3. Sets up Jolt Physics system with default settings  
4. Initializes ImGui for editor interface  
5. Creates default camera and renderer  
6. Sets up entity management system  

**Example:**
```cpp
Engine engine;
if (!engine.Initialize(1920, 1080, "My Physics Game")) {
    std::cerr << "Failed to initialize engine!" << std::endl;
    return -1;
}
```

**Common Failure Reasons:**  
- OpenGL 3.3+ not supported  
- Graphics drivers not installed  
- Insufficient memory  
- Display/window system issues  

### ▶️ Run

```cpp
int Run();
```

**Purpose:**  
Starts the main game loop and blocks until the application exits.

**Returns:**  
- Exit code (`0` = normal shutdown, non-zero = error)

**What It Does:**  
1. Runs main loop at target 60 FPS  
2. Handles window events (close, resize, key presses)  
3. Updates physics simulation at fixed 60Hz timestep  
4. Calls user update callback (if set)  
5. Renders scene and editor interface  
6. Continues until ESC pressed or window closed  

**Built-in Controls:**  
- **ESC:** Exit application  
- **F1:** Toggle editor panels (if built with editor support)  

**Example:**
```cpp
// Setup scene...
auto cube = engine.SpawnCube({0, 5, 0});

// Start main loop (blocks until exit)
return engine.Run();
```

### 🛑 Shutdown

```cpp
void Shutdown();
```

**Purpose:**  
Cleanly shuts down all engine systems.

- **Automatic:** Called by destructor, usually don't need to call manually
- **Manual Use Cases:**  
  - Need explicit cleanup timing  
  - Want to reinitialize engine in same program  
  - Custom error handling

**Example:**
```cpp
engine.Shutdown(); // Explicit cleanup
// Engine can be reinitialized after this
```

---

## 🟦 Entity Spawning (Physics-First)

All spawning methods create entities with physics enabled by default.

---

### 🟫 SpawnCube

```cpp
int SpawnCube(const glm::vec3& position = {0, 0, 0});
```

- **Creates:** 1×1×1 cube with box collision shape  
- **Physics:** Dynamic body with mass, affected by gravity  
- **Returns:** Entity ID for further manipulation

**Example:**
```cpp
auto cube = engine.SpawnCube({0, 5, 0});
engine.GetEntity(cube)
    .Color(1, 0, 0) // Red
    .Scale(1.5f);   // Bigger
```

### 🟢 SpawnSphere

```cpp
int SpawnSphere(const glm::vec3& position = {0, 0, 0});
```

- **Creates:** Unit sphere (radius 0.5) with sphere collision  
- **Physics:** Dynamic body, rolls and bounces naturally  
- **Returns:** Entity ID for further manipulation

**Example:**
```cpp
auto ball = engine.SpawnSphere({2, 8, 0});
engine.GetEntity(ball)
    .Color(0, 1, 0)   // Green
    .Scale(0.8f)      // Smaller
    .Push({5, 0, 0}); // Push sideways
```

### 🟫 SpawnPlane

```cpp
int SpawnPlane(const glm::vec3& position = {0, 0, 0},
               const glm::vec3& size = {10, 0.2f, 10});
```

- **Creates:** Rectangular plane with box collision  
- **Physics:** Dynamic by default (usually want to make static)  
- **Parameters:**  
  - `position`: Center position of the plane  
  - `size`: Dimensions (width, height, depth)  

**Common Pattern:**
```cpp
// Ground plane (essential pattern)
auto ground = engine.SpawnPlane({0, -1, 0}, {20, 0.2f, 20});
engine.GetEntity(ground)
    .Color(0.3f, 0.3f, 0.3f) // Gray
    .MakeStatic();           // MUST make static for ground
```

---

## 🛠️ Entity Manipulation

### 🔗 GetEntity

```cpp
EntityHandle GetEntity(int id);
```

- **Purpose:** Get a fluent API handle for an entity  
- **Parameters:** Entity ID returned from spawning methods  
- **Returns:** EntityHandle for chaining operations

**Example:**
```cpp
int cubeId = engine.SpawnCube({0, 5, 0});
EntityHandle cube = engine.GetEntity(cubeId);

// Use fluent API
cube.Color(1, 0, 0)
    .Scale(2.0f)
    .Push({0, 10, 0});
```

**Error Handling:**  
- Invalid IDs return a safe "null" handle  
- Operations on null handles are ignored  
- No crashes from invalid entity access  

---

## 🎥 Camera Control (Manual)

The camera system requires manual positioning – no mouse/keyboard controls yet.

### 📷 SetCameraPosition

```cpp
void SetCameraPosition(const glm::vec3& position);
```

- **Purpose:** Set camera world position

**Example:**
```cpp
engine.SetCameraPosition({10, 8, 10}); // Diagonal view
```

### 🎯 SetCameraTarget

```cpp
void SetCameraTarget(const glm::vec3& target);
```

- **Purpose:** Set point the camera looks at

**Example:**
```cpp
engine.SetCameraTarget({0, 2, 0}); // Look at action area
```

### 📝 Camera Positioning Tips

**Good Defaults for Physics Scenes:**
```cpp
// Diagonal overview
engine.SetCameraPosition({8, 6, 8});
engine.SetCameraTarget({0, 2, 0});

// Side view
engine.SetCameraPosition({15, 5, 0});
engine.SetCameraTarget({0, 3, 0});

// Top-down view
engine.SetCameraPosition({0, 20, 0});
engine.SetCameraTarget({0, 0, 0});

// Close action view
engine.SetCameraPosition({5, 3, 8});
engine.SetCameraTarget({0, 1, 0});
```

### 🔍 GetCameraPosition / GetCameraTarget

```cpp
glm::vec3 GetCameraPosition() const;
glm::vec3 GetCameraTarget() const;
```

- **Purpose:** Query current camera settings  
- **Use Cases:**  
  - Save/restore camera positions  
  - Animate camera movement  
  - Debug camera positioning  

---

## 🧑‍💻 System Access (Advanced)

For advanced users who need direct system access:

---

### ⚙️ GetPhysicsSystem

```cpp
PhysicsSystem* GetPhysicsSystem();
```

- **Purpose:** Direct access to Jolt Physics integration  
- **Use Cases:**  
  - Advanced physics operations  
  - Custom collision queries  
  - Performance profiling

**Example:**
```cpp
auto* physics = engine.GetPhysicsSystem();
// Advanced physics operations...
```

### 🗂️ GetEntitySystem

```cpp
EntitySystem* GetEntitySystem();
```

- **Purpose:** Direct access to entity management  
- **Use Cases:**  
  - Bulk entity operations  
  - Custom entity iteration  
  - Entity count queries  

### 🖼️ GetRenderer

```cpp
Renderer* GetRenderer();
```

- **Purpose:** Direct access to rendering system  
- **Use Cases:**  
  - Custom rendering operations  
  - Shader management  
  - Performance optimization  

---

## 🔄 Update Callback System

### ⏱️ SetUpdateCallback

```cpp
void SetUpdateCallback(std::function<void(Engine&, float)> callback);
```

- **Purpose:** Register a function to be called every frame  
- **Parameters:**  
  - `Engine&`: Reference to engine for entity access  
  - `float`: Delta time since last frame (in seconds)  
- **Called:** After physics update, before rendering

**Example:**
```cpp
float gameTime = 0;
engine.SetUpdateCallback([&](Engine& engine, float deltaTime) {
    gameTime += deltaTime;
    // Animate something
    if (gameTime > 5.0f) {
        auto cube = engine.GetEntity(cubeId);
        cube.Push({0, 10, 0});  // Bounce every 5 seconds
        gameTime = 0;
    }
});
```

**Use Cases:**  
- Game logic updates  
- Animation systems  
- Input handling (when implemented)  
- Custom physics interactions  

---

## 🧑‍💻 Complete Example

```cpp
#include "blacksite/Engine.h"
using namespace Blacksite;

int main() {
    Engine engine;

    // Initialize engine
    if (!engine.Initialize(1280, 720, "Physics Demo")) {
        return -1;
    }

    // Create ground
    auto ground = engine.SpawnPlane({0, -1, 0}, {15, 0.2f, 15});
    engine.GetEntity(ground)
        .Color(0.4f, 0.3f, 0.2f)
        .MakeStatic();

    // Create falling objects
    auto cube = engine.SpawnCube({0, 8, 0});
    engine.GetEntity(cube).Color(1, 0, 0);

    auto sphere = engine.SpawnSphere({3, 10, 0});
    engine.GetEntity(sphere).Color(0, 1, 0).Scale(0.8f);

    // Position camera
    engine.SetCameraPosition({12, 8, 12});
    engine.SetCameraTarget({0, 3, 0});

    // Optional: Add update logic
    float time = 0;
    engine.SetUpdateCallback([&](Engine& engine, float dt) {
        time += dt;

        // Periodic impulses
        if (static_cast<int>(time) % 4 == 0 &&
            time - static_cast<int>(time) < dt) {
            engine.GetEntity(cube).Push({0, 8, 0});
            engine.GetEntity(sphere).Push({-3, 5, 0});
        }
    });

    // Run main loop
    return engine.Run();  // ESC to exit, F1 for editor
}
```

---

## 🛡️ Error Handling

### 🚨 Initialization Errors

```cpp
if (!engine.Initialize()) {
    std::cerr << "Engine initialization failed!" << std::endl;
    // Check:
    // - OpenGL 3.3+ support
    // - Graphics drivers
    // - Available memory
    return -1;
}
```

### ⚠️ Runtime Errors
- Engine continues running when possible
- Invalid operations are ignored safely
- Check console panel for error messages
- Use debug builds for detailed error info

---

## 🚦 Performance Considerations

### ✅ Optimal Usage
- **10–100 entities:** Good performance
- **Static objects:** Use `.MakeStatic()` for scenery
- **Reasonable forces:** Avoid extreme physics values

### 📊 Performance Monitoring

Use the editor **Performance** panel to monitor:
- FPS and frame times
- Entity counts
- Physics simulation time

### ⚡ Limitations
- Not optimized for 1000+ entities
- Physics simulation is the main bottleneck
- Rendering is basic and not optimized

---

*Engine Core API for Blacksite Engine v0.2.0-alpha*
