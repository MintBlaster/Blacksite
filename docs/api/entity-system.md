# 🧩 Entity System API

The Entity System provides a fluent, chainable API for creating and manipulating physics objects in your scene.

---

## 📝 Overview

The Entity System is built around two core concepts:

- **Entity IDs**: Unique integers returned from spawning functions.
- **EntityHandle**: Fluent interface for chaining operations.

All entities spawn with **physics enabled by default**—this is the core philosophy of Blacksite Engine.

---

## 🦝 EntityHandle Class

The main interface for entity manipulation. All operations return `EntityHandle&` for method chaining.

```cpp
class EntityHandle {
public:
    // Transform operations
    EntityHandle& At(const glm::vec3& position);
    EntityHandle& Rotate(const glm::vec3& eulerAngles);
    EntityHandle& Scale(float uniformScale);
    EntityHandle& Scale(const glm::vec3& scale);

    // Physics operations
    EntityHandle& MakeStatic();
    EntityHandle& MakeDynamic();
    EntityHandle& Push(const glm::vec3& impulse);
    EntityHandle& SetVelocity(const glm::vec3& velocity);
    EntityHandle& AddForce(const glm::vec3& force);

    // Visual operations
    EntityHandle& Color(float r, float g, float b);
    EntityHandle& Color(const glm::vec3& color);
    EntityHandle& Shader(const std::string& shaderName);

    // Utility
    int GetId() const;
    bool IsValid() const;
    void Destroy();
};
```

---

## 🟦 Spawning Entities

### 🟫 SpawnCube

```cpp
int SpawnCube(const glm::vec3& position = {0, 0, 0});
```

- **Creates:** 1×1×1 cube with box collision shape
- **Physics:** Dynamic body with mass, falls immediately
- **Returns:** Entity ID for manipulation

**Example:**
```cpp
auto cube = engine.SpawnCube({0, 5, 0});
engine.GetEntity(cube)
    .Color(1, 0, 0)      // Red color
    .Scale(1.5f)         // Make bigger
    .Push({0, 5, 0});    // Upward impulse
```

---

### 🟢 SpawnSphere

```cpp
int SpawnSphere(const glm::vec3& position = {0, 0, 0});
```

- **Creates:** Unit sphere (radius 0.5) with sphere collision
- **Physics:** Dynamic body, rolls and bounces naturally
- **Returns:** Entity ID for manipulation

**Example:**
```cpp
auto ball = engine.SpawnSphere({2, 8, 0});
engine.GetEntity(ball)
    .Color(0, 1, 0)      // Green color
    .Scale(0.8f)         // Smaller
    .Push({5, 0, 0});    // Sideways impulse
```

---

### 🟫 SpawnPlane

```cpp
int SpawnPlane(const glm::vec3& position = {0, 0, 0}, const glm::vec3& size = {1, 1, 1});
```

- **Creates:** Rectangular plane with box collision
- **Physics:** Dynamic by default (usually make static)
- **Parameters:**
  - `position`: Center position of the plane
  - `size`: Dimensions (width, height, depth)

**Essential Pattern for Ground:**
```cpp
auto ground = engine.SpawnPlane({0, -1, 0}, {20, 0.2f, 20});
engine.GetEntity(ground)
    .Color(0.3f, 0.3f, 0.3f) // Gray color
    .MakeStatic();           // ESSENTIAL for ground
```

---

## 🔄 Transform Operations

### 📍 Position: At()

```cpp
EntityHandle& At(const glm::vec3& position);
```

- **Purpose:** Set world position immediately
- **Updates:** Both visual and physics body position
- **Thread-safe:** Yes, changes applied next frame

**Examples:**
```cpp
entity.At({5, 10, -2});           // Specific position
entity.At(glm::vec3(x, y, z));    // Using variables
entity.At({0, 0, 0});             // World origin
```

---

### 🔁 Rotation: Rotate()

```cpp
EntityHandle& Rotate(const glm::vec3& eulerAngles);
```

- **Purpose:** Set rotation using Euler angles in degrees
- **Order:** X → Y → Z rotation order
- **Range:** Any values (will wrap appropriately)

**Examples:**
```cpp
entity.Rotate({0, 45, 0});      // 45° around Y-axis
entity.Rotate({90, 0, 0});      // 90° around X-axis
entity.Rotate({0, 0, 180});     // 180° around Z-axis
entity.Rotate({15, 30, 45});    // Combined rotation
```

---

### 🗏 Scale: Scale()

```cpp
EntityHandle& Scale(float uniformScale);
EntityHandle& Scale(const glm::vec3& scale);
```

- **Purpose:** Set visual and collision scale
- **Uniform:** Single value scales all axes equally
- **Non-uniform:** Different scale per axis

**Examples:**
```cpp
entity.Scale(2.0f);             // Double size
entity.Scale(0.5f);             // Half size
entity.Scale({2, 1, 0.5f});     // Stretch X, compress Z
```

> **Physics Note:** Scale affects collision shape approximately. Large non-uniform scales may cause physics issues.

