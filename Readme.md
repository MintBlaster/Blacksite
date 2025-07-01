# Blacksite Engine

> Physics-First C++ Game Engine with Real-Time Editor — Experimental engine exploring physics-centric design.

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![Version](https://img.shields.io/badge/version-0.2.0--alpha-orange.svg)]()
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)]()
[![Physics](https://img.shields.io/badge/Physics-Jolt_Integrated-green.svg)]()
[![Editor](https://img.shields.io/badge/Editor-ImGui_Panels-blue.svg)]()
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

Blacksite is an experimental C++ game engine built around [Jolt Physics](https://github.com/jrouwe/JoltPhysics) with a comprehensive real-time physics editor. This learning project explores what happens when physics isn't an afterthought—everything is dynamic by default, with powerful tools for real-time experimentation.

New in v0.2.0-alpha: Complete physics editor with static/dynamic switching, force application, and comprehensive property editing.

## Core Philosophy

**Physics-First Design:** Every object spawns with physics by default. Static objects require explicit opt-out.
```cpp
auto cube = engine.SpawnCube({0, 5, 0});     // Falls and bounces immediately
auto ground = engine.SpawnPlane().MakeStatic(); // Must explicitly disable physics
```

**Real-Time Editing:** Switch between static and dynamic physics while simulation runs.
```cpp
// In editor: Toggle "Static Body" checkbox to see immediate results
// Or in code:
entity.MakeStatic();   // Stop physics movement
entity.MakeDynamic();  // Re-enable physics
```

**Fluent API:** Chain operations naturally without component complexity.
```cpp
engine.SpawnSphere({-3, 8, 0})
    .Color(1.0f, 0.3f, 0.3f)
    .Scale(1.5f)
    .Push({5, 0, 0});  // Spawns, colors, scales, and applies force
```

---

## Quick Start

### Prerequisites
- CMake 3.16+
- C++17 compatible compiler (GCC 7+, Clang 10+)
- OpenGL 3.3+ compatible graphics drivers
- Linux/macOS (Windows not yet supported)

### 1. Clone and Setup
```sh
git clone https://github.com/MintBlaster/BlacksiteEngine.git
cd BlacksiteEngine
./scripts/setup.sh  # One-time setup (installs dependencies, configures submodules)
```

### 2. Launch Physics Editor
```sh
./scripts/dev.sh editor     # Build and run complete physics editor
# Press F1 to toggle editor panels, ESC to exit
```

### 3. Quick Engine Test
```sh
./scripts/dev.sh quick      # Build and run basic physics demo
```

### 4. Your First Physics Scene
```cpp
#include "blacksite/Engine.h"
using namespace Blacksite;

int main() {
    Engine engine;
    if (!engine.Initialize(1280, 720, "Physics Demo")) return -1;

    // Ground (essential - must be static)
    auto ground = engine.SpawnPlane({0, -1, 0}, {20, 0.2f, 20});
    engine.GetEntity(ground).Color(0.3f, 0.3f, 0.3f).MakeStatic();

    // Falling objects (dynamic by default)
    auto cube = engine.SpawnCube({0, 5, 0});
    engine.GetEntity(cube).Color(1, 0, 0);

    auto sphere = engine.SpawnSphere({3, 8, 0});
    engine.GetEntity(sphere).Color(0, 1, 0).Scale(0.8f);

    // Camera positioning
    engine.SetCameraPosition({8, 6, 8});
    engine.SetCameraTarget({0, 2, 0});

    return engine.Run();  // F1 for editor, ESC to exit
}
```

Result: Physics simulation with real-time editor for experimenting with forces, static/dynamic switching, and property editing.

---

## Physics Editor Features

### Real-Time Physics Editing
- Static/Dynamic Switching — Convert physics bodies instantly while simulation runs
- Force Application — Apply forces and impulses directly to objects
- Velocity Control — Set velocities and watch objects respond immediately
- Transform Editing — Modify position, rotation, scale with live updates

### Editor Panels
- Scene Hierarchy — Entity management with physics state indicators
- Inspector — Complete property editing (transform, physics, visual)
- Console — Real-time logging with filtering and command input
- Performance — FPS monitoring and physics statistics
- Viewport — Camera controls and physics simulation controls

### Interactive Physics Experimentation
```cpp
// Use editor to experiment:
// 1. Select objects in Scene Hierarchy
// 2. Toggle Static/Dynamic in Inspector
// 3. Apply forces with "Apply Force" button
// 4. Watch real-time physics simulation
// 5. Reset physics to try again
```

---

## Current Status (v0.2.0-alpha)

### Production-Ready Features
- Physics Integration — Jolt Physics with static/dynamic switching, forces, impulses
- Physics Editor — Complete real-time editing interface with comprehensive controls
- Build System — Automated development workflow with `dev.sh` and `build.sh` scripts
- Entity System — Fluent API with physics-first design

### Working Features
- Engine Core — Initialization, main loop, system management
- Basic Rendering — OpenGL primitives with flat colors and basic lighting
- Camera System — Manual positioning with editor presets
- Math & Utilities — Transform system, GLM integration, comprehensive logging

### Basic Features
- Input System — F1/ESC only, no mouse/keyboard controls
- Asset Loading — Primitives only (cube, sphere, plane)
- Audio System — Not implemented

### Perfect For
- Learning Physics — Understand physics concepts with immediate visual feedback
- Rapid Prototyping — Test physics-based game mechanics quickly
- Education — Teach physics simulation and engine architecture
- Experimentation — Try "what if" scenarios with real-time editing

### Not Suitable For
- Production Games — Missing essential features (input, assets, audio)
- Large Scenes — Not optimized for 1000+ objects
- Complex Graphics — Basic rendering only

---

## Development Workflow

### Daily Development
```sh
# Most common commands
./scripts/dev.sh editor-fast    # Quick editor build (no clean)
./scripts/dev.sh quick          # Quick engine test
./scripts/dev.sh editor         # Clean editor build

# Before committing
./scripts/dev.sh format         # Format code
./scripts/dev.sh release        # Test release build
```

### Advanced Build Options
```sh
# Full control over build process
./scripts/build.sh --help       # See all options
./scripts/build.sh --target editor --clean --run
./scripts/build.sh --target test --release --jobs 8
```

### Project Structure
```
BlacksiteEngine/
├── blacksite/           # Engine core library
│   ├── include/blacksite/
│   └── src/
├── editor/              # Physics editor application
│   ├── include/editor/
│   └── src/
├── examples/            # Sample applications
├── scripts/             # Development workflow
└── docs/                # Comprehensive documentation
```

---

## Documentation

### Complete Modular Documentation
- [Documentation Index](docs/index.md) — Start here for comprehensive guides
- [Installation Guide](docs/getting-started/installation.md) — Detailed setup instructions
- [Quick Start Tutorial](docs/getting-started/quick-start.md) — 5-minute physics scene
- [API Reference](docs/api/overview.md) — Complete API documentation
- [Physics-First Design](docs/guides/physics-first.md) — Understanding our philosophy
- [Build System Guide](docs/guides/build-scripts.md) — Development workflow
- [Troubleshooting](docs/getting-started/troubleshooting.md) — Common issues and solutions

### Quick References
- [Engine Core API](docs/api/engine-core.md) — Main Engine class
- [Entity System API](docs/api/entity-system.md) — Fluent API and EntityHandle
- [Physics System API](docs/api/physics.md) — Jolt Physics integration
- [Editor Interface API](docs/api/editor.md) — Real-time editing capabilities

---

## The Physics-First Experiment

This project explores: What if physics was the foundation, not an add-on?

### Traditional Game Engine Approach
```cpp
// Physics is optional, easy to forget
auto object = engine.CreateObject();
object.AddComponent();
object.AddComponent();
// object.AddComponent();  // Oops! Forgot physics
```

### Blacksite Physics-First Approach
```cpp
// Physics by default, explicit opt-out required
auto object = engine.SpawnCube({0, 5, 0});  // Already falling!
// To disable physics: object.MakeStatic();
```

### Real-Time Experimentation
```cpp
// Editor allows instant physics state changes:
// - Toggle static/dynamic while simulation runs
// - Apply forces and see immediate results
// - Switch between physics states for gameplay mechanics
// - Perfect for learning and rapid prototyping
```

This is research—exploring ideas and learning, not claiming superiority over production engines.

---

## Contributing

We welcome contributions! See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

Areas where help is needed:
- Input system implementation
- Asset loading pipeline
- Cross-platform Windows support
- Performance optimization
- Advanced physics features

---

## License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.

---

## Acknowledgments

- [Jolt Physics](https://github.com/jrouwe/JoltPhysics) — Outstanding physics engine powering our simulation
- [Dear ImGui](https://github.com/ocornut/imgui) — Immediate mode GUI framework for our editor
- [GLFW](https://www.glfw.org/) — Cross-platform window and input handling
- [GLM](https://github.com/g-truc/glm) — OpenGL Mathematics library

---

Ready to experiment with physics-first design?

```sh
git clone https://github.com/MintBlaster/BlacksiteEngine.git
cd BlacksiteEngine
./scripts/setup.sh && ./scripts/dev.sh editor
# Press F1 to open editor panels and start experimenting!
```

Perfect for: Learning engine architecture • Physics experimentation • Rapid prototyping • Educational projects

Remember: This is a learning project focused on physics-first design—excellent for understanding concepts and trying ideas!
