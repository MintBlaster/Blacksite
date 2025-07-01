# ⚡ Physics System API

The Physics System provides Jolt Physics integration with real-time static/dynamic body switching and comprehensive force control.

---

## 🧩 Core Concepts

### 🚀 Physics-First Design

- **All entities have physics by default** – they fall with gravity immediately
- **Static/Dynamic switching** – convert between movable and immovable states
- **Real-time editing** – change physics properties while simulation runs
- **Jolt Physics backend** – industry-grade physics simulation

### 🔄 Static vs Dynamic Bodies

**Dynamic Bodies (Default):**
- Affected by gravity (`-9.81 m/s²`)
- Respond to forces and impulses
- Have finite mass and velocity
- Can move and rotate freely

**Static Bodies:**
- Not affected by gravity or forces
- Infinite mass (immovable)
- Still participate in collisions
- Perfect for ground, walls, platforms

---

## 🏗️ PhysicsSystem Class

```cpp
namespace Blacksite {
class PhysicsSystem {
public:
    // Body creation
    JPH::BodyID CreateBoxBody(const glm::vec3& position, const glm::vec3& size, bool isStatic = false);
    JPH::BodyID CreateSphereBody(const glm::vec3& position, float radius, bool isStatic = false);
    JPH::BodyID CreatePlaneBody(const glm::vec3& position, const glm::vec3& normal = {0, 1, 0});

    // Static/Dynamic conversion (KEY FEATURE)
    void MakeBodyStatic(JPH::BodyID bodyID);
    void MakeBodyDynamic(JPH::BodyID bodyID);
    bool IsBodyStatic(JPH::BodyID bodyID);

    // Transform control
    void SetBodyPosition(JPH::BodyID bodyID, const glm::vec3& position);
    void SetBodyRotation(JPH::BodyID bodyID, const glm::vec3& eulerAngles);
    glm::vec3 GetBodyPosition(JPH::BodyID bodyID);
    glm::vec3 GetBodyRotation(JPH::BodyID bodyID);

    // Force and motion control
    void AddForce(JPH::BodyID bodyID, const glm::vec3& force);
    void AddImpulse(JPH::BodyID bodyID, const glm::vec3& impulse);
    void SetVelocity(JPH::BodyID bodyID, const glm::vec3& velocity);
    void SetAngularVelocity(JPH::BodyID bodyID, const glm::vec3& angularVel);

    // State queries
    glm::vec3 GetVelocity(JPH::BodyID bodyID);
    glm::vec3 GetAngularVelocity(JPH::BodyID bodyID);
    bool IsBodyActive(JPH::BodyID bodyID);
    float GetBodyMass(JPH::BodyID bodyID);

    // Entity mapping
    JPH::BodyID GetBodyIDFromEntityID(int entityId);
    void MapEntityToBody(int entityId, JPH::BodyID bodyID);
};
}
```

---

## 🟦 Body Creation

### 🟫 CreateBoxBody

```cpp
JPH::BodyID CreateBoxBody(const glm::vec3& position, const glm::vec3& size, bool isStatic = false);
```

- **Purpose:** Create box-shaped physics body
- **Parameters:**
  - `position`: World position
  - `size`: Box dimensions (width, height, depth)
  - `isStatic`: Whether to create as static body
- **Returns:** Jolt BodyID for further operations

**Examples:**
```cpp
// Dynamic cube (falls with gravity)
auto cubeBody = physics->CreateBoxBody({0, 5, 0}, {1, 1, 1}, false);

// Static wall
auto wallBody = physics->CreateBoxBody({10, 5, 0}, {1, 10, 20}, true);

// Platform
auto platformBody = physics->CreateBoxBody({5, 3, 0}, {4, 0.5f, 2}, true);
```

### 🟢 CreateSphereBody

```cpp
JPH::BodyID CreateSphereBody(const glm::vec3& position, float radius, bool isStatic = false);
```

- **Purpose:** Create sphere-shaped physics body
- **Parameters:**
  - `position`: World position
  - `radius`: Sphere radius
  - `isStatic`: Whether to create as static body

**Examples:**
```cpp
// Bouncing ball
auto ballBody = physics->CreateSphereBody({0, 8, 0}, 0.5f, false);

// Large static sphere (decorative)
auto decorBody = physics->CreateSphereBody({20, 10, 0}, 3.0f, true);
```

### 🟫 CreatePlaneBody

```cpp
JPH::BodyID CreatePlaneBody(const glm::vec3& position, const glm::vec3& normal = {0, 1, 0});
```