---

## 🧲 Physics Operations

### ⚖️ Static/Dynamic Control

#### MakeStatic

```cpp
EntityHandle& MakeStatic();
```
- **Disables physics simulation** for this object
- **Still has collision**—other objects bounce off it
- **Cannot move**—infinite mass, unaffected by forces
- **Perfect for:** Ground, walls, platforms, scenery

#### MakeDynamic

```cpp
EntityHandle& MakeDynamic();
```
- **Enables physics simulation** (default state)
- **Affected by gravity** and forces
- **Can move and rotate** freely
- **Has mass** and responds to collisions

**Example Usage:**
```cpp
// Ground plane (essential pattern)
auto ground = engine.SpawnPlane({0, -1, 0}, {15, 0.2f, 15});
engine.GetEntity(ground).MakeStatic(); // Must be static

// Moving platform that becomes static
auto platform = engine.SpawnCube({5, 3, 0});
engine.GetEntity(platform)
    .Scale({3, 0.2f, 1}) // Platform shape
    .MakeStatic();       // Static platform

// Convert static to dynamic later
engine.GetEntity(platform).MakeDynamic(); // Now it falls
```

---

### 💥 Force Application

#### Push (Impulse)

```cpp
EntityHandle& Push(const glm::vec3& impulse);
```
- **Instant velocity change**—good for impacts, jumps
- **One-time effect**—doesn't accumulate over frames
- **Units:** Newton-seconds (impulse)

#### AddForce (Continuous)

```cpp
EntityHandle& AddForce(const glm::vec3& force);
```
- **Continuous acceleration**—good for propulsion, wind
- **Accumulates over time**—builds up velocity
- **Units:** Newtons (force)

#### SetVelocity (Direct)

```cpp
EntityHandle& SetVelocity(const glm::vec3& velocity);
```
- **Immediate velocity**—overrides current motion
- **Direct control**—bypasses physics simulation
- **Units:** Meters per second

**Force Examples:**
```cpp
// Jump/bounce effect
entity.Push({0, 10, 0}); // Upward impulse

// Launch projectile
entity.Push({15, 5, 0}); // Forward and up

// Continuous thrust
entity.AddForce({0, 0, 50}); // Forward thrust

// Stop object immediately
entity.SetVelocity({0, 0, 0}); // Full stop

// Set specific velocity
entity.SetVelocity({5, 0, 0}); // Move right at 5 m/s
```

**Force Magnitude Guidelines:**
- **Small impulses:** 1–5 units (gentle push)
- **Medium impulses:** 5–15 units (strong push, jump)
- **Large impulses:** 15+ units (launch, explosion)
- **Continuous forces:** 10–100 units (propulsion, wind)

---

## 🎨 Visual Operations

### 🌈 Color Assignment

```cpp
EntityHandle& Color(float r, float g, float b);
EntityHandle& Color(const glm::vec3& color);
```

- **Purpose:** Set flat color for entity rendering
- **Range:** 0.0 to 1.0 for each component
- **No validation:** Values outside range may cause issues

**Examples:**
```cpp
// Primary colors
entity.Color(1, 0, 0); // Red
entity.Color(0, 1, 0); // Green
entity.Color(0, 0, 1); // Blue

// Custom colors
entity.Color(0.8f, 0.4f, 0.2f); // Orange-brown
entity.Color(0.2f, 0.8f, 0.9f); // Cyan

// Using vector
glm::vec3 purple{0.5f, 0, 0.8f};
entity.Color(purple);

// Grayscale
entity.Color(0.3f, 0.3f, 0.3f); // Dark gray
entity.Color(0.8f, 0.8f, 0.8f); // Light gray
```

---

### ✨ Shader Assignment

```cpp
EntityHandle& Shader(const std::string& shaderName);
```

- **Purpose:** Set rendering shader for entity
- **Available Shaders:**
  - `"basic"` – Flat color with simple lighting (default)
  - `"rainbow"` – Animated rainbow colors
  - `"glow"` – Bright emission effect
  - `"holo"` – Holographic appearance
  - `"wireframe"` – Wireframe rendering
  - `"plasma"` – Animated plasma effect

**Examples:**
```cpp
entity.Shader("rainbow");    // Animated colors
entity.Shader("glow");       // Bright emission
entity.Shader("wireframe");  // Debug wireframe
entity.Shader("basic");      // Back to default
```

---

## 🛠️ Utility Operations

### 🆔 Entity Information

```cpp
int GetId() const;
bool IsValid() const;
```

- **GetId():** Returns unique entity identifier
- **IsValid():** Checks if entity still exists and is valid

**Examples:**
```cpp
int cubeId = entity.GetId(); // Store for later use
if (entity.IsValid()) {
    entity.Color(1, 0, 0); // Safe to use
}
```

---

### ❌ Entity Destruction

```cpp
void Destroy();
```

- **Purpose:** Mark entity for deletion
- **Timing:** Deleted at end of current frame
- **Note:** Does not return EntityHandle (breaks chaining)

