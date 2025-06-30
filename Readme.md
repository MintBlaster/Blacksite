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

## Development Status

### ✅ **Foundation Systems (Complete)**
- **Engine Core** - Initialization, main loop, and lifecycle management
- **Window Management** - GLFW-based window creation and event handling
- **Rendering System** - OpenGL backend with comprehensive shader management
- **Entity System** - ID-based entity management with fluent API design
- **Geometry Management** - Primitive generation (cubes, spheres, planes)
- **Camera System** - View/projection matrices and positioning
- **Transform System** - Position, rotation, and scale operations

### 🚧 **Physics Integration (In Progress)**
- **Jolt Physics Setup** - Library integration and initialization
- **Physics Bodies** - Rigidbody creation for basic shapes
- **Collision Shapes** - Box, sphere, and plane collision geometry
- **Physics Simulation** - Fixed timestep physics loop
- **Force Application** - Impulses, forces, and velocity control

### 📋 **Next Phase (Planned)**
- **Physics-First API** - The fluent spawning system shown above
- **Material System** - Physics materials and constraints
- **Collision Events** - Proper callback system
- **Asset Pipeline** - Mesh loading with auto-generated collision
- **Better Math** - Because GLM is fine but could be better

### 🎯 **Future Goals**
- **Vehicle Physics** - Proper car controllers that don't feel like garbage
- **Networking** - Because multiplayer physics is hard but necessary
- **Better Rendering** - OpenGL is fine for now, Vulkan later maybe
- **Scripting** - Lua or something, haven't decided yet