- **Purpose:** Create plane-shaped physics body (usually for ground)
- **Parameters:**
  - `position`: Plane center position
  - `normal`: Surface normal vector (default: upward)
- **Note:** Planes are typically made static after creation

**Examples:**
```cpp
// Ground plane
auto groundBody = physics->CreatePlaneBody({0, -1, 0});
physics->MakeBodyStatic(groundBody); // Essential!

// Angled ramp
auto rampBody = physics->CreatePlaneBody({5, 2, 0}, {0.5f, 1, 0});
physics->MakeBodyStatic(rampBody);
```

---

## 🔀 Static/Dynamic Conversion (Key Feature)

### 🛑 MakeBodyStatic

```cpp
void MakeBodyStatic(JPH::BodyID bodyID);
```

- **Purpose:** Convert dynamic body to static (disable physics)
- **Effects:**
  - Body stops moving immediately
  - No longer affected by gravity or forces
  - Becomes immovable (infinite mass)
  - Still participates in collisions
- **Use Cases:**
  - Ground planes and platforms
  - Walls and barriers
  - Puzzle elements that should be fixed
  - Performance optimization for scenery

**Examples:**
```cpp
// Essential for ground
auto ground = engine.SpawnPlane({0, -1, 0}, {20, 0.2f, 20});
auto groundBody = physics->GetBodyIDFromEntityID(ground);
physics->MakeBodyStatic(groundBody);

// Convert falling object to platform
physics->MakeBodyStatic(cubeBody);
```

### 🟢 MakeBodyDynamic

```cpp
void MakeBodyDynamic(JPH::BodyID bodyID);
```

- **Purpose:** Convert static body to dynamic (enable physics)
- **Effects:**
  - Body becomes affected by gravity
  - Can be moved by forces and impulses
  - Has finite mass and velocity
  - Will fall if not supported
- **Use Cases:**
  - Making platforms collapsible
  - Activating previously static objects
  - Physics puzzles and interactions
  - Dynamic level elements

**Examples:**
```cpp
// Make platform fall when triggered
physics->MakeBodyDynamic(platformBody);

// Activate dormant objects
physics->MakeBodyDynamic(decorativeBody);
```

### ❓ IsBodyStatic

```cpp
bool IsBodyStatic(JPH::BodyID bodyID);
```

- **Purpose:** Check current physics state
- **Returns:** `true` if static, `false` if dynamic

**Examples:**
```cpp
if (physics->IsBodyStatic(bodyID)) {
    std::cout << "Object is static (won't move)" << std::endl;
} else {
    std::cout << "Object is dynamic (affected by physics)" << std::endl;
}

// Conditional operations
if (!physics->IsBodyStatic(groundBody)) {
    physics->MakeBodyStatic(groundBody); // Ensure ground doesn't move
}
```

---

## 🎯 Transform Control

### 📍 SetBodyPosition

```cpp
void SetBodyPosition(JPH::BodyID bodyID, const glm::vec3& position);
```

- **Purpose:** Teleport body to new position
- **Effects:**
  - Moves body instantly
  - Preserves velocity (unless static)
  - Updates collision detection

**Examples:**
```cpp
// Reset fallen object
physics->SetBodyPosition(cubeBody, {0, 10, 0});

// Teleportation
physics->SetBodyPosition(playerBody, checkpointPosition);

// Animation (less efficient than forces)
physics->SetBodyPosition(platformBody, newPosition);
```

### 🔄 SetBodyRotation

```cpp
void SetBodyRotation(JPH::BodyID bodyID, const glm::vec3& eulerAngles);
```

- **Purpose:** Set body orientation (Euler angles in degrees)

**Examples:**
```cpp
// Level platform
physics->SetBodyRotation(platformBody, {0, 0, 0});

// Tilt platform
physics->SetBodyRotation(platformBody, {10, 0, 0});

// Complex orientation
physics->SetBodyRotation(objectBody, {30, 45, 60});
```

### 🔎 GetBodyPosition / GetBodyRotation

```cpp
glm::vec3 GetBodyPosition(JPH::BodyID bodyID);
glm::vec3 GetBodyRotation(JPH::BodyID bodyID);
```

- **Purpose:** Query current transform

**Examples:**
```cpp
glm::vec3 pos = physics->GetBodyPosition(cubeBody);
if (pos.y < -20) {
    // Object fell too far, reset
    physics->SetBodyPosition(cubeBody, {0, 10, 0});
}

glm::vec3 rot = physics->GetBodyRotation(platformBody);
// Use rotation for gameplay logic
```

