# Blacksite Engine API Reference

> **Modular API Documentation for Physics-First Engine v0.1.0-alpha**

[![Status](https://img.shields.io/badge/Status-Experimental-red.svg)]()
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)]()
[![Physics](https://img.shields.io/badge/Physics-Basic_Jolt-yellow.svg)]()
[![Rendering](https://img.shields.io/badge/Rendering-Basic_OpenGL-orange.svg)]()

---

## Module Status Legend

- 🟢 **Working** - Basic functionality implemented and tested
- 🟡 **Partial** - Some features work, others incomplete or buggy
- 🔴 **Minimal** - Very basic implementation, many limitations
- ⚪ **Missing** - Not implemented yet

---

## API Modules

| Module | Status | Description |
|--------|--------|-------------|
| [Engine Core](#engine-core) | 🟢 **Working** | Basic lifecycle, initialization, main loop |
| [Entity System](#entity-system) | 🟢 **Working** | Simple entity spawning with fluent API |
| [Physics Integration](#physics-integration) | 🟡 **Partial** | Basic Jolt physics, shapes, forces |
| [Rendering](#rendering) | 🔴 **Minimal** | Basic OpenGL, primitives, flat colors |
| [Editor Interface](#editor-interface) | 🔴 **Minimal** | Simple ImGui panels, basic interaction |
| [Camera System](#camera-system) | 🔴 **Minimal** | Manual positioning only, no controls |
| [Math & Utilities](#math--utilities) | 🟢 **Working** | Transform math, conversions, logging |
| [Input System](#input-system) | ⚪ **Missing** | No implementation |
| [Asset Loading](#asset-loading) | ⚪ **Missing** | No implementation |
| [Audio System](#audio-system) | ⚪ **Missing** | No implementation |

---

## Quick Start Guide

### Minimal Working Example

---

## Overview

Blacksite Engine is an **experimental physics-first C++ game engine** in early development. This is a learning project exploring what happens when physics isn't an afterthought - every object has physics by default.

**⚠️ Development Status**: This API is **unstable and incomplete**. Suitable for learning and experimentation only, not production use.

### Design Experiment
- **Physics-First**: All entities spawn with physics enabled (you must opt-out for static objects)
- **Fluent API**: Chain operations for readable code
- **Immediate Feedback**: Basic editor shows physics in real-time
- **Learning Focus**: C++17 with simple architecture for educational purposes

### Namespace
All Blacksite classes and functions are in the `Blacksite` namespace:
```cpp
using namespace Blacksite;
// or
Blacksite::Engine engine;
```

### Limitations
- **No input handling** - Camera control is manual only
- **No asset loading** - Primitives only (cube, sphere, plane)
- **Basic rendering** - No textures, materials, or advanced graphics
- **Minimal error handling** - May crash on invalid operations
- **Performance** - Not optimized, will struggle with many objects

---

## Quick Start

```cpp
#include "blacksite/Blacksite.h"
using namespace Blacksite;

int main() {
    Engine engine;
    
    // Initialize (check return value!)
    if (!engine.Initialize(1280, 720, "Physics Test")) {
        return -1;
    }
    
    // Ground plane (must make static)
    auto ground = engine.SpawnPlane({0, -1, 0}, {10, 0.2f, 10});
    engine.GetEntity(ground).Color(0.3f, 0.3f, 0.3f).MakeStatic();
    
    // Falling objects (physics by default)
    auto cube = engine.SpawnCube({0, 5, 0});
    engine.GetEntity(cube).Color(1, 0, 0);
    
    auto sphere = engine.SpawnSphere({2, 8, 0});
    engine.GetEntity(sphere).Color(0, 1, 0);
    
    // Set camera (manual only)
    engine.SetCameraPosition({8, 6, 8});
    engine.SetCameraTarget({0, 2, 0});
    
    return engine.Run();  // F1 for editor, ESC to exit
}
```

### What You'll See
- Objects immediately fall with physics
- Basic editor with F1 toggle
- Real-time physics simulation
- Simple colored shapes

---

## Engine Core
**Status: 🟢 Working** - Basic functionality reliable

The main engine class handles initialization, main loop, and basic system management.

### What Works
- ✅ Window creation and basic event handling
- ✅ System initialization (renderer, physics, UI)
- ✅ Main loop with fixed timestep
- ✅ Clean shutdown
- ✅ Basic error reporting

### What Doesn't
- ❌ Advanced error recovery
- ❌ Configuration files
- ❌ Plugin system
- ❌ Hot reloading

### Engine Class

```cpp
class Engine {
public:
    // Basic lifecycle
    Engine();
    ~Engine();
    bool Initialize(int width = 1280, int height = 720, 
                   const std::string& title = "Blacksite Engine");
    int Run();
    void Shutdown();
    
    // Entity spawning (all have physics by default)
    int SpawnCube(const glm::vec3& position = {0, 0, 0});
    int SpawnSphere(const glm::vec3& position = {0, 0, 0});
    int SpawnPlane(const glm::vec3& position = {0, 0, 0}, 
                   const glm::vec3& size = {10, 0.2f, 10});
    
    // Entity manipulation
    EntityHandle GetEntity(int id);
    
    // Basic camera (manual positioning only)
    void SetCameraPosition(const glm::vec3& position);
    void SetCameraTarget(const glm::vec3& target);
    
    // Optional update callback
    void SetUpdateCallback(std::function<void(Engine&, float)> callback);
    
    // System access (for advanced users)
    PhysicsSystem* GetPhysicsSystem();
    Renderer* GetRenderer();
    UI::UISystem* GetUI();
};
```

#### Core Methods

**`bool Initialize(int width, int height, const std::string& title)`**
```cpp
if (!engine.Initialize(1920, 1080, "My Game")) {
    std::cerr << "Failed to initialize engine" << std::endl;
    return -1;
}
```
- **What it does:** Sets up window, OpenGL, physics, and UI systems
- **Returns:** `true` if successful, `false` if any system fails
- **Always check the return value** - initialization can fail

**`int Run()`**
```cpp
return engine.Run();  // Blocks until exit
```
- **What it does:** Starts main loop with physics simulation and editor
- **Controls:** ESC to exit, F1 to toggle editor panels
- **Returns:** Exit code (0 = normal shutdown)
- **Note:** This blocks until the user exits

**`void Shutdown()`**
- **Usually automatic** - called by destructor
- **Manual use:** Call if you need explicit cleanup timing
- **Safe to call multiple times**

#### Entity Spawning

**🔥 Key Feature: Physics by Default**
All entities spawn with physics enabled - they fall immediately unless made static.

**`int SpawnCube(const glm::vec3& position = {0, 0, 0})`**
```cpp
auto cube = engine.SpawnCube({0, 5, 0});  // Falls immediately
engine.GetEntity(cube).Color(1, 0, 0);
```
- **Creates:** 1x1x1 cube with box collision
- **Returns:** Entity ID for further manipulation
- **Physics:** Falls with gravity immediately

**`int SpawnSphere(const glm::vec3& position = {0, 0, 0})`**
```cpp
auto ball = engine.SpawnSphere({2, 8, 0});
engine.GetEntity(ball).Color(0, 1, 0).Scale(0.5f);
```
- **Creates:** Unit sphere (radius 0.5) with sphere collision
- **Physics:** Bounces and rolls naturally

**`int SpawnPlane(const glm::vec3& position, const glm::vec3& size)`**
```cpp
auto ground = engine.SpawnPlane({0, -1, 0}, {20, 0.2f, 20});
engine.GetEntity(ground).Color(0.3f, 0.3f, 0.3f).MakeStatic();
```
- **Creates:** Rectangular plane (typically for ground)
- **Important:** Usually needs `.MakeStatic()` to work as ground
- **Physics:** Has collision even when static

#### Basic Camera (Manual Only)

**`void SetCameraPosition(const glm::vec3& position)`**
**`void SetCameraTarget(const glm::vec3& target)`**
```cpp
// Position camera for good view of physics
engine.SetCameraPosition({10, 8, 10});
engine.SetCameraTarget({0, 2, 0});
```
- **Manual positioning only** - no mouse controls yet
- **Set once** at startup, or update via code
- **Good defaults:** Position away from origin, target at scene center

#### Optional Update Loop

**`void SetUpdateCallback(std::function<void(Engine&, float)> callback)`**
```cpp
engine.SetUpdateCallback([&](Engine& engine, float deltaTime) {
    // Your per-frame logic here
    gameTime += deltaTime;
    
    // Example: spinning object
    engine.GetEntity(cubeId)
        .Rotate({0, gameTime * 45, 0});
});
```
- **Called every frame** after physics, before rendering
- **deltaTime:** Time since last frame (for smooth animation)
- **Optional:** Engine works fine without this

---

## Entity System
**Status: 🟢 Working** - Fluent API functional and reliable

The entity system provides a simple, chainable interface for manipulating spawned objects.

### What Works
- ✅ Fluent API chaining (`.At().Color().Scale()`)
- ✅ Basic transform operations (position, rotation, scale)
- ✅ Physics state changes (static/dynamic)
- ✅ Color assignment (flat colors only)
- ✅ Force application (basic)

### What Doesn't
- ❌ Complex entity hierarchies
- ❌ Component system
- ❌ Entity templates/prefabs
- ❌ Advanced property validation

### EntityHandle Class

The main interface for entity manipulation. **All operations return `EntityHandle&` for chaining.**

#### Core Operations

```cpp
// Transform operations (work reliably)
EntityHandle& At(const glm::vec3& position);           // Set world position
EntityHandle& Rotate(const glm::vec3& rotation);       // Set rotation (degrees)
EntityHandle& Scale(float uniformScale);               // Set uniform scale
EntityHandle& Scale(float x, float y, float z);        // Set non-uniform scale

// Physics operations (basic but functional)
EntityHandle& MakeStatic();                            // Disable physics
EntityHandle& MakeDynamic();                           // Enable physics
EntityHandle& Push(const glm::vec3& force);            // Apply impulse
EntityHandle& SetVelocity(const glm::vec3& velocity);  // Set velocity directly

// Visual operations (basic)
EntityHandle& Color(float r, float g, float b);        // Set RGB color
EntityHandle& Color(const glm::vec3& color);           // Set color (vector)

// Utility
int GetId() const;                                      // Get entity ID
void Destroy();                                         // Mark for deletion
```

#### Transform Operations

**Position: `At(const glm::vec3& position)`**
```cpp
entity.At({5, 10, -2});  // Move to specific world position
```
- **Works reliably** for both static and dynamic objects
- **Updates physics body** position immediately
- **No bounds checking** - can place anywhere

**Rotation: `Rotate(const glm::vec3& rotation)`**
```cpp
entity.Rotate({0, 45, 0});  // Rotate 45 degrees around Y axis
```
- **Parameters:** Euler angles in degrees (X, Y, Z)
- **Works for visual** representation
- **Physics rotation** is more limited

**Scale: `Scale(float)` or `Scale(float, float, float)`**
```cpp
entity.Scale(2.0f);           // Uniform scale
entity.Scale(2, 1, 0.5f);     // Non-uniform scale
```
- **Uniform scaling** works reliably
- **Non-uniform scaling** may cause physics issues
- **Physics collision** scales with visual (approximately)

#### Physics Operations

**Static/Dynamic Toggle (Reliable)**
```cpp
entity.MakeStatic();   // Disable physics (for ground, walls)
entity.MakeDynamic();  // Re-enable physics
```
- **MakeStatic()** is essential for ground planes
- **MakeDynamic()** converts static objects back to physics objects
- **Works reliably** - this is the main physics control

**Force Application (Experimental)**
```cpp
entity.Push({0, 10, 0});           // Upward impulse
entity.SetVelocity({5, 0, 0});     // Set velocity directly
```
- **Push()** applies impulse force (good for one-time impacts)
- **SetVelocity()** sets velocity directly (immediate movement)
- **Experimental values** - you'll need to experiment with force amounts
- **Results vary** depending on object mass and current state

#### Visual Properties

**Color Assignment (Works Well)**
```cpp
entity.Color(1, 0, 0);              // Red (RGB values 0-1)
entity.Color(0.5f, 0.8f, 0.2f);    // Custom color
entity.Color({0, 1, 0});            // Green (using vector)
```
- **Flat colors only** - no textures or materials
- **RGB values** from 0.0 to 1.0
- **No validation** - values outside 0-1 may cause issues
- **Immediate update** - color changes visible right away

#### Utility Operations

**`int GetId() const`**
```cpp
int cubeId = entity.GetId();  // Get unique identifier
```
- **Returns:** Unique entity ID
- **Use for:** Storing references, debugging
- **Note:** IDs may be reused after destruction

**`void Destroy()`**
```cpp
entity.Destroy();  // Mark for deletion (breaks chain)
```
- **Marks for deletion** on next frame
- **Does not return EntityHandle** (terminates chaining)
- **Basic implementation** - may not clean up everything perfectly

#### Fluent API Examples

**Basic Object Setup**
```cpp
auto cube = engine.SpawnCube({0, 5, 0});
engine.GetEntity(cube)
    .Color(1, 0, 0)      // Red color
    .Scale(1.5f)         // Make it bigger
    .Push({0, 5, 0});    // Give it upward push
```

**Ground Plane (Essential Pattern)**
```cpp
auto ground = engine.SpawnPlane({0, -1, 0}, {20, 0.2f, 20});
engine.GetEntity(ground)
    .Color(0.3f, 0.3f, 0.3f)
    .MakeStatic();       // MUST make static for ground
```

**Multiple Objects**
```cpp
for (int i = 0; i < 5; ++i) {
    auto obj = engine.SpawnSphere({i * 2.0f, 5, 0});
    engine.GetEntity(obj)
        .Color(i * 0.2f, 1.0f - i * 0.2f, 0.5f)
        .Scale(0.5f + i * 0.1f);
}
```

---

## Physics Integration
**Status: 🟡 Partial** - Basic physics working, advanced features limited

Jolt Physics integration providing the core physics-first functionality.

### What Works
- ✅ Gravity and falling objects
- ✅ Collision detection and response
- ✅ Basic shapes (box, sphere, plane)
- ✅ Static/dynamic body switching
- ✅ Force and impulse application
- ✅ 60Hz fixed timestep simulation

### What's Limited
- ⚠️ Advanced physics materials
- ⚠️ Joints and constraints  
- ⚠️ Complex collision shapes
- ⚠️ Fine-tuned physics parameters
- ⚠️ Performance optimization

### Physics Properties

**Simulation Settings**
- **Gravity:** -9.81 m/s² in Y direction (Earth-like)
- **Timestep:** Fixed 60Hz physics updates
- **Units:** Approximately meters/kilograms/seconds
- **Coordinate System:** Y-up, right-handed

### PhysicsSystem Class (Advanced)

Direct access to Jolt Physics for experimental use. **Use EntityHandle interface instead for normal use.**

```cpp
class PhysicsSystem {
public:
    bool Initialize();
    void Update(float deltaTime);
    void Shutdown();
    
    // Low-level body creation (advanced users only)
    JPH::BodyID CreateBox(const glm::vec3& position, const glm::vec3& size, bool isStatic = false);
    JPH::BodyID CreateSphere(const glm::vec3& position, float radius, bool isStatic = false);
    JPH::BodyID CreatePlane(const glm::vec3& position, const glm::vec3& size, bool isStatic = false);
    
    // Direct physics manipulation (experimental)
    void SetBodyPosition(JPH::BodyID bodyId, const glm::vec3& position);
    void SetBodyVelocity(JPH::BodyID bodyId, const glm::vec3& velocity);
    void AddImpulse(JPH::BodyID bodyId, const glm::vec3& impulse);
    void SetStatic(JPH::BodyID bodyId, bool isStatic);
    
    // Query functions
    glm::vec3 GetBodyPosition(JPH::BodyID bodyId) const;
    glm::vec3 GetBodyVelocity(JPH::BodyID bodyId) const;
};
```

**⚠️ Note:** Most users should use the EntityHandle interface instead of PhysicsSystem directly.

---

## Rendering
**Status: 🔴 Minimal** - Basic functionality only, no advanced features

Simple OpenGL-based rendering with colored primitives.

### What Works
- ✅ Basic primitive rendering (cube, sphere, plane)
- ✅ Flat color assignment per object
- ✅ Basic 3D perspective projection
- ✅ Simple lighting (hardcoded)
- ✅ Window management and basic events

### What's Missing
- ❌ Textures and materials
- ❌ Advanced lighting (shadows, multiple lights)
- ❌ Post-processing effects
- ❌ Performance optimization (culling, batching)
- ❌ Advanced shaders
- ❌ Particle systems

### Renderer Class (Low-Level)

```cpp
class Renderer {
public:
    bool Initialize(int width, int height);
    void Shutdown();
    
    void BeginFrame();
    void EndFrame();
    
    // Basic drawing (use EntityHandle instead)
    void DrawCube(const Transform& transform, const glm::vec3& color);
    void DrawSphere(const Transform& transform, const glm::vec3& color);
    void DrawPlane(const Transform& transform, const glm::vec3& color);
    
    // Camera matrices
    void SetViewMatrix(const glm::mat4& view);
    void SetProjectionMatrix(const glm::mat4& projection);
};
```

### Graphics Requirements

- **OpenGL 3.3+** Core Profile required
- **GLFW** for window management
- **GLEW** for extension loading
- **Modern GPU drivers** (integrated graphics should work)

### Coordinate System and Units
- **Y-Up coordinate system** (standard for physics)
- **Right-handed coordinates** (OpenGL standard)
- **Units:** Approximately meters (not precisely calibrated)
- **Camera:** Perspective projection with manual positioning

---

## Camera System
**Status: 🔴 Minimal** - Manual positioning only, no controls

Basic perspective camera with manual positioning.

### What Works
- ✅ Perspective projection setup
- ✅ Look-at camera positioning
- ✅ Basic view/projection matrix calculation
- ✅ Manual position and target setting

### What's Missing
- ❌ Mouse/keyboard camera controls
- ❌ Camera animation or movement
- ❌ Multiple camera support
- ❌ Orthographic projection
- ❌ Camera constraints or collision

### Camera Usage

```cpp
// Set camera position and target manually
engine.SetCameraPosition({10, 5, 10});  // Position camera away from origin
engine.SetCameraTarget({0, 0, 0});      // Look at world center

// Good defaults for physics scenes
engine.SetCameraPosition({8, 6, 8});    // Diagonal view
engine.SetCameraTarget({0, 2, 0});      // Look slightly above ground
```

### Camera Properties
- **Field of View:** 45 degrees (hardcoded)
- **Near Plane:** 0.1 units
- **Far Plane:** 100 units
- **Aspect Ratio:** Automatically calculated from window size
- **Projection:** Perspective only (no orthographic)

### Tips for Good Camera Positioning
```cpp
// For falling objects demo
engine.SetCameraPosition({15, 10, 15});
engine.SetCameraTarget({0, 5, 0});

// For ground-level action
engine.SetCameraPosition({5, 3, 10});
engine.SetCameraTarget({0, 1, 0});

// For tower/building scenes
engine.SetCameraPosition({20, 15, 20});
engine.SetCameraTarget({0, 8, 0});
```

---

## Editor Interface
**Status: 🔴 Minimal** - Basic panels working, no advanced interaction

Simple ImGui-based editor for basic physics experimentation.

### What Works
- ✅ Panel system with docking
- ✅ Entity hierarchy display
- ✅ Basic property editing
- ✅ Console with log filtering
- ✅ Performance monitoring
- ✅ F1 toggle for panel visibility

### What's Missing
- ❌ 3D scene manipulation (gizmos, handles)
- ❌ Mouse picking in 3D view
- ❌ Scene saving/loading
- ❌ Asset browser
- ❌ Advanced debugging tools
- ❌ Undo/redo system

### Panel Overview

| Panel | Status | Functionality |
|-------|--------|---------------|
| Scene Hierarchy | 🟡 **Basic** | Entity list, selection |
| Entity Inspector | 🟡 **Basic** | Transform, color, physics state |
| Console | 🟢 **Working** | Log display with filtering |
| Performance | 🟢 **Working** | FPS, timing, basic metrics |

### Using the Editor

**Opening/Closing**
```cpp
// F1 key toggles all panels
// ESC key exits application
```

**Scene Hierarchy Panel**
- Shows all spawned entities by ID
- Click entity to select it
- Selected entity highlighted in 3D view
- Shows entity type (Cube, Sphere, Plane)

**Entity Inspector Panel**
- **Transform:** Edit position, rotation, scale
- **Physics:** Toggle static/dynamic state
- **Visual:** Color picker for entity color
- **State:** Show/hide entity (basic)

**Console Panel**
- Real-time engine log output
- Filter by category: Core, Physics, Rendering, UI
- Color-coded by log level: Info (white), Warning (yellow), Error (red)
- Scroll to bottom for latest messages

**Performance Panel**
- **FPS:** Current frames per second
- **Frame Time:** Milliseconds per frame
- **Physics Time:** Physics simulation timing
- **Entity Count:** Number of active entities

### Editor Workflow
1. **Launch:** Run `./scripts/dev.sh quick`
2. **Toggle UI:** Press F1 to show/hide panels
3. **Select Entity:** Click in Scene Hierarchy
4. **Edit Properties:** Use Entity Inspector
5. **Monitor:** Check Console for issues, Performance for metrics
6. **Experiment:** Watch real-time physics simulation

### Editor Limitations
- **No 3D interaction** - can't click/drag objects in 3D view
- **No scene saving** - changes lost on exit
- **Basic editing only** - limited property control
- **Manual camera** - no mouse camera controls in 3D view

**Note:** This editor is designed for learning and basic experimentation, not production development.

---

## Math & Utilities
**Status: 🟢 Working** - Basic math operations reliable

Essential math structures and utility functions.

### Transform System

```cpp
struct Transform {
    glm::vec3 position{0.0f, 0.0f, 0.0f};    // World position
    glm::vec3 rotation{0.0f, 0.0f, 0.0f};    // Euler angles (degrees)
    glm::vec3 scale{1.0f, 1.0f, 1.0f};       // Scale factors
    
    glm::mat4 GetMatrix() const;             // Computed transformation matrix
};
```

**Matrix Calculation Order:** Translation × Rotation × Scale
**Rotation Order:** X → Y → Z (Euler angles)
**Units:** Position in world units, rotation in degrees, scale as multipliers

### GLM Integration

The engine uses GLM (OpenGL Mathematics) for all vector and matrix operations:

```cpp
#include <glm/glm.hpp>

// Common types used throughout the API
glm::vec3 position{0.0f, 5.0f, 0.0f};      // 3D vector
glm::mat4 transform = glm::mat4(1.0f);     // 4x4 matrix
glm::quat rotation = glm::quat(1,0,0,0);   // Quaternion (limited use)
```

### Physics-GLM Conversion Utilities

```cpp
// Convert between Jolt Physics and GLM types
JPH::Vec3 ToJoltPos(const glm::vec3& v);              // GLM → Jolt position
glm::vec3 ToGLMPos(const JPH::Vec3& v);               // Jolt → GLM position
JPH::Quat ToJoltRot(const glm::vec3& eulerAngles);    // GLM Euler → Jolt quaternion
glm::vec3 ToGLMRot(const JPH::Quat& q);               // Jolt quaternion → GLM Euler
```

**Note:** These are used internally - EntityHandle interface handles conversions automatically.

### Logging System

Comprehensive logging with categories and filtering:

```cpp
// Log categories
namespace LogCategory {
    enum Category { 
        CORE,       // Engine lifecycle, initialization
        PHYSICS,    // Physics simulation, collisions  
        RENDERING,  // Graphics, shaders, drawing
        UI,         // Editor interface, panels
        GAME,       // Game logic, entities
        EDITOR      // Editor-specific functionality
    };
}

// Logging macros (use these in your code)
BS_INFO(LogCategory::CORE, "Engine initialized successfully");
BS_WARNING(LogCategory::PHYSICS, "High velocity detected: {}", velocity);
BS_ERROR(LogCategory::RENDERING, "Shader compilation failed: {}", error);
BS_DEBUG(LogCategory::GAME, "Player position: ({}, {}, {})", x, y, z);
```

**Log Levels:**
- **BS_DEBUG** - Detailed information (development only)
- **BS_INFO** - General information messages
- **BS_WARNING** - Potential issues, non-fatal
- **BS_ERROR** - Error conditions, may affect functionality

**Viewing Logs:**
- **In Editor:** Console panel shows real-time logs with filtering
- **In Terminal:** Logs also output to stdout during development

### Common Constants

```cpp
// Useful constants for physics and positioning
constexpr float GRAVITY = -9.81f;           // Standard gravity (m/s²)
constexpr float PI = 3.14159265359f;        // π
constexpr float DEG_TO_RAD = PI / 180.0f;   // Convert degrees to radians
constexpr float RAD_TO_DEG = 180.0f / PI;   // Convert radians to degrees

// Common positions for camera and objects
const glm::vec3 ORIGIN{0, 0, 0};
const glm::vec3 WORLD_UP{0, 1, 0};
const glm::vec3 WORLD_FORWARD{0, 0, -1};
```

---

## Missing Systems
**Status: ⚪ Not Implemented** - Planned but not yet available

### Input System
**What's Missing:**
- Keyboard input handling
- Mouse input and camera controls
- Input binding/mapping system
- Gamepad support

**Current Workaround:**
```cpp
// Only ESC and F1 keys work (hardcoded)
// ESC = exit application  
// F1 = toggle editor panels
// Camera must be positioned in code
```

### Asset Loading System
**What's Missing:**
- Mesh loading (.obj, .fbx, .gltf)
- Texture loading (.png, .jpg)
- Asset management and caching
- Material definitions

**Current Workaround:**
```cpp
// Only built-in primitives available
engine.SpawnCube();     // ✅ Works
engine.SpawnSphere();   // ✅ Works  
engine.SpawnPlane();    // ✅ Works
// engine.LoadMesh("model.obj");  // ❌ Not implemented
```

### Audio System
**What's Missing:**
- Sound loading and playback
- 3D spatial audio
- Music and sound effects
- Audio mixing

**Current Workaround:** No audio at all - visual feedback only.

---

## Examples & Patterns

### Basic Physics Scene

```cpp
#include "blacksite/Blacksite.h"

int main() {
    Blacksite::Engine engine;
    engine.Initialize(1920, 1080, "Physics Demo");
    
    // Ground plane (static)
    auto ground = engine.SpawnPlane({0, -1, 0}, {20, 0.2f, 20});
    engine.GetEntity(ground)
        .Color(0.3f, 0.3f, 0.3f)
        .MakeStatic();
    
    // Falling objects (dynamic by default)
    std::vector<int> entities;
    for (int i = 0; i < 10; ++i) {
        auto cube = engine.SpawnCube({
            (i % 5) * 2.0f - 4.0f,  // X: spread across
            5 + (i / 5) * 2.0f,     // Y: stack vertically
            0                        // Z: center
        });
        
        engine.GetEntity(cube)
            .Color(i * 0.1f, 1.0f - i * 0.1f, 0.5f)
            .Scale(0.8f);
            
        entities.push_back(cube);
    }
    
    // Camera positioning
    engine.SetCameraPosition({15, 8, 15});
    engine.SetCameraTarget({0, 3, 0});
    
    return engine.Run();
}
```

### Interactive Physics

```cpp
int main() {
    Blacksite::Engine engine;
    engine.Initialize(1280, 720, "Interactive Physics");
    
    // Scene setup
    auto ground = engine.SpawnPlane({0, 0, 0}, {10, 0.1f, 10});
    engine.GetEntity(ground).Color(0.5f, 0.5f, 0.5f).MakeStatic();
    
    auto ball = engine.SpawnSphere({0, 5, 0});
    engine.GetEntity(ball).Color(1, 0, 0).Scale(0.5f);
    
    // Interactive update loop
    float time = 0;
    engine.SetUpdateCallback([&](Engine& engine, float dt) {
        time += dt;
        
        // Periodic impulses
        if (static_cast<int>(time) % 3 == 0 && 
            time - static_cast<int>(time) < dt) {
            engine.GetEntity(ball)
                .Push({(rand() % 100 - 50) * 0.1f, 5, 0});
        }
        
        // Respawn if falls too far
        auto pos = engine.GetPhysicsSystem()->GetBodyPosition(
            engine.GetEntity(ball).GetPhysicsBody());
        if (pos.y < -10) {
            engine.GetEntity(ball)
                .At({0, 5, 0})
                .SetVelocity({0, 0, 0});
        }
    });
    
    engine.SetCameraPosition({8, 6, 8});
    engine.SetCameraTarget({0, 2, 0});
    
    return engine.Run();
}
```

### Complex Scene Builder

```cpp
class SceneBuilder {
private:
    Engine& engine;
    
public:
    SceneBuilder(Engine& e) : engine(e) {}
    
    void CreateGround() {
        auto ground = engine.SpawnPlane({0, -0.5f, 0}, {20, 1, 20});
        engine.GetEntity(ground)
            .Color(0.4f, 0.3f, 0.2f)
            .MakeStatic();
    }
    
    void CreateTower(const glm::vec3& base, int height) {
        for (int i = 0; i < height; ++i) {
            auto cube = engine.SpawnCube({
                base.x, 
                base.y + i * 1.1f, 
                base.z
            });
            
            float hue = static_cast<float>(i) / height;
            engine.GetEntity(cube)
                .Color(hue, 1.0f - hue, 0.5f)
                .Scale(0.9f);
        }
    }
    
    void CreateWreckingBall(const glm::vec3& position) {
        auto ball = engine.SpawnSphere(position);
        engine.GetEntity(ball)
            .Color(0.8f, 0.8f, 0.8f)
            .Scale(2.0f)
            .Push({-20, 0, 0});  // Launch it!
    }
    
    void BuildScene() {
        CreateGround();
        CreateTower({5, 0, 0}, 8);
        CreateTower({7, 0, 2}, 6);
        CreateTower({3, 0, -1}, 10);
        CreateWreckingBall({-5, 5, 0});
    }
};

int main() {
    Blacksite::Engine engine;
    engine.Initialize(1920, 1080, "Tower Destruction");
    
    SceneBuilder builder(engine);
    builder.BuildScene();
    
    engine.SetCameraPosition({0, 8, 15});
    engine.SetCameraTarget({2, 4, 0});
    
    return engine.Run();
}
```

---

## Error Handling

### Initialization Errors

Always check `Initialize()` return value:
```cpp
if (!engine.Initialize(1280, 720, "My Game")) {
    std::cerr << "Failed to initialize engine" << std::endl;
    return -1;
}
```

### Entity Validation

EntityHandle operations are safe but check entity existence:
```cpp
int entityId = engine.SpawnCube();
if (entityId != -1) {  // Valid entity
    engine.GetEntity(entityId).Color(1, 0, 0);
}
```

### Common Issues

**Black Screen**: Check OpenGL drivers and compatibility
**Physics Not Working**: Ensure entities aren't accidentally made static
**Editor Not Showing**: Press F1 to toggle panel visibility
**Build Errors**: Check dependencies with `./scripts/dev.sh deps`

### Debug Information

Enable debug logging:
```cpp
#define BS_DEBUG_ENABLED
#include "blacksite/Blacksite.h"
```

Check performance panel for:
- FPS drops (rendering issues)
- High physics step time (too many objects)
- Memory usage (resource leaks)

---

---

## Getting Help & Troubleshooting

### Common Issues

**Black Screen on Launch**
- Check OpenGL drivers are installed and updated
- Ensure graphics card supports OpenGL 3.3+
- Try running: `glxinfo | grep "OpenGL version"` (Linux)

**Build Failures**
- Run `./scripts/dev.sh deps` to check dependencies
- Try `./scripts/dev.sh clean` then rebuild
- Check that git submodules are initialized

**Physics Not Working**
- Ensure ground plane is made static: `.MakeStatic()`
- Check that objects start above ground (positive Y)
- Verify camera is positioned to see the action

**Editor Not Showing**
- Press **F1** to toggle editor panels
- Check console output for UI initialization errors
- Ensure ImGui submodule is properly built

### Performance Issues

**Low FPS with Few Objects**
- This indicates graphics driver issues
- Update GPU drivers
- Check if running on integrated vs dedicated GPU

**Slow with Many Objects (>20)**
- This is expected - engine is not optimized
- Reduce number of entities for better performance
- Use fewer dynamic physics objects

### Getting Support

**Documentation:**
- Main README: Basic setup and overview
- This API Reference: Detailed function documentation
- Build Scripts README: Development workflow

**Debugging:**
- Use Console panel in editor for real-time logs
- Check terminal output during development
- Enable debug logging for more details

**Development:**
- Study `src/main.cpp` for working examples
- Experiment with the fluent API in small steps
- Use `./scripts/dev.sh info` for project diagnostics

---

## Version Information

**Engine Version**: 0.1.0-alpha  
**API Stability**: Unstable - expect breaking changes  
**Target Audience**: Learning, experimentation, education  
**Production Ready**: No - missing critical features

**Dependencies:**
- **C++17** compatible compiler
- **OpenGL 3.3+** graphics drivers  
- **Jolt Physics** (included as submodule)
- **ImGui** (included as submodule)
- **System libraries:** GLFW, GLEW, GLM, Assimp

**Platform Support:**
- **Linux**: Basic support, tested on Ubuntu 20.04+
- **macOS**: Experimental, may work with Homebrew
- **Windows**: Not supported yet

**Last Updated**: 2024  
**Documentation Version**: 0.1.0-alpha

**⚠️ Learning Project Notice**: This engine is designed for educational purposes and experimentation with physics-first design. It demonstrates working concepts but is not intended for production game development.

---

*For build instructions and project overview, see the main README.md file.*