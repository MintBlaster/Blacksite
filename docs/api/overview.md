# 🚀 Blacksite Engine API Overview

Welcome to the **Blacksite Engine API** documentation!  
This page introduces the core concepts, design philosophy, and unique features that set Blacksite apart as a *physics-first* C++ game engine.

---

## ⚡ Physics-First Philosophy

Unlike traditional game engines where physics is an optional component you add to objects, **Blacksite Engine is built around the concept that everything should have physics by default**.

### Traditional Approach

<details>
<summary><strong>Traditional Approach</strong></summary>

```cpp
// Traditional engine
auto object = engine.CreateObject();
object.AddComponent<RenderComponent>();
object.AddComponent<PhysicsComponent>(); // Optional, added later
```
</details>


### Blacksite Approach

<details>
<summary><strong>Blacksite Approach</strong></summary>

```cpp
// Blacksite Engine
auto object = engine.SpawnCube({0, 5, 0}); // Has physics immediately!
engine.GetEntity(object).MakeStatic(); // Opt-out if needed
```
</details>


### Why Physics-First?

- **Immediate Feedback:** Objects behave realistically from the moment they're created
- **Simplified Workflow:** No need to remember to add physics components
- **Natural Behavior:** Real-world objects have mass and respond to forces by default
- **Learning Friendly:** Physics concepts are front and center, not hidden

---

## 🧩 Core Design Principles

### 1. Fluent API Design

Chain operations for readable, expressive code:

```cpp
engine.GetEntity(cube)
    .At({5, 10, 0})   // Position
    .Color(1, 0, 0)   // Make it red
    .Scale(1.5f)      // Make it bigger
    .Push({0, 5, 0}); // Give it upward impulse
```

### 2. Immediate Mode Philosophy

Changes take effect immediately—no need to "apply" or "commit":

```cpp
entity.Color(0, 1, 0);      // Green immediately
entity.At({10, 0, 0});      // Moves immediately
entity.MakeStatic();         // Physics disabled immediately
```

### 3. Sensible Defaults

Everything works out of the box with reasonable defaults:

```cpp
auto cube = engine.SpawnCube(); // Default size, position, color, physics
// No setup required - it just works
```

### 4. Real-Time Editing

The editor provides immediate visual feedback for all changes:

- Modify properties and see results instantly
- Switch between static and dynamic physics in real-time
- Apply forces and watch objects respond immediately

---

## 🗂️ API Structure

### Core Modules

| Module           | Purpose                        | Status         |
|------------------|-------------------------------|----------------|
| **Engine**       | Main lifecycle/system manager  | 🟢 Stable      |
| **EntitySystem** | Object creation/management     | 🟢 Stable      |
| **PhysicsSystem**| Jolt Physics integration       | 🟢 Stable      |
| **Editor**       | Real-time editing interface    | 🟡 Working     |
| **Renderer**     | Basic OpenGL rendering         | 🔴 Basic       |

### Namespace Organization

```cpp
namespace Blacksite {
    class Engine;         // Main engine class
    class EntityHandle;   // Fluent API for entities
    class PhysicsSystem;  // Physics integration
    class Renderer;       // Graphics system

    namespace Editor {
        class EditorPanel;  // UI panel base class
        // Editor-specific classes
    }
}
```

---

## 🔄 Entity Lifecycle

### 1. Spawning

Entities spawn with physics enabled:

```cpp
int cubeId   = engine.SpawnCube({0, 5, 0});
int sphereId = engine.SpawnSphere({2, 8, 0});
int planeId  = engine.SpawnPlane({0, -1, 0}, {10, 0.2f, 0});
```

### 2. Manipulation

Get a handle for fluent API:

```cpp
EntityHandle cube = engine.GetEntity(cubeId);

// Chain operations
cube.Color(1, 0, 0)
    .Scale(1.5f)
    .Push({0, 10, 0});
```

### 3. Physics State Management

Convert between static and dynamic:

```cpp
cube.MakeStatic();   // Disable physics (for platforms, walls)
cube.MakeDynamic();  // Re-enable physics (default state)
```

### 4. Destruction

```cpp
cube.Destroy(); // Mark for deletion (happens next frame)
```

---

## 🗺️ Coordinate System

### World Coordinates

- **Y-Up:** Positive Y is "up" (standard for physics)
- **Right-Handed:** Standard OpenGL coordinate system
- **Units:** Approximately meters (not precisely calibrated)
- **Origin:** (0, 0, 0) is world center

### Typical Positions