---

## 💥 Force and Motion Control

### 🏹 AddForce – Continuous Force

```cpp
void AddForce(JPH::BodyID bodyID, const glm::vec3& force);
```

- **Purpose:** Apply continuous force over time
- **Physics:** Accelerates object gradually (`F = ma`)
- **Units:** Newtons (`kg⋅m/s²`)
- **Use Cases:**
  - Wind effects
  - Thruster simulation
  - Magnetic forces
  - Gradual acceleration

**Examples:**
```cpp
// Upward thrust
physics->AddForce(rocketBody, {0, 50, 0});

// Side wind
physics->AddForce(leafBody, {5, 0, 0});

// Gravity well effect
glm::vec3 direction = targetPos - objectPos;
physics->AddForce(objectBody, direction * 10.0f);
```

### 💨 AddImpulse – Instantaneous Force

```cpp
void AddImpulse(JPH::BodyID bodyID, const glm::vec3& impulse);
```

- **Purpose:** Apply instantaneous force (changes velocity immediately)
- **Physics:** Changes momentum instantly (`J = Δp`)
- **Units:** Newton-seconds (`kg⋅m/s`)
- **Use Cases:**
  - Launching projectiles
  - Jumping mechanics
  - Collision responses
  - Explosive forces

**Examples:**
```cpp
// Launch projectile
physics->AddImpulse(ballBody, {10, 15, 5});

// Jump
physics->AddImpulse(playerBody, {0, 8, 0});

// Explosion effect
for (auto& body : nearbyBodies) {
    glm::vec3 direction = bodyPos - explosionCenter;
    physics->AddImpulse(body, direction * explosionForce);
}
```

### 🚗 SetVelocity – Direct Velocity Control

```cpp
void SetVelocity(JPH::BodyID bodyID, const glm::vec3& velocity);
```

- **Purpose:** Set velocity directly (overrides current velocity)
- **Units:** Meters per second (`m/s`)
- **Use Cases:**
  - Precise movement control
  - Stopping objects
  - Constant speed platforms
  - Velocity-based animation

**Examples:**
```cpp
// Stop object
physics->SetVelocity(cubeBody, {0, 0, 0});

// Constant speed conveyor
physics->SetVelocity(conveyorBody, {2, 0, 0});

// Projectile with exact velocity
physics->SetVelocity(bulletBody, {20, 0, 0});
```

### 🌀 SetAngularVelocity – Rotation Control

```cpp
void SetAngularVelocity(JPH::BodyID bodyID, const glm::vec3& angularVel);
```

- **Purpose:** Set rotational velocity
- **Units:** Radians per second

**Examples:**
```cpp
// Spinning object
physics->SetAngularVelocity(spinnerBody, {0, 5, 0});

// Stop rotation
physics->SetAngularVelocity(cubeBody, {0, 0, 0});

// Complex rotation
physics->SetAngularVelocity(gyroBody, {1, 2, 3});
```

---

## 🧠 State Queries

### 🏃 GetVelocity / GetAngularVelocity

```cpp
glm::vec3 GetVelocity(JPH::BodyID bodyID);
glm::vec3 GetAngularVelocity(JPH::BodyID bodyID);
```

- **Purpose:** Query current motion state

**Examples:**
```cpp
glm::vec3 vel = physics->GetVelocity(ballBody);
float speed = glm::length(vel);

// Speed-based effects
if (speed > 15.0f) {
    SetTrailEffect(ballBody, true);
} else {
    SetTrailEffect(ballBody, false);
}

// Angular velocity for spinning effects
glm::vec3 angVel = physics->GetAngularVelocity(spinnerBody);
float spinRate = glm::length(angVel);
```

### 🔋 IsBodyActive

```cpp
bool IsBodyActive(JPH::BodyID bodyID);
```

- **Purpose:** Check if body is actively simulated
- **Returns:** `true` if body is moving/active, `false` if sleeping
- **Note:** Jolt automatically puts stationary bodies to sleep for performance

**Examples:**
```cpp
if (!physics->IsBodyActive(cubeBody)) {
    // Body is sleeping (not moving)
    // Wake it up with a small impulse
    physics->AddImpulse(cubeBody, {0, 1, 0});
}
```

### ⚖️ GetBodyMass

```cpp
float GetBodyMass(JPH::BodyID bodyID);
```

- **Purpose:** Get body mass in kilograms
- **Note:** Static bodies have infinite mass

