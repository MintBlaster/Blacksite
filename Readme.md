# Blacksite Engine

Blacksite is a physics-first C++ game engine in early development. Instead of treating physics like an optional plugin, everything is built around Jolt Physics from the ground up. Objects are dynamic by default, static by choice.

## What Makes Blacksite Different
* **Physics-First** - Every object has physics by default. You have to explicitly opt-out to make something static. No more forgetting to add rigidbodies.
* **Jolt-Powered** - Built on Jolt Physics because it's fast, stable, and doesn't suck like some other physics engines.
* **Fluent API** - Spawn objects, set properties, apply forces - all in one clean chain. No component hell.

## Planned API Design

```cpp
#include <blacksite.h>

int main() {
    Blacksite::Engine engine;
    engine.Init();

    auto scene = engine.CreateScene();

    // Everything has physics by default
    auto cube = scene.Spawn("cube.obj")
        .At({0, 5, 0})
        .WithMass(2.5f)
        .Push({0, 0, 10});  // Immediately falls and bounces

    // Want something static? You have to ask for it
    auto ground = scene.Spawn()
        .AsPlane()
        .Material(Material::ASPHALT)
        .MakeStatic();  // Only way to disable physics

    return engine.Run();
}
```

## Building

### Requirements
- CMake 3.16+
- OpenGL 3.3+
- C++17 compatible compiler
- GLFW3
- GLEW
- GLM
- Assimp

### Build Steps
```bash
git clone https://github.com/MintBlaster/Blacksite.git
cd blacksite
git submodule update --init --recursive
mkdir build && cd build
cmake ..
make -j4
./blacksite_test  # Run the editor
```

### Dependencies
The project automatically builds with these integrated libraries:
- **Jolt Physics** - High-performance physics simulation
- **ImGui** - Immediate mode GUI for the editor interface


## Development Status

### ✅ **Core Systems (Working)**
- **Engine Core** - Complete initialization, main loop, and lifecycle management
- **Window Management** - GLFW-based window creation and event handling with proper cleanup
- **Rendering System** - Full OpenGL backend with shader management and render commands
- **Entity System** - Working ID-based entity management with fluent API
- **Geometry Management** - Complete primitive generation (cubes, spheres, planes)
- **Camera System** - Functional view/projection matrices and positioning
- **Transform System** - Working position, rotation, and scale operations
- **Editor UI** - ImGui-based editor with multiple panels (Scene Hierarchy, Inspector, Console, Performance)
- **Logging System** - Comprehensive logging with categories and levels

### ✅ **Physics Integration (Working)**
- **Jolt Physics Setup** - Fully integrated and functional
- **Physics Bodies** - Working rigidbody creation for all basic shapes
- **Collision Shapes** - Complete box, sphere, and plane collision geometry
- **Physics Simulation** - Stable fixed timestep physics loop
- **Static/Dynamic Bodies** - Proper physics body management
- **Physics-First API** - The fluent spawning system is actually implemented!

### 🚧 **Currently Working**
- **Material System** - Physics materials and surface properties
- **Advanced Force Application** - More complex impulse and force systems
- **Collision Events** - Callback system for collision detection
- **Performance Optimization** - Batch rendering and physics optimizations

### 📋 **Next Phase (Planned)**
- **Asset Pipeline** - Mesh loading with auto-generated collision shapes
- **Constraint System** - Joints, springs, and other physics constraints
- **Advanced Rendering** - PBR materials and lighting improvements
- **Scene Serialization** - Save and load scenes
- **Input System** - Proper input management and binding

### 🎯 **Future Goals**
- **Vehicle Physics** - Dedicated vehicle controllers
- **Networking** - Multiplayer physics synchronization
- **Advanced Rendering** - Vulkan backend and modern techniques
- **Scripting** - Lua integration for gameplay logic
- **Audio System** - 3D spatial audio integration

## Current Features

The engine already supports the physics-first approach shown in the API example above:

```cpp
// This actually works right now!
Engine engine;
engine.Initialize(1920, 1080, "My Game");

// Everything has physics by default
auto cube = engine.SpawnCube({-3, 3, 0});
engine.GetEntity(cube).Color(1.0f, 0.3f, 0.3f);  // Falls with physics

auto sphere = engine.SpawnSphere({0, 5, 0});
engine.GetEntity(sphere).Color(0.3f, 1.0f, 0.3f);  // Also falls

// Static objects require explicit opt-out
auto ground = engine.SpawnPlane({0, -1, 0}, {20, 0.2f, 20});
engine.GetEntity(ground).Color(0.3f, 0.3f, 0.3f).MakeStatic();

engine.Run();  // Runs the editor with your scene
```

The editor includes:
- **Scene Hierarchy** - View and manage all entities
- **Entity Inspector** - Modify entity properties in real-time
- **Console** - View engine logs and debug information
- **Performance Monitor** - FPS and timing information