```cpp
// Ground level
{0, 0, 0}    // World center, ground level
{0, -1, 0}   // Slightly below ground (good for ground planes)

// Spawning positions
{0, 5, 0}    // 5 units above ground
{0, 10, 0}   // High drop for dramatic effect

// Camera positions
{8, 6, 8}    // Diagonal view, good for most scenes
{0, 15, 0}   // Top-down view
{15, 3, 0}   // Side view
```

---

## 🏗️ Physics Integration

### Jolt Physics Backend

- **High Performance:** Industry-grade physics simulation
- **Stable Simulation:** Consistent 60Hz timestep
- **Realistic Behavior:** Proper collision response and dynamics

### Physics Properties

```cpp
// Gravity:   -9.81 m/s² (Earth-like)
// Timestep:  1/60 second (60 FPS physics)
// Collision: Automatic between all objects
// Materials: Basic default material (no customization yet)
```

### Static vs Dynamic Bodies

**Dynamic Bodies (Default):**
- Affected by gravity and forces
- Can move and rotate freely
- Respond to collisions
- Have mass and velocity

**Static Bodies:**
- Not affected by gravity or forces
- Cannot move (infinite mass)
- Still participate in collisions
- Perfect for ground, walls, platforms

---

## 🖥️ Editor Integration

### Real-Time Editing

The editor provides immediate feedback for all property changes:

```cpp
// These changes are visible immediately in the editor:
entity.At({5, 0, 0});     // Position updates in real-time
entity.Color(0, 1, 0);    // Color changes immediately
entity.MakeStatic();      // Physics state changes instantly
```

### Editor Panels

- **Scene Hierarchy:**  
  Lists all entities with physics state indicators. Click to select entities for editing. Shows entity types (Cube, Sphere, Plane).

- **Inspector:**  
  Edit transform properties (position, rotation, scale). Toggle physics state (static/dynamic). Apply forces and set velocities. Change visual properties (color).

- **Console:**  
  Real-time engine logs with filtering. Monitor physics events and errors. Command input for advanced operations.

- **Performance:**  
  FPS and timing information. Entity and physics body counts. Memory usage tracking.

---

## 🛡️ Error Handling Philosophy

### Fail-Safe Defaults

The engine tries to continue working even with invalid input:

```cpp
entity.At({NaN, NaN, NaN});     // Clamps to valid position
entity.Color(-1, 5, 0.5f);      // Clamps to 0-1 range
entity.Scale(0);                // Uses minimum scale
```

### Graceful Degradation

If systems fail to initialize:
- Engine continues with reduced functionality
- Editor panels may be disabled
- Physics may fall back to simple collision

### Debug Information

Enable detailed logging for troubleshooting:

```cpp
#define BS_DEBUG_ENABLED
#include "blacksite/Engine.h"
// Provides detailed logs in console panel and terminal
```

---

## 🚦 Performance Characteristics

### Optimized For
- **Small to medium scenes** (10–100 objects)
- **Learning and experimentation**
- **Real-time editing and iteration**
- **Physics simulation accuracy**

### Not Optimized For
- **Large scenes** (1000+ objects)
- **Production game performance**
- **Complex rendering effects**
- **Memory efficiency**

### Performance Tips

```cpp
// Good: Reasonable number of dynamic objects
for (int i = 0; i < 20; ++i) {
    auto cube = engine.SpawnCube({i, 5, 0});
}

// Avoid: Too many dynamic objects
for (int i = 0; i < 1000; ++i) { // Will be slow!
    auto cube = engine.SpawnCube({i, 5, 0});
}

// Good: Use static objects for scenery
auto wall = engine.SpawnCube({10, 5, 0});
engine.GetEntity(wall).MakeStatic(); // No physics overhead
```

---

## 🏷️ Version and Compatibility

- **Current Version:** 0.2.0-alpha
- **API Stability:** Unstable – expect breaking changes
- **ABI Stability:** Not guaranteed between versions
- **Backward Compatibility:** Not maintained during alpha

### Supported Compilers
- **GCC 7+** (Linux)
- **Clang 10+** (macOS/Linux)
- **MSVC** (not yet supported)

### Supported Platforms
- **Linux:** Ubuntu 20.04+ (primary development platform)
- **macOS:** 10.15+ (experimental support)
- **Windows:** Not yet supported

---

## ⏭️ Next Steps

Now that you understand the core concepts:

1. **Try the Quick Start:** [Quick Start Guide](../getting-started/quick-start.md)
2. **Learn the Engine Class:** [Engine Core](./engine-core.md)
3. **Master the Entity System:** [Entity System](./entity-system.md)
4. **Explore Physics:** [Physics System](./physics.md)
5. **Use the Editor:** [Editor Interface](./editor.md)

---

*API Overview for Blacksite Engine v0.2.0-alpha*
