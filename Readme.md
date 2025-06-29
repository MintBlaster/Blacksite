# Blacksite Engine

**Blacksite** is a modular, physics-first C++ game engine built for real-time destruction, vehicles, and dynamic, interactive worlds. Powered by [Jolt Physics](https://github.com/jrouwe/JoltPhysics), it flips the usual game engine flow: **physics is the foundation**, not an optional plugin.

## Highlights

- **Physics-First Architecture**  
  Every object is dynamic by default. Rigidbodies, collisions, and forces aren't bolted on — they *are* the system. Powered by Jolt Physics.

- **Modular Core**  
  Clean separation between engine subsystems: windowing, physics, rendering, input, asset loading — you plug in what you need.

- **Modern Rendering Stack**  
  OpenGL (for now) with GLM math, shader support, and a clean pipeline coming next.

- **Cross-Platform**  
  Actively tested on Linux. Windows/macOS coming soon. No engine-specific IDE lock-in.

- **Clean AF API**  
  + Declarative API
  + Spawn objects with physics in one line. No setup, Just behavior.

---

## Physics-First API Design

```cpp
#include <blacksite.h>

int main() {
    Blacksite::Engine engine;
    engine.Init();
    
    auto scene = engine.CreateScene();
    
    // Everything is a physical object by default
    auto cube = scene.Spawn("cube.obj")  // Loads mesh + creates physics body
        .At({0, 5, 0})
        .WithMass(2.5f)
        .Push({0, 0, 10});  // Instantly has physics
    
    // Static objects opt-out of dynamics
    auto ground = scene.Spawn()
        .AsPlane()
        .Material(Material::ASPHALT)
        .MakeStatic();  // Only way to disable physics
    
    // UI elements live in screen space, not physics space
    auto hud = scene.UI()
        .Text("Score: 0")
        .At({10, 10});
    
    return engine.Run(); // Starts fixed-step simulation + rendering loop
}
```

**🛠️ API Breakdown:**
- `Spawn("cube.obj")` → loads mesh, creates physics body, makes it dynamic
- `Spawn()` → creates empty dynamic object (add shape with `.AsPlane()`, `.AsSphere()`, etc.)
- `.MakeStatic()` → only way to disable physics (opt-out, not opt-in)
- `scene.UI()` → separate from physics space entirely

✅ **Every spawned object is dynamic by default** - no components to add  
✅ **Physics methods work immediately** - `.Push()`, `.SetMass()`, `.Collide()`  
✅ **Static objects require explicit opt-out** - `.MakeStatic()`  
✅ **UI elements are separate from physics space** - `scene.UI()` vs `scene.Spawn()`

## Current Status

⚠️ **Alpha Phase – Bootstrapping**

Currently working systems:
- Engine core (init, main loop)
- Window management (GLFW-based)

Next up:
- Rendering system (OpenGL backend)
- Physics integration layer for Jolt
- Declarative entity/scene API

Nothing is stable yet — this is a moving target.


## Why Physics-First?

Because most engines treat physics like some shitty afterthought. Blacksite flips that:

- Objects behave like real objects by default.
- Destruction isn't faked — it's simulated.
- No extra work to "enable" physics — it's already on.
- Ideal for making chaotic, dynamic, or vehicular-heavy games.

If you're tired of manually toggling Rigidbody checkboxes or writing boilerplate to add collision logic, this engine is for you.

## Contributing

+ Want to help shape a true physics-core engine from scratch? Good.

+ Pull requests welcome — but bring ideas, not just code.

- Physics abstraction layer (Jolt wrappers, body/component sync)
- OpenGL rendering pipeline + ECS-friendly material/shader system
- Scene graph with component system (basic ECS already bootstrapped)
- Asset loading (mesh, textures, prefabs)
- Input system (GLFW or platform-agnostic)

Open an issue or pull request if you're serious.

## License

MIT
Use it, fork it, break it, rebuild it.