**Examples:**
```cpp
float mass = physics->GetBodyMass(cubeBody);
std::cout << "Cube mass: " << mass << " kg" << std::endl;

// Mass-based force calculation
float forceNeeded = mass * desiredAcceleration;
physics->AddForce(cubeBody, {0, forceNeeded, 0});
```

---

## 🗺️ Entity Mapping

### 🆔 GetBodyIDFromEntityID

```cpp
JPH::BodyID GetBodyIDFromEntityID(int entityId);
```

- **Purpose:** Get physics body ID from entity ID
- **Use Cases:**
  - Direct physics operations on entities
  - Advanced physics queries
  - Custom physics interactions

**Examples:**
```cpp
int cubeEntity = engine.SpawnCube({0, 5, 0});
JPH::BodyID cubeBody = physics->GetBodyIDFromEntityID(cubeEntity);

// Now use direct physics operations
physics->AddImpulse(cubeBody, {0, 10, 0});
```

### 🔗 MapEntityToBody

```cpp
void MapEntityToBody(int entityId, JPH::BodyID bodyID);
```

- **Purpose:** Associate entity with physics body (internal use)
- **Note:** Usually handled automatically by the engine

---

## ⚙️ Physics Configuration

### 🧮 Simulation Parameters

- Fixed timestep: 1/60 second (60 Hz)
- Gravity: -9.81 m/s² (Earth-like)
- Collision: Enabled between all bodies
- Sleeping: Automatic for performance

### 🦦 Performance Characteristics

- **Optimized for:** 10–100 dynamic bodies
- **Good performance:** Up to 200 bodies
- **Slow performance:** 500+ bodies
- **Memory usage:** ~1KB per body

---

## 🧑‍🔬 Common Physics Patterns

### 🟫 Ground Setup (Essential)

```cpp
// Every scene needs static ground
auto ground = engine.SpawnPlane({0, -1, 0}, {20, 0.2f, 20});
auto groundBody = physics->GetBodyIDFromEntityID(ground);
physics->MakeBodyStatic(groundBody); // Critical!
```

### 🏏 Projectile System

```cpp
auto projectile = engine.SpawnSphere({0, 1, 0});
auto projBody = physics->GetBodyIDFromEntityID(projectile);

// Launch with calculated trajectory
glm::vec3 launchVel = CalculateTrajectory(startPos, targetPos);
physics->SetVelocity(projBody, launchVel);
```

### 🚨 Physics Trigger

```cpp
auto trigger = engine.SpawnCube({5, 0.1f, 5});
auto triggerBody = physics->GetBodyIDFromEntityID(trigger);
physics->MakeBodyStatic(triggerBody);

// Check for overlaps in update loop
// (Collision detection not yet exposed in API)
```

### 🏗️ Collapsing Platform

```cpp
auto platform = engine.SpawnCube({10, 5, 0});
auto platformBody = physics->GetBodyIDFromEntityID(platform);
physics->MakeBodyStatic(platformBody);

// Later, when triggered:
physics->MakeBodyDynamic(platformBody); // Platform falls!
```

### 🌀 Force Field Effect

```cpp
// In update callback
for (auto& bodyID : affectedBodies) {
    glm::vec3 bodyPos = physics->GetBodyPosition(bodyID);
    glm::vec3 forceDirection = forceCenter - bodyPos;
    float distance = glm::length(forceDirection);

    if (distance < forceRadius) {
        forceDirection = glm::normalize(forceDirection);
        float forceMagnitude = forceStrength / (distance * distance);
        physics->AddForce(bodyID, forceDirection * forceMagnitude);
    }
}
```

---

## 🐞 Debugging Physics

### 👀 Visual Debugging

- Use editor to visualize:
  - Body positions and orientations
  - Velocity vectors (in inspector)
  - Static vs dynamic state
  - Mass and activation state

### 📊 Performance Monitoring

- Check performance panel for:
  - Physics simulation time
  - Number of active bodies
  - Number of sleeping bodies
  - Collision detection cost

### ❗ Common Issues

**Objects Fall Through Ground:**
```cpp
// Solution: Make ground static
physics->MakeBodyStatic(groundBody);
```

**Objects Don't Move:**
```cpp
// Check if accidentally made static
if (physics->IsBodyStatic(bodyID)) {
    physics->MakeBodyDynamic(bodyID);
}
```

**Poor Performance:**
```cpp
// Too many dynamic bodies – make scenery static
for (auto& sceneryBody : sceneryBodies) {
    physics->MakeBodyStatic(sceneryBody);
}
```

---

*Physics System API for Blacksite Engine v0.2.0-alpha*