**Example:**
```cpp
if (shouldRemove) {
    entity.Destroy(); // Mark for deletion
    // Don't use entity after this
}
```

---

## 🔗 Fluent API Patterns

### Method Chaining

The fluent API allows chaining multiple operations:

```cpp
// Basic chaining
entity.At({0, 5, 0}).Color(1, 0, 0).Scale(1.5f);

// Complex setup
entity.At({5, 10, 0})
    .Rotate({0, 45, 0})
    .Scale(2.0f)
    .Color(0.8f, 0.2f, 0.9f)
    .Shader("glow");

// Conditional chaining
entity.Color(1, 0, 0);
if (shouldScale) {
    entity.Scale(2.0f);
}
if (shouldPush) {
    entity.Push({0, 10, 0});
}
```

---

### Entity Collections

Working with multiple entities:

```cpp
std::vector<int> entities;

// Create multiple entities
for (int i = 0; i < 10; ++i) {
    auto cube = engine.SpawnCube({i * 2.0f, 5, 0});
    entities.push_back(cube);
    engine.GetEntity(cube)
        .Color(i * 0.1f, 1.0f - i * 0.1f, 0.5f)
        .Scale(0.5f + i * 0.1f);
}

// Modify all entities
for (int id : entities) {
    engine.GetEntity(id).Push({0, 5, 0});
}
```

---

### Conditional Operations

```cpp
// Different behavior based on entity type
auto entity = engine.GetEntity(entityId);
if (isGround) {
    entity.MakeStatic().Color(0.3f, 0.3f, 0.3f);
} else if (isPlayer) {
    entity.Color(0, 1, 0).Scale(1.2f);
} else {
    entity.Color(1, 0, 0).Push({0, 10, 0});
}
```

---

## 🧠 Advanced Usage

### Entity State Management

```cpp
class EntityManager {
private:
    std::unordered_map<int, EntityData> entities;

public:
    void CreatePlayer(const glm::vec3& pos) {
        auto id = engine.SpawnCube(pos);
        engine.GetEntity(id).Color(0, 1, 0);
        entities[id] = {EntityType::PLAYER, pos};
    }

    void CreateEnemy(const glm::vec3& pos) {
        auto id = engine.SpawnSphere(pos);
        engine.GetEntity(id)
            .Color(1, 0, 0)
            .Scale(0.8f);
        entities[id] = {EntityType::ENEMY, pos};
    }

    void UpdateEntities(float deltaTime) {
        for (auto it = entities.begin(); it != entities.end(); ) {
            auto entity = engine.GetEntity(it->first);
            if (!entity.IsValid()) {
                it = entities.erase(it);
                continue;
            }
            // Update based on type
            if (it->second.type == EntityType::ENEMY) {
                // AI behavior
                entity.AddForce({0, 0, 10 * deltaTime});
            }
            ++it;
        }
    }
};
```

---

### Physics Interactions

```cpp
// Collision response (manual)
void HandleCollisions() {
    // Note: Automatic collision response is handled by physics
    // This is for custom game logic

    for (int id : dynamicEntities) {
        auto entity = engine.GetEntity(id);

        // Get physics position
        auto pos = engine.GetPhysicsSystem()->GetBodyPosition(
            entity.GetPhysicsBodyId());

        // Custom logic based on position
        if (pos.y < -10) {
            // Fell off world - respawn
            entity.At({0, 10, 0}).SetVelocity({0, 0, 0});
        }
    }
}
```

---

## 🛡️ Error Handling

### Safe Entity Access

```cpp
// Always check validity for stored entity IDs
int storedEntityId = GetStoredEntityId();
auto entity = engine.GetEntity(storedEntityId);

if (entity.IsValid()) {
    entity.Color(1, 0, 0); // Safe to use
} else {
    // Entity was destroyed or invalid ID
    HandleMissingEntity(storedEntityId);
}
```

---

### Graceful Degradation

```cpp
// Operations on invalid entities are ignored
auto invalidEntity = engine.GetEntity(-1); // Invalid ID
invalidEntity.Color(1, 0, 0); // Ignored safely
invalidEntity.Push({0, 10, 0}); // Ignored safely
// No crashes, operations just do nothing
```

---

## 🚦 Performance Considerations

### Efficient Entity Management

```cpp
// Good: Batch operations
std::vector<int> entities = CreateManyEntities();
for (int id : entities) {
    engine.GetEntity(id).Color(1, 0, 0).Scale(0.5f);
}

// Avoid: Repeated GetEntity calls
for (int i = 0; i < 100; ++i) {
    auto entity = engine.GetEntity(entityId); // Repeated lookup
    entity.Push({0, 0.1f, 0}); // Small repeated force
}

// Better: Single GetEntity, larger force
auto entity = engine.GetEntity(entityId);
entity.Push({0, 10, 0}); // One larger force
```

### Memory Management

- Entity IDs may be reused after destruction
- Don't store EntityHandle objects—store IDs instead
- Check IsValid() for long-lived entity references

---

*Entity System API for Blacksite Engine v0.2.0-alpha*
