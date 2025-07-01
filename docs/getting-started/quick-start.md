# 🚀 Quick Start Guide

Welcome to Blacksite Engine!  
Get your first physics scene running in **5 minutes**.

---

## 📦 Prerequisites

Before you begin, make sure you've completed the [Installation Guide](./installation.md).

---

## 🛠️ Your First Physics Scene

### 1️⃣ Create a New Example

Create a file:  
`examples/my_first_scene.cpp`

```cpp
#include "blacksite/Engine.h"
using namespace Blacksite;

int main() {
    // Initialize the engine
    Engine engine;
    if (!engine.Initialize(1280, 720, "My First Physics Scene")) {
        return -1;
    }

    // Create a ground plane (must be static)
    auto ground = engine.SpawnPlane({0, -1, 0}, {10, 0.2f, 10});
    engine.GetEntity(ground)
        .Color(0.3f, 0.3f, 0.3f)  // Gray color
        .MakeStatic();             // Essential for ground!

    // Create a falling cube (physics enabled by default)
    auto cube = engine.SpawnCube({0, 5, 0});
    engine.GetEntity(cube)
        .Color(1, 0, 0)           // Red color
        .Scale(1.5f);             // Make it bigger

    // Position the camera for a good view
    engine.SetCameraPosition({8, 6, 8});
    engine.SetCameraTarget({0, 2, 0});

    // Run the simulation
    return engine.Run();  // Press ESC to exit, F1 for editor
}
```

---

### 2️⃣ Build and Run

**Quick build and run:**
```bash
./scripts/build.sh --target test --run
```
Or use the development script:
```bash
./scripts/dev.sh quick
```

---

### 3️⃣ What You Should See

- 🟥 A red cube falls from the sky
- ⬜ It lands on a gray ground plane and stops
- ⚡ Physics simulation runs at 60 FPS
- 🖱️ Press **ESC** to exit
- 🛠️ Press **F1** to toggle the editor interface

---

## 🧑‍💻 Understanding the Code

### Engine Initialization

```cpp
Engine engine;
if (!engine.Initialize(1280, 720, "My First Physics Scene")) {
    return -1; // Always check for initialization failure
}
```
- Creates the main engine instance
- Sets up window, OpenGL, physics, and editor systems
- **Always check the return value** – initialization can fail

---

### Physics-First Design

```cpp
auto cube = engine.SpawnCube({0, 5, 0}); // Has physics by default!
```
- **Key Concept:** All entities spawn with physics enabled
- They immediately fall with gravity unless made static
- This is different from traditional engines where you add physics later

---

### Making Static Objects

```cpp
engine.GetEntity(ground).MakeStatic(); // Essential for ground planes
```
- Static objects don't move but still have collision
- Perfect for ground, walls, and platforms
- **Must do this for ground planes** or they'll fall too!

---

### Fluent API

```cpp
engine.GetEntity(cube)
    .Color(1, 0, 0)   // Chain operations
    .Scale(1.5f)      // for readable code
    .Push({0, 10, 0}); // Apply upward force
```
- Chain multiple operations together
- Makes code more readable and concise
- Each operation returns the EntityHandle for chaining

---

## 🖥️ Exploring with the Editor

### Opening the Editor

Press **F1** while your scene is running to open the editor panels.

---

### Editor Panels Overview

- **Scene Hierarchy:**  
  See all entities, select and inspect them, view physics state (Static/Dynamic)
- **Inspector:**  
  Edit position, rotation, scale, toggle static/dynamic, apply forces, change color
- **Console:**  
  View real-time logs, filter by category (Core, Physics, Rendering), monitor errors/warnings
- **Performance:**  
  Monitor FPS, frame times, entity/physics body counts, spot bottlenecks

---

### 🧪 Try These Experiments

**1. Add More Objects**
```cpp
// Add multiple falling spheres
for (int i = 0; i < 5; ++i) {
    auto sphere = engine.SpawnSphere({i * 2.0f, 8, 0});
    engine.GetEntity(sphere)
        .Color(0, 1, 0)
        .Scale(0.8f);
}
```

**2. Apply Forces**
```cpp
// Give the cube an initial push
engine.GetEntity(cube).Push({5, 0, 0}); // Push sideways
```

**3. Interactive Updates**
```cpp
// Add animation or interaction
engine.SetUpdateCallback([&](Engine& engine, float deltaTime) {
    static float time = 0;
    time += deltaTime;
    // Bounce the cube every 3 seconds
    if (static_cast<int>(time) % 3 == 0) {
        engine.GetEntity(cube).Push({0, 8, 0});
    }
});
```

---

## 🏗️ Common Patterns

### Ground Setup (Essential)
```cpp
auto ground = engine.SpawnPlane({0, -1, 0}, {20, 0.2f, 20});
engine.GetEntity(ground)
    .Color(0.4f, 0.3f, 0.2f) // Brown/tan color
    .MakeStatic();            // MUST be static
```

### Camera Positioning
```cpp
// Good defaults for physics scenes
engine.SetCameraPosition({10, 8, 10}); // Diagonal view
engine.SetCameraTarget({0, 3, 0});     // Look at action area

// For close-up action
engine.SetCameraPosition({5, 3, 8});
engine.SetCameraTarget({0, 1, 0});

// For wide overview
engine.SetCameraPosition({15, 12, 15});
engine.SetCameraTarget({0, 5, 0});
```

### Color Schemes
```cpp
// Primary colors
.Color(1, 0, 0)   // Red
.Color(0, 1, 0)   // Green
.Color(0, 0, 1)   // Blue

// Neutral colors
.Color(0.3f, 0.3f, 0.3f) // Dark gray
.Color(0.8f, 0.8f, 0.8f) // Light gray
.Color(0.5f, 0.3f, 0.1f) // Brown

// Bright colors
.Color(1, 1, 0)   // Yellow
.Color(1, 0, 1)   // Magenta
.Color(0, 1, 1)   // Cyan
```

---

## ⏭️ Next Steps

Now that you have a working scene:

1. **Experiment with the Editor:** Try modifying properties in real-time
2. **Learn the Physics System:** [Physics Documentation](../api/physics.md)
3. **Explore More Examples:** [Code Examples](../guides/examples.md)
4. **Understand the Architecture:** [Engine Core](../api/engine-core.md)

---

## 🛠️ Troubleshooting

**Black Screen:**  
- Check that OpenGL drivers are installed  
- Verify camera position is not inside objects

**No Physics:**  
- Ensure objects start above the ground (positive Y)  
- Check that ground plane is made static

**Editor Not Showing:**  
- Press F1 to toggle editor panels  
- Check console output for ImGui errors

**Build Errors:**  
- Run `./scripts/dev.sh deps` to check dependencies  
- Try `./scripts/dev.sh clean` then rebuild

---

*Quick Start Guide for Blacksite Engine v0.2.0-alpha*
