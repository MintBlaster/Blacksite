# Blacksite Engine

> **Early-Stage Physics-First C++ Game Engine** - Learning project exploring physics-centric design.

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![Version](https://img.shields.io/badge/version-0.1.0--alpha-red.svg)]()
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)]()
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

Blacksite is an **experimental C++ game engine** built around [Jolt Physics](https://github.com/jrouwe/JoltPhysics) from the ground up. This is a learning/research project exploring what happens when physics isn't an afterthought - everything is dynamic by default, and static objects require explicit opt-out.

**⚠️ Early Development**: This engine is in active development and **not suitable for production use**. It's great for learning, experimentation, and prototyping simple physics scenarios.

## 🎯 Core Philosophy

**Physics-First Design**: Every object spawns with physics by default. Static objects require explicit opt-out.
```cpp
auto cube = engine.SpawnCube({0, 5, 0});     // Falls and bounces
auto ground = engine.SpawnPlane().MakeStatic(); // Must explicitly disable physics
```

**Fluent API**: Chain operations naturally without component hell.
```cpp
engine.SpawnSphere({-3, 8, 0})
    .Color(1.0f, 0.3f, 0.3f)
    .Scale(1.5f)
    .Push({5, 0, 0});  // Spawns, colors, scales, and applies force in one line
```

---

## 🚀 Quick Start

### Prerequisites
- **CMake 3.16+**
- **C++17 compatible compiler** (GCC 8+, Clang 7+, MSVC 2019+)
- **OpenGL 3.3+** compatible graphics drivers

### 1. Clone and Build
```bash
git clone https://github.com/MintBlaster/Blacksite.git
cd blacksite
./scripts/setup.sh  # One-time setup (installs dependencies, builds submodules)
./scripts/dev.sh quick  # Build and run editor
```

### 2. Your First Physics Scene
```cpp
#include "blacksite/Blacksite.h"

int main() {
    Blacksite::Engine engine;

    if (!engine.Initialize(1920, 1080, "Physics Demo")) {
        return -1;
    }

    // Ground (static)
    auto ground = engine.SpawnPlane({0, -1, 0}, {20, 0.2f, 20});
    engine.GetEntity(ground).Color(0.3f, 0.3f, 0.3f).MakeStatic();

    // Falling objects (dynamic by default)
    auto redCube = engine.SpawnCube({-3, 5, 0});
    engine.GetEntity(redCube).Color(1.0f, 0.3f, 0.3f);

    auto greenSphere = engine.SpawnSphere({0, 8, 0});
    engine.GetEntity(greenSphere).Color(0.3f, 1.0f, 0.3f);

    // Camera setup
    engine.SetCameraPosition({10, 6, 10});
    engine.SetCameraTarget({0, 2, 0});

    return engine.Run();  // Launches editor with your scene
}
```

That's it! You now have a physics simulation with falling objects and a full editor interface.

---

## 📦 Current Status

### ✅ **Working Features**
- **Engine Core** - Initialize, run main loop, shutdown
- **Physics Integration** - Jolt Physics working, objects fall and bounce naturally
- **Entity System** - Simple entity spawning with fluent API
- **Basic Rendering** - Colored primitives (cubes, spheres, planes)
- **Editor Interface** - ImGui-based editor with scene hierarchy and inspector
- **Build System** - Comprehensive script-based development workflow

### 🚧 **Major Limitations**
- **No Input System** - Camera is code-only, no keyboard/mouse controls
- **No Asset Pipeline** - Primitives only, no mesh/texture loading
- **Basic Rendering** - No materials, lighting, or visual polish
- **No Audio** - No sound system at all
- **Limited Physics** - Basic shapes only, no joints or complex constraints

### 🎯 **Good For**
- **Learning** - Understanding engine architecture and physics integration
- **Experimenting** - Testing the physics-first design approach
- **Simple Prototypes** - Basic physics scenarios with colored shapes

### ❌ **Not Good For**
- **Real Games** - Missing too many essential features
- **Performance Testing** - Not optimized
- **Production Use** - This is a learning/research project

---

## 🛠️ Development

### Quick Commands
```bash
# Development (most common)
./scripts/dev.sh quick     # Clean build + run editor
./scripts/dev.sh fast      # Quick build + run (no clean)

# Release builds
./scripts/dev.sh release   # Optimized build + run

# Maintenance
./scripts/dev.sh clean     # Clean all build artifacts
./scripts/dev.sh format    # Format code with clang-format
```

### Manual Build
```bash
mkdir build && cd build
cmake ..
make -j4
./blacksite_test
```

---

## 📚 Documentation

- **[API Reference](docs/API_Reference.md)** - Complete API documentation with examples
- **[Build System](docs/Build_Guide.md)** - Detailed build and setup instructions
- **[Contributing](CONTRIBUTING.md)** - Development guidelines and contribution process

---

## 🎯 The Physics-First Experiment

This project explores: **What if physics wasn't an afterthought?**

### Traditional Approach
```cpp
// Easy to forget physics
GameObject obj = scene.CreateGameObject();
obj.AddComponent<MeshRenderer>();
// obj.AddComponent<RigidBody>();  // Oops! Object won't fall
```

### Our Approach
```cpp
// Physics by default - explicit opt-out required
auto obj = engine.SpawnCube({0, 5, 0});  // Falls immediately
// To make it static: obj.MakeStatic();
```

**This is research** - exploring ideas, not claiming superiority over existing engines.

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## 🤝 Acknowledgments

- **[Jolt Physics](https://github.com/jrouwe/JoltPhysics)** - Outstanding physics engine
- **[ImGui](https://github.com/ocornut/imgui)** - Immediate mode GUI framework
- **[GLFW](https://www.glfw.org/)** - Window and input handling

---

**Want to experiment with physics-first design?** Clone and run `./scripts/setup.sh` to try it out!

**Remember**: This is a learning project - great for understanding concepts and trying ideas, but not ready for real development work. 🧪